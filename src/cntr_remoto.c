#include "cntr_remoto.h"

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
void user_request(char **request){

    if (strstr(*request, "GET /blue_on") != NULL)
    {
        gpio_put(LED_B, 1);
    }
    else if (strstr(*request, "GET /blue_off") != NULL)
    {
        gpio_put(LED_B, 0);
    }
    else if (strstr(*request, "GET /green_on") != NULL)
    {
        gpio_put(LED_G, 1);
    }
    else if (strstr(*request, "GET /green_off") != NULL)
    {
        gpio_put(LED_G, 0);
    }
    else if (strstr(*request, "GET /red_on") != NULL)
    {
        gpio_put(LED_R, 1);
    }
    else if (strstr(*request, "GET /red_off") != NULL)
    {
        gpio_put(LED_R, 0);
    }
    else if (strstr(*request, "GET /on") != NULL)
    {
        cyw43_arch_gpio_put(LED_PIN, 1);
    }
    else if (strstr(*request, "GET /off") != NULL)
    {
        cyw43_arch_gpio_put(LED_PIN, 0);
    }
};

// Leitura da temperatura interna
float temp_read(void){
    uint16_t raw_value = 3000;
    const float conversion_factor = 3.3f / (1 << 12);
    float temperature = 27.0f - ((raw_value * conversion_factor) - 0.706f) / 0.001721f;
        return temperature;
}

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
    
    // Leitura da temperatura interna
    float temperature = temp_read();

    // Cria a resposta HTML
    char html[1024];

    // Instruções html do webserver
    snprintf(html, sizeof(html), // Formatar uma string e armazená-la em um buffer de caracteres
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html\r\n"
             "\r\n"
             "<!DOCTYPE html>\n"
             "<html>\n"
             "<head>\n"
             "<title> Embarcatech - LED Control </title>\n"
             "<style>\n"
             "body { background-color: #b5e5fb; font-family: Arial, sans-serif; text-align: center; margin-top: 50px; }\n"
             "h1 { font-size: 64px; margin-bottom: 30px; }\n"
             "button { background-color: LightGray; font-size: 36px; margin: 10px; padding: 20px 40px; border-radius: 10px; }\n"
             ".temperature { font-size: 48px; margin-top: 30px; color: #333; }\n"
             "</style>\n"
             "</head>\n"
             "<body>\n"
             "<h1>Embarcatech: LED Control</h1>\n"
             "<form action=\"./blue_on\"><button>Ligar Azul</button></form>\n"
             "<form action=\"./blue_off\"><button>Desligar Azul</button></form>\n"
             "<form action=\"./green_on\"><button>Ligar Verde</button></form>\n"
             "<form action=\"./green_off\"><button>Desligar Verde</button></form>\n"
             "<form action=\"./red_on\"><button>Ligar Vermelho</button></form>\n"
             "<form action=\"./red_off\"><button>Desligar Vermelho</button></form>\n"
             "<p class=\"temperature\">Temperatura Interna: %.2f &deg;C</p>\n"
             "</body>\n"
             "</html>\n",
             temperature);

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