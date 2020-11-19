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
void i2c_wr(int); // write a byte over i2c
void i2c_wr_mem(int, int , char *, int); // write multiple bytes over i2c


#ifdef __cplusplus
}
#endif

#endif