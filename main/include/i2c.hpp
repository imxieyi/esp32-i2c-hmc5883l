#ifndef I2C_H_
#define I2C_H_

#include "driver/gpio.h"
#include "rom/ets_sys.h"

class I2C {
private:
	gpio_num_t scl_pin, sda_pin;
	void delay();
	bool read_scl();
	bool read_sda();
	void clear_scl();
	void clear_sda();
	void start();
	void stop();
	void write_bit(bool bit);
	bool read_bit();
	bool write(uint8_t data);
	uint8_t read(bool ack);

public:
	I2C(gpio_num_t scl, gpio_num_t sda);
	bool slave_write(uint8_t slave_addr,uint8_t reg_addr, uint8_t data);
	bool slave_read(uint8_t slave_addr, uint8_t data, uint8_t *buf, uint32_t len);
};

#endif
