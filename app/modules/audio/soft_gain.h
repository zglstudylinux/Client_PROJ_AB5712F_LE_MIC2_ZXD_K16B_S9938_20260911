#ifndef _SOFT_GAIN_DRC_H
#define _SOFT_GAIN_DRC_H

/*****************************************************************************
* Module    : 软件数字增益调节表
*****************************************************************************/
//正增益
#define SOFT_GAIN_P31_5DB      (SOFT_GAIN_P0_0DB * 37.583740)      //307886
#define SOFT_GAIN_P31_0DB      (SOFT_GAIN_P0_0DB * 35.481339)      //290663
#define SOFT_GAIN_P30_5DB      (SOFT_GAIN_P0_0DB * 33.496544)      //274403
#define SOFT_GAIN_P30_0DB      (SOFT_GAIN_P0_0DB * 31.622777)      //259053
#define SOFT_GAIN_P29_5DB      (SOFT_GAIN_P0_0DB * 29.853826)      //244562
#define SOFT_GAIN_P29_0DB      (SOFT_GAIN_P0_0DB * 28.183829)      //230881
#define SOFT_GAIN_P28_5DB      (SOFT_GAIN_P0_0DB * 26.607251)      //217966
#define SOFT_GAIN_P28_0DB      (SOFT_GAIN_P0_0DB * 25.118864)      //205773
#define SOFT_GAIN_P27_5DB      (SOFT_GAIN_P0_0DB * 23.713737)      //194262
#define SOFT_GAIN_P27_0DB      (SOFT_GAIN_P0_0DB * 22.387211)      //183396
#define SOFT_GAIN_P26_5DB      (SOFT_GAIN_P0_0DB * 21.134890)      //173137
#define SOFT_GAIN_P26_0DB      (SOFT_GAIN_P0_0DB * 19.952623)      //163451
#define SOFT_GAIN_P25_5DB      (SOFT_GAIN_P0_0DB * 18.836491)      //154308
#define SOFT_GAIN_P25_0DB      (SOFT_GAIN_P0_0DB * 17.782794)      //145676
#define SOFT_GAIN_P24_5DB      (SOFT_GAIN_P0_0DB * 16.788040)      //137527
#define SOFT_GAIN_P24_0DB      (SOFT_GAIN_P0_0DB * 15.848932)      //129834
#define SOFT_GAIN_P23_5DB      (SOFT_GAIN_P0_0DB * 14.962357)      //122571
#define SOFT_GAIN_P23_0DB      (SOFT_GAIN_P0_0DB * 14.125375)      //115715
#define SOFT_GAIN_P22_5DB      (SOFT_GAIN_P0_0DB * 13.335214)      //109242
#define SOFT_GAIN_P22_0DB      (SOFT_GAIN_P0_0DB * 12.589254)      //103131
#define SOFT_GAIN_P21_5DB      (SOFT_GAIN_P0_0DB * 11.885022)      //97362
#define SOFT_GAIN_P21_0DB      (SOFT_GAIN_P0_0DB * 11.220185)      //91915
#define SOFT_GAIN_P20_5DB      (SOFT_GAIN_P0_0DB * 10.592537)      //86774
#define SOFT_GAIN_P20_0DB      (SOFT_GAIN_P0_0DB * 10.000000)      //81920
#define SOFT_GAIN_P19_5DB      (SOFT_GAIN_P0_0DB * 9.440609)      //77337
#define SOFT_GAIN_P19_0DB      (SOFT_GAIN_P0_0DB * 8.912509)      //73011
#define SOFT_GAIN_P18_5DB      (SOFT_GAIN_P0_0DB * 8.413951)      //68927
#define SOFT_GAIN_P18_0DB      (SOFT_GAIN_P0_0DB * 7.943282)      //65071
#define SOFT_GAIN_P17_5DB      (SOFT_GAIN_P0_0DB * 7.498942)      //61431
#define SOFT_GAIN_P17_0DB      (SOFT_GAIN_P0_0DB * 7.079458)      //57994
#define SOFT_GAIN_P16_5DB      (SOFT_GAIN_P0_0DB * 6.683439)      //54750
#define SOFT_GAIN_P16_0DB      (SOFT_GAIN_P0_0DB * 6.309573)      //51688
#define SOFT_GAIN_P15_5DB      (SOFT_GAIN_P0_0DB * 5.956621)      //48796
#define SOFT_GAIN_P15_0DB      (SOFT_GAIN_P0_0DB * 5.623413)      //46067
#define SOFT_GAIN_P14_5DB      (SOFT_GAIN_P0_0DB * 5.308844)      //43490
#define SOFT_GAIN_P14_0DB      (SOFT_GAIN_P0_0DB * 5.011872)      //41057
#define SOFT_GAIN_P13_5DB      (SOFT_GAIN_P0_0DB * 4.731513)      //38760
#define SOFT_GAIN_P13_0DB      (SOFT_GAIN_P0_0DB * 4.466836)      //36592
#define SOFT_GAIN_P12_5DB      (SOFT_GAIN_P0_0DB * 4.216965)      //34545
#define SOFT_GAIN_P12_0DB      (SOFT_GAIN_P0_0DB * 3.981072)      //32612
#define SOFT_GAIN_P11_5DB      (SOFT_GAIN_P0_0DB * 3.758374)      //30788
#define SOFT_GAIN_P11_0DB      (SOFT_GAIN_P0_0DB * 3.548134)      //29066
#define SOFT_GAIN_P10_5DB      (SOFT_GAIN_P0_0DB * 3.349654)      //27440
#define SOFT_GAIN_P10_0DB      (SOFT_GAIN_P0_0DB * 3.162278)      //25905
#define SOFT_GAIN_P9_5DB      (SOFT_GAIN_P0_0DB * 2.985383)      //24456
#define SOFT_GAIN_P9_0DB      (SOFT_GAIN_P0_0DB * 2.818383)      //23088
#define SOFT_GAIN_P8_5DB      (SOFT_GAIN_P0_0DB * 2.660725)      //21796
#define SOFT_GAIN_P8_0DB      (SOFT_GAIN_P0_0DB * 2.511886)      //20577
#define SOFT_GAIN_P7_5DB      (SOFT_GAIN_P0_0DB * 2.371374)      //19426
#define SOFT_GAIN_P7_0DB      (SOFT_GAIN_P0_0DB * 2.238721)      //18339
#define SOFT_GAIN_P6_5DB      (SOFT_GAIN_P0_0DB * 2.113489)      //17313
#define SOFT_GAIN_P6_0DB      (SOFT_GAIN_P0_0DB * 1.995262)      //16345
#define SOFT_GAIN_P5_5DB      (SOFT_GAIN_P0_0DB * 1.883649)      //15430
#define SOFT_GAIN_P5_0DB      (SOFT_GAIN_P0_0DB * 1.778279)      //14567
#define SOFT_GAIN_P4_5DB      (SOFT_GAIN_P0_0DB * 1.678804)      //13752
#define SOFT_GAIN_P4_0DB      (SOFT_GAIN_P0_0DB * 1.584893)      //12983
#define SOFT_GAIN_P3_5DB      (SOFT_GAIN_P0_0DB * 1.496236)      //12257
#define SOFT_GAIN_P3_0DB      (SOFT_GAIN_P0_0DB * 1.412538)      //11571
#define SOFT_GAIN_P2_5DB      (SOFT_GAIN_P0_0DB * 1.333521)      //10924
#define SOFT_GAIN_P2_0DB      (SOFT_GAIN_P0_0DB * 1.258925)      //10313
#define SOFT_GAIN_P1_5DB      (SOFT_GAIN_P0_0DB * 1.188502)      //9736
#define SOFT_GAIN_P1_0DB      (SOFT_GAIN_P0_0DB * 1.122018)      //9191
#define SOFT_GAIN_P0_5DB      (SOFT_GAIN_P0_0DB * 1.059254)      //8677
#define SOFT_GAIN_P0_0DB      (SOFT_GAIN_Q13_VAL * 1.000000)     //8192
#define SOFT_GAIN_Q13_VAL     (1<<SOFT_GAIN_Q)
#define SOFT_GAIN_Q           (15)

