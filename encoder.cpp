/**
 * @file encoder.cpp
 * @author Daniel Quadros
 * @brief Tratamento do rotary encoder (com botão)
 * @version 0.1
 * @date 2022-11-16
 * 
 * Esta é uma versão simplificada do código em
 * https://github.com/pimoroni/pimoroni-pico/tree/main/drivers/encoder
 *  
 * Aqui estamos interessados apenas em gerar "teclas" UP ou DOWN conforme
 * o eixo for movido em sendio horário ou anti-horário.
 * ver http://dqsoft.blogspot.com/2020/07/usando-um-rotary-encoder.html
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdlib.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/clocks.h"
#include "encoder.pio.h"

#include "picotermostato.h"

#define DEBOUNCE_MS 100
#define T_FILA 32

// Definicoes da codificacao do estado

static const uint32_t STATE_A_MASK      = 0x80000000;
static const uint32_t STATE_B_MASK      = 0x40000000;
static const uint32_t STATE_A_LAST_MASK = 0x20000000;
static const uint32_t STATE_B_LAST_MASK = 0x10000000;

static const uint32_t STATES_MASK = STATE_A_MASK | STATE_B_MASK |
                                    STATE_A_LAST_MASK | STATE_B_LAST_MASK;

static const uint32_t TIME_MASK   = 0x0fffffff;

#define LAST_STATE(state)  ((state) & 0b0011)
#define CURR_STATE(state)  (((state) & 0b1100) >> 2)

// Controle da movimentacao

enum StepDir {
    NO_DIR    = 0,
    INCREASING = 1,
    DECREASING = -1,
};

enum MicroStep : uint8_t {
    MICROSTEP_0 = 0b00,
    MICROSTEP_1 = 0b10,
    MICROSTEP_2 = 0b11,
    MICROSTEP_3 = 0b01,
};

// variáveis locais
static struct repeating_timer timer;

static PIO enc_pio;
static uint enc_sm;
static uint enc_pin_a, enc_pin_b, enc_pin_sw;

static volatile bool enc_state_a = false;
static volatile bool enc_state_b = false;
static volatile bool enc_sw_apertado = false;
static volatile int enc_cnt_debounce = 0;

static int fila[T_FILA];
static volatile int poe, tira;

// coloca tecla na fila
static inline void poeTecla(int tecla) {
    int prox = (poe + 1) % T_FILA;
    if (prox != tira) {
        fila[poe] = tecla;
        poe = prox;
    } else {
        // fila cheia, ignora
    }
}

// Teste periódigo das teclas
static bool testaBotao(struct repeating_timer *t) {
        bool atual = ! gpio_get (enc_pin_sw);
        if (atual == enc_sw_apertado) {
            // Mantem o estado atual
            enc_cnt_debounce = 0;
        } else {
            if (enc_cnt_debounce == 0) {
                // Mudou, inicia a contagem de debounce
                enc_cnt_debounce = DEBOUNCE_MS/10;
            } else if (--enc_cnt_debounce == 0) {
                // Validou a mudança de estado
                enc_sw_apertado = atual;
            if (atual) {
                // Coloca na fila quando aperta
                poeTecla (TECLA_ENTER);
            }
        }
    }
    return true; // continuar chamando periodicamente
}

// Trata a interrupção da PIO
static void pio_interrupt_handler() {
    StepDir step;

    // Trata os dados na fila de recepcao
    while(enc_pio->ints1 & (PIO_IRQ1_INTS_SM0_RXNEMPTY_BITS << enc_sm)) {
        uint32_t received = pio_sm_get(enc_pio, enc_sm);

        // Extrai o estado atual e anterior do valor retirado da fila
        enc_state_a = (bool)(received & STATE_A_MASK);
        enc_state_b = (bool)(received & STATE_B_MASK);
        uint8_t states = (received & STATES_MASK) >> 28;

        step = NO_DIR;

        // Trata o passo, so nos interessam dois casos
        if ((LAST_STATE(states) == MICROSTEP_0) && (CURR_STATE(states) == MICROSTEP_1)) {
            // A ____|‾‾‾‾
            // B _________
            step = INCREASING;
        } else if ((LAST_STATE(states) == MICROSTEP_3) && (CURR_STATE(states) == MICROSTEP_2)) {
            // A ____|‾‾‾‾
            // B ‾‾‾‾‾‾‾‾‾
            step = DECREASING;
        }
        
        if (step != NO_DIR) {
            // Gera tecla UP or DOWN
            poeTecla(step == INCREASING? TECLA_UP: TECLA_DN);
        }
    }    
}


// iniciação do módulo
void encoderInit (PIO pio, uint pin_a, uint pin_b, uint pin_sw) {
    // Salva parametros
    enc_pio = pio;
    enc_pin_a = pin_a;
    enc_pin_b = pin_b;
    enc_pin_sw = pin_sw;

    // Inicia a fila
    poe = tira = 0; 

    // Inicia o botão
    gpio_init(pin_sw);
    gpio_set_dir(pin_sw, GPIO_IN);
    gpio_pull_up(pin_sw);
    enc_sw_apertado = false;
    enc_cnt_debounce = 0;
    add_repeating_timer_ms(10, testaBotao, NULL, &timer);

    // Aloca uma maquina de estado
    enc_sm = pio_claim_unused_sm(pio, true);

    // Carrega o programa
    uint offset = pio_add_program(pio, &encoder_program);

    // Inicia os pinos conectados ao encoder
    pio_gpio_init(pio, enc_pin_a);
    pio_gpio_init(pio, enc_pin_b);
    gpio_pull_up(enc_pin_a);
    gpio_pull_up(enc_pin_b);
    pio_sm_set_consecutive_pindirs(pio, enc_sm, enc_pin_a, 1, false);
    pio_sm_set_consecutive_pindirs(pio, enc_sm, enc_pin_b, 1, false);

    // Configura a maquina de estado
    pio_sm_config c = encoder_program_get_default_config(offset);
    sm_config_set_jmp_pin(&c, enc_pin_a);
    sm_config_set_in_pins(&c, enc_pin_b);
    sm_config_set_in_shift(&c, false, false, 1);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);
    sm_config_set_clkdiv_int_frac(&c, 250, 0);
    pio_sm_init(pio, enc_sm, offset, &c);

    // Configura a interrupcao
    hw_set_bits(&pio->inte1, PIO_IRQ1_INTE_SM0_RXNEMPTY_BITS << enc_sm);
    if(pio_get_index(pio) == 0) {
        irq_add_shared_handler(PIO0_IRQ_1, pio_interrupt_handler, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
        irq_set_enabled(PIO0_IRQ_1, true);
    } else {
        irq_add_shared_handler(PIO1_IRQ_1, pio_interrupt_handler, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
        irq_set_enabled(PIO1_IRQ_1, true);
    }

    // Inicia o estado
    enc_state_a = gpio_get(enc_pin_a);
    enc_state_b = gpio_get(enc_pin_b);

    // Inicia o registrador X, executando a instrução "SET X,state"
    pio_sm_exec(pio, enc_sm, pio_encode_set(pio_x, (uint)enc_state_a << 1 | (uint)enc_state_b));

    // Dispara a execucao da maquina de estado
    pio_sm_set_enabled(pio, enc_sm, true);    
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
