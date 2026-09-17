# 排错记录：无线连接建立后数码管变暗且持续闪烁

| 项目 | 内容 |
| --- | --- |
| 硬件 | AB5712F（K16B 板），发送端 emit（wireless_mic_emit）+ 接收端 adapter（wireless_adapter） |
| SDK | 中科蓝讯 bt8930 硅族 SDK，`app/projects/microphone`（CONFIG_WIRELESS_LE_MIC 方案） |
| 现象 | 发送端开机数码管亮度正常；与接收端建立无线连接后明显变暗，并持续闪烁；断开连接后恢复 |
| 分支 | `branch_02` |
| 原因结论 | 旧版原厂库把数码管消隐绑定到 TMR1。24MHz 待机档 TMR1 写入未锁存；无线连接后系统升至 196.608MHz，TMR1 在每个 COM 槽被成功武装并触发消隐，亮度骤降且消隐窗口随段数变化，形成“变暗+闪烁”。 |
| 最终修复 | 采用原厂提供的新版 `libplatform.a`（MD5 `A038D0F43636BF4AE4E9847BB72AC040`）：原厂将 `ledseg_ajust()` 的 one-shot 消隐改用 TMR4。应用层仅接入其 `timer4_irq_init()`，并在 `timer4_isr()` 完成 one-shot 清屏；不再自行配置或改写 TMR1/TMR4 的初始化寄存器。 |
| 验收状态 | 用户实机确认：待机、连接和断开后数码管均正常显示，无明显变暗或持续闪烁。 |

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
| 占空/消隐（旧库） | 闭源库 `ledseg_ajust()`（旧 `libplatform.a`） | 配置 TMR1；`timer1_isr()` 在 TMR1 溢出时调 `ledseg_7p7s_clr()` 关屏 |
| 占空/消隐（当前） | 原厂新版 `ledseg_ajust()`（当前 `libplatform.a`） | 配置 TMR4；`timer4_isr()` 在 TMR4 溢出时调 `ledseg_7p7s_clr()` 关屏 |
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
- 本轮采样尚不能单独区分 `s_bcnt` 路径和 TMR1 寄存器可访问性，故继续进行了第 3.5 节的原厂消隐路径验证。

### 3.5 原厂消隐路径验证（最终闭环）

为避免修复代码停止 TMR1 后把 ISR 计数归零，诊断构建**只在测试时**保留了原厂 `ledseg_ajust()` 后的 TMR1 消隐流程；除增加计数外，不重配 TMR1、不改写 PR。实测：

```text
────── 待机（24MHz，亮度正常）──────
LEDSEG_DIAG clk=24000000 scan=5000 lit=4935 bcnt0=0 bcntnz=4935 unarmed=4935 armed=0 tmr1isr=0 last=48/2/0/ffffffff
LEDSEG_DIAG clk=24000000 scan=5000 lit=5000 bcnt0=0 bcntnz=5000 unarmed=5000 armed=0 tmr1isr=0 last=76/5/0/ffffffff

────── 连接后（196.608MHz，变暗且闪烁）──────
LEDSEG_DIAG clk=196608000 scan=5000 lit=5000 bcnt0=0 bcntnz=5000 unarmed=4756 armed=244 tmr1isr=243 last=4f/5/85/320
LEDSEG_DIAG clk=196608000 scan=5000 lit=5000 bcnt0=0 bcntnz=5000 unarmed=0 armed=5000 tmr1isr=5000 last=7e/6/85/384
LEDSEG_DIAG clk=196608000 scan=5000 lit=5000 bcnt0=0 bcntnz=5000 unarmed=0 armed=5000 tmr1isr=5000 last=73/5/85/320
```

这组数据给出确定结论：

- 两档 `bcnt0=0`：与原厂 `ledseg_ajust()` 使用相同 `s_bcnt(disp_seg)` 和相同段码，排除“低频 `s_bcnt` 返回 0，因而没有执行武装分支”。
- 24MHz 时，原厂写入后的 `TMR1CON=0`、`TMR1PR=0xFFFFFFFF`，且 `tmr1isr=0`；即写入没有在 TMR1 外设侧锁存，消隐没有启动。
- 196.608MHz 稳定后，5000/5000 扫描均读到 `TMR1CON=0x85` 和有效 PR，且 `tmr1isr=5000`；每一个点亮槽都在原厂设定的窗口后被 `timer1_isr()` 清屏。
- 连接瞬间的 `unarmed=4756 / armed=244` 是 5 秒统计窗口跨越切频时刻的过渡数据；其后的完整窗口才代表稳定高频状态。

