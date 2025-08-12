/**
 ****************************************************************************************************
 * @file        usart2.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-03-24
 * @brief       ����2 ��������
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

#include "./BSP/USART2/usart2.h"


/**
 * @brief       ����2��ʼ������
 * @param       sclk: ����X��ʱ��ԴƵ��(��λ: MHz)
 *              ����1 �� ����6 ��ʱ��Դ����: rcc_pclk2 = 100Mhz
 *              ����2 - 5 / 7 / 8 ��ʱ��Դ����: rcc_pclk1 = 100Mhz
 * @note        ע��: ����������ȷ��sclk, ���򴮿ڲ����ʾͻ������쳣.
 * @param       baudrate: ������, �����Լ���Ҫ���ò�����ֵ
 * @retval      ��
 */
void usart2_init(uint32_t sclk, uint32_t baudrate)
{
    uint32_t temp = 0;

    USART2_TX_GPIO_CLK_ENABLE();    /* ʹ�ܴ���TX��ʱ�� */
    USART2_RX_GPIO_CLK_ENABLE();    /* ʹ�ܴ���RX��ʱ�� */
    USART2_UX_CLK_ENABLE();         /* ʹ�ܴ���ʱ�� */

    sys_gpio_set(USART2_TX_GPIO_PORT, USART2_TX_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);/* ����TX�� ģʽ���� */

    sys_gpio_set(USART2_RX_GPIO_PORT, USART2_RX_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);/* ����RX�� ģʽ���� */

    sys_gpio_af_set(USART2_TX_GPIO_PORT, USART2_TX_GPIO_PIN, USART2_TX_GPIO_AF);    /* TX�� ���ù���ѡ��, ����������ȷ */
    sys_gpio_af_set(USART2_RX_GPIO_PORT, USART2_RX_GPIO_PIN, USART2_RX_GPIO_AF);    /* RX�� ���ù���ѡ��, ����������ȷ */

    temp = (sclk * 1000000 + baudrate / 2) / baudrate;              /* �õ�USARTDIV@OVER8 = 0, ��������������� */
    /* ���������� */
    USART2_UX->BRR = temp;      /* ����������@OVER8 = 0 */
    USART2_UX->CR1 = 0;         /* ����CR1�Ĵ��� */
    USART2_UX->CR1 |= 0 << 28;  /* ����M1 = 0 */
    USART2_UX->CR1 |= 0 << 12;  /* ����M0 = 0 & M1 = 0, ѡ��8λ�ֳ� */
    USART2_UX->CR1 |= 0 << 15;  /* ����OVER8 = 0, 16�������� */
    USART2_UX->CR1 |= 1 << 3;   /* ���ڷ���ʹ�� */

    USART2_UX->CR1 |= 1 << 0;   /* ����ʹ�� */
}






