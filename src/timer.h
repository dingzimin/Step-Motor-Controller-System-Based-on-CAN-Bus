#pragma once
#ifndef _TIMER_H_
#define _TIMER_H_

#include <functional>
#include <vector>
#include "stm32f10x.h"
#include "stm32f10x_it.h"
#include "stm32f10x_tim.h"

class Timer;

class TickHandler
{
  public:
    TickHandler(const std::function<void()> &f, uint16_t t)
    {
        handler = std::move(f);
        time = t;
        count = t;
    }
    std::function<void()> handler;
    uint16_t time;
    uint16_t count;
};

inline void operator+=(std::vector<TickHandler> &a, const TickHandler &b)
{
    a.push_back(std::move(b));
}

inline void operator+=(std::vector<TickHandler> &a, const std::function<void()> &b)
{
    a.push_back(TickHandler(b, 1));
}

class Timer
{
  public:
    static void Init();
    static inline uint32_t GetTick()
    {
        return tick;
    }
    static std::vector<TickHandler> OnTick;

  private:
    static uint32_t tick;
};

#endif