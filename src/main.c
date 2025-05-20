#include "main.h"

int main()
{
    //Declaração de Variáveis
    ssd1306_t ssd;
    bool cor = true;    //Estado LEDs display
    float volume = 2.0;

    //Incialização do sistema
    init_local_def();
    config_i2c_display(&ssd);

    ssd1306_fill(&ssd, !cor); // Limpa o display
    ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
    ssd1306_draw_string(&ssd, "  EMBARCATECH", 5, 15); // Desenha uma string
    ssd1306_draw_string(&ssd, "   RESTIC 37", 5, 29); // Desenha uma string  
    ssd1306_draw_string(&ssd, "    AGUARDE", 5, 43); // Desenha uma string      
    ssd1306_send_data(&ssd); // Atualiza o display
    campainha(volume, 1000, slice_buzzer, buz_A);

    init_remote_def(&ssd);

    while (true)
    {
        int c = getchar_timeout_us(1000); //Fazer leitura da serial
        if(c == '*')
            modo_gravacao();
        
        cyw43_arch_poll(); // Necessário para manter o Wi-Fi ativo

        verif_status(&ssd);

        sleep_ms(500);      // Reduz o uso da CPU
    }

    //Desligar a arquitetura CYW43.
    cyw43_arch_deinit();
    return 0;
}