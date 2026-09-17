# 排错记录：无线连接建立后数码管变暗且持续闪烁

| 项目 | 内容 |
| --- | --- |
| 硬件 | AB5712F（K16B 板），发送端 emit（wireless_mic_emit）+ 接收端 adapter（wireless_adapter） |
| SDK | 中科蓝讯 bt8930 硅族 SDK，`app/projects/microphone`（CONFIG_WIRELESS_LE_MIC 方案） |
| 现象 | 发送端开机数码管亮度正常；与接收端建立无线连接后明显变暗，并持续闪烁；断开连接后恢复 |
| 分支 | `branch_01` |
| 结论 | 消隐是原厂按点亮段数均衡亮度的设计（占空约 2~3.5%），连接后正常生效，与待机态实测的全亮状态形成落差，观感为"变暗+闪烁"；修复为在每次扫描后强制关闭消隐，统一为全亮占空 |

---

## 1. 问题现象

发送端串口日志（连接成功前后）：

```text
gui_box_flicker_set 
addr: d7 9a ef e0 42 41 
WIRELESS_CONNECTED,0 1
WIRELESS_CON_ROLE, 0
TX: pdu=(150B, 742), intv=(15000, 817), retry=(3, 8)
LED_USER_CONN
```

- 连接建立（`WIRELESS_CONNECTED`）后数码管立刻变暗，并持续闪烁；
- 断开连接后数码管恢复初始亮度；
- 接收端（adapter）同样存在该问题（修复位于两端共用的扫描代码中）。

初步怀疑方向（用户第一直觉）："连接后抬主频推屏幕会变暗"。

---

## 2. 显示驱动背景（排查前需要了解的机制）

数码管为 7COM×7SEG（7P7S）动态扫描，驱动链路：

| 环节 | 代码位置 | 说明 |
| --- | --- | --- |
| 1ms tick 中断 | `app/system/system.c` `usr_tmr1ms_isr()` | TMR0 跑固定 1MHz（PCLKSEL=110），每 1ms 调 `gui_scan()` |
| COM 扫描 | `app/modules/gui/ledseg/ledseg_7p7s.c` `ledseg_7p7s_scan()` | 7 个 COM 轮流点亮，帧率 ≈ 1000/7 ≈ 143Hz |
| 占空/消隐 | 闭源库 `ledseg_ajust()`（libplatform.a） | 配置 TMR1；`timer1_isr()`（`app/system/interrupt.c`）在 TMR1 溢出时调 `ledseg_7p7s_clr()` 关屏 |
| 点亮 COM | `app/projects/microphone/port/port_ledseg.c` `ledseg_7p7s_set()` | GPIO 高驱动，先全关再点亮 |

系统主频：开机基础主频 `SYS_CLK_SEL = SYS_24M`（`config_wireless_mic.h:24`）；无线连接成功回调里抬高（`app/modules/wireless/wireless_proc.c`，`sys_clk_req(INDEX_KARAOK, SYS_120M)` → `SYS_160M`，**实测 `SYS_160M` 档实际频率为 196.608MHz**）；断开后 `sys_clk_free(INDEX_KARAOK)` 回到 24M。

---

## 3. 排查过程（三轮，含错误路线）

### 3.1 第一轮：TMR1 跑主频域的假设（错误）→ 导致 adapter 连接复位

**假设**：亮度占空由 TMR1 消隐窗口决定，点亮时长 = TMR1PR ÷ 主频；TMR1 默认跑系统主频（手册复位默认 PCLKSEL=000），lib 按开机 24M 写死 PR，连接后主频抬到 196.6M → 点亮窗口缩为 24/196.6 ≈ 12% → 变暗。

**第一版修复**（已被否定回滚）：在 `ledseg_7p7s_scan()` 的 `ledseg_ajust()` 之后，停表 → `TMR1PR = TMR1PR × 当前主频 / 24M` → 恢复 CON。

**实测结果——修复引入了更严重的问题**：

