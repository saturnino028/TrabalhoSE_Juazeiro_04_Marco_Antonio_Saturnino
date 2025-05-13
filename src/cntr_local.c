#include "cntr_local.h"

//Funções de configuração
/**
 * @brief inicia os pinos de GPIO
 */
void config_pins_gpio()
{
    //Configuração do botao A
    gpio_init(bot_A);
    gpio_pull_up(bot_A);
    gpio_set_dir(bot_A, GPIO_IN);

    //Configuração do botao B
    gpio_init(bot_B);
    gpio_pull_up(bot_B);
    gpio_set_dir(bot_B, GPIO_IN);

    //Configuração do LED vermelho
    gpio_init(LED_R);
    gpio_set_dir(LED_R, GPIO_OUT);

    //Configuração do LED verde
    gpio_init(LED_G);
    gpio_set_dir(LED_G, GPIO_OUT);

    //Configuração do LED azul
    gpio_init(LED_B);
    gpio_set_dir(LED_B, GPIO_OUT);
}