//负增益
#define SOFT_GAIN_N0DB            (SOFT_GAIN_P0_0DB / 1.000000)
#define SOFT_GAIN_N1DB            (SOFT_GAIN_P0_0DB / 1.122018)
#define SOFT_GAIN_N2DB            (SOFT_GAIN_P0_0DB / 1.258925)
#define SOFT_GAIN_N3DB            (SOFT_GAIN_P0_0DB / 1.412538)
#define SOFT_GAIN_N4DB            (SOFT_GAIN_P0_0DB / 1.584893)
#define SOFT_GAIN_N5DB            (SOFT_GAIN_P0_0DB / 1.778279)
#define SOFT_GAIN_N6DB            (SOFT_GAIN_P0_0DB / 1.995262)
#define SOFT_GAIN_N7DB            (SOFT_GAIN_P0_0DB / 2.238721)
#define SOFT_GAIN_N8DB            (SOFT_GAIN_P0_0DB / 2.511886)
#define SOFT_GAIN_N9DB            (SOFT_GAIN_P0_0DB / 2.818383)
#define SOFT_GAIN_N10DB           (SOFT_GAIN_P0_0DB / 3.162278)
#define SOFT_GAIN_N11DB           (SOFT_GAIN_P0_0DB / 3.548134)
#define SOFT_GAIN_N12DB           (SOFT_GAIN_P0_0DB / 3.981072)
#define SOFT_GAIN_N13DB           (SOFT_GAIN_P0_0DB / 4.466836)
#define SOFT_GAIN_N14DB           (SOFT_GAIN_P0_0DB / 5.011872)
#define SOFT_GAIN_N15DB           (SOFT_GAIN_P0_0DB / 5.623413)
#define SOFT_GAIN_N16DB           (SOFT_GAIN_P0_0DB / 6.309573)
#define SOFT_GAIN_N17DB           (SOFT_GAIN_P0_0DB / 7.079458)
#define SOFT_GAIN_N18DB           (SOFT_GAIN_P0_0DB / 7.943282)
#define SOFT_GAIN_N19DB           (SOFT_GAIN_P0_0DB / 8.912509)
#define SOFT_GAIN_N20DB           (SOFT_GAIN_P0_0DB / 10.000000)
#define SOFT_GAIN_N21DB           (SOFT_GAIN_P0_0DB / 11.220185)
#define SOFT_GAIN_N22DB           (SOFT_GAIN_P0_0DB / 12.589254)
#define SOFT_GAIN_N23DB           (SOFT_GAIN_P0_0DB / 14.125375)
#define SOFT_GAIN_N24DB           (SOFT_GAIN_P0_0DB / 15.848932)
#define SOFT_GAIN_N25DB           (SOFT_GAIN_P0_0DB / 17.782794)
#define SOFT_GAIN_N26DB           (SOFT_GAIN_P0_0DB / 19.952623)
#define SOFT_GAIN_N27DB           (SOFT_GAIN_P0_0DB / 22.387211)
#define SOFT_GAIN_N28DB           (SOFT_GAIN_P0_0DB / 25.118864)
#define SOFT_GAIN_N29DB           (SOFT_GAIN_P0_0DB / 28.183829)
#define SOFT_GAIN_N30DB           (SOFT_GAIN_P0_0DB / 31.622777)
#define SOFT_GAIN_N31DB           (SOFT_GAIN_P0_0DB / 35.481339)
#define SOFT_GAIN_N32DB           (SOFT_GAIN_P0_0DB / 39.810717)
#define SOFT_GAIN_N33DB           (SOFT_GAIN_P0_0DB / 44.668359)
#define SOFT_GAIN_N34DB           (SOFT_GAIN_P0_0DB / 50.118723)
#define SOFT_GAIN_N35DB           (SOFT_GAIN_P0_0DB / 56.234133)
#define SOFT_GAIN_N36DB           (SOFT_GAIN_P0_0DB / 63.095734)
#define SOFT_GAIN_N37DB           (SOFT_GAIN_P0_0DB / 70.794578)
#define SOFT_GAIN_N38DB           (SOFT_GAIN_P0_0DB / 79.432823)
#define SOFT_GAIN_N39DB           (SOFT_GAIN_P0_0DB / 89.125094)
#define SOFT_GAIN_N40DB           (SOFT_GAIN_P0_0DB / 100.000000)
#define SOFT_GAIN_N41DB           (SOFT_GAIN_P0_0DB / 112.201845)
#define SOFT_GAIN_N42DB           (SOFT_GAIN_P0_0DB / 125.892541)
#define SOFT_GAIN_N43DB           (SOFT_GAIN_P0_0DB / 141.253754)
#define SOFT_GAIN_N44DB           (SOFT_GAIN_P0_0DB / 158.489319)
#define SOFT_GAIN_N45DB           (SOFT_GAIN_P0_0DB / 177.827941)
#define SOFT_GAIN_N46DB           (SOFT_GAIN_P0_0DB / 199.526231)
#define SOFT_GAIN_N47DB           (SOFT_GAIN_P0_0DB / 223.872114)
#define SOFT_GAIN_N48DB           (SOFT_GAIN_P0_0DB / 251.188643)
#define SOFT_GAIN_N49DB           (SOFT_GAIN_P0_0DB / 281.838293)
#define SOFT_GAIN_N50DB           (SOFT_GAIN_P0_0DB / 316.227766)
#define SOFT_GAIN_N51DB           (SOFT_GAIN_P0_0DB / 354.813389)
#define SOFT_GAIN_N52DB           (SOFT_GAIN_P0_0DB / 398.107171)
#define SOFT_GAIN_N53DB           (SOFT_GAIN_P0_0DB / 446.683592)
#define SOFT_GAIN_N54DB           (SOFT_GAIN_P0_0DB / 501.187234)
#define SOFT_GAIN_N55DB           (SOFT_GAIN_P0_0DB / 562.341325)
#define SOFT_GAIN_N56DB           (SOFT_GAIN_P0_0DB / 630.957344)
#define SOFT_GAIN_N57DB           (SOFT_GAIN_P0_0DB / 707.945784)
#define SOFT_GAIN_N58DB           (SOFT_GAIN_P0_0DB / 794.328235)
#define SOFT_GAIN_N59DB           (SOFT_GAIN_P0_0DB / 891.250938)
#define SOFT_GAIN_N60DB           (SOFT_GAIN_P0_0DB / 1000.00000)
#define SOFT_GAIN_N65DB           (SOFT_GAIN_P0_0DB / 1778.27941)
#define SOFT_GAIN_N110DB           0

#define PCM_MAX_V               32767
#define PCM_MIN_V               -32768

typedef void (*audio_callback_t)(u8 *ptr, u32 samples, int ch_mode, void *param);
typedef s16 mic_pcm_t;

void soft_gain_init(void);
void soft_gain_proc(mic_pcm_t *ptr, u32 samples);
void soft_gain_set_level(uint level);

void spk_soft_gain_init(u8 sample_rate, u16 samples, u8 channel);
void spk_soft_gain_input(u8 *ptr, u32 samples, int ch_mode, void *params);
void spk_soft_gain_audio_output_callback_set(audio_callback_t callback);
void spk_soft_gain_set_level(uint level);
#endif