```text
adapter 端：WIRELESS_CONNECTED,0 0 之后立刻复位重启
Hello BT893X: 88010039
SDK : v0150, LIBS: v0150
WDT reset                      ← 看门狗复位，说明刚才是被拖死的
...
emit 端：WIRELESS_CONNECT_FAIL, 0 反复出现   ← adapter 在反复重启，自然连不上
```

**当时的诊断打印还暴露了关键疑点**：

```text
LEDSEG CLK: 24000000 Hz, TMR1CON=0, TMR1PR=-1
```

`TMR1PR` 读出来是 `0xFFFFFFFF`：若 lib 真把 PR 写成 -1，数码管将永远不消隐（全亮），与"待机亮度正常但也不是刺眼的全亮"矛盾——**说明停表状态下读 TMR1PR 返回的是无效值**。第一版修复"读-改-写 PR"拿到垃圾值回写，TMR1 行为异常引发中断风暴，系统被拖死进 WDT 复位循环。

> 教训 1：不要对语义未确认的寄存器做"读-改-写"。
> 教训 2：诊断打印本身就是试金石——读值明显违背硬件常识时，先怀疑读值路径，再怀疑理论。

### 3.2 第二轮：反汇编闭源库 + 查原厂手册（定性）

不猜寄存器，两条证据链：

**证据 A：反汇编 libplatform.a 的 `ledseg_ajust`**（工具链 `riscv32-elf-nm` 定位符号 + capstone 解码，重定位项还原真实地址）：

```text
ledseg_ajust():
  *(0x118) = 0          // TMR1CNT = 0（sfr.h:48，清计数）
  TMR1PR = 1000 - 占空比查表值   // *(0x11C)，查表按当前 COM 槽点亮段数
  TMR1CON = 0x85        // *(0x110)：TMREN=1, PCLKSEL=010, IRQ_EN=1
```

> **事后拿到原厂本芯片 lib 源码，与反汇编逐条吻合**（即最终采纳的参考实现）：
>
> ```c
> void ledseg_ajust(uint disp_seg)
> {
>     uint seg_num = s_bcnt(disp_seg);
>     if (seg_num > 0) {
>         //根据需要点亮的SEG数，设定不同的点亮时间。 seg_num越小，点亮时间越短
>         TMR1CNT  = 0;
>         TMR1PR   = 1000 - ledseg_tbl[seg_num-1];
>         TMR1CON  = BIT(7) | BIT(2) | BIT(0);   //timer1 interrupt en, timer1 counter mode
>     }
> }
> ```
>
> 注意：源码中**没有任何主频档位门控**——只要本槽点亮段数 >0 就武装消隐定时器。

**证据 B：原厂手册《ab571x_usermanual.pdf》Register 4-1（第 37 页）**：

```text
TMRxCON bits[3:1] PCLKSEL（Pre-divider clock source selection）：
  000: system clock        100: rc2m
  001: sys_clkppp          101: rtc_rc2m
  010: xosc26m_clk         110: tmr_inc_cr
  011: tmr_inc_pin         111: clkout_pinp
```

`0x85` 的 PCLKSEL=010 → **TMR1 跑 26M 晶振（固定频率），与系统主频无关**。
第一轮的"抬主频缩短消隐窗口"理论被彻底推翻。

### 3.3 第三轮：只读诊断固件（定位真因）

在全只读、不写任何定时器寄存器的前提下加三处诊断：

1. `usr_tmr1ms_isr()` 入口读 `TMR0CNT`：TMR0 溢出后从 0 重数（固定 1MHz，1 count = 1µs），**该读值 = "溢出 → ISR 实际执行"经过的微秒数**，直接量化 RF 对扫描中断的挤占程度；维护 max/avg，每 5 秒由主循环打印；
2. `ledseg_7p7s_scan()` 内 `ledseg_ajust()` 之后采样 `TMR1CON/TMR1PR`（运行态读值）；
3. `ledseg_display()` 里主频变化或每 5 秒打印一行汇总。

