#include <functional>
#include <vector>
#include "stm32f10x_rcc.h"
#include "bitband.h"
#include "can.h"
#include "key.h"
#include "led.h"
#include "motor.h"
#include "ptl.h"
#include "timer.h"

#define MOTOR
#define LED
#define MASTER
#define SLAVE

#if defined(MOTOR) // 步进电机
struct MotorData
{
    uint16_t time;
    uint16_t count;
    int8_t direct;
};

#if defined(MASTER)
constexpr MotorData mData[] = {
    {2000, 36000, 1},
    {5000, 3200, -1},
    {2000, 5400, 1},
    {8000, 7600, 1},
    {1000, 40000, -1},
    {8000, 1000, 1},
    {3000, 25000, 1},
    {2000, 21000, -1},
    {9300, 10000, -1},
    {2000, 1300, -1},
    {10000, 62000, 1},
    {6000, 32200, 1},
    {10000, 36000, -1},
    {2000, 8000, -1},
    {7000, 3000, 1},
    {500, 64000, -1},
};
#endif
#endif

#if defined(LED) // LED 交通灯
constexpr uint8_t AllOff = 0x00;
constexpr uint8_t RedOn = 0x01;
constexpr uint8_t GreenOn = 0x02;
constexpr uint8_t AllOn = RedOn | GreenOn;

struct LedData
{
    uint16_t time;
    uint8_t action;
};

#if defined(MASTER)
constexpr LedData lData[8] = {
    {5000, AllOn},
    {10000, RedOn},
    {15000, GreenOn},
    {20000, AllOff},
    {5000, RedOn},
    {10000, AllOn},
    {15000, GreenOn},
    {20000, AllOff},
};
#endif
#endif

// 异步任务
std::function<void()> Task;
bool hasTask = false;
bool Async(const std::function<void()> &task);

int main()
{
    Timer::Init();
    Led::Init();
    Key::Init();
    Can::Init();
    Motor::Init(Port21, Port22);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);
    CanFilter filter(Fifo0);
    filter.IdMask32(Filter32(0, 0, 0, 0), Filter32(0, 0, 0, 0));
    Can::AddFilter(filter);
#if defined(MASTER)
    // 按键事件，异步发送步进电机、LED 数据
    Key::OnKeyEvent = [&](uint8_t id, KeyEvent event) {
        if (event == KeyDown)
        {
#if defined(LED)
            if (id & 0x01)
                Async([&]() {
                    Ptl::SendData(1, (uint8_t *)lData, sizeof(lData));
                });
#endif
#if defined(MOTOR)
            if (id & 0x02)
                Async([&]() {
                    Ptl::SendData(2, (uint8_t *)mData, sizeof(mData));
                });
#endif
        }
    };
#endif

#if defined(SLAVE)
#if defined(LED)
    LedData ledData[8];
    bool light = false;
#endif
#if defined(MOTOR)
    MotorData motorData[16];
    bool motor = false;
#endif
    // 准备缓冲区，开始接收数据
    Ptl::SetStreamStartHandler([&](uint32_t id, uint8_t size, uint8_t seq, uint32_t crc) {
        switch (id)
        {
#if defined(LED)
        case 1:
            Async([&]() {
                if (Ptl::ReceiveData(id, (uint8_t *)ledData, size, seq, crc))
                    light = true;
            });
#endif
#if defined(MOTOR)
        case 2:
            Async([&]() {
                if (Ptl::ReceiveData(id, (uint8_t *)motorData, size, seq, crc))
                    motor = true;
            });
#endif
        }
    });

#if defined(LED)
    Timer::OnTick += TickHandler([&]() {
        static uint8_t ledIndex = 0;
        if (light)
        {
            Led::Red((BitAction)(ledData[ledIndex].action & RedOn));
            Led::Green((BitAction)(ledData[ledIndex].action & GreenOn));
            ledData[ledIndex].time--;
            if (ledData[ledIndex].time == 0)
                ledIndex++;
            if (ledIndex == 8)
            {
                Led::Red((BitAction)0);
                Led::Green((BitAction)0);
                light = false;
                ledIndex = 0;
            }
        }
    }, 1);
#endif
#endif
    while (true)
    {
        if (hasTask)
        {
            Task();
            hasTask = false;
        }
#if defined(MOTOR) && defined(SLAVE)
        if (motor)
        {
            for (auto& data : motorData)
            {
                while (Motor::IsRunning())
                    ;
                Motor::InvTime = data.time;
                Motor::Direction = data.direct;
                Motor::StepCount = data.count;
                Motor::Start();
            }
            motor = false;
        }
#endif
    }
}

bool Async(const std::function<void()> &task)
{
    if (hasTask)
        return false;
    Task = std::move(task);
    hasTask = true;
    return true;
}
