/**
* @file ms1022.c
* @author zc (387646983@qq.com)
* @brief 
* @version 0.1
* @date 2024-01-05
* 
* @copyright Copyright (c) 2024
* 
*/
#include "elab_ms1022.h"
#include "../../common/elab_assert.h"
#include "elab/edf/normal/elab_pin.h"
#include "elab/edf/normal/elab_spi.h"
#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("Edf_MS1022");

/* Private variables ---------------------------------------------------------*/
static const elab_dev_ops_t MS1022_ops =
{
   .enable = NULL,
   .read = NULL,
   .write = NULL,
#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_POLL)
   .poll = NULL,
#endif
};

static const osMutexAttr_t _mutex_attr_ms1022 =
{
   .name = "MS1022_mutex", //锁名称
   .attr_bits = osMutexPrioInherit,
   .cb_mem = NULL,
   .cb_size = 0, 
};

/**
* @brief MS1022 芯片注册
* 
* @param me   elab_spidev_ms1022_t ms1022设备类
* @param name 设备名称
* @param spi_name  基于的spi总线名称
* @param pin_name  中断引脚名称
* @param user_data 注册者数据
*/
void elab_ms1022_register(elab_spidev_ms1022_t * me,
                       const char *name, const char *spi_name,const char * pin_name)
{
   elab_assert(me != NULL);
   elab_assert(name != NULL);
   elab_assert(pin_name != NULL);
   elab_assert(spi_name != NULL);
   elab_assert(!elab_device_valid(name));
   elab_assert(elab_device_valid(spi_name));
   elab_assert(elab_device_valid(pin_name));

   me->spi = elab_device_find(spi_name);//找到已注册的spi总线 
   me->stop_INT_pin = elab_device_find(pin_name);//找到已注册的pin设备 (用于中断回调) 
   me->super.user_data=NULL;//无底层驱动数据 (基于spi总线)

   me->mutex = osMutexNew(&_mutex_attr_ms1022); //锁
   elab_assert(NULL != me->mutex);

   elab_device_attr_t attr_ms1022 =
   {
       .name = name,
       .sole = true,
       .type = ELAB_DEVICE_UNKNOWN,
   }; //设备属性 

   me->super.ops=&MS1022_ops;
   elab_device_register(&me->super, &attr_ms1022);//设备层注册
   elab_err_t ret=elab_ms1022_init(&me->super);
   assert(ret==ELAB_OK);
}



/**
 * @brief 读取32位数并分离整数和小数转为浮点数输出
 * 
 * @param data 
 * @return float 
 */
float convertToFloat(unsigned int data) {
    // 分离整数部分和小数部分
    unsigned int integerPart = (data >> 16) & 0xFFFF;
    unsigned int decimalPart = data & 0xFFFF;

    // 将整数部分和小数部分组合成浮点数
    float result = (float)integerPart + ((float)decimalPart / 65536.0); // 65536 = 2^16

    return result;
}


/**
* @brief ms1022命令发送
* 
* @param me elab_spidev_ms1022_t ms1022设备
* @param cmd 命令
* @param timeout 超时时间
* @return elab_err_t 
*/
static elab_err_t ms1022_send_cmd(elab_spidev_ms1022_t *const me,uint8_t cmd,uint32_t timeout)
{
   assert(me != NULL);
   elab_err_t ret=ELAB_OK;
   ret=elab_spi_send(me->spi, &cmd, 1, timeout);
   return ret;
}






/**
 * @brief combine_write_reg_buffer
 * 
 * @param reg     寄存器
 * @param buffer spi待写入缓冲区
 */
static void combine_write_reg_buffer(uint32_t reg,uint8_t *buffer)
{
int i=0;
for ( i=3;i>=0;i--)
{
buffer++;
(*buffer)=reg>>(8*i); //依次移位
}
}

                           
/**
 * @brief 
 * 
 * @param me 
 * @param cmd 
 * @param timeout 
 * @return elab_err_t 
 * @note spi 一次写入 命令 +数据 5个字节
 */
