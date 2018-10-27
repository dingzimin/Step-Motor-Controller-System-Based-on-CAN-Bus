#include "led.h"

void Led::Init()
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE, ENABLE);
    GPIO_InitTypeDef GPIOInit;
    GPIOInit = {
        GPIO_Pin_2,
        GPIO_Speed_50MHz,
        GPIO_Mode_Out_PP};
    GPIO_Init(GPIOD, &GPIOInit);
    GPIOInit = {
        GPIO_Pin_15,
        GPIO_Speed_50MHz,
        GPIO_Mode_Out_PP};
    GPIO_Init(GPIOE, &GPIOInit);
}