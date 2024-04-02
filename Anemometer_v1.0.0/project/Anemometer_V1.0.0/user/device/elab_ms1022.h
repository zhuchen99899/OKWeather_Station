#ifndef __ELAB_MS1022_H
#define __ELAB_MS1022_H
#include "elab/edf/normal/elab_spi.h"

/* Opcodes  操作码-------------------------------------------------------------------*/

/* addr 地址-------------------------------------------------------------------*/
//写入配置寄存器 0x80+ADDR
#define   MS1022_REG0_ADDR    0x80 //写入配置寄存器0
#define   MS1022_REG1_ADDR    0x81 //写入配置寄存器1 
#define   MS1022_REG2_ADDR    0x82 //写入配置寄存器2
#define   MS1022_REG3_ADDR    0x83 //写入配置寄存器3
#define   MS1022_REG4_ADDR    0x84 //写入配置寄存器4
#define   MS1022_REG5_ADDR    0x85 //写入配置寄存器5
#define   MS1022_REG6_ADDR    0x86 //写入配置寄存器6

//读取寄存器 操作码 0xB0 +ADDR
#define   MS1022_RES0_ADDR     0xB0 //测量结果1 ，16为整数16位小数
#define   MS1022_RES1_ADDR     0xB1 //测量结果2 ，16为整数16位小数
#define   MS1022_RES2_ADDR     0xB2 //测量结果3 ，16为整数16位小数
#define   MS1022_RES3_ADDR     0xB3 //测量结果4 ，16为整数16位小数
#define   MS1022_STAT_ADDR     0xB4 //状态寄存器 16位
#define   MS1022_TEST_ADDR     0xB5//读取配置寄存器1高八位，用于测试通讯
#define   MS1022_PW1ST_ADDR    0xB8 //1位整数 7位小数 第一个回波半波宽度和给定比值


//0xBx  1 0 0 0 0 A A A  读取地址 A  8, 16 或32 位数据
//0x8x  1 0 1 1 0 A A A  写入地址 A  24 位或者 32 位数据
#define   MS1022CMD_R_ID_Bits              0xB7 //Read ID bit 56 位 ID
#define   MS1022CMD_R_PW1ST_RES            0xB8 //Read PW1ST 8 位
#define   MS1022CMD_W_EEPROM_From_REGS     0xC0 //写配置寄存器到 EEPROM
#define   MS1022CMD_Cfg_REGS_From_EEPROM   0xF0 //将 EEPROM 内容传回配置寄存器中
#define   MS1022CMD_Compare_EEPROM         0xC6 //将 EEPROM 和配置寄存器内容进行比较
#define   MS1022CMD_Init                   0x70 //Init
#define   MS1022CMD_Power_On_Reset         0x50 //Power_On_Reset
#define   MS1022CMD_Start_TOF              0x01 //Start_Cycle
#define   MS1022CMD_Start_Temp             0x02 //Start_Temp
#define   MS1022CMD_Start_Cal_Resonator    0x03 // Resonator 谐振器
#define   MS1022CMD_Start_Cal_TDC          0x04 //Start_Cal_TDC
#define   MS1022CMD_Start_TOF_Restart      0x05 //Start_TOF_Restart
#define   MS1022CMD_Start_Temp_Restart     0x06 //Start_Temp_Restart

/**
* 配置寄存器
* 
*/

