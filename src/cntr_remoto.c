#include "cntr_remoto.h"
#include "cntr_local.h"

/********************* Variaveis Globais ************************/


/***************** Implementação das Funções *********************/
/**
 * @brief inicializar servidor remoto
 */
void init_remote_def(ssd1306_t *ssd)
{    
    bool cor = true;    //Estado LEDs display
    float volume = 2.0; //Volume do buzzer
    if(!start_remote())
    {
        campainha(volume, 1000, slice_buzzer, buz_A);
        duty_cicle(100,slice[G], LED_G);
    }
    else
    {
        campainha(volume, 1000, slice_buzzer, buz_A);
        ssd1306_fill(ssd, !cor); // Limpa o display
        ssd1306_draw_string(ssd, "Erro de Conex.", 5, 15); // Desenha uma string
        ssd1306_draw_string(ssd, "Verifique.", 5, 29);  
        ssd1306_draw_string(ssd, "Apenas Local", 5, 43); // Desenha uma string  
        ssd1306_send_data(ssd); // Atualiza o display
        duty_cicle(100, slice[R], LED_R);
        sleep_ms(3000);
    }
    
}

/**
 * @brief Inicializa o sistema remoto (webserver)
 * @return retorna 0 se tudo certo, código de erro se algo deu errado
 */
int start_remote()
{
    // Inicializa a arquitetura do CYW43
    if (cyw43_arch_init() != 0)
    {
        for (int i = 0; i < 15; i++)
        {
            gpio_put(LED_R, 1);
            sleep_ms(100);
            gpio_put(LED_R, 0);
            sleep_ms(100);
        }
        return 1; // Falha na inicialização
    }

    // Ativa Wi-Fi no modo Station
    cyw43_arch_enable_sta_mode();

    // Conecta ao Wi-Fi com timeout
    if (!cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 20000))
    {
        gpio_put(LED_G, 1); // Conectado com sucesso
    }
    else
    {
        for (int i = 0; i < 15; i++)
        {
            gpio_put(LED_R, 1);
            sleep_ms(100);
            gpio_put(LED_R, 0);
            sleep_ms(100);
        }
        return 1; // Falha na conexão Wi-Fi
    }

    // Cria o servidor TCP
    struct tcp_pcb *server = tcp_new();
    if (!server)
    {
        for (int i = 0; i < 15; i++)
        {
            gpio_put(LED_R, 1);
            sleep_ms(100);
            gpio_put(LED_R, 0);
            sleep_ms(100);
        }
        return 1; // Falha ao criar servidor
    }

    // Vincula o servidor à porta 80
    if (tcp_bind(server, IP_ADDR_ANY, 80) != ERR_OK)
    {
        for (int i = 0; i < 15; i++)
        {
            gpio_put(LED_R, 1);
            sleep_ms(100);
            gpio_put(LED_R, 0);
            sleep_ms(100);
        }
        return 1; // Falha no bind
    }

    // Coloca o servidor para escutar conexões
    server = tcp_listen(server);

    // Define função de callback para conexões TCP
    tcp_accept(server, tcp_server_accept);

    return 0; // Tudo certo
}


// Tratamento do request do usuário - digite aqui
void user_request(char **request)
{
    if (strstr(*request, "GET /sis_on") != NULL)
    {
        campainha(2.0, 100, slice_buzzer, buz_A);
    }
    else if (strstr(*request, "GET /sis_off") != NULL)
    {
        campainha(2.0, 500, slice_buzzer, buz_A);
    }
};

// Função de callback para processar requisições HTTP
err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (!p)
    {
        tcp_close(tpcb);
        tcp_recv(tpcb, NULL);
        return ERR_OK;
    }

    // Alocação do request na memória dinámica
    char *request = (char *)malloc(p->len + 1);
    memcpy(request, p->payload, p->len);
    request[p->len] = '\0';

    printf("Request: %s\n", request);

    // Tratamento de request - Controle dos LEDs
    user_request(&request);

    // Cria a resposta HTML
    char html[1024];

    // Instruções html do webserver
    snprintf
    (
        html, sizeof(html), // Formatar uma string e armazená-la em um buffer de caracteres
        
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "\r\n"

        "<!DOCTYPE html>\n"

        "<html>\n"
            "<head>\n"
                "<title>Embarcatech</title>\n"
            "</head>\n"
            "<body style='background-color:black; color:white; text-align:center; font-family:sans-serif;'>\n"
                "<h1>Sistema de Monitoramento de Ativos</h1>\n" 

                "<div style='margin-top:40px;'>\n"
                "  <div><strong>Temperatura:</strong></div>\n"
                "  <div style='font-size:28px;'>%.1f &ordm;C</div>\n"
                "</div>\n"

                "<div style='margin-top:30px;'>\n"
                "  <strong>EMBARCATECH</strong>\n"
                "</div>\n"

                "<div style='margin-top:30px;'>\n"
                "  <div><strong>Ventoinha:</strong></div>\n"
                "  <div style='font-size:28px;'>%d RPM</div>\n"
                "</div>\n"

                "<div style='margin-top:40px;'>\n"
                "  <form action=\"./sis_on\" method=\"get\" style='display:inline-block; margin:10px;'>\n"
                "    <button style='font-size:20px; padding:10px 20px;'>Ligar</button>\n"
                "  </form>\n"
                "  <form action=\"./sis_off\" method=\"get\" style='display:inline-block; margin:10px;'>\n"
                "    <button style='font-size:20px; padding:10px 20px;'>Desligar</button>\n"
                "  </form>\n"
                "</div>\n"

            "</body>\n"
        "</html>\n",

        temperature, velocidade_ventoinha
    );

    // Escreve dados para envio (mas não os envia imediatamente).
    tcp_write(tpcb, html, strlen(html), TCP_WRITE_FLAG_COPY);

    // Envia a mensagem
    tcp_output(tpcb);

    //libera memória alocada dinamicamente
    free(request);
    
    //libera um buffer de pacote (pbuf) que foi alocado anteriormente
    pbuf_free(p);

    return ERR_OK;
}



// Função de callback ao aceitar conexões TCP
err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    tcp_recv(newpcb, tcp_server_recv);
    return ERR_OK;
}