诊断代码原文（临时代码，验证后已删除）：

```c
// system.c
volatile u32 dbg_tmr0_lat_max;      //1ms tick 中断进入延迟(µs)——统计窗口内最大值
volatile u32 dbg_tmr0_lat_sum;      //延迟累加(µs)
volatile u16 dbg_tmr0_lat_cnt;      //采样次数

void usr_tmr1ms_isr(void)
{
    //TMR0 跑固定 1MHz(sel=6)，溢出后从 0 重新计数；ISR 进来时读 CNT
    //即"溢出→ISR 实际执行"经过的 µs，用于量化 RF 对扫描中断的挤占
    u32 dbg_lat = TMR0CNT;
    dbg_tmr0_lat_sum += dbg_lat;
    dbg_tmr0_lat_cnt++;
    if (dbg_lat > dbg_tmr0_lat_max) {
        dbg_tmr0_lat_max = dbg_lat;
    }
    ...
}

// ledseg_7p7s.c
volatile u32 dbg_tmr1con;           //ajust 后 TMR1CON/TMR1PR 采样
volatile u32 dbg_tmr1pr;
...
ledseg_ajust(disp_seg);
dbg_tmr1con = TMR1CON;
dbg_tmr1pr = TMR1PR;
ledseg_7p7s_set(disp_seg, com_cnt);

// ledseg_common.c  ledseg_display() 内
static u32 last_nhz;
static u16 rep_cnt;
u32 nhz = get_sysclk_nhz();
if (nhz != last_nhz || ++rep_cnt >= 5000) {     //主频变化或每5秒报告一次
    printf("LEDSEG DBG: clk=%d, TMR1CON=%x TMR1PR=%x, tick_lat max=%d avg=%d us\n", ...);
}
```

**实测数据（发送端，关键节选）**：

```text
────── 待机（未连接，亮度正常）──────
LEDSEG DBG: clk=24000000, TMR1CON=0 TMR1PR=ffffffff, tick_lat max=34 avg=10 us
LEDSEG DBG: clk=24000000, TMR1CON=0 TMR1PR=ffffffff, tick_lat max=34 avg=10 us

────── 连接建立后（变暗+闪烁）──────
WIRELESS_CONNECTED,0 1
LEDSEG DBG: clk=196608000, TMR1CON=85 TMR1PR=320, tick_lat max=34 avg=5 us
LEDSEG DBG: clk=196608000, TMR1CON=85 TMR1PR=1f4, tick_lat max=4 avg=0 us
LEDSEG DBG: clk=196608000, TMR1CON=85 TMR1PR=320, tick_lat max=4 avg=0 us
LEDSEG DBG: clk=196608000, TMR1CON=85 TMR1PR=384, tick_lat max=4 avg=0 us
LEDSEG DBG: clk=196608000, TMR1CON=85 TMR1PR=28a, tick_lat max=4 avg=0 us
```

**数据判读**：

| 状态 | 主频 | TMR1CON | TMR1PR | 点亮占空 | 结论 |
| --- | --- | --- | --- | --- | --- |
| 待机 | 24M | 0（未启动） | ffffffff（复位值） | **100%**（每 COM 槽全程点亮） | lib 不消隐 → 亮 |
| 连接后 | 196.608M | 0x85（消隐开启） | 0x172~0x384（370~900） | 19~35µs / 1000µs ≈ **2~3.5%** | lib 消隐 → 暗 |

- 中断延迟：待机 max=34µs、连接后 max=4µs → **RF 没有挤占扫描中断**，"中断挤占"候选排除；
- 消隐窗口跑 26M 晶振固定时钟 → 与主频无关，"主频推暗"候选排除；
- **真因：连接后 `ledseg_ajust()` 按原厂设计启动了 TMR1 消隐（占空压到 2~3.5%），且 PR 随显示内容在 370~900 间跳变（约 2.4 倍亮度波动），叠加射频抖动 → 视觉上的"变暗 + 持续闪烁"；待机档实测未武装消隐（全亮）是异常基准，与源码行为不符，机制在闭源部分无法进一步确认。**