static elab_err_t ms1022_write_reg(elab_spidev_ms1022_t *const me,uint8_t cmd,uint32_t timeout)
{
   assert(me != NULL);
   elab_err_t ret=ELAB_ERR_EMPTY;
   uint8_t sendbuffer[5]; //ms1022写入 写入命令后需要跟着数据 写入完成后才能拉起cs片选
   sendbuffer[0]=cmd;

   switch (cmd)
   {
   case MS1022_REG0_ADDR:
   combine_write_reg_buffer(me->regs.cfg_regs.REG0.all,sendbuffer);
   break;
   case MS1022_REG1_ADDR:
   combine_write_reg_buffer(me->regs.cfg_regs.REG1.all,sendbuffer);
   break;
   case MS1022_REG2_ADDR:
   combine_write_reg_buffer(me->regs.cfg_regs.REG2.all,sendbuffer);
   break;
   case MS1022_REG3_ADDR:
   combine_write_reg_buffer(me->regs.cfg_regs.REG3.all,sendbuffer);
   break;
   case MS1022_REG4_ADDR:
   combine_write_reg_buffer(me->regs.cfg_regs.REG4.all,sendbuffer);
   break;
   case MS1022_REG5_ADDR:
   combine_write_reg_buffer(me->regs.cfg_regs.REG5.all,sendbuffer);
   break;
   case MS1022_REG6_ADDR:
   combine_write_reg_buffer(me->regs.cfg_regs.REG6.all,sendbuffer);
       break;
   default:
       break;
   }

//  for (int i = 0; i < sizeof(sendbuffer) / sizeof(sendbuffer[0]); ++i) {
//          elog_debug("%x",sendbuffer[i]); // %x表示以十六进制格式输出变量值
//     }

   ret=elab_spi_send(me->spi,sendbuffer,4,timeout);
   return ret;

}



