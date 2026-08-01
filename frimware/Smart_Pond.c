#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/* ========================================== */
/* HARDWARE & REGISTER MAPPING CONFIGURATION  */
/* ========================================== */
// Simulated register addresses for microcontroller ports
#define PORT_DIR_REG    (*(volatile uint8_t*)0x24)
#define PORT_OUT_REG    (*(volatile uint8_t*)0x25)
#define PORT_IN_REG     (*(volatile uint8_t*)0x23)

#define PIN_AERATOR_1   (1 << 0) // Pin 8 equivalent
#define PIN_AERATOR_2   (1 << 1) // Pin 9 equivalent
#define PIN_BUZZER      (1 << 2) // Pin 10 equivalent

/* ========================================== */
/* CONSTANTS & THRESHOLDS                     */
/* ========================================== */
#define DO_CRITICAL_LOW 4.0f
#define DO_TARGET_LOW   5.5f
#define DO_TARGET_HIGH  7.2f
#define TEMP_MAX_LIMIT  32.0f

#define ADC_MAX_VAL     1023.0f  // 10-bit ADC standard for AVR microcontrollers
#define REF_VOLTAGE     5.0f

typedef struct {
    float dissolved_oxygen;
    float temperature;
    float ph_level;
} sensor_data_t;

typedef struct {
    bool pump_primary_active;
    bool pump_aux_active;
    bool alarm_triggered;
} actuator_state_t;

/* ========================================== */
/* FUNCTION PROTOTYPES                        */
/* ========================================== */
void system_init(void);
uint16_t adc_read_channel(uint8_t channel);
void read_sensors(sensor_data_t *data);
void evaluate_thresholds(const sensor_data_t *data, actuator_state_t *state);
void drive_actuators(const actuator_state_t *state);
void transmit_uart_packet(const sensor_data_t *data, const actuator_state_t *state);

/* ========================================== */
/* SENSOR & ADC PROCESSING                    */
/* ========================================== */
uint16_t adc_read_channel(uint8_t channel) {
    // Basic moving average filter implementation in pure C
    uint32_t accumulator = 0;
    uint8_t samples = 8;
    
    for (uint8_t i = 0; i < samples; i++) {
        // Mock hardware read register placeholder
        accumulator += (512 + (channel * 10)); 
    }
    return (uint16_t)(accumulator / samples);
}

void read_sensors(sensor_data_t *data) {
    uint16_t raw_do = adc_read_channel(0);
    uint16_t raw_temp = adc_read_channel(1);
    uint16_t raw_ph = adc_read_channel(2);

    // Linear conversion formulas
    data->dissolved_oxygen = ((float)raw_do / ADC_MAX_VAL) * 14.0f;
    data->temperature      = ((float)raw_temp / ADC_MAX_VAL) * 50.0f;
    data->ph_level         = ((float)raw_ph / ADC_MAX_VAL) * 14.0f;
}

/* ========================================== */
/* HYSTERESIS CONTROL ALGORITHM               */
/* ========================================== */
void evaluate_thresholds(const sensor_data_t *data, actuator_state_t *state) {
    float do_val = data->dissolved_oxygen;
    float temp_val = data->temperature;

    if (do_val <= DO_CRITICAL_LOW || temp_val >= TEMP_MAX_LIMIT) {
        state->pump_primary_active = true;
        state->pump_aux_active     = true;
        state->alarm_triggered     = true;
    } 
    else if (do_val > DO_CRITICAL_LOW && do_val < DO_TARGET_LOW) {
        state->pump_primary_active = true;
        state->pump_aux_active     = false;
        state->alarm_triggered     = false;
    } 
    else if (do_val >= DO_TARGET_HIGH) {
        state->pump_primary_active = false;
        state->pump_aux_active     = false;
        state->alarm_triggered     = false;
    }
    // Deadband region: maintains previous state to prevent relay chatter
}

/* ========================================== */
/* ACTUATOR HARDWARE INTERFACE                */
/* ========================================== */
void drive_actuators(const actuator_state_t *state) {
    // Active-low relay board implementation via bitwise operations
    uint8_t port_mask = 0x00;

    if (state->pump_primary_active) port_mask |= PIN_AERATOR_1;
    if (state->pump_aux_active)     port_mask |= PIN_AERATOR_2;
    if (state->alarm_triggered)     port_mask |= PIN_BUZZER;

    // Apply inverted logic for active-low relays
    PORT_OUT_REG = ~port_mask;
}

void transmit_uart_packet(const sensor_data_t *data, const actuator_state_t *state) {
    // Standard low-level formatting string for serial telemetry transmission
    printf("PKT|DO:%.2f|TMP:%.2f|PH:%.2f|P1:%d|P2:%d|ALM:%d\n",
           data->dissolved_oxygen,
           data->temperature,
           data->ph_level,
           state->pump_primary_active,
           state->pump_aux_active,
           state->alarm_triggered);
}

/* ========================================== */
/* SYSTEM INITIALIZATION                      */
/* ========================================== */
void system_init(void) {
    // Set data direction registers for output pins
    PORT_DIR_REG |= (PIN_AERATOR_1 | PIN_AERATOR_2 | PIN_BUZZER);
    
    // Default safe state: All relays OFF
    PORT_OUT_REG |= (PIN_AERATOR_1 | PIN_AERATOR_2);
}

/* ========================================== */
/* MAIN EXECUTION LOOP                        */
/* ========================================== */
int main(void) {
    sensor_data_t current_sensor_data = {0};
    actuator_state_t system_state = {0};

    system_init();

    // Infinite embedded execution loop
    while (1) {
        read_sensors(&current_sensor_data);
        evaluate_thresholds(&current_sensor_data, &system_state);
        drive_actuators(&system_state);
        transmit_uart_packet(&current_sensor_data, &system_state);
    }

    return 0;
}
