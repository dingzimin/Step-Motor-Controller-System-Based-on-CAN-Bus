#include "timer.h"

std::vector<TickHandler> Timer::OnTick;
uint32_t Timer::tick = 0;

void Timer::Init()
{
    SysTick_Config(72000);
    OnTick.reserve(4);
    OnTick += [&]() {
        tick++;
    };
}

extern "C"
{
    void SysTick_Handler()
    {
        for (auto &e : Timer::OnTick)
        {
            if (e.handler)
            {
                e.count--;
                if (e.count == 0)
                {
                    e.handler();
                    e.count = e.time;
                }
            }
        }
    }
}