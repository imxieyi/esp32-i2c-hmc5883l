#include "include/i2c.hpp"
#include "esp_log.h"

#define _DELAY ets_delay_us(1)
#define _SDA1 gpio_set_level(sda_pin,1)
#define _SDA0 gpio_set_level(sda_pin,0)

#define _SCL1 gpio_set_level(scl_pin,1)
#define _SCL0 gpio_set_level(scl_pin,0)

#define _SDAX gpio_get_level(sda_pin)
#define _SCLX gpio_get_level(scl_pin)

#define CLK_HALF_PERIOD_US (1)

#define CLK_STRETCH  (10)

I2C::I2C(gpio_num_t scl, gpio_num_t sda)
{
    scl_pin = scl;
    sda_pin = sda;

    gpio_set_pull_mode(scl_pin,GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(sda_pin,GPIO_PULLUP_ONLY);

    gpio_set_direction(scl_pin,GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(sda_pin,GPIO_MODE_INPUT_OUTPUT);

    // I2C bus idle state.
    gpio_set_level(scl_pin,1);
    gpio_set_level(sda_pin,1);
}

void I2C::delay() {
    ets_delay_us(CLK_HALF_PERIOD_US);
}

bool I2C::read_scl() {
    gpio_set_level(scl_pin, 1);
    return gpio_get_level(scl_pin);
}

bool I2C::read_sda() {
    gpio_set_level(sda_pin, 1);
    delay();
    return gpio_get_level(sda_pin);
}

void I2C::clear_scl() {
    gpio_set_level(scl_pin, 0);
}

void I2C::clear_sda() {
    gpio_set_level(sda_pin, 0);
}

void I2C::start() {
    if(read_sda() == 0) {
        ESP_LOGE("I2C", "arbitration lost in i2c_start");
    }
    clear_sda();
    delay();
    clear_scl();
}

void I2C::stop() {
    uint32_t clk_stretch = CLK_STRETCH;
    clear_sda();
    delay();
    while(read_scl()==0 && clk_stretch--);
    delay();
    if(read_sda() == 0) {
        ESP_LOGE("I2C", "arbitration lost in i2c_stop");
    }
    delay();
}

void I2C::write_bit(bool bit) {
    uint32_t clk_stretch = CLK_STRETCH;
    if (bit) {
        read_sda();
    } else {
        clear_sda();
    }
    delay();
    while(read_scl()==0 && clk_stretch--);
    if (bit && read_sda() == 0) {
        ESP_LOGE("I2C", "arbitration lost in i2c_write_bit");
    }
    delay();
    clear_scl();
}

bool I2C::read_bit() {
    uint32_t clk_stretch = CLK_STRETCH;
    bool bit;
    read_sda();
    delay();
    while(read_scl()==0 && clk_stretch--);
    bit = read_sda();
    delay();
    clear_scl();
    return bit;
}

// return: true - ACK; false - NACK
bool I2C::write(uint8_t data) {
    bool nack;
    uint8_t bit;
    for (bit = 0; bit < 8; bit++) {
        write_bit((data & 0x80) != 0);
        data <<= 1;
    }
    nack = read_bit();
    return !nack;
}

uint8_t I2C::read(bool ack) {
    uint8_t data = 0;
    uint8_t bit;
    for(bit=0;bit<8;bit++) {
        data = (data << 1) | read_bit();
    }
    write_bit(ack);
    return data;
}

bool I2C::slave_write(uint8_t slave_addr,uint8_t reg_addr, uint8_t data) {
    bool success = false;
    do {
        start();
        if(!write(slave_addr << 1))
            break;
        if(!write(reg_addr))
            break;
        if(!write(data))
            break;
        stop();
        success = true;
    } while(0);
    if(!success) {
        ESP_LOGE("I2C", "write error");
    }
    return success;
}

bool I2C::slave_read(uint8_t slave_addr, uint8_t data, uint8_t *buf, uint32_t len) {
    bool success = false;
    do {
        start();
        if(!write(slave_addr << 1))
            break;
        write(data);
        stop();
        start();
        if(!write(slave_addr << 1 | 1))
            break;
        while(len) {
            *buf = read(len == 1);
            buf++;
            len--;
        }
        success = true;
    } while(0);
    stop();
    if(!success) {
        ESP_LOGE("I2C", "read error");
    }
    return success;
}
