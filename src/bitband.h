#pragma once
#ifndef _BITBAND_H_
#define _BITBAND_H_

#include <cstdint>
#include "stm32f10x.h"

class BitBand
{
  public:
    template <typename T>
    static constexpr uint32_t *Ram(T *base)
    {
        return (uint32_t *)(SRAM_BB_BASE | ((uint32_t)base - SRAM_BASE) << 5);
    }
    template <typename T>
    static constexpr uint32_t *Ram(T *base, uint8_t bit)
    {
        return (uint32_t *)((SRAM_BB_BASE | (((uint32_t)base - SRAM_BASE) << 5)) + ((uint32_t)bit << 2));
    }
    template <typename T>
    static constexpr uint32_t *Periph(T *base)
    {
        return (uint32_t *)(PERIPH_BB_BASE | ((uint32_t)base - PERIPH_BASE) << 5);
    }
    template <typename T>
    static constexpr uint32_t *Periph(T *base, uint8_t bit)
    {
        return (uint32_t *)((PERIPH_BB_BASE | (((uint32_t)base - PERIPH_BASE) << 5)) + ((uint32_t)bit << 2));
    }
};

#endif