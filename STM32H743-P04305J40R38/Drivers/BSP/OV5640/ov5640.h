/**
 ****************************************************************************************************
 * @file        ov5640.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-24
 * @brief       OV5640 ��������
 * @license     Copyright (c) 2022-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� STM32H743������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20230324
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#ifndef _OV5640_H
#define _OV5640_H

#include "./SYSTEM/sys/sys.h"
#include "./BSP/OV5640/sccb.h"

/******************************************************************************************/
/* RESET ���� ����, PWDN��PCF8574T���� */

#define OV_RESET_GPIO_PORT              GPIOA
#define OV_RESET_GPIO_PIN               SYS_GPIO_PIN15
#define OV_RESET_GPIO_CLK_ENABLE()      do{ RCC->AHB4ENR |= 1 << 0; }while(0)   /* PA��ʱ��ʹ�� */

/******************************************************************************************/

/* IO���ƺ��� */
#define OV5640_RST(x)       sys_gpio_pin_set(OV_RESET_GPIO_PORT, OV_RESET_GPIO_PIN, x)  /* ��λ�����ź� */

/* OV5640��ID�ͷ��ʵ�ַ */
#define OV5640_ID           0X5640      /* OV5640��оƬID */
#define OV5640_ADDR         0X78        /* OV5640��IIC��ַ */
 
/* OV5640��ؼĴ������� */
#define OV5640_CHIPIDH      0X300A      /* OV5640оƬID���ֽ� */
#define OV5640_CHIPIDL      0X300B      /* OV5640оƬID���ֽ� */
 

/* ����ӿں��� */
uint8_t ov5640_read_reg(uint16_t reg);
uint8_t ov5640_write_reg(uint16_t reg,uint8_t data);
void ov5640_pwdn_set(uint8_t sta);

uint8_t ov5640_init(void);  
void ov5640_jpeg_mode(void);
void ov5640_rgb565_mode(void);

uint8_t ov5640_focus_init(void);
uint8_t ov5640_focus_single(void);
uint8_t ov5640_focus_constant(void);

void ov5640_flash_ctrl(uint8_t sw);
void ov5640_test_pattern(uint8_t mode);

void ov5640_sharpness(uint8_t sharp);
void ov5640_brightness(uint8_t bright);
void ov5640_contrast(uint8_t contrast);
void ov5640_exposure(uint8_t exposure);
void ov5640_light_mode(uint8_t mode);
void ov5640_special_effects(uint8_t eft);
void ov5640_color_saturation(uint8_t sat);

uint8_t ov5640_outsize_set(uint16_t offx,uint16_t offy,uint16_t width,uint16_t height);
uint8_t ov5640_image_window_set(uint16_t offx,uint16_t offy,uint16_t width,uint16_t height); 

#endif





















