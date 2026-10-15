#include "delay.h"

/*
 * delay_us() is implemented with the Cortex-M3 DWT cycle counter instead of a
 * NOP loop: the timing is exact for any SystemCoreClock (72 MHz on STM32F103)
 * and does not have to be re-tuned when the clock configuration changes.
 */
static void delay_dwt_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; /* enable trace/DWT unit */
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;            /* start cycle counter */
}

void delay(uint32_t ms)
{
    delay_ms(ms);
}

void delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

void delay_us(uint32_t us)
{
    uint32_t start;
    uint32_t ticks;

    if (us == 0U)
    {
        return;
    }

    if ((DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) == 0U)
    {
        delay_dwt_init();
    }

    ticks = us * (SystemCoreClock / 1000000U); /* 1us = SystemCoreClock/1e6 cycles */
    start = DWT->CYCCNT;
    while ((DWT->CYCCNT - start) < ticks)
    {
    }
}
