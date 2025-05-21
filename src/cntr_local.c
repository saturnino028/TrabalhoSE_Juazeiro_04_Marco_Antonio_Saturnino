#include "cntr_local.h"

/********************* Variaveis Globais ************************/

volatile uint32_t passado = 0; //Usada para implementar o debouncing

volatile uint16_t top_wrap = 1000; //Define um top wrap padrão

volatile float temperature, temperature_ant = 0.0;
volatile uint8_t velocidade_ventoinha = 20;
volatile float volume_buz = 2.0;

volatile bool flag_de_parada = 0;

volatile uint slice_buzzer;
volatile uint slice[3];

typedef struct 
{
    uint _slice;
    uint8_t _pin;
} def_canais_pwm;

def_canais_pwm dados;

/***************** Implementação das Funções *********************/
/**
 * @brief configura as definições locais - inicia
 */
void init_local_def()
{
    set_sys_clock_khz(1250000,false); //Cofigura o clock
    stdio_init_all();
    config_pins_gpio();

    adc_init();
    adc_set_temp_sensor_enabled(true);

    slice_buzzer = config_pwm(buz_A, 1000);

    slice[R] = config_pwm(LED_R, 1000);
    slice[G] = config_pwm(LED_G, 1000);
    slice[B] = config_pwm(LED_B, 1000);

    gpio_set_irq_enabled_with_callback(bot_A, GPIO_IRQ_EDGE_FALL, true, &botoes_callback);
    gpio_set_irq_enabled_with_callback(bot_B, GPIO_IRQ_EDGE_FALL, true, &botoes_callback);
}

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
 * @brief Configura os pinos PWM
 */
uint config_pwm(uint8_t _pin, uint16_t _freq_Hz)
{
    uint slice; float Fpwm;
    top_wrap = 1000000/_freq_Hz;
    gpio_set_function(_pin, GPIO_FUNC_PWM); //Habilita a função PWM
    slice = pwm_gpio_to_slice_num(_pin);//Obter o valor do slice correspondente ao pino
    pwm_set_clkdiv(slice, 125.0); //Define o divisor de clock
    pwm_set_wrap(slice, top_wrap); //Define valor do wrap
    Fpwm = 125000000/(125.0*top_wrap);
    printf("PWM definido para %.2f Hz\n", Fpwm);
    return slice; //Retorna o slice correspondente
}

/**
 * @brief ajusta o duty cicle
 */
void duty_cicle(float _percent, uint _slice, uint8_t _pin)
{
    pwm_set_enabled(_slice, false); //Desabilita PWM
    uint16_t valor_pwm = (_percent/100)*top_wrap; //Configura DutyCicle
    pwm_set_gpio_level(_pin, valor_pwm); //Configura DutyCicle
    pwm_set_enabled(_slice, true); //Habilitar PWM
}

/**
 * @brief função para som no buzzer
 */
void campainha(float _dc, uint32_t _duracao_ms, uint _slice, uint8_t _pin)
{
    duty_cicle(_dc, _slice, _pin);
    dados._slice = _slice;
    dados._pin = _pin;
    add_alarm_in_ms(_duracao_ms, fim_campainha, &dados, false);
}

/**
 * @brief função de callback para desativar a campainha
 */
int64_t fim_campainha(alarm_id_t id, void *user_data)
{
    def_canais_pwm *data = (def_canais_pwm *)user_data;
    uint _slice = data -> _slice;
    uint8_t _pin = data -> _pin;
    duty_cicle(0.0, _slice, _pin);
    return 0;
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
            flag_de_parada = 0;
        else if(gpio == bot_B)
            flag_de_parada = 1;
    }
}

// Leitura da temperatura interna
void temp_read(void)
{
    adc_select_input(4);
    uint16_t raw_value = adc_read();
    const float conversion_factor = 3.3f / (1 << 12);
    temperature = 27.0f - ((raw_value * conversion_factor) - 0.706f) / 0.001721f;
}