因此，已实证“连接后变暗”的直接机制和“低频不暗”的前置条件。手册没有公开 Clock Manager/CLKGAT 的位定义，故不能把低频写入未锁存进一步归因到某一具体门控位；但“低频档 TMR1 外设访问/写通路未就绪，高频档就绪”的现象已被实测确认。

---

## 4. 根因

**确证的机制链**（诊断固件实测 + 运行时反汇编 + 原厂源码三方交叉验证）：

1. 原厂 lib 的 `ledseg_ajust()`（已拿到源码，并与 app.bin 运行时反汇编、重定位符号表三方核对一致；`__riscv_save_0`/`__riscv_restore_0` 为 `-msave-restore` 编译选项的序言/尾声辅助函数）在每个扫描槽用 `s_bcnt()`（xbs1 自定义指令实现的位计数，位于 libplatform.a 的 s_sys.o）统计点亮段数，按 `ledseg_tbl` 查表设置 TMR1 消隐窗口（`TMR1CNT=0`、`TMR1PR=1000-查表值`、`TMR1CON=0x85`，时钟源 PCLKSEL=010=xosc26m 固定 26MHz）——**消隐是原厂的亮度控制设计，占空约 2~3.5%**。
2. 原厂消隐路径诊断固件实测（每 5 秒统计 5000 次扫描）：
   - **待机档（SYS_24M，未连接）**：`bcnt0=0`，但 `TMR1CON` 恒为 0、`TMR1PR` 恒为复位值 0xFFFFFFFF、`tmr1isr=0`。原厂分支具备执行条件，然而 TMR1 写入**没有锁存**，消隐从未工作，数码管以 100% 占空全亮；
   - **连接后（主频抬至 196.608MHz，即"160M"档实际频率）**：稳定窗口内每次扫描均为 `TMR1CON=0x85`、`TMR1PR=370~900`、`tmr1isr=1`，消隐工作，占空骤降为 2~3.5%。
3. 用户感知的"变暗+闪烁"= 待机全亮（消隐未生效）与连接后原厂设计亮度（消隐生效）之间的落差，叠加 PR 随显示内容在 370~900 间跳变（约 2.4 倍亮度波动）与射频抖动。
4. 同时排除的候选：RF 中断挤占扫描（tick 中断进入延迟实测 max 4~54µs，连接后反而更小）；消隐窗口随主频缩短（TMR1 时钟源为固定 26M 晶振）；显示内容缺失（`ledseg_cb.buf` 七槽镜像恒为 "2402" 内容）。

**待机档 TMR1 写入不生效的芯片级原因**仍需原厂最终确认。`s_bcnt=0` 已被实测排除；仅剩的证据支持方向是低频工作配置下 TMR1 的外设时钟域、寄存器总线或其写使能尚未就绪，导致读回总线默认值。手册未提供 Clock Manager/CLKGAT 位定义、TMR1 外设门控条件与 `sys_clk_set_do()` 实现，不能在没有原厂资料的情况下再指定到具体门控位。

> 排查过程中被证据修正过的推测：①"TMR1 跑系统主频域、抬频后窗口缩短"（手册证实时钟源为固定 26M 晶振）；②"lib 按主频档位门控消隐"（原厂源码无门控）；③"lib 版本与源码不一致"（反汇编中的两次"调用"实为 `-msave-restore` 的序言/尾声辅助函数）。
> 第一版修复（读-改-写 TMR1PR 按主频补偿）失败的机理也已闭环：待机档 TMR1PR 读回的是总线默认值 0xFFFFFFFF，按比例回写垃圾值，连接档时钟域激活后定时器行为异常引发中断风暴，设备被 WDT 复位。

原厂最终将这条消隐路径改到 TMR4，且用户实机验证正常。该结果确认问题的工程边界是“旧库的 TMR1 消隐路径与待机/无线切频工作状态不兼容”，而不是屏幕供电、RF 抢占扫描或应用显示内容问题。由于公开用户手册未给出旧 TMR1 在 24MHz 档未锁存的内部时钟/总线门控条件，本记录不把它进一步断言为某一个具体硬件门控位；最终以原厂 TMR4 实现作为可维护的修复依据。

---

## 5. 最终修复：采用原厂 TMR4 消隐实现

