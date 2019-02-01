#include <stdint.h>

class RestartCounter
{
    public:

    RestartCounter();
    ~RestartCounter();

    int32_t Get();
    void Set(int32_t restart_counter);
};
