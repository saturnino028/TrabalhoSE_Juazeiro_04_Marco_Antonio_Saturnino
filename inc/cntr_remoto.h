#ifndef CNTR_REMOTO_H
#define CNTR_REMOTO_H

/***********************  Includes ***********************/
#include <stdio.h>
#include <string.h>  
#include <stdlib.h>  

#include "pico/stdlib.h" 
#include "pico/cyw43_arch.h"

#include "lwip/pbuf.h" 
#include "lwip/tcp.h"  
#include "lwip/netif.h" 

#include "credenciais_wifi.h"
#include "pinout.h"

#include "ssd1306.h"

/******************* Defines e Variaveis Globais ******************/

#define LED_PIN CYW43_WL_GPIO_LED_PIN   // GPIO do CI CYW43 

/******************* Prototipo de Funções *************************/

void init_remote_def(ssd1306_t *ssd);

err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err);

err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

void user_request(char **request);

int start_remote();

#endif //CNTR_REMOTO_H