用户从原厂取得新的 `libplatform.a` 后，已替换工程中的
`app/libs/bt8930/libplatform.a`。本次库文件 MD5 为
`A038D0F43636BF4AE4E9847BB72AC040`，导出了新增的
`timer4_irq_init()`；该库中的 `ledseg_ajust()` 已将 one-shot 消隐从 TMR1
迁移到 TMR4（原厂实现使用 `PCLKSEL=6`）。这一步完全采用原厂提供的库和实现，应用层不重写定时器初始化、不推测寄存器位含义。

应用层只有两项配套接入：

1. `app/modules/gui/gui.c`：7P7S/6C6S 显示初始化时调用原厂库的 `timer4_irq_init()`，替代旧库的 `timer1_irq_init()`。
2. `app/system/interrupt.c`：新增 `timer4_isr()`。ISR 仅遵循原厂 one-shot 使用方式：确认 pending、清 TMR4 pending、停止当前 one-shot，然后调用对应的 `ledseg_*_clr()` 清屏。原 `timer1_isr()` 保留，避免影响其他可能的 TMR1 使用者，但当前数码管路径不再依赖它。

```c
// gui.c：初始化交给原厂新版库
timer4_irq_init();

// interrupt.c：TMR4 one-shot 到期后消隐当前 COM 槽
if (TMR4CON & BIT(9)) {
    TMR4CPND = BIT(9);
    TMR4CON = 0;
    ledseg_7p7s_clr();
}
```

`ledseg_7p7s.c` 恢复为原始扫描顺序：`ledseg_ajust(disp_seg)` 后立即
`ledseg_7p7s_set(disp_seg, com_cnt)`；其中不再含任何 TMR1 “强制关闭”、PR 比例补偿或诊断代码。

### 已撤销/放弃的路线

| 路线 | 结论与原因 |
| --- | --- |
| 按主频比例读-改-写 TMR1PR | 已撤销。待机档读到 `0xFFFFFFFF` 的无效值，回写后造成异常定时器行为和 WDT 复位。 |
| `ledseg_ajust()` 后强制关闭 TMR1 | 仅为临时观感规避，改变原厂的亮度均衡策略、提升平均电流，且不能解决 TMR1 路径本身在切频后的不确定性；已删除。 |
| 手工重配 TMR1 的 PCLKSEL/PR | 不采纳。该做法需要猜测时钟/门控语义，与“优先使用原厂实现”的约束不符。 |
| 取消无线连接后的升频 | 不采纳。196.608MHz 档用于无线音频处理，修改会扩大影响面。 |

---

## 6. 验证结果

### 实机功能验收

- 用户已用替换后的原厂库和本节 App 配套修改烧录验证：发送端在待机、无线连接成功和断开后三种状态均正常显示；连接后不再出现明显亮度下降或持续闪烁。
- 连接日志仍正常进入 `WIRELESS_CONNECTED`，没有再出现早期 PR 补偿实验导致的 WDT 循环复位。
- 原先用于定位的 `LEDSEG_*_DIAG` 计数与调试打印均已删除，正式固件不会持续输出诊断日志。

### 构建验收

使用工程的官方 Code::Blocks 配置等价命令行构建脚本执行：

```powershell
cd app/projects/microphone
powershell -ExecutionPolicy Bypass -NoProfile -File .\build.ps1 -Rebuild
```

该命令会先执行 `Output/bin/prebuild.bat`，生成 `res.bin`、`xcfg.bin`、`res.h`、`xcfg.h`、`effect.c` 和 `effect.h`，再编译链接、由 `postbuild.bat` 生成下载镜像。完整构建在本次提交前已通过；最终产物位于 `app/projects/microphone/Output/bin/`。

### 复现资源

此前根目录 `.gitignore` 忽略了整个 `app/projects/*/Output/`，使新 clone 缺少 `app.cbp` 的前/后置构建脚本和资源输入。本提交将强制纳入下列**构建输入和生成资源**，但不纳入可重新生成的对象文件、map、`app.rv32`、`app.bin` 或 `app.dcf`：

- `Output/bin/prebuild.bat`、`postbuild.bat`、`app.xm`、`download.xm`；
- xmaker 输入和结果：`res.xm`、`xcfg.xm`、`effect.bsln`、`effect.bts`、`res.bin`、`xcfg.bin`、`res.h`、`xcfg.h`、`effect.c`、`effect.h`；
- 打包依赖：`header.bin`、`unpack.bin`、`updater.bin`；
- 原始语音/效果资源 `Output/bin/res/` 和 xmaker 设置/方案文件 `Output/bin/Settings/`。

因此，clone 后具备本项目所需的厂商 RV32 工具链和 xmaker 时，可直接运行上述命令完成从零构建。

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
