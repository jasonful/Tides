#include <Arduino.h>
#include "dbg.h"

size_t Dbg::printf(const char *format, ...)
{
#if DEBUG
    // Copied from packages\esp32\hardware\esp32\1.0.1\cores\esp32\Print.cpp
    char loc_buf[64];
    char *temp = loc_buf;
    va_list arg;
    va_list copy;
    va_start(arg, format);
    va_copy(copy, arg);
    size_t len = vsnprintf(NULL, 0, format, arg);
    va_end(copy);
    if (len >= sizeof(loc_buf))
    {
        temp = new char[len + 1];
        if (temp == NULL)
        {
            return 0;
        }
    }
    len = vsnprintf(temp, len + 1, format, arg);
    Serial.write((uint8_t *)temp, len);
    va_end(arg);
    if (len >= sizeof(loc_buf))
    {
        delete[] temp;
    }
    return len;
#else
    return 0;
#endif
}
