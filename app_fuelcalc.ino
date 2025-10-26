#include <stdint.h>
#include <stdlib.h>  // for abs()

// Buffer size calculation:
// Time to travel 10km at 20km/h = 10/20 = 0.5 hours = 1800 seconds
// One sample per second = 1800 samples needed
#define SPEED_BUFFER_SIZE 1800
#define MIN_FUEL_CHANGE 0.5f    // Minimum fuel change to trigger consumption calculation

struct speed_buffer_t {
    float speeds[SPEED_BUFFER_SIZE];  // Array to store speed values
    uint16_t write_index;            // Current position in buffer
    uint32_t last_update;           // Last update timestamp
    bool buffer_full;              // Indicates if buffer has wrapped around
    float total_distance;         // Accumulated distance in km
} speed_buffer;

struct fuel_data_t {
    float current_level;           // Current fuel level in liters
    float last_level;             // Previous fuel level in liters
    uint16_t last_speed_index;    // Buffer index when last fuel level was recorded
    bool is_tracking;             // Whether we have valid fuel tracking
    float last_consumption;       // Last calculated consumption in L/100km
} fuel_data;

/**
 * @brief Initialize the speed buffer
 */
void init_speed_buffer() {
    for(uint16_t i = 0; i < SPEED_BUFFER_SIZE; i++) {
        speed_buffer.speeds[i] = 0.0f;
    }
    speed_buffer.write_index = 0;
    speed_buffer.last_update = 0;
    speed_buffer.buffer_full = false;
    speed_buffer.total_distance = 0.0f;
}

/**
 * @brief Store a speed value in the circular buffer
 * Only stores one value per second
 * 
 * @param speed Current speed in km/h
 * @return true if a new value was stored
 * @return false if not enough time has passed
 */
bool store_speed(float speed) {
    uint32_t current_time = millis();
    
    // Only store one value per second
    if (current_time - speed_buffer.last_update >= 1000) {
        // Store the speed
        speed_buffer.speeds[speed_buffer.write_index] = speed;
        
        // Calculate distance traveled in this second
        // distance (km) = speed (km/h) * (1/3600) h = speed / 3600
        speed_buffer.total_distance += speed / 3600.0f;
        
        // Update write index
        speed_buffer.write_index = (speed_buffer.write_index + 1) % SPEED_BUFFER_SIZE;
        
        // Mark buffer as full if we've wrapped around
        if (speed_buffer.write_index == 0) {
            speed_buffer.buffer_full = true;
        }
        
        // Update timestamp
        speed_buffer.last_update = current_time;
        return true;
    }
    
    return false;
}

/**
 * @brief Get number of valid samples in buffer
 * 
 * @return uint16_t Number of valid samples
 */
uint16_t get_sample_count() {
    return speed_buffer.buffer_full ? SPEED_BUFFER_SIZE : speed_buffer.write_index;
}

/**
 * @brief Get the average speed over all stored samples
 * 
 * @return float Average speed in km/h, or 0 if no data
 */
float get_average_speed() {
    uint16_t count = get_sample_count();
    if (count == 0) {
        return 0.0f;
    }
    
    float sum = 0.0f;
    for (uint16_t i = 0; i < count; i++) {
        sum += speed_buffer.speeds[i];
    }
    
    return sum / count;
}

/**
 * @brief Get the total distance traveled
 * 
 * @return float Total distance in kilometers
 */
float get_total_distance() {
    return speed_buffer.total_distance;
}

/**
 * @brief Get the maximum speed from stored samples
 * 
 * @return float Maximum speed in km/h, or 0 if no data
 */
float get_max_speed() {
    uint16_t count = get_sample_count();
    if (count == 0) {
        return 0.0f;
    }
    
    float max_speed = speed_buffer.speeds[0];
    for (uint16_t i = 1; i < count; i++) {
        if (speed_buffer.speeds[i] > max_speed) {
            max_speed = speed_buffer.speeds[i];
        }
    }
    
    return max_speed;
}

/**
 * @brief Reset the speed buffer and all statistics
 */
void reset_speed_buffer() {
    init_speed_buffer();
}

/**
 * @brief Initialize fuel tracking data
 */
