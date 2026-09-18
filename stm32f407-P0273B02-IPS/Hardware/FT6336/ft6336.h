// ft6336.h
#ifndef __FT6336_H
#define __FT6336_H

#include "main.h"
#include "stdint.h"
#include "iic_hal.h"
#include "delay.h"

/* FT6236 dev pin definitions */
#define TOUCH_RST_PIN    GPIO_PIN_13    // Reset pin
#define TOUCH_INT_PIN    GPIO_PIN_1    // Interrupt pin
#define TOUCH_RST_PORT   GPIOC
#define TOUCH_INT_PORT   GPIOB

/* Function macros */
#define TOUCH_RST_0      HAL_GPIO_WritePin(TOUCH_RST_PORT, TOUCH_RST_PIN, GPIO_PIN_RESET)
#define TOUCH_RST_1      HAL_GPIO_WritePin(TOUCH_RST_PORT, TOUCH_RST_PIN, GPIO_PIN_SET)

/* Device addresses */
#define FT6236_ADDR      0x38          // Default I2C address
#define FT6236_ADDR_FL   0x39          // Alternative address when ADDR pin is low

/* FT6236 register addresses */
#define FT6236_REG_MODE         0x00   // Device mode (normal, test, etc.)
#define FT6236_REG_TD_STATUS    0x02   // Touch status
#define FT6236_REG_TOUCH1_XH    0x03   // Touch 1 X high byte
#define FT6236_REG_TOUCH1_XL    0x04   // Touch 1 X low byte
#define FT6236_REG_TOUCH1_YH    0x05   // Touch 1 Y high byte
#define FT6236_REG_TOUCH1_YL    0x06   // Touch 1 Y low byte
#define FT6236_REG_TOUCH1_WEIGHT 0x07  // Touch 1 weight
#define FT6236_REG_TOUCH1_MISC  0x08   // Touch 1 misc info
#define FT6236_REG_TOUCH2_XH    0x09   // Touch 2 X high byte
#define FT6236_REG_TOUCH2_XL    0x0A   // Touch 2 X low byte
#define FT6236_REG_TOUCH2_YH    0x0B   // Touch 2 Y high byte
#define FT6236_REG_TOUCH2_YL    0x0C   // Touch 2 Y low byte
#define FT6236_REG_TOUCH2_WEIGHT 0x0D  // Touch 2 weight
#define FT6236_REG_TOUCH2_MISC  0x0E   // Touch 2 misc info
#define FT6236_REG_THRESHOLD    0x80   // Touch threshold
#define FT6236_REG_FILTER_COE   0x85   // Filter coefficient
#define FT6236_REG_CTRL         0x86   // Control register
#define FT6236_REG_TIMEENTERMONITOR 0x87 // Time to enter monitor mode
#define FT6236_REG_PERIODACTIVE 0x88   // Report rate in active mode
#define FT6236_REG_PERIODMONITOR 0x89  // Report rate in monitor mode
#define FT6236_REG_RADIAN_VALUE 0x91   // Radian value
#define FT6236_REG_OFFSET_LEFT_RIGHT 0x92 // Left/right offset
#define FT6236_REG_OFFSET_UP_DOWN 0x93 // Up/down offset
#define FT6236_REG_DISTANCE_LEFT_RIGHT 0x94 // Left/right distance
#define FT6236_REG_DISTANCE_UP_DOWN 0x95 // Up/down distance
#define FT6236_REG_DISTANCE_ZOOM   0x96 // Zoom distance
#define FT6236_REG_LIB_VERSION_H   0xA1 // Library version high byte
#define FT6236_REG_LIB_VERSION_L   0xA2 // Library version low byte
#define FT6236_REG_CHIP_ID         0xA3 // Chip ID
#define FT6236_REG_G_MODE          0xA4 // Interrupt mode
#define FT6236_REG_POWER_MODE      0xA5 // Power mode
#define FT6236_REG_FIRMARE_ID      0xA6 // Firmware ID
#define FT6236_REG_FOCALTECH_ID    0xA8 // FocalTech's vendor ID
#define FT6236_REG_RELEASE_CODE_ID 0xAF // Release code version

/* Touch coordinate structure */
typedef struct {
    uint16_t X_Pos;
    uint16_t Y_Pos;
    uint8_t Touch_Count;
    uint8_t Touch_Event;
} FT6236_Info;

/* Gesture IDs */
typedef enum {
    FT6236_GESTURE_NO_GESTURE     = 0x00,
    FT6236_GESTURE_MOVE_UP        = 0x10,
    FT6236_GESTURE_MOVE_LEFT      = 0x14,
    FT6236_GESTURE_MOVE_DOWN      = 0x18,
    FT6236_GESTURE_MOVE_RIGHT     = 0x1C,
    FT6236_GESTURE_ZOOM_IN        = 0x48,
    FT6236_GESTURE_ZOOM_OUT       = 0x49,
} FT6236_GestureID_TypeDef;

/* Touch event types */
typedef enum {
    FT6236_TOUCH_EVENT_DOWN       = 0x00,
    FT6236_TOUCH_EVENT_UP         = 0x01,
    FT6236_TOUCH_EVENT_CONTACT    = 0x02,
    FT6236_TOUCH_EVENT_NONE       = 0x03,
} FT6236_TouchEvent_TypeDef;

/* Power modes */
typedef enum {
    FT6236_POWER_MODE_ACTIVE      = 0x00,
    FT6236_POWER_MODE_MONITOR     = 0x01,
    FT6236_POWER_MODE_HIBERNATE   = 0x03,
} FT6236_PowerMode_TypeDef;

/* Interrupt modes */
typedef enum {
    FT6236_INT_MODE_POLLING       = 0x00,
    FT6236_INT_MODE_TRIGGER       = 0x01,
    FT6236_INT_MODE_CONTINUOUS    = 0x02,
} FT6236_IntMode_TypeDef;

extern FT6236_Info FT6236_Instance;

/* Initialization functions */
void FT6236_GPIO_Init(void);
void FT6236_RESET(void);
void FT6236_Init(void);

/* Touch operations */
void FT6236_Get_Touch_Data(void);
uint8_t FT6236_Get_Touch_Count(void);
uint8_t FT6236_Get_ChipID(void);
uint8_t FT6236_Get_FocaltechID (void);
uint16_t FT6236_Get_LibVersion(void);

/* I2C read/write functions */
void FT6236_IIC_WriteREG(uint8_t addr, uint8_t dat);
uint8_t FT6236_IIC_ReadREG(uint8_t addr);

/* Configuration functions */
void FT6236_Set_Threshold(uint8_t threshold);
void FT6236_Set_PowerMode(FT6236_PowerMode_TypeDef mode);
void FT6236_Set_InterruptMode(FT6236_IntMode_TypeDef mode);
void FT6236_Set_ActiveRate(uint8_t rate);
void FT6236_Set_MonitorRate(uint8_t rate);
void FT6236_Wakeup(void);
void FT6236_Sleep(void);

#endif

