#ifndef CNTR_LOCAL_H
#define CNTR_LOCAL_H

/***********************  Includes ***********************/
#include "hardware/pwm.h"
#include "pico/bootrom.h"
#include "cntr_remoto.h"
#include "pinout.h"

/***********************  Defines ***********************/
#define R 0
#define G 1
#define B 2

/******************* Prototipo de Funções *************************/
void config_pins_gpio();
uint config_pwm(uint8_t _pin, uint16_t _freq_Hz);
void duty_cicle(float _percent, uint _slice, uint8_t _pin);
void campainha(float _dc, uint32_t _duracao_ms, uint _slice, uint8_t _pin);
int64_t fim_campainha(alarm_id_t id, void *user_data);

void modo_gravacao();
void botoes_callback(uint gpio, uint32_t events);

#endif //CNTR_LOCAL_H