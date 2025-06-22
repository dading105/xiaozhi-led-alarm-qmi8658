#include "tc5020_led.h"
#include <esp_log.h>
#include <driver/gpio.h>

#define TAG "TC5020Led"
#include "../../application.h"

// Updated constructor with OE pin
TC5020Led::TC5020Led(gpio_num_t data_pin, gpio_num_t clk_pin, gpio_num_t latch_pin, 
                     gpio_num_t oe_pin, bool oe_active_level,
                     uint8_t num_digits, uint8_t num_indicators)
    : data_pin_(data_pin), clk_pin_(clk_pin), latch_pin_(latch_pin),
      oe_pin_(oe_pin), oe_active_level_(oe_active_level),
      num_digits_(num_digits), num_indicators_(num_indicators) {
    
    // Initialize all GPIOs
    gpio_reset_pin(data_pin_);
    gpio_set_direction(data_pin_, GPIO_MODE_OUTPUT);
    gpio_reset_pin(clk_pin_);
    gpio_set_direction(clk_pin_, GPIO_MODE_OUTPUT);
    gpio_reset_pin(latch_pin_);
    gpio_set_direction(latch_pin_, GPIO_MODE_OUTPUT);
    gpio_reset_pin(oe_pin_);
    gpio_set_direction(oe_pin_, GPIO_MODE_OUTPUT);

    // Initialize states
    digit_states_.resize(num_digits_, 0);
    indicator_states_.resize(num_indicators_, false);
    
    // Start with output disabled
    EnableOutput(false);
}

TC5020Led::~TC5020Led() {
    // Turn off all LEDs and disable output
    for(auto& digit : digit_states_) digit = 0;
    for(auto&& ind : indicator_states_) ind = false;
    Refresh();
    EnableOutput(false);
}

// New OE control method
void TC5020Led::EnableOutput(bool enable) {
    output_enabled_ = enable;
    gpio_set_level(oe_pin_, enable ? oe_active_level_ : !oe_active_level_);
}

void TC5020Led::SetDigit(uint8_t digit_index, uint8_t value) {
    if(digit_index < num_digits_) {
        digit_states_[digit_index] = value & 0x7F;  // 7 segments
    }
}

void TC5020Led::SetIndicator(uint8_t index, bool state) {
    if(index < num_indicators_) {
        indicator_states_[index] = state;
    }
}

void TC5020Led::ShiftOut(uint16_t data) {
    for(int i = 15; i >= 0; i--) {
        gpio_set_level(data_pin_, (data >> i) & 1);
        gpio_set_level(clk_pin_, 1);
        gpio_set_level(clk_pin_, 0);
    }
}

void TC5020Led::LatchData() {
    gpio_set_level(latch_pin_, 1);
    gpio_set_level(latch_pin_, 0);
}

void TC5020Led::Refresh() {
    if (!output_enabled_) {
        EnableOutput(true);  // Auto-enable output during refresh
    }
    
    // Combine digit and indicator states
    uint16_t output = 0;
    
    // Pack indicators (bits 0-5)
    for(int i = 0; i < num_indicators_; i++) {
        if(indicator_states_[i]) output |= (1 << i);
    }
    
    // Pack digits (bits 6-13: digit0, 14-21: digit1, etc.)
    for(int i = 0; i < num_digits_; i++) {
        output |= (static_cast<uint16_t>(digit_states_[i]) << (6 + i*8));
    }
    
    ShiftOut(output);
    LatchData();
}

// Implement OE control in state changes
void TC5020Led::OnStateChanged() {
    // Enable output when device is active, disable when sleeping
    bool should_enable = (Application::GetInstance().GetDeviceState() != kDeviceStateIdle);
    if (output_enabled_ != should_enable) {
        EnableOutput(should_enable);
    }
    Refresh();
}