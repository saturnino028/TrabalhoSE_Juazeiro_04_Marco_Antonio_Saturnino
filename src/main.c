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

    ssd1306_fill(&ssd, !cor); // Limpa o display
    ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
    ssd1306_draw_string(&ssd, "  EMBARCATECH", 5, 15); // Desenha uma string
    ssd1306_draw_string(&ssd, "   RESTIC 37", 5, 29); // Desenha uma string  
    ssd1306_draw_string(&ssd, "    AGUARDE", 5, 43); // Desenha uma string      
    ssd1306_send_data(&ssd); // Atualiza o display
    
    if(!start_remote())
    {
        if (netif_default) // Caso seja a interface de rede padrão - imprimir o IP do dispositivo.
        {
            ssd1306_fill(&ssd, !cor); // Limpa o display
            ssd1306_draw_string(&ssd, "Conectado no IP", 5, 13); // Desenha uma string
            ssd1306_draw_string(&ssd, ipaddr_ntoa(&netif_default->ip_addr), 15, 35);  
            ssd1306_draw_string(&ssd, "    Porta 80", 5, 52); // Desenha uma string  
            ssd1306_send_data(&ssd); // Atualiza o display
        }
    }
    else
    {
        ssd1306_fill(&ssd, !cor); // Limpa o display
        ssd1306_draw_string(&ssd, "Erro de Conex.", 5, 15); // Desenha uma string
        ssd1306_draw_string(&ssd, "Verifique.", 5, 29);  
        ssd1306_draw_string(&ssd, "Apenas Local", 5, 43); // Desenha uma string  
        ssd1306_send_data(&ssd); // Atualiza o display
        gpio_put(LED_R, 1);
        gpio_put(LED_G, 0);
        gpio_put(LED_B, 0);
    }

    while (true)
    {
        int c = getchar_timeout_us(1000); //Fazer leitura da serial
        if(c == '*')
            modo_gravacao();
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