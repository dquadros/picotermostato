/**
 * @file picotermostato.h
 * @author Daniel Quadros
 * @brief Definições globais para o projeto picotermostato
 * @version 2.0
 * @date 2022-11-16
 * 
 * @copyright Copyright (c) 2022
 * 
 */

// Conexões do circuito
#define PIN_SENSOR 10

#define PIN_ENC_SW     11
#define PIN_ENC_DT     12
#define PIN_ENC_CLK    13

#define SPI_ID spi1
#define PIN_SCLK  14
#define PIN_SDIN  15
#define PIN_DC    18
#define PIN_RESET 19
#define PIN_SCE   20

#define PIN_RELE  21

#define I2C_ID i2c1
#define PIN_SDA  26
#define PIN_SCL  27

// Teclas
#define TECLA_ENTER 0
#define TECLA_UP    1
#define TECLA_DN    2

// Encoder
void encoderInit (PIO pio, uint pin_a, uint pin_b, uint pin_sw);
int tecLe (void);

// Display
void displayInit (void);
void displayRefresh (void);
void displayCar (int l, int c, char car);
void displayStr (int l, int c, const char *str);
void displayDigDD (int l, int c, char dig);
void displayClear (void);

// Sensor
void sensorInit (void);
int sensorLe (void);

// EEProm
void eepromInit(uint pinSDA, uint pinSCL);
bool eepromRead(uint8_t *buffer, uint16_t addr, int n);
bool eepromWrite(uint8_t *buffer, uint16_t addr, int n);

