/**
 * @file temp_sim.h
 * @brief Simulated temperature source, used until a real sensor is wired up.
 */
#ifndef TEMP_SIM_H
#define TEMP_SIM_H

#include <stdint.h>

/**
 * Deterministic "sensor" that slowly rises from 20.0 C to 26.0 C and back,
 * one full cycle every 60 seconds.
 * @param uptime_ms time since power-up in milliseconds
 * @return temperature in tenths of a degree Celsius
 */
int32_t temp_sim_deci_celsius(uint32_t uptime_ms);

#endif /* TEMP_SIM_H */