/*****************寄存器0(地址0) 见芯片1.8 pdf page 13-14***************/
struct REG0_BITS{         //  bits  description                config
uint32_t ID0:8;           //  7:0   自由位，可存储版本号         reserved
uint32_t NEG_START:1;     //  8     反向start通道1输入           0 = 非反向输入信号 – 上升沿1 = 反向输入信号 – 下降沿
uint32_t NEG_STOP1:1;     //  9     反向stop通道1输入            0 = 非反向输入信号 – 上升沿1 = 反向输入信号 – 下降沿
uint32_t NEG_STOP2:1;     //  10    反向stop通道2输入            0 = 非反向输入信号 – 上升沿1 = 反向输入信号 – 下降沿
uint32_t MESSB2:1;        //  11    选择测量范围2                0 = 测量范围 1 1 = 测量范围 2
uint32_t NO_CAL_AUTO:1;   //  12    开启/关闭在TDC中运行自动校准  0 = 在测量后自动校准 1 = 关闭自动校准功能
uint32_t CALLBARTE:1;     //  13    开启/关闭在ALU中的校准        0 = 校准关闭(仅测量范围 1 中允许) 1 = 校准开启 (推荐)
uint32_t SEL_ECLK_TMP:1;  //  14    选择温度测量内部 cycle 时钟的参考信号 0 = 应用 32.768 kHz 作为时钟1 = 应用 128 * CLKHS 作为时钟周期 (在 4MHz 晶振的时候为 32μs )
uint32_t ANZ_FAKE:1;      //  15    在温度测量前的热身伪测量      0 = 2 次热身伪测量 1 = 7 次热身伪测量
uint32_t TCYCLE:1;        //  16    设置温度测量的循环时间        170 = 128 μs @ 4 MHz 1 = 512 μs @ 4 MHz (推荐)
uint32_t ANZ_PORT:1;      //  17    设置应用温度测量的端口数      0 = 2 个温度测量端口(PT1 和 PT2) 1 = 4 个温度测量端口
uint32_t START_CLKHS_LOW_2:2;   //  18:19 定义了在开启晶振后和测量开始前的晶振启动时间间隔.注意:调整 START_CLKS 的最高位是在寄存器 6 的第 20 位.当设置启动时间为 2.44 ms 和 5.14 ms 时,这位必须设置为 1  0 = 晶振关闭1 = 晶振一直开启 2 = 启振延迟 480 μs 3 = 启振延迟 1.46 ms 4 = 启振延迟 2.44 ms 5 到 7 = 启振延迟 5.14 ms 
uint32_t DIV_CLKHS:2;     //  20:21 设置 CLKHS 高速参考时钟的分频因数 0 = 不分频， 1 = 2 分频 2 = 4 分频， 3 = 4 分频
uint32_t ANZ_PER_CALRES:2;//  22:23 设置应用校准陶瓷晶振的 32k 时钟周期数  0 = 2 个周期= 61.035 μs 1 = 4 个周期= 122.07 μs 2 = 8 个周期= 244.14 μs 3 = 16 个周期= 488.281μs
uint32_t DIV_FIRE:4;      //  24:27 设置内部时钟信号产生脉冲的分频因数 0 = 不分频 1 = 2 分频 … 15 = 16 分频
uint32_t ANZ_FIRE_LOW_3:4;      //  27:31 设置 fire 端口发射的脉冲个数，其余 3 位在寄存器 6 中设置。若发射脉冲个数 ANZ_FIRE >15,则相位设置 PHFIRE 不能应用。0 = 关闭 1 = 1 个脉冲 … 127 = 127 个脉冲
};

union MS1022_REG0
{
uint32_t all;
struct REG0_BITS bits;
};



