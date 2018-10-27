#include "key.h"

KeyEventHandler Key::OnKeyEvent;

void Key::Init()
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    GPIO_InitTypeDef GPIOInitStruct{
        GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5,
        GPIO_Speed_50MHz,
        GPIO_Mode_IPU};
    GPIO_Init(GPIOD, &GPIOInitStruct);
    Timer::OnTick += TickHandler(Scan, 10);
}

void Key::Scan()
{
    if (OnKeyEvent)
    {
        static uint8_t preData = (GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5) >> 3;
        uint8_t data = (GPIO_ReadInputData(GPIOD) & (GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5)) >> 3;
        uint8_t e;
        e = preData & (~data);
        if (e != 0)
            OnKeyEvent(e, KeyDown);
        e = (~preData) & data;
        if (e != 0)
            OnKeyEvent(e, KeyUp);
        preData = data;
    }
}