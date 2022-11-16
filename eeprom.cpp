/**
 * @file eeprom.c
 * @author Daniel Quadros
 * @brief Driver simples para EEProm 24C32
 * @version 1.0
 * @date 2022-11-16
 * 
 * Baseado em exemplo do livro "Knowing the RP2040"
 * 
 * @copyright Copyright (c) 2022, Daniel Quadros
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"

#include "picotermostato.h"

// I2C Configuration
#define BAUD_RATE 100000   // standard 100KHz

// EEProm
// Assume tamanho da página potência de 2 e tamanho total menor que 64K
#define EEPROM_ADDR 0x50
#define PAGE_SIZE   32
#define PAGE_MASK   (~(PAGE_SIZE-1))

// Inicia o I2C para acesso a EEPROM
void eepromInit(uint pinSDA, uint pinSCL) {
    // Inicia o I2C
    uint baud = i2c_init (I2C_ID, BAUD_RATE);
    printf ("I2C @ %u Hz\n", baud);
    
    // Acerta os pinos
    gpio_set_function(pinSCL, GPIO_FUNC_I2C);
    gpio_set_function(pinSDA, GPIO_FUNC_I2C);
    gpio_pull_up(pinSCL);
    gpio_pull_up(pinSDA);
}

// Le da EEPROM
bool eepromRead(uint8_t *buffer, uint16_t addr, int n) {
    uint8_t bufAddr[2];
    
    bufAddr[0] = addr >> 8;
    bufAddr[1] = addr & 0xFF;
    int ret = i2c_write_blocking (I2C_ID, EEPROM_ADDR, bufAddr, 2, true);
    if (ret == 2) {
        ret = i2c_read_blocking(I2C_ID, EEPROM_ADDR, buffer, n, false);
        if (ret == n) {
            return true;
        }
    }

    return false;
}

// Grava na EEProm
bool eepromWrite(uint8_t *buffer, uint16_t addr, int n) {
    uint8_t bufAux[2+PAGE_SIZE];    // endereço e dados precisam ir na mesma transação

    // Grava aos pedaços, respeitando as paginas
    while (n > 0) {
        uint16_t nextPage = (addr & PAGE_MASK) + PAGE_SIZE;
        int nWrt = nextPage - addr;
        if (nWrt > n) {
            nWrt = n;
        }
        bufAux[0] = addr >> 8;
        bufAux[1] = addr & 0xFF;
        memcpy (bufAux+2, buffer, nWrt);
        int ret = i2c_write_blocking (I2C_ID, EEPROM_ADDR, bufAux, 2+nWrt, false);
        if (ret == (2+nWrt)) {
            // Espera concluir gravação
            // 24C32 responde ao endereço somente quando concluir
            uint8_t aux;
            while (i2c_read_blocking(I2C_ID, EEPROM_ADDR, &aux, 1, false) != 1) {
                sleep_ms(1);
            }
            n -= nWrt;
            buffer += nWrt;
            addr += nWrt;
        } else {
            return false;
        }
    }
    return true;
}
