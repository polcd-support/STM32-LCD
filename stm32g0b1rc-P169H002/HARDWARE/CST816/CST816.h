#ifndef __CST816_H
#define __CST816_H
 
#include "stdint.h"
#include "main.h"
 
#if IICSOFT
#include "iic_hal.h"
#else
#include "iic_hal_hard.h"
#endif
 
#include "delay.h"
 
 
 
 
/* CST816 dev pin defination */
//PB8 PB9 两个按钮 
 
 
#define TOUCH_RST_PIN GPIO_PIN_3 //复位
#define TOUCH_INT_PIN GPIO_PIN_8 //中断
#define TOUCH_RST_PORT GPIOD
#define TOUCH_INT_PORT GPIOC
 
/* functions define */ 
 
#define TOUCH_RST_0 HAL_GPIO_WritePin(TOUCH_RST_PORT, TOUCH_RST_PIN, GPIO_PIN_RESET)
#define TOUCH_RST_1 HAL_GPIO_WritePin(TOUCH_RST_PORT, TOUCH_RST_PIN, GPIO_PIN_SET)
 
 
/* 设备地址 */
#define Device_Addr 	0x15
 
 
/* 触摸屏寄存器 */
#define GestureID 			0x01 //触摸屏中与手势识别
#define FingerNum 			0x02 //记录触摸手指数量
#define XposH 					0x03 //X 坐标高位相关的寄存器地址
#define XposL 					0x04 //X 坐标低位部分的寄存器地址
#define YposH 					0x05 //Y 坐标高位相关的寄存器地址
#define YposL 					0x06 //Y 坐标低位部分的寄存器地址
#define ChipID 					0xA7 //访问触摸屏芯片的唯一标识符寄存器的地址
#define SleepMode				0xE5 //控制触摸屏进入或退出睡眠模式的寄存器地址
#define MotionMask 			0xEC //对某些运动相关操作的屏蔽或启用设置
#define IrqPluseWidth 	0xED //中断低脉冲宽度相关的寄存器地址
#define NorScanPer 			0xEE //触摸屏正常扫描周期相关的寄存器地址
#define MotionSlAngle 	0xEF //涉及到运动滑动角度相关
#define LpAutoWakeTime 	0xF4 //长按自动唤醒时间
#define LpScanTH 				0xF5 //触摸屏的长按扫描阈值
#define LpScanWin 			0xF6 //触摸屏的长按扫描窗口相关
#define LpScanFreq 			0xF7 //触摸屏的长按扫描频率
#define LpScanIdac 			0xF8 //长按扫描电流
#define AutoSleepTime 	0xF9 //触摸屏自动进入睡眠模式的时间相关
#define IrqCtl 					0xFA //触摸屏的中断控制相关
#define AutoReset 			0xFB //触摸屏自动复位相关
#define LongPressTime 	0xFC //触摸屏的长按时间
#define IOCtl 					0xFD //触摸屏的输入输出控制
#define DisAutoSleep 		0xFE //禁止触摸屏自动进入睡眠模式
 
 
/* 触摸屏坐标结构体 */
typedef struct
{
	unsigned int X_Pos;
	unsigned int Y_Pos;
} CST816_Info; //坐标控制器
 
/* 手势ID识别选项*/
typedef enum
{
	NOGESTURE = 	0x00,//无操作
	DOWNGLIDE = 	0x01,//下滑
	UPGLIDE = 		0x02,//上滑
	LEFTGLIDE = 	0x03,//左滑
	RIGHTGLIDE = 	0x04,//右滑 
	CLICK = 			0x05, //点击
	DOUBLECLICK = 0x0B, //双击
	LONGPRESS = 	0x0C, //长按
} GestureID_TypeDef;
 
/* 连续动作配置选项 */
typedef enum
{
	M_DISABLE = 	0x00,// 不需要任何连续动作
	EnConLR = 		0x01,// 左右滑动
	EnConUD = 		0x02,//上下滑动
	EnDClick = 		0x03, //双击
	M_ALLENABLE = 0x07, //启用所有连续动作配置
} MotionMask_TypeDef;
 
/* 中断低脉冲发射方式选项 */
typedef enum
{
	OnceWLP = 		0x00,//单词脉冲发射
	EnMotion = 		0x10,//运动变化
	EnChange = 		0x20,//检测某些特定变化
	EnTouch = 		0x40,//触摸事件
	EnTest = 			0x80,//检测中断是否正常
} IrqCtl_TypeDef;


extern CST816_Info CST816_Instance;

 
/* 触摸屏初始化相关函数 */
void CST816_GPIO_Init(void);
void CST816_RESET(void);
void CST816_Init(void);
 
/* 触摸屏操作函数 */
void CST816_Get_XY_AXIS(void);
uint8_t CST816_Get_ChipID(void);
uint8_t CST816_Get_FingerNum(void);
/* 触摸屏读写函数 */
void CST816_IIC_WriteREG(uint8_t addr, uint8_t dat);
uint8_t CST816_IIC_ReadREG(unsigned char addr);
 
/* 触摸屏有关参数配置函数 */
void CST816_Config_MotionMask(uint8_t mode);
void CST816_Config_AutoSleepTime(uint8_t time);
void CST816_Config_MotionSlAngle(uint8_t x_right_y_up_angle);
void CST816_Config_NorScanPer(uint8_t Period);
void CST816_Config_IrqPluseWidth(uint8_t Width);
void CST816_Config_LpScanTH(uint8_t TH);
void CST816_Wakeup(void);
void CST816_Sleep(void);
 
 
 
 
#endif


