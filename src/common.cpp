
#include <mbed.h>
#include "lv_drv_conf.h"
#include "common.h"
// #include "BMA42X-Sensor-Driver/src/bma4_defs.h"

/* GLOBAL PINSSSSSS */
#if GC9A01_SPI_BITS == 8
DigitalOut cmd_data(P1_10); // MDBT50Q-DB board
// DigitalOut cmd_data(P0_6); // Adafruit
#endif
#if GC9A01_SPI_BITS == 9
uint8_t cmd_data;
#endif
DigitalOut reset(P0_29); // MDBT50Q-DB board
// DigitalOut reset(P0_7); // Adafruit

#if GC9A01_SPI_BITS == 8
# if GC9A01_SPI_EXT_CS
// SPI spi(P1_13, NC, P1_12, NC);  // MDBT50Q-DB board
SPI spi(SPI_PSELMOSI0, NC, SPI_PSELSCK0, NC);
# else
SPI spi(P1_15, NC, P1_13, P1_11); // MDBT50Q-DB board
// SPI spi(SPI_PSELMOSI0, NC, SPI_PSELSCK0, SPI_PSELSS0); // Adafruit
# endif
#endif
#if GC9A01_SPI_BITS == 9
DigitalOut spi_mosi(SPI_PSELMOSI0);
DigitalOut spi_sck(SPI_PSELSCK0);
#endif
#if GC9A01_SPI_EXT_CS
DigitalOut spi_cs(SPI_PSELSS0);
#endif

// BMA423 Accelerator
I2C i2c(P0_26, P0_5);

void pin_rst_set(int val) {
	reset = val;
}

void pin_cmd_set(int val) {
	cmd_data = val;
}

void spi_cs_set(int val) {
#if GC9A01_SPI_EXT_CS
	spi_cs = val;
#endif
}

// write a single byte but mbed SPI write doesnt have one for writing
// a single byte so we just write it as a 32 bits integer
void spi_wr(int data)
{
#if GC9A01_SPI_BITS == 8
	spi.write(data);
#endif
#if GC9A01_SPI_BITS == 9
	spi_sck = 0; // Clock low
	if (cmd_data) spi_mosi = 1;
	else				  spi_mosi = 0;
	spi_sck = 1; // Clock high

	// Fast SPI bitbang
	for(uint8_t bit = 0x80; bit; bit >>= 1) {
		spi_sck = 0;
		if (data & bit) spi_mosi = 1;
		else            spi_mosi = 0;
		spi_sck = 1;
	}
#endif
}

void spi_wr_mem(char *addr, int len)
{
#if GC9A01_SPI_BITS == 8
	// uint8_t *ptr = (uint8_t *)addr;
	// for (uint32_t i = 0; i < len; i++) {
	// 	spi.write(*ptr);
	// 	// spi_wr(*ptr);
	// 	ptr++;
	// }
  spi.write(addr, len, NULL, 0);
#endif
#if GC9A01_SPI_BITS == 9
	cmd_data = 1; // Set to data
	uint8_t *ptr = (uint8_t *)addr;
	for (uint32_t i = 0; i < len; i++) {
		spi_wr(*ptr);
		ptr++;
	}
#endif
}

// // @TODO need to check if this logic is correct for writing to an address on the spi bus
// void spi_wr_mem(uint32_t addr, uint32_t data) {
// 	spi_cs = 0;
// 	spi.write(addr); // write to address 1st
// 	spi.write(data); //9 write them bytes
// 	spi_cs = 1;
// }

void spi_mode(int bits, int mode)
{
#if GC9A01_SPI_BITS == 8
	spi.format(bits, mode);
#endif
}
 
void spi_set_freq(int val)
{
#if GC9A01_SPI_BITS == 8
	spi.frequency(val);
#endif
}

// Set i2c speed
void i2c_set_freq(int freq)
{
	i2c.frequency(freq);
}

// write a single byte but mbed I2C write doesnt have one for writing
// a single byte so we just write it as a 32 bits integer
void i2c_wr(int data)
{
	i2c.lock();

	i2c.write(data);

	i2c.unlock();
}

void i2c_wr_mem(int dev_addr, int reg_addr, char *data, int len)
{
	i2c.lock();

	// Write register
	i2c.write(dev_addr, (const char *)&reg_addr, 1, true); // Do not send stop have more data

	// Write data
	i2c.write(dev_addr, (const char *)data, len);

	i2c.unlock();
}

void delay_ms(int val) {
  wait_us(val * 1000);
}

void delay_us(int val) {
  wait_us(val);
}
