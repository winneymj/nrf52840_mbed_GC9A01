#ifndef __COMMON_H__
#define __COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

// /* FNC PROTOTYPES */
void pin_rst_set(int); // set reset pin to a value
void pin_cmd_set(int); // set cmd dta pin to a value
void spi_cs_set(int); // setting chip select pin to a value
void spi_wr(int); // write a byte over spi
void spi_wr_mem(char *, int);
void spi_set_freq(int); // set baudrate
void spi_mode(int, int);
void delay_ms(int);
void delay_us(int);

// I2C
void i2c_set_freq(int); // set speed over i2c
/**
 * write multiple bytes over i2c, return 0 if success
 */ 
int  i2c_wr(uint8_t dev_addr, const char *data, int len, int repeated);
int  i2c_rd(int, char *, int, int); // read multiple bytes over i2c


#ifdef __cplusplus
}
#endif

#endif