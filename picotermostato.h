/**
 * @file picotermostato.h
 * @author Daniel Quadros
 * @brief Definições globais para o projeto picotermostato
 * @version 0.1
 * @date 2022-11-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */

// Conexões do circuito
#define PIN_SENSOR 10

#define PIN_TEC_UP     11
#define PIN_TEC_DN     12
#define PIN_TEC_ENTER  13

#define SPI_ID spi1
#define PIN_SCLK  14
#define PIN_SDIN  15
#define PIN_DC    18
#define PIN_RESET 19
#define PIN_SCE   20

#define PIN_RELE  21

// Teclado
typedef struct {
    int gpio;
    bool apertada;
    int cnt_debounce;
} TECLA;

// Teclado
void tecInit (TECLA *teclas, int nteclas);
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