/*****************寄存器1(地址1) 见芯片1.8 pdf page 14-15 *****************/
struct REG1_BITS{         //  bits  description               config
uint32_t ID1:8;           //  0:7  自由位,例如可存储版本号等    
uint32_t SEL_TSTO1:3;     //  8:10 定义 FIRE_IN 管脚的功能.除了MS1022 中的功能外这个管脚还可以用作其它输出信号.如果SEL_TSTO1 >1 那么 FIRE_IN 内部将会被连接到 GND。0 = 与 MS1002 中同样功能, 声环法的Fire_in 输入 1 = START_TDC 输出 2 = STOP1 TDC 输出 3 = STOP2 TDC 输出 4 = 开启 Stop 温度测量输出 5 = TOF=UP, =1 当 TOF_UP 测量开启的时候 6 = RUN_HA, =1 当 hardmacro 开启的时候 7 = 32 kHz clock
uint32_t SEL_TSTO2:3;     // 11:13 定义 EN_START 管脚的功能.除了在 MS1022 中的功能之外,此管脚可以用于不同信号的输出.如果设 0 = MS1002 功能相同， 高平将开启 START管脚。1 = START_TDC 输出
uint32_t SEL_START_FIRE:1;//   14  脉冲用作触发 TDC start.Start 输入将被关闭  0 = 与 MS1002 相同 1 = 应用 fire 内部触发 Start
uint32_t CURR32K:1;       //   15  32kHz 晶振的低功耗选项。一般来说不必应用高电流选项，低电流也可以保证晶振的正常工作。0 = 低电流  1 = 高电流(原 MS1002 模式)
uint32_t HITIN1:3;        // 16:18 在 stop 通道 1 的预期脉冲数  
uint32_t HITIN2:3;        // 19:21  在 stop 通道 ２ 的预期脉冲数 0 = stop 通道关闭 1 = 1 个脉冲 2 = 2 个脉冲 3 = 3 个脉冲 4 = 4 个脉冲 5 到 7 = 不允许设置
uint32_t DEFAULT_VALUE:1; //   22   保持默认值=1
uint32_t EN_FAST_INIT:1;  //   23  启动快速初始化功能 0 = 关闭功能 1 = 启动功能
uint32_t HIT1:4;          // 24:27 用于定义 ALU 计算结果的方式： MRange1: HIT1 - HIT2 MRange2: HIT2 - HIT1
uint32_t HIT2:4;          // 28:31 // MRange1:     MRange2:
                                   // 0 = Start     
                                   // 1 = 1. Stop Ch1  1 = Start
                                   // 2 = 2. Stop Ch1  2 =1.Stop Ch1
                                   // 3 = 3. Stop Ch1  3 =2.Stop Ch1
                                   // 4 = 4. Stop Ch1  4 =3.Stop Ch1
                                   // 5 = 无动作
                                   // 6 = Cal1 Ch1
                                   // 7 = Cal2 Ch1
                                   // 9 = 1. Stop Ch2
                                   // A = 2. Stop Ch2
                                   // B = 3. Stop Ch2
                                   // C = 4. Stop Ch2
};


union MS1022_REG1
{
uint32_t all;
struct REG1_BITS bits;
};


/******************寄存器2(地址2) 见芯片1.8 pdf page 15 ******************/
struct REG2_BITS{         //  bits  description               config
uint32_t ID2:8;           // 0:7   自由位,例如可存储版本号等
uint32_t DELVAL1_DECIMAL:5;// DELVAL1 小数部分
uint32_t DELVAL1:14;      // 8:26  为使能 stop 脉冲而设置的延时，以 start 通道的第一个脉冲为起点开始计时。14 位整数部分，5位小数部分，Tref 的倍数。 DELVAL1 = 0 到 16383.96875
uint32_t RFEDGE1:1;       //  27   通道 1 的边沿敏感性          0 = 上升或下降沿 1 = 上升和下降沿
uint32_t RFEDGE2:1;       //  28   通道 2 的边沿敏感性          0 = 上升或下降沿 1 = 上升和下降沿
uint32_t EN_INT_LOW_3:3;        // 29:31 利用或门启动不同的中断触发,还有一位在寄存器 6   Bit 31 = 1： Timeout 中断触发位 Bit 30 = 1： End Hits 中断触发位 Bit 29 = 1： ALU 中断触发位
};

union MS1022_REG2
{
uint32_t all;
struct REG2_BITS bits;
};