/**
* @brief ms1022配置寄存器设置
* 
* @param me 
*/
static void ms1022_regs_set(elab_spidev_ms1022_t *const me)
{

   //默认值位
   me->regs.cfg_regs.REG1.bits.DEFAULT_VALUE=1;
   me->regs.cfg_regs.REG3.if_set_firstWave_bits.DEFAULT_VALUE=0;
   me->regs.cfg_regs.REG4.if_set_firstWave_bits.DEFAULT_VALUE1=0;
   me->regs.cfg_regs.REG4.if_set_firstWave_bits.DEFAULT_VALUE2=4; //手册默认值为2 实际是第三位为1 即4
   me->regs.cfg_regs.REG6.bits.DEFAULT_VALUE1=0;
   me->regs.cfg_regs.REG6.bits.DEFAULT_VALUE2=0;
   //无版本信息
   me->regs.cfg_regs.REG0.bits.ID0=0x00;   
   me->regs.cfg_regs.REG1.bits.ID1=0x00;
   me->regs.cfg_regs.REG2.bits.ID2=0x00;
   me->regs.cfg_regs.REG3.if_set_firstWave_bits.ID3=0x00;
   me->regs.cfg_regs.REG4.if_set_firstWave_bits.ID4=0x00;
   me->regs.cfg_regs.REG5.bits.ID5=0x00;
   me->regs.cfg_regs.REG6.bits.ID6=0x00;

   
/*测量模式2 芯片pdf1.8 page29*/
/*设置流程* 芯片pdf1.8 page30*/

   /*a.选择测量范围 2  page30*/
   me->regs.cfg_regs.REG0.bits.MESSB2=1;//选择测量范围 2

   /*b.选择参考时钟 page30*/
   me->regs.cfg_regs.REG0.bits.SEL_ECLK_TMP=0;//选择温度测量内部 cycle 时钟的参考信号0=32.768 kHz
   me->regs.cfg_regs.REG0.bits.START_CLKHS_LOW_2=0x01; 
   me->regs.cfg_regs.REG6.bits.START_CLKHS_HIGH_1=0;  //START_CLKHS=1 定义了在开启晶振后和测量开始前的晶振启动时间间隔.1 = 晶振一直开启
   me->regs.cfg_regs.REG0.bits.DIV_CLKHS=1; //设置 CLKHS 高速参考时钟的分频因数 1 = 2 分频   4/2=2Mhz
   me->regs.cfg_regs.REG0.bits.ANZ_PER_CALRES=0; //设置应用校准陶瓷晶振的 32k 时钟周期数 0 = 2 个周期= 61.035 μs
   me->regs.cfg_regs.REG0.bits.DIV_FIRE=9;//设置内部时钟信号产生脉冲的分频因数 9=8分频
   me->regs.cfg_regs.REG1.bits.CURR32K=0;//32kHz 晶振的低功耗选项。0 = 低电流 低功耗模式
   me->regs.cfg_regs.REG1.bits.EN_FAST_INIT=0; //关闭快速初始化功能

   /*c.设置所需脉冲数次数 page31*/
   //例如: 希望接受 2 stop 脉冲，则设置 HITIN1 = 3, HITIN2 = 0。 设定HTIN1脉冲总比希望接收脉冲多1
   //希望接收3个 stop脉冲
   me->regs.cfg_regs.REG1.bits.HITIN1=4;
   me->regs.cfg_regs.REG1.bits.HITIN2=0;

   /*d． 选择校准 page31 */
   //在测量范围 2 中必须进行校准这两个数据作为 Cal1 和Cal2 存储起来。有两种方法可用来更新校准值 Cal1 和 Cal2:
   //·通过 SPI 接口发送 Start_Cal_TDC 指令来单独校正
   //·通过设定寄存器 0 的 Bit12 在大多数应用中都会首选 NO_CAL_AUTO =0 自动校准。
   me->regs.cfg_regs.REG0.bits.CALLBARTE=1;//开启在 ALU 中的校准
   me->regs.cfg_regs.REG0.bits.NO_CAL_AUTO=0;//0 = 在测量后自动校准


   /*e． 定义 ALU 数据处理 page31*/
   //EN_AUTOCALC_MB2=1时 MS1022 将会自动计算所有的接收脉冲然后自动写入相应寄存
   //器.另外,还会计算出一个 3 个结果的和值. 然后将这个数据写入到结果寄存器 3 中。这将会
   //简化单片机和 MS1022 的通信,因为无需再从新写入寄存器 1 的计算符了。
   me->regs.cfg_regs.REG3.if_set_firstWave_bits.EN_AUTOCALC_MB2=1;//仅在测量范围 2 中: 自动计算所有开启获得的脉冲。而这些结果的和将会写入到结果寄存器3当中。
   //MRange1=HIT1 - HIT2   MRange1=HIT2-HIT1
   me->regs.cfg_regs.REG1.bits.HIT1=1;
   me->regs.cfg_regs.REG1.bits.HIT2=2; //MRange1=-1，MRange1=1

   //fire脉冲
   me->regs.cfg_regs.REG0.bits.ANZ_FIRE_LOW_3=0x0A;  
   me->regs.cfg_regs.REG6.bits.ANZ_FIRE_HIGH_3=0;//设置 fire 端口发射的脉冲个数 10=10个脉冲

   /*f． 选择输入触发方式 page32*/ 
   //当 RFEDGE = 0 时，NEG_X = 0 则上升沿触发，NEG_X = 1 则下降沿触发。

   //通过设置寄存器 2 的 Bit27&28（RFEDGE1&RFEDGE2）,用户可以选择 STOP 输入是上升沿或下降沿单独触发(RFEDGE = 0)还是上升沿和下降沿同时触发(RFEDGE = 1)。
   me->regs.cfg_regs.REG2.bits.RFEDGE1=0;//通道 1 的边沿敏感性 0 = 上升或下降沿
   me->regs.cfg_regs.REG2.bits.RFEDGE2=0;//通道 2 的边沿敏感性 0 = 上升或下降沿

   //用户可通过设置寄器 0的 Bit8-10（NEG_X）在每一个输入端口（Start，Stop1 和 Stop2）增加一内部反相器。
   me->regs.cfg_regs.REG0.bits.NEG_START=0;//反向 start 通道 1 输入 0 = 非反向输入信号 – 上升沿
   me->regs.cfg_regs.REG0.bits.NEG_STOP1=0;//反向 stop 通道 1 输入 0 = 非反向输入信号 – 上升沿
   me->regs.cfg_regs.REG0.bits.NEG_STOP2=0;//反向 stop 通道 1 输入 0 = 非反向输入信号 – 上升沿
   /*g． 中断*/

//中断引脚（PIN8，INTN）可以有不同的中断源，在寄存器 2 的 Bit29-31（EN_INT）中进行选择, 以及寄存器 6 Bit 21（EN_INT）。

   me->regs.cfg_regs.REG2.bits.EN_INT_LOW_3=0x05;//0x05   101
   me->regs.cfg_regs.REG6.bits.EN_INT_HIGH_1=0; // ALU 中断 或 Timeout 中断触发


   /*功能定义*/
   me->regs.cfg_regs.REG0.bits.ANZ_FAKE=0;//0 = 在温度测量前进行2 次热身伪测量
   me->regs.cfg_regs.REG0.bits.TCYCLE=1;//设置温度测量的循环时间0 = 128 μs @ 4 MHz
   me->regs.cfg_regs.REG0.bits.ANZ_PORT=0;// 设置应用温度测量的端口数 0 = 2 个温度测量端口(PT1 和 PT2)
   me->regs.cfg_regs.REG1.bits.SEL_TSTO1=0;//定义 FIRE_IN 管脚的功能 :0 = 与 MS1002 中同样功能, 声环法的Fire_in 输入
   me->regs.cfg_regs.REG1.bits.SEL_TSTO2=0;//定义 EN_START管脚的功能:0 = MS1002 功能相同， 高平将开启 START管脚。
   me->regs.cfg_regs.REG1.bits.SEL_START_FIRE=1;//fire 脉冲用作触发 TDC start.Start 输入将被关闭. 1 = 应用 fire 内部触发 Start
   
   //stop时间窗口 ，@4MHz时钟时
   me->regs.cfg_regs.REG2.bits.DELVAL1_DECIMAL=0;
   me->regs.cfg_regs.REG2.bits.DELVAL1=250; //使能 stop 脉冲而设置的250us延时，以 start 通道的第一个脉冲为起点开始计时。


   //回波第一波检测
   me->regs.cfg_regs.REG3.if_set_firstWave_bits.DELREL1=3;//设置第一波检测到第 1 个 stop 是接收第3个回波周期
   me->regs.cfg_regs.REG3.if_set_firstWave_bits.DELREL2=4; //设置第一波检测到第 2 个 stop 是接收第4个回波周期
   me->regs.cfg_regs.REG3.if_set_firstWave_bits.DELREL3=5; //设置第一波检测到第 3 个 stop 是接收第5个回波周期
   me->regs.cfg_regs.REG3.if_set_firstWave_bits.SEL_TIMO_MB2=2;//在测量范围 2 内为溢出选择时间限制 2=1024us
   me->regs.cfg_regs.REG3.if_set_firstWave_bits.EN_ERR_VAL=0;//关闭由于时间溢出强迫 ALU 写入0xFFFFFFFF 到结果寄存器
   me->regs.cfg_regs.REG3.if_set_firstWave_bits.EN_FIRST_WAVE=1;//开启自动第一波检测功能。

   
   // offset
   me->regs.cfg_regs.REG4.if_set_firstWave_bits.OFFS=15; //比较器 offset +15mV
   me->regs.cfg_regs.REG4.if_set_firstWave_bits.OFFSRNG1=0;
   me->regs.cfg_regs.REG4.if_set_firstWave_bits.OFFSRNG2=1; // offset +20mv
   me->regs.cfg_regs.REG4.if_set_firstWave_bits.EDGE_FW=0;  //设置第一波识别上升沿
   me->regs.cfg_regs.REG4.if_set_firstWave_bits.DIS_PW=0; //0 = 开启脉冲宽度测量

   me->regs.cfg_regs.REG5.bits.PHFIRE=0; //脉冲序列不反向
   me->regs.cfg_regs.REG5.bits.REPEAT_FIRE=0;//用于声环法的脉冲序列的重复次数 0: 不重复
   me->regs.cfg_regs.REG5.bits.DIS_PHASESHIFT=0;//当应用 MS1022 参考时钟产生 start 脉冲的时候(例如将fire 接到 start),请开启这个单元通过平均可以降低系统误差。0 = 开启相位噪声移位单元，
   me->regs.cfg_regs.REG5.bits.EN_STARTNOISE=1;//给 start 通道信号外加噪声
   me->regs.cfg_regs.REG5.bits.CONF_FIRE=0x02; //Bit 30 = 1: 关闭输出 FIRE_UP

   //模拟
   me->regs.cfg_regs.REG6.bits.TEMP_PORTDIR=0;//温度测量端口测量顺序反向0 = PT1 > PT2 > PT3 > PT4
   me->regs.cfg_regs.REG6.bits.DOUBLE_RES=0;//在测量范围 2 中将测量精度加倍 0 = 关闭 (MS1002 模式)
   me->regs.cfg_regs.REG6.bits.QUAD_RES=1;//在测量范围 2 中将测量精度提高 4 倍从 75 ps 到 19 ps 1 = 开启
   me->regs.cfg_regs.REG6.bits.FIREO_DEF=1;//定义非活动状态 fire 通道的默认水平。当应用内部集成的模拟部分时，必须设置为 1 (低)
   me->regs.cfg_regs.REG6.bits.HZ60=0;//两次测温延时 0 = 50 Hz 为基础, 20 ms
   me->regs.cfg_regs.REG6.bits.CYCLE_TOF=0;//选择触发第二次时间测量的定时器  0=0.5ms
   me->regs.cfg_regs.REG6.bits.CYCLE_TEMP=0;//选择触发第二次温度测量的定时器 0=0.5ms
   me->regs.cfg_regs.REG6.bits.TW2=3; //当应用内部模拟部分时,给所推荐的 RC 的电容进行充电的时间 3 = 300 µs
   me->regs.cfg_regs.REG6.bits.DA_KORR=0;//设置比较器 offset 
   me->regs.cfg_regs.REG6.bits.NEG_STOP_TEMP=1; //内部比较器使用史密斯触发器
   me->regs.cfg_regs.REG6.bits.EN_ANALOG=1;//1 = 模拟部分开启


}   

