#include "temp_sim.h"

int32_t temp_sim_deci_celsius(uint32_t uptime_ms)
{
    uint32_t second_of_cycle = (uptime_ms / 1000u) % 60u;              /* 0..59 */
    uint32_t rise = (second_of_cycle < 30u) ? second_of_cycle
                                            : 60u - second_of_cycle;   /* 0..30 */

    return (int32_t)(200u + rise * 2u);                                /* 200..260 */
}
