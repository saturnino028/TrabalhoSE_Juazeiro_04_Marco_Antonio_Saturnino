#include "main.h"

int main()
{
    //Declaração de Variáveis
    ssd1306_t ssd;
    bool cor = true;    //Estado LEDs display

    //Incialização do sistema
    stdio_init_all();
    config_pins_gpio();
    config_i2c_display(&ssd);
    start_remote();

    while (true)
    {
        /* 
        * Efetuar o processamento exigido pelo cyw43_driver ou pela stack TCP/IP.
        * Este método deve ser chamado periodicamente a partir do ciclo principal 
        * quando se utiliza um estilo de sondagem pico_cyw43_arch 
        */
        cyw43_arch_poll(); // Necessário para manter o Wi-Fi ativo
        sleep_ms(100);      // Reduz o uso da CPU
    }

    //Desligar a arquitetura CYW43.
    cyw43_arch_deinit();
    return 0;
}