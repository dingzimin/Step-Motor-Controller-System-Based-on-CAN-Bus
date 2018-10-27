#pragma once
#ifndef _LED_H_
#define _LED_H_

#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

class Led
{
  public:
    static void Init();
    static inline void Red(BitAction action)
    {
        GPIO_WriteBit(GPIOE, GPIO_Pin_15, action);
    }
    static inline void Green(BitAction action)
    {
        GPIO_WriteBit(GPIOD, GPIO_Pin_2, action);
    }
};

#endif