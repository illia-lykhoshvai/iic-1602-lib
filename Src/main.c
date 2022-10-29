#include "stm32f4xx.h"

int main(void) {
    // startup
    unsigned long i;
    RCC->AHB1ENR |= (1 << 2); // GPIOC en
    GPIOC->MODER |= 1 << (13*2);
    GPIOC->OTYPER |= ( 1 << 13 ); // PC13-LED
    while(1) {
        GPIOC->ODR ^= (1 << 13);
        for (i = 0;i < 1600000;i++)
            asm("nop");
    }
}
