/**
 * @file display.cpp
 * @author Daniel Quadros
 * @brief Módulo simples para apresentar números e texto num display Nokia 5110
 * @version 2.0
 * @date 2022-09-16
 * 
 * @copyright Copyright (c) 2022, Daniel Quadros
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <pico/platform.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/pio.h"

#include "picotermostato.h"

// Seleção Dado/Comando
#define LCD_CMD   0
#define LCD_DAT   1

// Tamanho da tela
#define LCD_DX    84
#define LCD_DY    48

// Tamanho caracter normal
#define LARG_F    5     // largura na fonte
#define LARG_C    7     // largura na tela

// Gerador de caracteres normal
static const uint8_t __in_flash() ASCII[][LARG_F]  =
{
 {0x00, 0x00, 0x00, 0x00, 0x00} // 20  
,{0x00, 0x00, 0x5f, 0x00, 0x00} // 21 !
,{0x00, 0x07, 0x00, 0x07, 0x00} // 22 "
,{0x14, 0x7f, 0x14, 0x7f, 0x14} // 23 #
,{0x24, 0x2a, 0x7f, 0x2a, 0x12} // 24 $
,{0x23, 0x13, 0x08, 0x64, 0x62} // 25 %
,{0x36, 0x49, 0x55, 0x22, 0x50} // 26 &
,{0x00, 0x05, 0x03, 0x00, 0x00} // 27 '
,{0x00, 0x1c, 0x22, 0x41, 0x00} // 28 (
,{0x00, 0x41, 0x22, 0x1c, 0x00} // 29 )
,{0x14, 0x08, 0x3e, 0x08, 0x14} // 2a *
,{0x08, 0x08, 0x3e, 0x08, 0x08} // 2b +
,{0x00, 0x50, 0x30, 0x00, 0x00} // 2c ,
,{0x08, 0x08, 0x08, 0x08, 0x08} // 2d -
,{0x00, 0x60, 0x60, 0x00, 0x00} // 2e .
,{0x20, 0x10, 0x08, 0x04, 0x02} // 2f /
,{0x3e, 0x51, 0x49, 0x45, 0x3e} // 30 0
,{0x00, 0x42, 0x7f, 0x40, 0x00} // 31 1
,{0x42, 0x61, 0x51, 0x49, 0x46} // 32 2
,{0x21, 0x41, 0x45, 0x4b, 0x31} // 33 3
,{0x18, 0x14, 0x12, 0x7f, 0x10} // 34 4
,{0x27, 0x45, 0x45, 0x45, 0x39} // 35 5
,{0x3c, 0x4a, 0x49, 0x49, 0x30} // 36 6
,{0x01, 0x71, 0x09, 0x05, 0x03} // 37 7
,{0x36, 0x49, 0x49, 0x49, 0x36} // 38 8
,{0x06, 0x49, 0x49, 0x29, 0x1e} // 39 9
,{0x00, 0x36, 0x36, 0x00, 0x00} // 3a :
,{0x00, 0x56, 0x36, 0x00, 0x00} // 3b ;
,{0x08, 0x14, 0x22, 0x41, 0x00} // 3c <
,{0x14, 0x14, 0x14, 0x14, 0x14} // 3d =
,{0x00, 0x41, 0x22, 0x14, 0x08} // 3e >
,{0x02, 0x01, 0x51, 0x09, 0x06} // 3f ?
,{0x32, 0x49, 0x79, 0x41, 0x3e} // 40 @
,{0x7e, 0x11, 0x11, 0x11, 0x7e} // 41 A
,{0x7f, 0x49, 0x49, 0x49, 0x36} // 42 B
,{0x3e, 0x41, 0x41, 0x41, 0x22} // 43 C
,{0x7f, 0x41, 0x41, 0x22, 0x1c} // 44 D
,{0x7f, 0x49, 0x49, 0x49, 0x41} // 45 E
,{0x7f, 0x09, 0x09, 0x09, 0x01} // 46 F
,{0x3e, 0x41, 0x49, 0x49, 0x7a} // 47 G
,{0x7f, 0x08, 0x08, 0x08, 0x7f} // 48 H
,{0x00, 0x41, 0x7f, 0x41, 0x00} // 49 I
,{0x20, 0x40, 0x41, 0x3f, 0x01} // 4a J
,{0x7f, 0x08, 0x14, 0x22, 0x41} // 4b K
,{0x7f, 0x40, 0x40, 0x40, 0x40} // 4c L
,{0x7f, 0x02, 0x0c, 0x02, 0x7f} // 4d M
,{0x7f, 0x04, 0x08, 0x10, 0x7f} // 4e N
,{0x3e, 0x41, 0x41, 0x41, 0x3e} // 4f O
,{0x7f, 0x09, 0x09, 0x09, 0x06} // 50 P
,{0x3e, 0x41, 0x51, 0x21, 0x5e} // 51 Q
,{0x7f, 0x09, 0x19, 0x29, 0x46} // 52 R
,{0x46, 0x49, 0x49, 0x49, 0x31} // 53 S
,{0x01, 0x01, 0x7f, 0x01, 0x01} // 54 T
,{0x3f, 0x40, 0x40, 0x40, 0x3f} // 55 U
,{0x1f, 0x20, 0x40, 0x20, 0x1f} // 56 V
,{0x3f, 0x40, 0x38, 0x40, 0x3f} // 57 W
,{0x63, 0x14, 0x08, 0x14, 0x63} // 58 X
,{0x07, 0x08, 0x70, 0x08, 0x07} // 59 Y
,{0x61, 0x51, 0x49, 0x45, 0x43} // 5a Z
,{0x00, 0x7f, 0x41, 0x41, 0x00} // 5b [
,{0x02, 0x04, 0x08, 0x10, 0x20} // 5c ¥
,{0x00, 0x41, 0x41, 0x7f, 0x00} // 5d ]
,{0x04, 0x02, 0x01, 0x02, 0x04} // 5e ^
,{0x40, 0x40, 0x40, 0x40, 0x40} // 5f _
,{0x00, 0x01, 0x02, 0x04, 0x00} // 60 `
,{0x20, 0x54, 0x54, 0x54, 0x78} // 61 a
,{0x7f, 0x48, 0x44, 0x44, 0x38} // 62 b
,{0x38, 0x44, 0x44, 0x44, 0x20} // 63 c
,{0x38, 0x44, 0x44, 0x48, 0x7f} // 64 d
,{0x38, 0x54, 0x54, 0x54, 0x18} // 65 e
,{0x08, 0x7e, 0x09, 0x01, 0x02} // 66 f
,{0x0c, 0x52, 0x52, 0x52, 0x3e} // 67 g
,{0x7f, 0x08, 0x04, 0x04, 0x78} // 68 h
,{0x00, 0x44, 0x7d, 0x40, 0x00} // 69 i
,{0x20, 0x40, 0x44, 0x3d, 0x00} // 6a j 
,{0x7f, 0x10, 0x28, 0x44, 0x00} // 6b k
,{0x00, 0x41, 0x7f, 0x40, 0x00} // 6c l
,{0x7c, 0x04, 0x18, 0x04, 0x78} // 6d m
,{0x7c, 0x08, 0x04, 0x04, 0x78} // 6e n
,{0x38, 0x44, 0x44, 0x44, 0x38} // 6f o
,{0x7c, 0x14, 0x14, 0x14, 0x08} // 70 p
,{0x08, 0x14, 0x14, 0x18, 0x7c} // 71 q
,{0x7c, 0x08, 0x04, 0x04, 0x08} // 72 r
,{0x48, 0x54, 0x54, 0x54, 0x20} // 73 s
,{0x04, 0x3f, 0x44, 0x40, 0x20} // 74 t
,{0x3c, 0x40, 0x40, 0x20, 0x7c} // 75 u
,{0x1c, 0x20, 0x40, 0x20, 0x1c} // 76 v
,{0x3c, 0x40, 0x30, 0x40, 0x3c} // 77 w
,{0x44, 0x28, 0x10, 0x28, 0x44} // 78 x
,{0x0c, 0x50, 0x50, 0x50, 0x3c} // 79 y
,{0x44, 0x64, 0x54, 0x4c, 0x44} // 7a z
,{0x00, 0x08, 0x36, 0x41, 0x00} // 7b {
,{0x00, 0x00, 0x7f, 0x00, 0x00} // 7c |
,{0x00, 0x41, 0x36, 0x08, 0x00} // 7d }
,{0x10, 0x08, 0x08, 0x10, 0x08} // 7e ←
,{0x78, 0x46, 0x41, 0x46, 0x78} // 7f →
};

// Gerador de caracteres dupla altura / dupla largura (só dígitos)
static uint8_t DIGITOS[10][LARG_F*4];

// Comandos de iniciação do display
static const uint8_t __in_flash() lcdInit[] = { 0x21, 0xB0, 0x04, 0x15, 0x20, 0x0C };

// Compando para colocar o cursor no início da tela
static const uint8_t __in_flash() lcdHome[] =  { 0x40, 0x80 };

// Cada byte na memória da tela controla 8 pixels alinhados verticalmente
// Temos duas copias, uma é transferida enquanto a outra é atualizada
static uint8_t screen[2][LCD_DX*LCD_DY/8];
static int screenDMA = 0;  // screen programmed in DMA

// Configuração do SPI
#define BAUD_RATE 4000000   // 4 MHz
#define DATA_BITS 8

// Número do canal de DMA
static int dma_chan;

// Indica que o DMA completou a atualização da tela
static volatile bool screenUpdated = true;

// Esta rotina é executada quando o DMA termina a transferência
static void dma_irq_handler() {
    // Limpa o pedido de interrupção
    dma_hw->ints0 = 1u << dma_chan;
    // Indica que a tela foi atualizada
    screenUpdated = true;
}

// Inicia o DMA
static void initDMA() {
    // Obtem um canal
    dma_chan = dma_claim_unused_channel(true);

    // Configura o canal
   dma_channel_config c = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, spi_get_dreq(SPI_ID, true));
    dma_channel_configure(
        dma_chan,
        &c,
        &spi_get_hw(SPI_ID)->dr,
        &screen[0][0],   
        LCD_DX*LCD_DY/8,      
        false   // Don't start yet.
    );

    // DMA gera IRQ0 ao final da transferência
    dma_channel_set_irq0_enabled(dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_irq_handler);
    irq_set_enabled(DMA_IRQ_0, true);
}

// Gera a fonte dupla altura / dupla largura
static void geraFonteDD() {
    uint8_t orig;
    uint16_t novo;
    for (int i = 0; i <= 9; i++) {
        for (int j = 0; j < LARG_F; j++) {
            orig = ASCII[i+0x10][j];
            novo = 0;
            for (int k = 0; k < 8; k++) {
                novo = novo >> 2;
                if (orig & 1) {
                    novo |= 0xC000;
                }
                orig = orig >> 1;
            }
            DIGITOS[i][2*j] = (uint8_t) (novo & 0xFF);
            DIGITOS[i][2*j+1] = (uint8_t) (novo & 0xFF);
            DIGITOS[i][2*(LARG_F+j)] = (uint8_t) (novo >> 8);
            DIGITOS[i][2*(LARG_F+j)+1] = (uint8_t) (novo >> 8);
        }
    }
}

// Inicia o Display
void displayInit() {
    // Configura os pinos de GPIO
    gpio_init(PIN_SCE);
    gpio_set_dir(PIN_SCE, true);
    gpio_put(PIN_SCE, true);
    gpio_init(PIN_RESET);
    gpio_set_dir(PIN_RESET, true);
    gpio_put(PIN_RESET, true);
    gpio_init(PIN_DC);
    gpio_set_dir(PIN_DC, true);
    gpio_put(PIN_DC, true);

    // Configura o SPI
    uint baud = spi_init (SPI_ID, BAUD_RATE);
    printf ("SPI @ %u Hz\n", baud);
    spi_set_format (SPI_ID, DATA_BITS, SPI_CPOL_1, SPI_CPHA_1, 
                    SPI_MSB_FIRST);

    // Configura os pinos de SPI
    gpio_set_function(PIN_SCLK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SDIN, GPIO_FUNC_SPI);

    // Reseta o controlador do display
    gpio_put(PIN_RESET, false);
    sleep_ms(200);
    gpio_put(PIN_RESET, true);

    // Inicia o controlador do display
    // (Não usa DMA)
    gpio_put(PIN_SCE, false);   // deixa selecionado
    gpio_put(PIN_DC, false);
    spi_write_blocking(SPI_ID, lcdInit, sizeof(lcdInit));
    gpio_put(PIN_DC, true);

    // Prepara o DMA
    initDMA();

    // Inicia a tela
    displayRefresh();

    // Gera a fonte dupla altura / dupla largura
    geraFonteDD();
}

// Atualiza a tela
void displayRefresh() {
    // Garante que a atualização anterior foi concluída
    while (!screenUpdated) {
        tight_loop_contents();
    }
    screenUpdated = false;

    // Muda de buffer
    screenDMA = 1 - screenDMA;

    // Copia o conteudo atual
    memcpy (screen[1 - screenDMA], screen[screenDMA], LCD_DX*LCD_DY/8);

    // Posiona cursor no início da memória (sem DMA)
    gpio_put(PIN_DC, false);
    spi_write_blocking(SPI_ID, lcdHome, sizeof(lcdHome));
    gpio_put(PIN_DC, true);

    // Dispara o DMA
    dma_channel_set_read_addr(dma_chan, screen[screenDMA], true);
}

// Escreve um caracter na tela
// l = linha (0 a 5), c = col (0 a 11)
void displayCar(int l, int c, char car) {
    int pos = (l*LCD_DX) + c*LARG_C;
    uint8_t *ps = &screen[1-screenDMA][pos];
    const uint8_t *pf = ASCII[car-0x20];
    *ps++ = 0x00;
    for (int i = 0; i < LARG_F; i++) {
        *ps++ = *pf++;
    }
    *ps++ = 0x00;
}

// Escreve um string na tela
// l = linha (0 a 5), c = col (0 a 11)
void displayStr(int l, int c, const char *str) {
    while (*str) {
        displayCar (l, c, *str++);
        if (c < 11) {
            c++;
        } else {
            c = 0;
            l++;
        }
    }
}


// Escreve um dígito dupla altura / dupla largura
// l = linha (0 a 4), c = col (0 a 10), dig = digito (0 a 9)
void displayDigDD(int l, int c, char dig) {
    int pos = (l*LCD_DX) + c*LARG_C;
    uint8_t *ps = screen[1-screenDMA]+pos;
    uint8_t *pd = DIGITOS[dig];
    for (int i = 0; i < 2; i++) {
        *ps++ = 0x00;
        *ps++ = 0x00;
        for (int i = 0; i < 2*LARG_F; i++) {
            *ps++ = *pd++;
        }
        *ps++ = 0x00;
        *ps++ = 0x00;
        ps += LCD_DX - 2*LARG_C;
    }
}

// Limpa a tela
void displayClear() {
    memset (screen[1-screenDMA], 0, LCD_DX*LCD_DY/8);
}

