
#include "delay.h"


void delay(uint32_t ms){
	delay_ms(ms);
}


void delay_ms(uint32_t ms) {
    uint32_t start = HAL_GetTick(); // 直接获取毫秒时间戳
    uint32_t elapsed = 0;
    while (elapsed < ms) {
        uint32_t now = HAL_GetTick();
        elapsed = now - start; // 计算经过的毫秒数
    }
}

void delay_us(uint32_t us){
    uint32_t start = HAL_GetTick() * 1000; // 转换为微秒
    uint32_t elapsed = 0;
    while (elapsed < us) {
        uint32_t now = HAL_GetTick() * 1000;
        elapsed = now - start;
    }
}
