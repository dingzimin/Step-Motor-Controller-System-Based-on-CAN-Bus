#include "motor.h"

void ExDac::Init()
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
    GPIO_InitTypeDef GPIOInit{
        GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 |
            GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14,
        GPIO_Speed_50MHz,
        GPIO_Mode_Out_PP};
    GPIO_Init(GPIOE, &GPIOInit);
}

uint32_t Motor::InvTime = 0;
uint32_t Motor::StepCount = 0;
uint32_t Motor::phase = 0;
int8_t Motor::Direction = 1;
bool Motor::running = false;
DacPort Motor::pa, Motor::pb;

void Motor::Init(DacPort a, DacPort b)
{
    ExDac::Init();
    Stop();

    pa = a;
    pb = b;
}

void Motor::Start()
{
    if (StepCount == 0)
        return;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_TimeBaseInitTypeDef TimInit{
        (uint16_t)(InvTime >> 16),
        TIM_CounterMode_Up,
        (uint16_t)(InvTime & 0xFFFF),
        TIM_CKD_DIV1,
        0};
    TIM_TimeBaseInit(TIM2, &TimInit);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitTypeDef NVICInit{
        TIM2_IRQn,
        0, 1, ENABLE};
    NVIC_Init(&NVICInit);
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
    phase %= 720;
    running = true;
}

void Motor::Stop()
{
    TIM_Cmd(TIM2, DISABLE);
    ExDac::Output(pa, 0, Positive);
    ExDac::Output(pb, 0, Positive);
    running = false;
}

void Motor::Step()
{
    Amplitude a = QSin(phase);
    Amplitude b = QSin(phase - 180); // * Direction);
    if (Direction == 1)
    {
        ExDac::Output(pa, a.value, a.polarity);
        ExDac::Output(pb, b.value, b.polarity);
    }
    else if (Direction == -1)
    {
        ExDac::Output(pb, a.value, a.polarity);
        ExDac::Output(pa, b.value, b.polarity);
    }
    phase++;
    StepCount--;
    if (StepCount == 0)
    {
        Motor::Stop();
    }
}

extern "C"
{
    void TIM2_IRQHandler()
    {
        if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
        {
            Motor::Step();
            TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        }
    }
}

constexpr uint8_t triValue[181] = {
    0, 2, 4, 7, 9, 11, 13, 16, 18, 20, 22, 24, 27, 29, 31, 33,
    35, 38, 40, 42, 44, 46, 49, 51, 53, 55, 57, 60, 62, 64, 66, 68,
    70, 72, 75, 77, 79, 81, 83, 85, 87, 89, 91, 93, 96, 98, 100, 102,
    104, 106, 108, 110, 112, 114, 116, 118, 120, 122, 124, 126, 128, 129, 131, 133,
    135, 137, 139, 141, 143, 144, 146, 148, 150, 152, 153, 155, 157, 159, 160, 162,
    164, 166, 167, 169, 171, 172, 174, 176, 177, 179, 180, 182, 183, 185, 186, 188,
    190, 191, 192, 194, 195, 197, 198, 200, 201, 202, 204, 205, 206, 208, 209, 210,
    211, 213, 214, 215, 216, 217, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228,
    229, 230, 231, 232, 233, 234, 235, 236, 236, 237, 238, 239, 240, 240, 241, 242,
    243, 243, 244, 244, 245, 246, 246, 247, 247, 248, 248, 249, 249, 250, 250, 251,
    251, 252, 252, 252, 253, 253, 253, 253, 254, 254, 254, 254, 254, 255, 255, 255,
    255, 255, 255, 255, 255};

constexpr Amplitude QSin(int32_t value)
{
    uint16_t v720 = value % 720;
    uint16_t v360 = v720 % 360;
    return Amplitude{
        v720 < 360 ? Positive : Negative,
        triValue[v360 <= 180 ? v360 : (360 - v360)]};
}