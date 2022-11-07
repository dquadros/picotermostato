/**
 * @file picotermostato.cpp
 * @author Daniel Quadros
 * @brief  Implementação simples de um termostato
 *         O objetivo é exercitar os recursos do RP2040
 *         *** NÃO É UMA APLICAÇÃO PARA USO REAL ***
 * @version 0.1
 * @date 2022-11-03
 * 
 * @copyright Copyright (c) 2022, Daniel Quadros
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "picotermostato.h"

// Controles do termostato
// Num projeto real tempLiga e tempDesliga seriam salvos em memória não volátil
static int tempAtual = 20;
static int tempLiga = 22;
static int tempDesliga = 25;
static bool ligado = false;

static critical_section_t critTemp;

// Definição das teclas
static TECLA teclas[] = {
     { PIN_TEC_UP, false, 0 },
     { PIN_TEC_DN, false, 0 },  
     { PIN_TEC_ENTER, false, 0 }
};
#define N_TECLAS (sizeof(teclas)/sizeof (TECLA))
#define TECLA_UP    0
#define TECLA_DN    1
#define TECLA_ENTER 2

// Campos durante a configuração
#define CPO_NENHUM  0
#define CPO_LIGA    1
#define CPO_DESLIGA 2


// Atualiza a tela
static void atualizaTela(int cpo) {
    displayClear();
    displayStr(0,0, "Atual");
    displayDigDD(0, 6, tempAtual / 10);
    displayDigDD(0, 8, tempAtual % 10);
    if (ligado) {
        displayCar(0, 11, '*');
    }
    switch (cpo) {
        case CPO_NENHUM:
            displayStr(3,0, "Liga Desliga");
            break;
        case CPO_LIGA:
            displayStr(3,0, "LIGA Desliga");
            break;
        case CPO_DESLIGA:
            displayStr(3,0, "Liga DESLIGA");
            break;
    }
    displayDigDD(4, 0, tempLiga / 10);
    displayDigDD(4, 2, tempLiga % 10);
    displayDigDD(4, 5, tempDesliga / 10);
    displayDigDD(4, 7, tempDesliga % 10);
    displayRefresh();
}

// Lógica do termostato
static void termostato() {
    while (true) {
        // Provavelmente um exagero usar critical_section nesse
        // caso, mas vamos pela segurança
        int tempNova = sensorLe();
        critical_section_enter_blocking(&critTemp);
        tempAtual = tempNova;
        critical_section_exit(&critTemp);

        // Aciona ou desaciona o rele conforme necessário
        bool ligarRele = ligado;
        if (tempAtual < tempLiga) {
            ligarRele = true;
        } else if (tempAtual > tempDesliga) {
            ligarRele = false;;
        }
        if (ligarRele != ligado) {
            gpio_put(PIN_RELE, ligarRele);
            ligado = ligarRele;
        }
    }
}

// Programa principal
int main() {
    int tempAnt;

    // Inicia rele
    gpio_init(PIN_RELE);
    gpio_set_dir(PIN_RELE, true);
    gpio_put(PIN_RELE, false);

    // Inicia stdio para debug
    stdio_init_all();

    // Inicia display
    displayInit();
    displayStr(0,0, "DQSoft 1.00");
    displayStr(2,0, "Termostato");
    displayRefresh();

    // Inicia teclado
    tecInit(teclas, N_TECLAS);

    // Inicia Sensores
    critical_section_init(&critTemp);
    sensorInit();
    tempAnt = tempAtual = sensorLe();

    // Inicia a tela
    int cpo = CPO_NENHUM;
    atualizaTela (cpo);

    // Lógica do termostato roda no outro core
    multicore_launch_core1 (termostato);

    // Laço principal (core 0)
    while (true) {
        // Trata teclado
        int tec = tecLe();
        if (cpo == CPO_NENHUM) {
            if (tec == TECLA_ENTER) {
                cpo = CPO_LIGA;     // entra na configuração
                atualizaTela(cpo);
            } else {  // ignora outras teclas fora da configuração
                int tempNova;
                critical_section_enter_blocking(&critTemp);
                tempNova = tempAtual;
                critical_section_exit(&critTemp);
                if (tempAnt != tempNova) {
                    // Atualiza temperatura
                    atualizaTela(cpo);
                    tempAnt = tempNova;
                }
            } 
        } else if (tec != -1) {
            int *pVal = (cpo == CPO_LIGA) ? &tempLiga : &tempDesliga;
            int valMin = (cpo == CPO_LIGA) ? 0 : tempLiga+1;
            int valMax = (cpo == CPO_LIGA) ? tempDesliga-1 : 99;
            switch (tec) {
                case TECLA_UP:
                    if (*pVal < valMax) {
                        (*pVal)++;
                    }
                    break;
                case TECLA_DN:
                    if (*pVal > valMin) {
                        (*pVal)--;
                    }
                    break;
                case TECLA_ENTER:
                    cpo = (cpo == CPO_LIGA)? CPO_DESLIGA : CPO_NENHUM;
                    break;
            }
            atualizaTela(cpo);
        }
        sleep_ms(50);
    }
}
