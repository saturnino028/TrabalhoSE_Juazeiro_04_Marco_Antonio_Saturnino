#ifndef CNTR_LOCAL_H
#define CNTR_LOCAL_H

/***********************  Includes ***********************/
#include "pico/bootrom.h"
#include "cntr_remoto.h"
#include "pinout.h"

/***********************  Defines ***********************/

/******************* Prototipo de Funções *************************/
void config_pins_gpio();
void botoes_callback(uint gpio, uint32_t events);

#endif //CNTR_LOCAL_H