void verif_status(ssd1306_t *ssd)
{
    temp_read();

    if(temperature != temperature_ant)
    {
        char buffer[17]; // Tamanho suficiente para "IP" + endereço IP
        bool cor = true;
        temperature_ant = temperature;

        if (netif_default) // Caso seja a interface de rede padrão - imprimir o IP do dispositivo.
            sprintf(buffer, "IP %s", ipaddr_ntoa(&netif_default->ip_addr));
        else
            sprintf(buffer, "%s", "Nao Conectado");

        ssd1306_fill(ssd, !cor); // Limpa o display
        ssd1306_rect(ssd, 3, 3, 122, 60, cor, !cor); // Desenha um retângulo
        ssd1306_line(ssd, 3, 16, 123, 16, cor); // Desenha uma linha
        ssd1306_line(ssd, 3, 37, 123, 37, cor); // Desenha uma linha   
        ssd1306_draw_string(ssd, " SUPERVISORIO", 5, 6); // Desenha uma string
        ssd1306_draw_string(ssd, buffer, 4, 18); // Desenha uma string
        ssd1306_draw_string(ssd, "Porta 80", 5, 28); // Desenha uma string 
        ssd1306_draw_string(ssd, "C    V    ", 20, 41); // Desenha uma string
        ssd1306_line(ssd, 44, 37, 44, 60, cor); // Desenha uma linha vertical 
        ssd1306_line(ssd, 84, 37, 84, 60, cor); // Desenha uma linha vertical   
        
        if (temperature < 35.0) //Temperatura Normal até 35°C
        {
            velocidade_ventoinha = 20;

            duty_cicle(0, slice[R], LED_R);
            duty_cicle(velocidade_ventoinha, slice[G], LED_G);
            duty_cicle(0, slice[B], LED_B);

            sprintf(buffer, "%0.1f", temperature);
            ssd1306_draw_string(ssd, buffer, 8, 52); // Desenha uma string  
            sprintf(buffer, "_%d", velocidade_ventoinha);
            ssd1306_draw_string(ssd, buffer, 49, 52); // Desenha uma string
            ssd1306_draw_string(ssd, "NOR", 95, 48);

        }
        else if(temperature < 45.0) //Temperatura Alta Até 45°C
        {
            velocidade_ventoinha =70;

            duty_cicle(0, slice[R], LED_R);
            duty_cicle(0, slice[G], LED_G);
            duty_cicle(velocidade_ventoinha, slice[B], LED_B);

            sprintf(buffer, "%0.1f", temperature);
            ssd1306_draw_string(ssd, buffer, 8, 52); // Desenha uma string  
            sprintf(buffer, "_%d", velocidade_ventoinha);
            ssd1306_draw_string(ssd, buffer, 49, 52); // Desenha uma string
            ssd1306_draw_string(ssd, "MED", 95, 48);
        }
        else if(temperature < 60.0) //Temperatura Muito Alta Superior a 45°
        {
            velocidade_ventoinha = 100;

            duty_cicle(velocidade_ventoinha, slice[R], LED_R);
            duty_cicle(0, slice[G], LED_G);
            duty_cicle(0, slice[B], LED_B);

            sprintf(buffer, "%0.1f", temperature);
            ssd1306_draw_string(ssd, buffer, 8, 52); // Desenha uma string  
            sprintf(buffer, "_%d", velocidade_ventoinha);
            ssd1306_draw_string(ssd, buffer, 49, 52); // Desenha uma string
            ssd1306_draw_string(ssd, "ALT", 95, 48);

            campainha(volume_buz, 2000, slice_buzzer, buz_A);
        }
        else //Temperatura Maior que 60°C Modo de proteção
        {
            duty_cicle(0, slice[G], LED_G);
            duty_cicle(0, slice[B], LED_B);

            ssd1306_draw_string(ssd, "OVR", 8, 52); // Desenha uma string  
            ssd1306_draw_string(ssd, "STP", 49, 52); // Desenha uma string
            ssd1306_draw_string(ssd, "OFF", 95, 48);

            for(int i = 100.0; i>-1; i--)
            {
                duty_cicle(i, slice[R], LED_R); //Desligando a ventoinha
                sleep_ms(30);
            }
        }
        ssd1306_send_data(ssd);
    }
}