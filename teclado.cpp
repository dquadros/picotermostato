/**
 * @file teclado.cpp
 * @author Daniel Quadros
 * @brief Tratamento de teclas
 * @version 0.1
 * @date 2022-11-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdlib.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "picotermostato.h"

#define DEBOUNCE_MS 200
#define T_FILA 5

// variáveis locais
static TECLA *teclas;
static int nteclas;
static struct repeating_timer timer;
static int fila[T_FILA];
static volatile int poe, tira;

// Teste periódigo das teclas
static bool testaTeclas(struct repeating_timer *t) {
    for (int i = 0; i < nteclas; i++) {
        int pin = teclas[i].gpio;
        bool atual = ! gpio_get (pin);
        if (atual == teclas[i].apertada) {
            // Mantem o estado atual
            teclas[i].cnt_debounce = 0;
        } else if (teclas[i].cnt_debounce == 0) {
            // Mudou, inicia a contagem de debounce
            teclas[i].cnt_debounce = DEBOUNCE_MS/10;
        } else if (--teclas[i].cnt_debounce == 0) {
            // Validou a mudança de estado
            teclas[i].apertada = atual;
            if (atual) {
                // Coloca na fila quando aperta
                int prox = (poe + 1) % T_FILA;
                if (prox != tira) {
                    fila[poe] = i;
                    poe = prox;
                } else {
                    // fila cheia, ignora
                }
            }
        }
    }
    return true; // continuar chamando periodicamente
}

// iniciação do módulo
void tecInit (TECLA *tec, int n) {
    teclas = tec;
    nteclas = n;
    for (int i = 0; i < nteclas; i++) {
        int pin = teclas[i].gpio;
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        gpio_pull_up(pin);
        teclas[i].apertada = false;
        teclas[i].cnt_debounce = 0;
    }
    poe = tira = 0; // inicia a fila
    add_repeating_timer_ms(10, testaTeclas, NULL, &timer);
}

// pega próxima tecla da fila, retorna -1 se fila vazia
int tecLe () {
    if (tira == poe) {
        return -1;
    }
    int tecla = fila[tira];
    tira = (tira + 1) % T_FILA;
    return tecla;
}