/**
* @brief ms1022配置寄存器写入
* 
* @param me 
*/
static elab_err_t ms1022_config_all_regs(elab_spidev_ms1022_t *const me)
{
   assert(me != NULL);
   elab_err_t ret=ELAB_ERR_EMPTY;
   ms1022_regs_set(me);
   ret=ms1022_write_reg(me,MS1022_REG0_ADDR,100);
   ret=ms1022_write_reg(me,MS1022_REG1_ADDR,100);
   ret=ms1022_write_reg(me,MS1022_REG2_ADDR,100);
   ret=ms1022_write_reg(me,MS1022_REG3_ADDR,100);
   ret=ms1022_write_reg(me,MS1022_REG4_ADDR,100);
   ret=ms1022_write_reg(me,MS1022_REG5_ADDR,100);
   ret=ms1022_write_reg(me,MS1022_REG6_ADDR,100);
   return ret;
}

/**
 * @brief 读取寄存器0
 * 
 * @param me 
 */
static void elab_ms1022_read_res0(elab_device_t *const me)
{
assert(me != NULL);
elab_spidev_ms1022_t *ms1022=(elab_spidev_ms1022_t*) me;

uint8_t cmd=MS1022_RES0_ADDR;
uint8_t buffer[5];
elab_spi_xfer(ms1022->spi,&cmd,&buffer,5,100);
ms1022->regs.read_regs.RES0=(uint32_t)buffer[1]<<24|(uint32_t)(buffer[2]<<16)|(uint32_t)(buffer[3]<<8)|(uint32_t)(buffer[4]);

}

