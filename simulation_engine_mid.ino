


/**
 * @file simulation_engine_mid.ino
 * @brief Simulates car OBD PID values for testing and development
 * 
 * This file contains functionality to simulate various car parameters like engine RPM,
 * speed, gear, oil temperature, coolant temperature, fuel level and consumption.
 * It generates smooth random values within defined ranges for each parameter.
 * 
 * @details The simulation maintains state between calls to provide smooth transitions
 * between values. Each parameter has defined minimum and maximum values along with
 * a maximum variation step per update to ensure realistic behavior.
 *
 * Simulated parameters include:
 * - Engine RPM (800-7000 RPM)
 * - Vehicle Speed (0-240 km/h) 
 * - Current Gear (1-6)
 * - Oil Temperature (50-150°C)
 * - Coolant Temperature (40-120°C)
 * - Fuel Level (0-50 L)
 * - Fuel Consumption (0-20 L/100km)
 *
 * @note This is intended for testing and development purposes only
 * 
 * @see types.h
 * @see car_t
 * @see obd_pid_states
 */
//#include "types.h"

// Simulation ranges for each OBD PID state
struct pid_range_t {
    float min_value;
    float max_value;
    float variation_step;  // Maximum change per update
} pid_ranges[] = {
    // ENG_RPM (typically 800-7000 RPM)
    {800.0f, 7000.0f, 500.0f},
    
    // SPEED (0-240 km/h)
    {0.0f, 240.0f, 5.0f},
    
    // GEAR_C (1-6)
    {1.0f, 6.0f, 1.0f},
    
    // OIL (50-150°C)
    {50.0f, 150.0f, 2.0f},
    
    // COOLANT (40-120°C)
    {40.0f, 120.0f, 2.0f},
    
    // FUEL_CUSTOM (0-50 L)
    {0.0f, 50.0f, 0.5f},
    
    // FUEL_CONSUMPTION (0-20 L/100km)
    {0.0f, 500.0f, 0.1f}
};

// Keep track of last values for smooth transitions
static float last_simulated_values[8] = {0};
static bool first_run = true;

/**
 * @brief Generate a smooth random value within a range
 * 
 * @param min_val Minimum value
 * @param max_val Maximum value
 * @param max_step Maximum change per call
 * @param last_val Previous value
 * @return float New simulated value
 */
float generate_smooth_random(float min_val, float max_val, float max_step, float last_val) {
    float range = max_val - min_val;
    float random_change = ((float)random(0, 1000) / 1000.0f * 2.0f - 1.0f) * max_step;
    float new_val = last_val + random_change;
    
    // Ensure value stays within bounds
    if (new_val < min_val) new_val = min_val;
    if (new_val > max_val) new_val = max_val;
    
    return new_val;
}

/**
 * @brief Simulate OBD PID value based on state
 * 
 * @param car Pointer to car data structure
 * @param pid_state The OBD PID state to simulate
 * @return float Simulated value for the requested PID
 */
float simulate_pid_value( car_t* car, obd_pid_states pid_state) {
    // Initialize on first run
    if (first_run) {
        for (int i = 0; i < PID_N; i++) {
            last_simulated_values[i] = (pid_ranges[i].max_value + pid_ranges[i].min_value) / 2.0f;
        }
        first_run = false;
    }
    
    // Validate pid_state
    if (pid_state >= PID_N) {
        return 0.0f;  // Invalid state
    }
    
    // Generate new value based on last value and allowed range
    float new_value = generate_smooth_random(
        pid_ranges[pid_state].min_value,
        pid_ranges[pid_state].max_value,
        pid_ranges[pid_state].variation_step,
        last_simulated_values[pid_state]
    );
    
    // Store the new value for next iteration
    last_simulated_values[pid_state] = new_value;
    
    // Update car structure based on PID state
    switch(pid_state) {
        case ENG_RPM:
            car->rpm = new_value;
            car->update_flags |= (1UL << ENG_RPM) | (1UL << V_ENG_RPM);  // Set both RPM flags
            break;
        case SPEED:
            car->speed = new_value;
            car->update_flags |= (1UL << SPEED);
            break;
        case GEAR_C:
            car->gear = new_value;
            car->update_flags |= (1UL << GEAR_C);
            break;
        case OIL:
            car->oil = new_value;
            car->update_flags |= (1UL << OIL);
            break;
        case COOLANT:
            car->coolant = new_value;
            car->update_flags |= (1UL << COOLANT);
            break;
        case FUEL_CUSTOM:
            car->fuel = new_value;
            car->update_flags |= (1UL << FUEL_CUSTOM);
            break;
        case FUEL_CONSUMPTION:
            car->consumption = new_value;
            car->update_flags |= (1UL << FUEL_CONSUMPTION);
            break;
        default:
            break;
    }
    
    return new_value;
}

/**
 * @brief Reset simulation to initial state
 */
void reset_simulation() {
    first_run = true;
}