### 3.4 第四轮：扫描缓冲/武装率采样（闭环验证）

在 v2 基础上于 `ledseg_7p7s_scan()` 内增加只读采样：扫描计数、武装计数（ajust 后 CON≠0 的比例）、`disp_en` 提前返回计数、`ledseg_cb.buf` 七槽镜像、最近一次的段码/槽号：

```text
────── 待机（24M，未连接）──────
LEDSEG DBG: scan=196 arm=0   early=0 | buf=7e 48 73 76 4b 4f 25 | last seg=4f com=5 con=0   pr=ffffffff
LEDSEG DBG: scan=195 arm=0   early=0 | buf=7e 48 73 76 4b 4f 25 | last seg=25 com=6 con=0   pr=ffffffff

────── 连接后（196.6M）──────
LEDSEG DBG: scan=154 arm=68  early=0 | buf=7e 48 73 76 4b 4f 25 | last seg=4f com=5 con=85  pr=320
LEDSEG DBG: scan=35  arm=35  early=0 | buf=7e 48 73 76 4b 4f 25 | last seg=48 com=1 con=85  pr=172
LEDSEG DBG: scan=36  arm=36  early=0 | buf=7e 48 73 76 4b 4f 25 | last seg=4b com=4 con=85  pr=28a
```

（scan 计数为窗口内扫描次数，与主循环速度相关，仅作相对参考；关键量是 arm/scan 比例）

**判读**：

- 待机：`buf` 七槽全非零（"2402"+图标确实在扫描缓冲中）且 195 次扫描 `arm=0`——**输入非零而 `s_bcnt` 判定/武装从未发生**；
- 连接后：`arm=scan`（100% 武装），`pr` 在 0x172~0x384 随槽位内容跳变；
- `early=0` 全程——`disp_en` 门控无影响；
- `scan` 反汇编（xbs1 自定义指令实现，capstone 无法解码，位于 s_sys.o）——其行为与主频档关联的确切机理需原厂确认（另一候选为 TMR1 所在 xosc26m 域在待机档被门控，寄存器读写为总线默认值，同样与全部观测吻合）。

---

## 4. 根因

**确证的机制链**（诊断固件实测 + 运行时反汇编 + 原厂源码三方交叉验证）：

1. 原厂 lib 的 `ledseg_ajust()`（已拿到源码，并与 app.bin 运行时反汇编、重定位符号表三方核对一致；`__riscv_save_0`/`__riscv_restore_0` 为 `-msave-restore` 编译选项的序言/尾声辅助函数）在每个扫描槽用 `s_bcnt()`（xbs1 自定义指令实现的位计数，位于 libplatform.a 的 s_sys.o）统计点亮段数，按 `ledseg_tbl` 查表设置 TMR1 消隐窗口（`TMR1CNT=0`、`TMR1PR=1000-查表值`、`TMR1CON=0x85`，时钟源 PCLKSEL=010=xosc26m 固定 26MHz）——**消隐是原厂的亮度控制设计，占空约 2~3.5%**。
2. 诊断固件实测（每 5 秒统计一次，195 次扫描采样零例外）：
   - **待机档（SYS_24M，未连接）**：`TMR1CON` 读回恒为 0、`TMR1PR` 读回恒为复位值 0xFFFFFFFF——对 TMR1 的写入**不生效**，消隐从未工作，数码管以 100% 占空全亮；
   - **连接后（主频抬至 196.608MHz，即"160M"档实际频率）**：每次扫描 `TMR1CON=0x85`、`TMR1PR=370~900` 全部生效，消隐工作，占空骤降为 2~3.5%。