/*******************寄存器3(地址3) 见芯片1.8 pdf page 15-16 *******************/
struct REG3_BITS_DISABLE_FIRST_WAVE //  bits  description               config
{
uint32_t ID3:8;                     // 0:7   自由位,例如可存储版本号等
uint32_t DELVAL2_DECIMAL;           // DELVAL2 小数部分
uint32_t DELVAL2:19;                // 8:26  为使能 stop 脉冲而设置的延时，以 start 通道的第一个脉冲为起点开始计时。14 位整数部分，5位小数部分，Tref 的倍数。 DELVAL2 = 0 到 16383.96875
uint32_t SEL_TIMO_MB2:2;            // 27:28 在测量范围 2 内为溢出选择时间限制 0 = 64 µs 1 = 256 µs 2 = 1024 µs 3 = 4096 µs @ 4 MHz ClkHS
uint32_t EN_ERR_VAL:1;              // 29    由于时间溢出强迫 ALU 写入0xFFFFFFFF 到结果寄存器 0 = 关闭 1 = 开启
uint32_t EN_FIRST_WAVE:1;           // 30 开启自动第一波检测功能。如果开启，那么在寄存器 3 和寄存器4 中将会有新的意义产生。0 = 关闭  1 = 开启
uint32_t EN_AUTOCALC_MB2:1;         // 31 仅在测量范围 2 中: 自动计算所有开启获得的脉冲。而这些结果的和将会写入到寄存 器 4 当中。 0 = 关闭 1 = 开启
};//EN_FIRST_WAVE 此模式下为=0

struct REG3_BITS_FIRST_WAVE{         //  bits  description               config
uint32_t ID3:8;                     // 0:7   自由位,例如可存储版本号等
uint32_t DELREL1:6;                 // 8:13  设置第一波检测到第 1 个 stop 是接收第几个回波周期  3 到 63
uint32_t DELREL2:6;                 // 14:19 设置第一波检测到第 2 个 stop 是接收第几个回波周期  4 到 63
uint32_t DELREL3:6;                 // 20:25 设置第一波检测到第 3 个 stop 是接收第几个回波周期  5 到 63
uint32_t DEFAULT_VALUE:1;           // 保持默认值=0
uint32_t SEL_TIMO_MB2:2;            // 27:28 在测量范围 2 内为溢出选择时间限制 0 = 64 µs 1 = 256 µs 2 = 1024 µs 3 = 4096 µs @ 4 MHz ClkHS
uint32_t EN_ERR_VAL:1;              // 29    由于时间溢出强迫 ALU 写入0xFFFFFFFF 到结果寄存器 0 = 关闭 1 = 开启
uint32_t EN_FIRST_WAVE:1;           // 30 开启自动第一波检测功能。如果开启，那么在寄存器 3 和寄存器4 中将会有新的意义产生。0 = 关闭  1 = 开启
uint32_t EN_AUTOCALC_MB2:1;         // 31 仅在测量范围 2 中: 自动计算所有开启获得的脉冲。而这些结果的和将会写入到寄存 器 4 当中。 0 = 关闭 1 = 开启
};//EN_FIRST_WAVE 此模式下为=1


union MS1022_REG3
{
uint32_t all;
struct REG3_BITS_FIRST_WAVE if_set_firstWave_bits;
struct REG3_BITS_DISABLE_FIRST_WAVE if_reset_firstWave_bits;
};


/******************寄存器4(地址4) 见芯片1.8 pdf page 17 ******************/
struct REG4_BITS_DISABLE_FIRST_WAVE{         //  bits  description               config
uint32_t ID4:8;                      // 0:7   自由位,例如可存储版本号等
uint32_t DELVAL3_DECIMAL:5;          //DELVAL3 小数部分
uint32_t DELVAL3:14;                 // 8:26  为使能 stop 脉冲而设置的延时，以 start 通道的第一个脉冲为起点开始计时。14 位整数部分，5位小数部分，Tref 的倍数。DELVAL3 = 0 到 16383.96875
uint32_t DEFAULT_VALUE:5;            // 27:31  保持默认值=2
};//EN_FIRST_WAVE 此模式下为=0


