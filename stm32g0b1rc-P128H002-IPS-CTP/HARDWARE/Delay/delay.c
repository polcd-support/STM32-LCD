
#include "delay.h"


void delay(uint32_t ms){
	delay_ms(ms);
}


void delay_ms(uint32_t ms) {
    uint32_t start = HAL_GetTick(); // ֱ�ӻ�ȡ����ʱ���
    uint32_t elapsed = 0;
    while (elapsed < ms) {
        uint32_t now = HAL_GetTick();
        elapsed = now - start; // ���㾭���ĺ�����
    }
}

static void delay_1us(){
	for(uint8_t i = 0;i<16;i++){
		__nop();__nop();__nop();__nop();
	}
}

void delay_us(uint32_t us){
    for(uint32_t i=0;i<us;i++){
		delay_1us();
	}
}