3. 用户感知的"变暗+闪烁"= 待机全亮（消隐未生效）与连接后原厂设计亮度（消隐生效）之间的落差，叠加 PR 随显示内容在 370~900 间跳变（约 2.4 倍亮度波动）与射频抖动。
4. 同时排除的候选：RF 中断挤占扫描（tick 中断进入延迟实测 max 4~54µs，连接后反而更小）；消隐窗口随主频缩短（TMR1 时钟源为固定 26M 晶振）；显示内容缺失（`ledseg_cb.buf` 七槽镜像恒为 "2402" 内容）。

**待机档 TMR1 写入不生效的芯片级原因**待原厂最终确认（两个候选均与实测吻合：xosc26m 时钟域在待机档被门控导致寄存器读写为总线默认值；或 `s_bcnt` 所用 xbs1 自定义指令在低档位返回 0 使武装分支未执行），对本项目的修复方案无影响。

> 排查过程中被证据修正过的推测：①"TMR1 跑系统主频域、抬频后窗口缩短"（手册证实时钟源为固定 26M 晶振）；②"lib 按主频档位门控消隐"（原厂源码无门控）；③"lib 版本与源码不一致"（反汇编中的两次"调用"实为 `-msave-restore` 的序言/尾声辅助函数）。
> 第一版修复（读-改-写 TMR1PR 按主频补偿）失败的机理也已闭环：待机档 TMR1PR 读回的是总线默认值 0xFFFFFFFF，按比例回写垃圾值，连接档时钟域激活后定时器行为异常引发中断风暴，设备被 WDT 复位。

---

## 5. 修复方案（最终保留的代码）

位置：`app/modules/gui/ledseg/ledseg_7p7s.c`，`ledseg_7p7s_scan()` 内，`ledseg_ajust()` 之后：

```c
ledseg_ajust(disp_seg);
//原厂 lib 源码：ajust 在点亮段数>0 时启动 TMR1 消隐（PR=1000-按段数查表，
//窗口约 14~35µs / 1ms，占空仅 ~2-3.5%，用于按段数均衡亮度）。实测连接后
//消隐生效，整屏明显变暗且随内容/射频抖动闪烁。这里统一关闭消隐，保持全亮。
if (TMR1CON != 0) {
    TMR1CON = 0;
    TMR1CPND = BIT(9);              //清溢出 pending，防止残留触发 timer1_isr 关屏
}
ledseg_7p7s_set(disp_seg, com_cnt);
```

要点：

- 只有无条件的寄存器写（CON=0 + 清 pending），**不做任何 PR 读-改-写**（第一版崩溃的根源）；
- 必须放在 `ledseg_ajust()` 之后——lib 每次扫描都会重新配置 TMR1，放在之前会被覆盖；
- **这是有意偏离原厂设计**：原厂按段数均衡亮度（占空 2~3.5%），本修复统一为全亮（100% 占空），显示平均电流相应提高约一个数量级——与本项目待机态的历史行为一致，经整机实测接受；
- 修复位于 emit/adapter 共用的扫描代码，两端同时生效。

### 曾考虑但放弃的方案

| 方案 | 放弃原因 |
| --- | --- |
| 按主频比例放大 TMR1PR（提高连接态占空） | 需要读-改-写 PR，第一版实测引发中断风暴/WDT 复位 |
| 按原厂设计保留消隐、两档统一 ~2-3.5% 占空 | 整屏明显偏暗且占空随内容跳变的闪烁感仍在，不符合需求 |
| 取消连接后的抬主频 | 196.6M 是无线链路处理（LC3S 编码、算法）所需，动它影响音频 |

---

## 6. 验证结果

修复后发送端实测（数码管连接后与待机亮度一致、不闪）：

```text
────── 待机 ──────
LEDSEG DBG: clk=24000000, TMR1CON=0 TMR1PR=ffffffff, tick_lat max=34 avg=10 us
────── 连接后（lib 仍尝试开消隐，被修复代码关闭）──────
WIRELESS_CONNECTED,0 1
LEDSEG DBG: clk=196608000, TMR1CON=85 TMR1PR=320, tick_lat max=34 avg=5 us
LEDSEG DBG: clk=196608000, TMR1CON=85 TMR1PR=1f4, tick_lat max=4 avg=1 us
```

