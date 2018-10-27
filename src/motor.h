#pragma once
#ifndef _MOTOR_H_
#define _MOTOR_H_

#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

enum DacPort : uint16_t
{
    Port11 = 0x0000,
    Port12 = 0x0100,
    Port21 = 0x0200,
    Port22 = 0x0300,
};

enum Polarity : uint8_t
{
    Positive = 1,
    Negative = 0
};

struct Amplitude
{
    Polarity polarity;
    uint8_t value;
};

constexpr Amplitude QSin(int32_t value);

class ExDac
{
  public:
    static void Init();
    static inline void Output(DacPort port, uint8_t value, Polarity polarity)
    {
        GPIO_Write(GPIOE, (GPIO_ReadOutputData(GPIOE) & 0xFCFF) | port);
        GPIO_ResetBits(GPIOE, GPIO_Pin_10);
        GPIO_Write(GPIOE, (GPIO_ReadOutputData(GPIOE) & 0xFF00) | value);
        GPIO_SetBits(GPIOE, GPIO_Pin_10);
        GPIO_WriteBit(GPIOE, 0x0800 << (port >> 8), (BitAction)polarity);
    }
};

class Motor
{
  public:
    static void Init(DacPort a, DacPort b);
    static void Start();
    static void Stop();
    static void Step();
    static uint32_t InvTime;
    static uint32_t StepCount;
    static int8_t Direction;
    static inline bool IsRunning()
    {
        return running;
    }

  private:
    static uint32_t phase;
    static bool running;
    static DacPort pa, pb;
};

#endif