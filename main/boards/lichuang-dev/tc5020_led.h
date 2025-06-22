#ifndef _TC5020_LED_H_
#define _TC5020_LED_H_

#include "../../led/led.h"
#include <driver/gpio.h>
#include <vector>

class TC5020Led : public Led {
public:
    const char* GetDriverName() const override { return "TC5020Led"; }
    // Added oe_pin and oe_active_level
    TC5020Led(gpio_num_t data_pin, gpio_num_t clk_pin, gpio_num_t latch_pin, 
              gpio_num_t oe_pin, bool oe_active_level,
              uint8_t num_digits = 4, uint8_t num_indicators = 6);
    virtual ~TC5020Led();

    virtual void OnStateChanged() override;
    
    // LED control methods
    void SetDigit(uint8_t digit_index, uint8_t value);
    void SetIndicator(uint8_t index, bool state);
    void Refresh();
    void EnableOutput(bool enable);  // New method for OE control

private:
    gpio_num_t data_pin_;
    gpio_num_t clk_pin_;
    gpio_num_t latch_pin_;
    gpio_num_t oe_pin_;           // New OE pin
    bool oe_active_level_;        // Active level (true = high enable, false = low enable)
    uint8_t num_digits_;
    uint8_t num_indicators_;
    std::vector<uint8_t> digit_states_;
    std::vector<bool> indicator_states_;
    bool output_enabled_ = false; // Track output state
    
    void ShiftOut(uint16_t data);
    void LatchData();
};

#endif // _TC5020_LED_H_