`TMR1CON=85 TMR1PR=320` 是采样点（在关闭动作之前）的值，说明 lib 仍在尝试启动消隐、被本修复关闭——符合预期。

---

## 7. 排查决策过程

```mermaid
flowchart TD
    A[现象: 连接后数码管变暗+持续闪烁<br/>断开恢复] --> B{候选原因}
    B --> C1[假设1: 抬主频缩短TMR1窗口<br/>TMR1跑系统主频域]
    B --> C2[候选2: RF中断挤占1ms扫描]
    B --> C3[候选3: 射频发射电源跌压]
    C1 --> D1[第一版修复: 读改写TMR1PR按主频补偿]
    D1 --> E1[❌ adapter连接即WDT复位<br/>停表态读PR得到0xFFFFFFFF垃圾值]
    E1 --> F[改用证据链定位]
    F --> G[反汇编 libplatform.a<br/>ledseg_ajust: PR=1000-查表值, CON=0x85]
    G --> H[查手册 Register 4-1<br/>PCLKSEL=010 → xosc26m_clk 固定26M]
    H --> I[❌ 假设1推翻: 消隐窗口与主频无关]
    I --> J[只读诊断固件<br/>TMR0CNT测中断延迟 + ISR采样TMR1CON/PR]
    J --> K{数据判读}
    K --> L[tick_lat max=4~34us<br/>❌ 候选2排除]
    K --> M[待机 CON=0 全亮占空100%<br/>连接后 CON=0x85 PR=370~900 占空2~3.5%]
    M --> N[✅ 真因: 连接后消隐生效占空骤降<br/>与待机全亮相成落差]
    N --> O[修复: ajust后无条件关闭TMR1<br/>不做PR读改写]
    O --> P[✅ 实测连接后亮度与待机一致，不闪]
```

## 8. 扫描与消隐时序（修复前后的差异）

```mermaid
sequenceDiagram
    participant T0 as TMR0(1ms tick)
    participant SC as ledseg_7p7s_scan
    participant AJ as ledseg_ajust(库)
    participant T1 as TMR1(26M晶振)
    participant ISR1 as timer1_isr
    participant SET as ledseg_7p7s_set

    Note over T0,SET: 每个 1ms COM 槽的时序
    T0->>SC: 溢出中断（实测进入延迟 max 4~54us）
    SC->>AJ: ledseg_ajust(本槽段码)
    alt 待机档 24M
        AJ--)T1: 不启动（CON=0）→ 槽内 100% 点亮（亮）
    else 连接档 196.6M
        AJ->>T1: CNT=0, PR=370~900, CON=0x85
        AJ->>SET: 点亮本 COM
        T1-->>ISR1: 19~35us 后溢出
        ISR1->>SET: ledseg_7p7s_clr() 关屏 → 槽内占空仅 2~3.5%（暗）
    end
    Note over SC,SET: 修复：ajust 之后强制 CON=0 + 清 pending<br/>连接档与待机档统一为 100% 点亮
```

---

## 9. 附：可复用的工具与方法

1. **反汇编闭源库**：`riscv32-elf-nm -S libplatform.a` 找符号与尺寸 → 从 .a 中提取对应 ELF 成员 → capstone（`CS_ARCH_RISCV, CS_MODE_RISCV32|CS_MODE_RISCVC`）解码，配合重定位项还原 SFR 真实地址（本芯片 SFR_BASE=0x100，TMR1CON=0x110/TMR1CNT=0x118/TMR1PR=0x11C，与 `include/sfr.h` 一致）；
2. **中断延迟测量**：固定 1MHz 的 tick 定时器溢出后 CNT 从 0 重数，ISR 入口读 CNT 即为"溢出 → ISR 实际执行"的微秒数，无需额外硬件定时器；
3. **手册是最终依据**：`docs/datasheet/ab571x_usermanual.pdf` Register 4-1（TMRxCON）、Register 4-2（TMRxCPND）。
