#include "cntr_local.h"

/********************* Variaveis Globais ************************/

volatile uint32_t passado = 0; //Usada para implementar o debouncing


/***************** Implementação das Funções *********************/
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

/**
 * Coloca o Pico no modo gravação
 */
void modo_gravacao()
{    
    printf("Entrando no modo de gravacao...\n");
    reset_usb_boot(0, 0); 
}

/**
 * @brief trata a interrupção gerada pelos botões A e B da BitDog
 * @param gpio recebe o pino que gerou a interrupção
 * @param events recebe o evento que causou a interrupção
 */
void botoes_callback(uint gpio, uint32_t events)
{
    printf("Interrupcao");
    // Obtém o tempo atual em microssegundos
    uint32_t agora = to_us_since_boot(get_absolute_time());
    // Verifica se passou tempo suficiente desde o último evento
    if (agora - passado > 500000) // 500 ms de debouncing
    {
        passado  = agora;
        if(gpio == bot_A)
        {

        }
        else if(gpio == bot_B)
            modo_gravacao();
    }
}