void init_fuel_tracking() {
    fuel_data.current_level = 0.0f;
    fuel_data.last_level = 0.0f;
    fuel_data.last_speed_index = 0;
    fuel_data.is_tracking = false;
    fuel_data.last_consumption = 0.0f;
}

/**
 * @brief Calculate average speed between two buffer indices
 * 
 * @param start_idx Starting index
 * @param end_idx Ending index
 * @param wrapped Whether buffer has wrapped around
 * @return float Average speed in km/h
 */
float calculate_average_speed_between(uint16_t start_idx, uint16_t end_idx, bool wrapped) {
    float sum = 0.0f;
    uint16_t count;
    
    if (wrapped) {
        // If buffer has wrapped, calculate from start to buffer end and from 0 to end
        count = (SPEED_BUFFER_SIZE - start_idx) + end_idx;
        for (uint16_t i = start_idx; i < SPEED_BUFFER_SIZE; i++) {
            sum += speed_buffer.speeds[i];
        }
        for (uint16_t i = 0; i < end_idx; i++) {
            sum += speed_buffer.speeds[i];
        }
    } else {
        // Simple case: calculate from start to end
        count = end_idx - start_idx;
        for (uint16_t i = start_idx; i < end_idx; i++) {
            sum += speed_buffer.speeds[i];
        }
    }
    
    return (count > 0) ? (sum / count) : 0.0f;
}

/**
 * @brief Update consumption based on new fuel level
 * 
 * @param new_fuel_level Current fuel level in liters
 * @return float Calculated consumption in L/100km, or -1 if invalid
 */
float update_consumption(float new_fuel_level) {
    // Check if fuel change is significant
    if (abs(new_fuel_level - fuel_data.current_level) < MIN_FUEL_CHANGE) {
        return fuel_data.last_consumption;
    }
    
    if (fuel_data.is_tracking) {
        uint16_t current_idx = speed_buffer.write_index;
        float avg_speed;
        float elapsed_time;
        float distance;
        float fuel_used;
        
        // Determine if buffer has wrapped since last fuel update
        bool wrapped = (current_idx < fuel_data.last_speed_index) || speed_buffer.buffer_full;
        
        // Calculate average speed between fuel updates
        avg_speed = calculate_average_speed_between(fuel_data.last_speed_index, current_idx, wrapped);
        
        // Calculate elapsed time in hours
        if (wrapped) {
            elapsed_time = ((SPEED_BUFFER_SIZE - fuel_data.last_speed_index) + current_idx) / 3600.0f;
        } else {
            elapsed_time = (current_idx - fuel_data.last_speed_index) / 3600.0f;
        }
        
        // Calculate distance (km) = speed (km/h) * time (h)
        distance = avg_speed * elapsed_time;
        
        // Calculate fuel used
        fuel_used = fuel_data.current_level - new_fuel_level;
        
        // Calculate consumption in L/100km if we've traveled enough distance
        if (distance >= 0.1f && fuel_used > 0) {  // Minimum 100m
            fuel_data.last_consumption = (fuel_used / distance) * 100.0f;
        }
    }
    
    // Update fuel tracking data
    fuel_data.last_level = fuel_data.current_level;
    fuel_data.current_level = new_fuel_level;
    fuel_data.last_speed_index = speed_buffer.write_index;
    fuel_data.is_tracking = true;
    
    return fuel_data.last_consumption;
}

/**
 * @brief Process new fuel level and get consumption
 * Call this function whenever you get a new fuel level reading
 * 
 * @param fuel_level Current fuel level in liters
 * @param consumption Pointer to store calculated consumption
 * @return true if consumption was updated, false if no change
 */
bool process_new_fuel_level(float fuel_level, float* range_calc) {
    float current_consumption = update_consumption(fuel_level);
    bool was_updated = abs(fuel_level - fuel_data.last_level) >= MIN_FUEL_CHANGE;
    
    if (range_calc != NULL) {
        //*consumption = current_consumption;
        // Calculate range in kilometers (fuel_level / consumption per km)
        float range = (current_consumption > 0) ? ((fuel_level / current_consumption) * 100.0f) : 0.0f;
        *range_calc = range;
    }
    
    return was_updated;
}