struct REG4_BITS_FIRST_WAVE{     //  bits  description               config
uint32_t ID4:8;                      // 0:7  自由位,例如可存储版本号等
uint32_t OFFS:5;                     // 8:12 设置比较器 offset，2 的补码形式设置，单位为 1 mV 0 = 0 mV ... 1 = +1 mV 15 = +15 mV 16 = -16 mV 17 = -15 mV ... 31 = -1 mV
uint32_t OFFSRNG1:1;                 // 13   增加额外的 offset - 20 mV 0 = 关闭 1 = 开启
uint32_t OFFSRNG2:1;                 // 14   增加额外的 offset + 20 mV 0 = 关闭 1 = 开启
uint32_t EDGE_FW:1;                  // 15   设置第一波识别的边沿敏感。设置为下降沿敏感的话则对于负的幅值比较有意义。0 = 上升沿 1 = 下降沿
uint32_t DIS_PW:1;                   // 16   关闭脉冲宽度测量功能  0 = 开启脉冲宽度测量 1 = 关闭脉冲宽度测量
uint32_t DEFAULT_VALUE1:10;          // 17:26  保持默认值=0
uint32_t DEFAULT_VALUE2:5;           // 27:31  保持默认值=2
};//EN_FIRST_WAVE 此模式下为=1

union MS1022_REG4
{
uint32_t all;
struct REG4_BITS_FIRST_WAVE if_set_firstWave_bits;
struct REG4_BITS_DISABLE_FIRST_WAVE if_reset_firstWave_bits;
};

/******************寄存器5(地址5) 见芯片1.8 pdf page 18 ******************/
struct REG5_BITS{          //  bits  description               config
uint32_t ID5:8;            //  0:7   自由位,例如可存储版本号等
uint32_t PHFIRE:16;        //  8:23  对最多 15 个脉冲的脉冲序列中的每个脉冲进行相位设置 0 = 不反向 1 = 反向
uint32_t REPEAT_FIRE:3;    //  24:26 用于声环法的脉冲序列的重复次数 0 = 不重复 1 = 1 次重复 s... 7 = 7 次重复
uint32_t DIS_PHASESHIFT:1; //  27    噪声单元. 当应用 MS1022 参考时钟产生 start 脉冲的时候(例如将fire 接到 start),请开启这个单元通过平均可以降低系统误差。1 = 关闭相位噪声移位单元，MS1002 必须设置为 1,0 = 开启
uint32_t EN_STARTNOISE:1;  //  28    给 start 通道信号外加噪声  1 = 开启噪声单元
uint32_t CONF_FIRE:3;      //  29:31 脉冲触发器的输出设置,3’b 011 不可以设置 Bit 31 = 1: 2 个都打开 (反向 FIRE_DOWN) Bit 30 = 1: 关闭输出 FIRE_UP Bit 29 = 1: 关闭输出 FIRE_DOWN
};

union MS1022_REG5
{
uint32_t all;
struct REG5_BITS bits;
};