/*******************************public functions***********************************/
/**
 * @brief ms1022测试通讯
 * @param me 
 * @return elab_err_t 
 */
elab_err_t  ms1022_test_communication(elab_device_t *const me)
{
   assert(me != NULL);
   elab_spidev_ms1022_t *ms1022=(elab_spidev_ms1022_t*) me;
   elab_err_t ret=ELAB_ERROR;
   uint8_t cmd;
   uint8_t test[2]; 
   ms1022->regs.cfg_regs.REG1.all=0x21444000;
   ms1022_write_reg(ms1022,MS1022_REG1_ADDR,100);
 
   cmd=MS1022_TEST_ADDR;
//ms1022读取需要写入命令后不置位片选继续虚写一个字节再读取，
//所以接收两个字节， 一个为空，后一个为读取的8位寄存器数
   elab_spi_xfer(ms1022->spi,&cmd,&test,2,100);


   if (test[1]==0x21)
   {
   return ret=ELAB_OK;
   }

   return ret;
}



/**
 * @brief ms1022时钟频率校准
 * 
 * @param me 
 */
float elab_ms1022_freq_corr_fact(elab_device_t *const me)
{
   assert(me != NULL);
   float CLKHS_freq_corr_fact;

   elab_spidev_ms1022_t *ms1022=(elab_spidev_ms1022_t*) me;
   ms1022_send_cmd(ms1022,MS1022CMD_Init,100);
	ms1022_send_cmd(ms1022, MS1022CMD_Start_Cal_Resonator,100);
   while(elab_pin_get_status(ms1022->stop_INT_pin)==true);

   elab_ms1022_read_res0(me);
   float temp=convertToFloat(ms1022->regs.read_regs.RES0);

   // for(int i=0 ;i<=4;i++)
   // {
   // elog_debug("buffer[%d]=%x",i,buffer[i]);
   // }

   // elog_debug("reg=%x",ms1022->regs.read_regs.RES0);
   // elog_debug("temp=%f",temp);
   CLKHS_freq_corr_fact = 61.03515625/temp*2;
   return CLKHS_freq_corr_fact;
}

