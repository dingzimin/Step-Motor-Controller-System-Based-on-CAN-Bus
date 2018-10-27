#pragma once
#ifndef _CAN_H_
#define _CAN_H_

#include <functional>
#include <cstring>
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_can.h"
#include "timer.h"

typedef uint32_t filter32;
typedef uint16_t filter16;
typedef std::function<void(CanRxMsg &)> CanReceiveHandler;

enum Fifo : uint8_t
{
    Fifo0 = 0,
    Fifo1 = 1,
};

constexpr filter32 Filter32(uint32_t stid, uint32_t exid, uint8_t ide, uint8_t rtr)
{
    return ((stid & 0x00007FF) << 21) | ((exid & 0x0003FFFF) << 3) | ((ide & 0x01) << 2) | ((rtr & 0x01) << 1);
}

constexpr filter16 Filter16(uint32_t stid, uint32_t exid, uint8_t ide, uint8_t rtr)
{
    return ((stid & 0x00007FF) << 5) | ((exid & 0x0003FFFF) >> 15) | ((ide & 0x01) << 4) | ((rtr & 0x01) << 5);
}

class Can;

class CanFilter
{
  public:
    CanFilter()
    {
        FilterInitStruct = {
            0, 0, 0, 0, 0, 0, 0, 0, ENABLE};
    }
    CanFilter(Fifo fifo)
    {
        FilterInitStruct = {
            0, 0, 0, 0, fifo, 0, 0, 0, ENABLE};
    }
    inline void IdMask32(filter32 mask, filter32 id)
    {
        FNum = 1;
        FilterInitStruct.CAN_FilterMode = CAN_FilterMode_IdMask;
        FilterInitStruct.CAN_FilterScale = CAN_FilterScale_32bit;
        FilterInitStruct.CAN_FilterIdHigh = id >> 16;
        FilterInitStruct.CAN_FilterIdLow = id & 0xFFFF;
        FilterInitStruct.CAN_FilterMaskIdHigh = mask >> 16;
        FilterInitStruct.CAN_FilterMaskIdLow = mask & 0xFFFF;
    }
    inline void IdList32(filter32 id0, filter32 id1)
    {
        FNum = 2;
        FilterInitStruct.CAN_FilterMode = CAN_FilterMode_IdList;
        FilterInitStruct.CAN_FilterScale = CAN_FilterScale_32bit;
        FilterInitStruct.CAN_FilterIdHigh = id0 >> 16;
        FilterInitStruct.CAN_FilterIdLow = id0 & 0xFFFF;
        FilterInitStruct.CAN_FilterMaskIdHigh = id0 >> 16;
        FilterInitStruct.CAN_FilterMaskIdLow = id0 & 0xFFFF;
    }
    inline void IdMask16(filter16 mask0, filter16 id0, filter16 mask1, filter16 id1)
    {
        FNum = 2;
        FilterInitStruct.CAN_FilterMode = CAN_FilterMode_IdList;
        FilterInitStruct.CAN_FilterScale = CAN_FilterScale_16bit;
        FilterInitStruct.CAN_FilterIdHigh = id1;
        FilterInitStruct.CAN_FilterIdLow = id0;
        FilterInitStruct.CAN_FilterMaskIdHigh = mask1;
        FilterInitStruct.CAN_FilterMaskIdLow = mask0;
    }
    inline void IdList16(filter16 id0, filter16 id1, filter16 id2, filter16 id3)
    {
        FNum = 4;
        FilterInitStruct.CAN_FilterMode = CAN_FilterMode_IdList;
        FilterInitStruct.CAN_FilterScale = CAN_FilterScale_16bit;
        FilterInitStruct.CAN_FilterIdHigh = id2;
        FilterInitStruct.CAN_FilterIdLow = id0;
        FilterInitStruct.CAN_FilterMaskIdHigh = id3;
        FilterInitStruct.CAN_FilterMaskIdLow = id1;
    }
    inline void SetFifo(Fifo fifo)
    {
        FilterInitStruct.CAN_FilterFIFOAssignment = fifo;
    }
    friend class Can;

  private:
    CAN_FilterInitTypeDef FilterInitStruct;
    uint8_t FNum;
};

class Can
{
  public:
    static void Init();
    static uint8_t AddFilter(CanFilter &filter);
    static inline uint8_t Transmit(CanTxMsg &msg)
    {
        return CAN_Transmit(CAN1, &msg);
    }
    static inline uint8_t Transmit(uint32_t id, uint8_t ide, uint8_t rtr, uint8_t data[], uint8_t length)
    {
        CanTxMsg msg{
            id, id, ide, rtr, length, {0}};
        memcpy(msg.Data, data, length > 8 ? 8 : length);
        return CAN_Transmit(CAN1, &msg);
    }
    static inline bool Receice(CanRxMsg &msg)
    {
        canMsg = &msg;
        rec = true;
        uint32_t timeMax = Timer::GetTick() + 100;
        while (true)
        {
            if (!rec)
                return true;
            if (Timer::GetTick() > timeMax)
                return false;
        }
    }
    static CanReceiveHandler OnReceive;
    static CanRxMsg *canMsg;
    static bool rec;

  private:
    static uint8_t Fifo0n, Fifo1n;
};

#endif