/******************寄存器6(地址6) 见芯片1.8 pdf page 18-20 ******************/
struct REG6_BITS{               //  bits  description               config
uint32_t ID6:8;                 //  0:7   自由位,例如可存储版本号等
uint32_t ANZ_FIRE_HIGH_3:3;     //  8:10  高 3 位设置发出脉冲的个数. 同时参见寄存器 0。如果 ANZ_FIRE> 15 那么 PHFIRE 对于脉冲相位的设置将不在有效 0 = 关闭 1 = 1 个脉冲 2 = 2 个脉冲 … 127 = 127 个脉冲
uint32_t TEMP_PORTDIR:1;        //  11    温度测量端口测量顺序反向 0 = PT1 > PT2 > PT3 > PT4 1 = PT4 > PT3 > PT2 > PT1
uint32_t DOUBLE_RES:1;          //  12    在测量范围 2 中将测量精度加倍从 75 ps 到 37 ps 0 = 关闭 (MS1002 模式) 1 = 开启
uint32_t QUAD_RES:1;            //  13    在测量范围 2 中将测量精度提高4 倍从 75 ps 到 19 ps 0 = 关闭 (MS1002 模式) 1 = 开启
uint32_t FIREO_DEF:1;           //  14    定义非活动状态 fire 通道的默认水平。如：如果 FIRE_UP 活动状态，那么 FIRE_DOWN buffer 则连接到默认的水平。当应用内部集成的模拟部分时，必须设置为 1 0 = High-Z (MS1002 模式) 1 = 低
uint32_t HZ60:1;                //  15    MS1022 可以进行一个完整的上游飞行时间和下游飞行时间，以及两次温度测量。 在两次测量之间的延迟是基于 50 或 60 Hz 时钟0 = 50 Hz 为基础, 20 ms 1 = 60 Hz 为基础, 16.67ms
uint32_t CYCLE_TOF:2;           //  16:17 选择触发第二次时间测量的定时器，以 50/60Hz 的倍数给出0 = 0.5 1 = 0.75 2 = 1 3 = 1.25
uint32_t CYCLE_TEMP:2;          //  18:19 选择触发第二次温度测量的定时器，以 50/60Hz 的倍数给出0 = 0.5 1 = 0.75 2 = 1 3 = 1.25
uint32_t START_CLKHS_HIGH_1:1;  //  20    最高位设置高速晶振的启振时间。低位则在寄存器 0 中设置0 = 关闭 1 = 持续开启 2 = 480 μs 延迟 3 = 1.46 ms 4 = 2.44 ms 5 到 7 = 5.14 ms
uint32_t EN_INT_HIGH_1:1;       //  21    其他位参见寄存器 2 的低三位EN_INT. 不同的中断源内部通过或门连接. EEPROM 动作, 例如EEPROM_COMPARE,将通过 MS1022运行，尤其是 EEPROM 写操作时需要持续最多 130ms. 因此，显示操作的结束非常有必要.  1 = EEPROM 动作结束
uint32_t TW2:2;                 //  22:23 当应用内部模拟部分时,给所推荐的 RC 的电容进行充电的时间 充电时间:0 = 90 µs 1 = 120 µs 2 = 150 µs 3 = 300 µs
uint32_t DEFAULT_VALUE1:1;      //  24    保持默认值=0
uint32_t DA_KORR:4;             //  25:28 设置比较器 offset 从 -8 mV 到 +7 mV。 以 2 的 补码形式 15 = - 1 mV 7 = 7 mV  14 = - 2 mV 6 = 6 mV ... ... 9 = - 7 mV 1 = 1 mV 8 = - 8 mV 0 = 0 mV 
uint32_t DEFAULT_VALUE2:1;      //  29    保持默认值=0
uint32_t NEG_STOP_TEMP:1;       //  30    反向 SenseT 输入信号.这个在内部比较器应用，而不是在 MS1002应用外部比较器的时候必须开启0 = 外部 74HC14 应用(MS1002 兼容) 1 = 应用内部施密特触发器
uint32_t EN_ANALOG:1;           //  31    开启在超声波流量测量中需要的模拟测量部分.若开启,那么这个部分将会在测量的时候通电以节省电流. STOP1 和 STOP2 这个时候内部将自动选择为模拟输入端0 = STOP1 和 STOP2 为数字输入端 (与MS1002 兼容) 1 = 模拟部分开启
};

union MS1022_REG6
{
uint32_t all;
struct REG6_BITS bits;
};

