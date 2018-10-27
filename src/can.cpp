#include "can.h"

CanReceiveHandler Can::OnReceive;
uint8_t Can::Fifo0n = 0;
uint8_t Can::Fifo1n = 0;
CanRxMsg *Can::canMsg;
bool Can::rec;

void Can::Init()
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef GPIOInit;
    GPIOInit.GPIO_Speed = GPIO_Speed_50MHz;
    GPIOInit.GPIO_Pin = GPIO_Pin_11;
    GPIOInit.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIOInit);
    GPIOInit.GPIO_Pin = GPIO_Pin_12;
    GPIOInit.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIOInit);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
    CAN_InitTypeDef CANInit{
        6,
        CAN_Mode_Normal,
        CAN_SJW_2tq,
        CAN_BS1_4tq,
        CAN_BS2_1tq,
        DISABLE,
        DISABLE,
        DISABLE,
        DISABLE,
        DISABLE,
        DISABLE,
    };
    CAN_Init(CAN1, &CANInit);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitTypeDef NVICInit{
        USB_LP_CAN1_RX0_IRQn, 0, 0, ENABLE};
    NVIC_Init(&NVICInit);
    NVICInit = {
        CAN1_RX1_IRQn, 0, 0, ENABLE};
    NVIC_Init(&NVICInit);
    CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
    CAN_ITConfig(CAN1, CAN_IT_FMP1, ENABLE);
}

uint8_t Can::AddFilter(CanFilter &filter)
{
    if (filter.FNum == 0)
        return 0xFF;
    switch (filter.FilterInitStruct.CAN_FilterFIFOAssignment)
    {
    case CAN_Filter_FIFO0:
        filter.FilterInitStruct.CAN_FilterNumber = Fifo0n;
        Fifo0n++;
        break;
    case CAN_Filter_FIFO1:
        filter.FilterInitStruct.CAN_FilterNumber = Fifo1n;
        Fifo1n++;
        break;
    }
    CAN_FilterInit(&filter.FilterInitStruct);
    return 0;
}

extern "C"
{
    void USB_LP_CAN1_RX0_IRQHandler()
    {
        if (CAN_GetITStatus(CAN1, CAN_IT_FMP0) == SET)
        {
            if (Can::rec)
            {
                CAN_Receive(CAN1, CAN_FIFO0, Can::canMsg);
                Can::rec = false;
            }
            else if (Can::OnReceive)
            {
                CanRxMsg msg;
                CAN_Receive(CAN1, CAN_FIFO0, &msg);
                Can::OnReceive(msg);
            }
            CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
        }
    }

    void CAN1_RX1_IRQHandler()
    {
        if (CAN_GetITStatus(CAN1, CAN_IT_FMP1) == SET)
        {
            if (Can::rec)
            {
                CAN_Receive(CAN1, CAN_FIFO0, Can::canMsg);
                Can::rec = false;
            }
            else if (Can::OnReceive)
            {
                CanRxMsg msg;
                CAN_Receive(CAN1, CAN_FIFO0, &msg);
                Can::OnReceive(msg);
            }
            CAN_ClearITPendingBit(CAN1, CAN_IT_FMP1);
        }
    }
}