float elab_ms1022_read_TOF(elab_device_t *const me)
{
   assert(me != NULL);
   elab_spidev_ms1022_t *ms1022=(elab_spidev_ms1022_t*) me;
   elab_ms1022_read_res0(me);
   float data=convertToFloat(ms1022->regs.read_regs.RES0);
   return data;
}

/**
 * @brief elab_ms1022_read_all_cfgregs 读取所有配置寄存器的配置
 * 
 * @param me 
 */
void elab_ms1022_read_all_cfgregs(elab_device_t *const me)
{
   assert(me != NULL);
   elab_spidev_ms1022_t *ms1022=(elab_spidev_ms1022_t*) me;
   elog_debug("reg0=%x",ms1022->regs.cfg_regs.REG0.all);
   elog_debug("reg1=%x",ms1022->regs.cfg_regs.REG1.all);
   elog_debug("reg2=%x",ms1022->regs.cfg_regs.REG2.all);
   elog_debug("reg3=%x",ms1022->regs.cfg_regs.REG3.all);
   elog_debug("reg4=%x",ms1022->regs.cfg_regs.REG4.all);
   elog_debug("reg5=%x",ms1022->regs.cfg_regs.REG5.all);
   elog_debug("reg6=%x",ms1022->regs.cfg_regs.REG6.all);
}


/**
 * @brief 等待ms1022_中断引脚传出信号
 * 
 * @param me 
 */
void elab_ms1022_wait_int(elab_device_t *const me)
{
   assert(me != NULL);
   elab_spidev_ms1022_t *ms1022=(elab_spidev_ms1022_t*) me;
   while(elab_pin_get_status(ms1022->stop_INT_pin)==true);
}

/**
 * @brief ms1022发送命令
 * 
 * @param me 
 * @param cmd 
 */
void elab_ms1022_send_cmd(elab_device_t *const me,uint8_t cmd)
{
   assert(me != NULL);
   elab_spidev_ms1022_t *ms1022=(elab_spidev_ms1022_t*) me;
   ms1022_send_cmd(ms1022,cmd,100);
}



uint16_t elab_ms1022_read_error_bit(elab_device_t *const me)
{
   uint8_t cmd;
   uint8_t buffer[3];
   assert(me != NULL);
   elab_spidev_ms1022_t *ms1022=(elab_spidev_ms1022_t*) me;
   cmd=MS1022_STAT_ADDR;
   uint16_t STAT_REG = 0x0000;
   elab_spi_xfer(ms1022->spi,&cmd,&buffer,3,100);
   for(int i=0 ;i<=2;i++)
   {
   elog_debug("buffer[%d]=%x",i,buffer[i]);
   }

   //elab_spi_xfer(ms1022->spi,&cmd,&ms1022->regs.read_regs.STAT,3,100);
}


/**
* @brief ms1022风向风速应用初始化
* 
* @param me 设备
*/
elab_err_t elab_ms1022_init(elab_device_t *const me)
{
   assert(me != NULL);
   elab_spidev_ms1022_t *ms1022=(elab_spidev_ms1022_t*) me;
   assert(ms1022->spi!=NULL);
   assert(ms1022->stop_INT_pin!=NULL);
   osStatus_t ret_os; 
   //上锁
   ret_os = osMutexAcquire(ms1022->mutex, osWaitForever);
   assert(ret_os == osOK);

   elab_spi_t *myspi=(elab_spi_t*)ms1022->spi;
   elab_device_t *cspin=myspi->pin_cs;

/*spi 片选操作*/
   elab_pin_set_status(cspin, true);
   osDelay(10);
   elab_pin_set_status(cspin, false);
   osDelay(10);
   elab_pin_set_status(cspin, true);

   //1.Power on Reset to ms1022
   elab_err_t ret=ms1022_send_cmd(ms1022,MS1022CMD_Power_On_Reset,100);
   osDelay(1); //500us wait for ms1022 power reset

   //2.test_communication
   ret=ms1022_test_communication(me);
   if(ret!=ELAB_OK)
   {
   goto exit;
   }

   //3. writing to fonfiguration registers
   ret=ms1022_config_all_regs(ms1022);

   //4.sending init
   ret=ms1022_send_cmd(ms1022,MS1022CMD_Init,100);

   elab_ms1022_freq_corr_fact(me);

exit:
   ret_os = osMutexRelease(ms1022->mutex);
   assert(ret_os == osOK);
   (void)ret_os;
   return ret;
}

#ifdef __cplusplus
}

#endif