/******************配置寄存器 ******************/
struct MS1022_CONFIG_REGS
{
union MS1022_REG0 REG0;  //配置寄存器REG0   
union MS1022_REG1 REG1;  //配置寄存器REG1   
union MS1022_REG2 REG2;  //配置寄存器REG2   
union MS1022_REG3 REG3;  //配置寄存器REG3   
union MS1022_REG4 REG4;  //配置寄存器REG4   
union MS1022_REG5 REG5;  //配置寄存器REG5   only write
union MS1022_REG6 REG6;  //配置寄存器REG6   only write
};



/**
* 读寄存器
* only read
* 见芯片1.8 pdf page 20
*/
//测量结果寄存器



//STAT状态寄存器
struct STAT_DATA
{
uint16_t RESULT_PONIT:3;              //结果地址指针
uint16_t CHANNEL1_PULSE_COUNT:3;      //通道1接收到的脉冲数
uint16_t CHANNEL2_PULSE_COUNT:3;      //通道2接收到的脉冲数
uint16_t TDC_OVERFLOW_FLAG:1;         //TDC单元溢出  1=溢出
uint16_t ROUGH_OVERFLOW_FLAG:1;       //粗值计数溢出 1=溢出
uint16_t TEMPERATURE_OPEN_CIRCUIT:1;  //温度测量开路 1=开路
uint16_t TEMPERATURE_SHORT_CIRCUIT:1; //温度测量短路 1=短路
uint16_t EEPROM_ERROR:1;              //在eeprom 中有一个错误并被纠正 1=错误
uint16_t EEPROM_DED:1;                //多次错误，在eeprom中有2处错误且不能纠正 1=2处错误
uint16_t EEPROM_eq_CRE:1;             //显示配置寄存器中的内容是否与EEPROM中相同
};

union MS1022_STAT
{
uint16_t all;
struct STAT_DATA data; 
};


//PW1ST寄存器
struct PW1ST_DATA
{
uint8_t integer:1;
uint8_t decimal:7;
};

union MS1022_PW1ST
{
uint8_t all;
struct PW1ST_DATA data;
};

struct MS1022_READ_REGS
{
uint32_t RES0;//读寄存器0 测量结果1 地址0
uint32_t RES1;//读寄存器1 测量结果2 地址1
uint32_t RES2;//读寄存器2 测量结果3 地址2
uint32_t RES3;//读寄存器3 测量结果4 地址3
union MS1022_STAT STAT; //stat状态寄存器
uint8_t testCommunicate;//测试通讯寄存器  写寄存器中的高八位
union MS1022_PW1ST PW1ST;//PW1ST 给出了第一个回波半波 (在给定的偏移电压情况下)和给出的所需接收回波半波的宽度比值。数据范围: 0 到 1.99219
};



/**
* MS1022寄存器
* 见芯片1.8 pdf 
*/
struct MS1022_REGS
{
   struct MS1022_CONFIG_REGS cfg_regs;
   struct MS1022_READ_REGS   read_regs;
};



typedef struct elab_spidev_ms1022
{
elab_device_t super;         //设备层基类
elab_device_t *spi;          //spi设备
elab_device_t *stop_INT_pin;  //回波中断引脚
osMutexId_t mutex;           //设备锁
struct MS1022_REGS regs;     //ms1022寄存器
void *user_data;             //驱动数据
}elab_spidev_ms1022_t;


void elab_ms1022_register(elab_spidev_ms1022_t * me,
                       const char *name, const char *spi_name,const char * pin_name);
elab_err_t elab_ms1022_init(elab_device_t *const me);

elab_err_t  ms1022_test_communication(elab_device_t *const me);
float elab_ms1022_read_TOF(elab_device_t *const me);
void elab_ms1022_read_all_cfgregs(elab_device_t *const me);
float elab_ms1022_freq_corr_fact(elab_device_t *const me);
uint16_t elab_ms1022_read_error_bit(elab_device_t *const me);
void elab_ms1022_wait_int(elab_device_t *const me);
void elab_ms1022_send_cmd(elab_device_t *const me,uint8_t cmd);
#endif
