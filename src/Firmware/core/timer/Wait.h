#pragma once

static inline void wait(uint32_t time)
{
    while(time--)
    {
        _delay_ms(1);
    }
}