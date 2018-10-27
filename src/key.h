#pragma once
#ifndef _KEY_H_
#define _KEY_H_

#include <functional>
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "timer.h"

enum KeyEvent : uint8_t
{
    KeyDown = 0,
    KeyUp = 1,
};

typedef std::function<void(uint8_t id, KeyEvent event)> KeyEventHandler;

class Key
{
  public:
    static void Init();
    static KeyEventHandler OnKeyEvent;

  private:
    static void Scan();
};

#endif