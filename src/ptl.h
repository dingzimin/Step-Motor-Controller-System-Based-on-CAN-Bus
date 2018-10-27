#pragma once
#ifndef _PTL_H_
#define _PTL_H_

#include <functional>
#include <algorithm>
#include <cstdlib>
#include "stm32f10x_crc.h"
#include "timer.h"
#include "can.h"
#include "bitband.h"

#define min(a, b) (((a) > (b)) ? (b) : (a))

typedef std::function<void(uint32_t, uint8_t, uint8_t, uint32_t)> StreamStartHandler;

class Ptl
{
  public:
    static bool SendData(uint32_t id, uint8_t *data, uint8_t size);
    static bool ReceiveData(uint32_t id, uint8_t *buffer, uint8_t size, uint8_t sseq, uint32_t ccrc);
    static inline void SetStreamStartHandler(const StreamStartHandler &handler)
    {
        OnStreamStart = std::move(handler);
        Can::OnReceive = Can::OnReceive = [&](CanRxMsg &msg) {
            if (OnStreamStart && (((uint16_t *)msg.Data)[1] ^ ((uint16_t *)msg.Data)[2] ^ ((uint16_t *)msg.Data)[3] == ((uint16_t *)msg.Data)[0]))
            {
                Can::Transmit(msg.StdId, CAN_Id_Standard, CAN_RTR_Data, msg.Data, 8);
                OnStreamStart(msg.StdId, msg.Data[2], msg.Data[3], ((uint32_t *)msg.Data)[1]);
            }
        };
    }

  private:
    static StreamStartHandler OnStreamStart;
};

#endif