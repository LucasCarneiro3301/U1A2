#include "lib/config/config.h"
#include "lib/ws2812/ws2812.h"
#include "lib/cyw43/cyw43.h"
#include "lib/utils/utils.h"

#include <stdio.h>               // Biblioteca padrão para entrada e saída
#include <string.h>              // Biblioteca manipular strings
#include <stdlib.h>              // funções para realizar várias operações, incluindo alocação de memória dinâmica (malloc)

volatile uint8_t mode = 0;              // 0 (IDLE), 1 (HEAT), 2 (COLD)
volatile uint8_t status = 1;            // 0 (STOP), 1 (NORMAL), 2 (ALERT), 3 (DANGER)
volatile uint16_t pwm = 625;
volatile uint32_t last_time = 0;        // Armazena o tempo do último evento (em microssegundos)
volatile float tempI = 0.0;
volatile float tempA = 0.0;

// Frequências das notas musicais (em Hz)
enum Notes {
    DO = 2640, // Dó
    RE = 2970, // Ré
    MI = 3300, // Mi
    FA = 3520, // Fá
    SOL = 3960, // Sol
    LA = 4400, // Lá
    SI = 4950,  // Si
    DO_ALTO = 5280,  // Dó uma oitava acima (C5)
    LA_BAIXO = 880
};

int setup_tcp_server(void);                                                                 // Função de callback para configurar e iniciar o servidor TCP
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err);               // Função de callback ao aceitar conexões TCP
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);   // Função de callback para processar requisições HTTP
void user_request(char **request);                                                          // Tratamento do request do usuário
void gpio_irq_handler(uint gpio, uint32_t events);                                          // Prototipação da função de interrupção

// Retorna a leitura de um determinado canal ADC
uint16_t select_adc_channel(unsigned short int channel) {
    adc_select_input(channel);
    return adc_read();
}

int main()
{
    PIO pio = pio0;
    int sm = 0;
    ssd1306_t ssd;
    bool cor = true, control = true;
    char str[16];
    uint8_t last_stt = 0;

    init(pio, sm, &ssd);    // Inicializa e configura os componentes

    symbol('*');                                    // Desliga a matriz de LEDs
    ssd1306_fill(&ssd, false);  					// Limpa o display
    ssd1306_draw_string(&ssd, "AGUARDE", 28, 28);	// Desenha uma string 
    ssd1306_draw_string(&ssd, "A CONEXAO...", 24, 40);	// Desenha uma string
    ssd1306_send_data(&ssd);    					// Atualiza o display

    // Inicializa e configura o CI CYW43
    if(cyw43_setup() != 1) {
        ssd1306_fill(&ssd, false);  					// Limpa o display
        ssd1306_draw_string(&ssd, "CONEXAO", 28, 28);	// Desenha uma string 
        ssd1306_draw_string(&ssd, "MAL SUCEDIDA", 24, 40);	// Desenha uma string
        ssd1306_send_data(&ssd);    					// Atualiza o display

        return -1;
    }

    // Inicialização do servidor TCP
    if (setup_tcp_server() != 0) {
        return -1;
    }

    gpio_set_irq_enabled_with_callback(BTNA, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler); //Callback de interrupção do Botão A
    gpio_set_irq_enabled_with_callback(BTNB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler); //Callback de interrupção do Botão B

    while (true)
    {
        cyw43_arch_poll();  // Necessário para manter o Wi-Fi ativo

        tempI = (select_adc_channel(1) * 2e2) / 4095.0;   // Eixo X (0 - 4095).
        tempA = (select_adc_channel(0) * 60) / 4095.0;    // Eixo Y (0 - 4095).

        status = action(status, mode, tempI, tempA, pwm)%4;

        ssd1306_fill(&ssd, !cor);                       // Limpa o display
        ssd1306_rect(&ssd, 1, 1, 123, 63, cor, !cor);   // Retângulo da área útil
        if(!status) {
            ssd1306_draw_string(&ssd, "TI:--" ,4, 4);
            ssd1306_draw_string(&ssd, "TA:--", 4, 20);
            ssd1306_draw_string(&ssd, "SPEED:--",4, 36);
            ssd1306_draw_string(&ssd, "STATUS:STOP",4, 52);    
            symbol('*');
            gpio_put(RED,false);gpio_put(GREEN,false);
        } else {
            sprintf(str, "TI:%.2f", tempI);
            ssd1306_draw_string(&ssd, str ,4, 4);
            sprintf(str, "TA:%.2f", tempA);
            ssd1306_draw_string(&ssd, str,4, 20);
            sprintf(str, "SPEED:%.1f%%", (pwm/1250.0)*100.0);
            ssd1306_draw_string(&ssd, str,4, 36);
            if(status==1) {
                ssd1306_draw_string(&ssd, "STATUS:NORMAL",4, 52);    
                symbol('v');
                gpio_put(RED,false);gpio_put(GREEN,true);
                (status!=last_stt)?play_buzzer(SOL):pwm_set_gpio_level(BUZZER,0);
            } else if(status==2) {
                ssd1306_draw_string(&ssd, "STATUS:ALERT",4, 52);    
                symbol('w');
                gpio_put(RED,true);gpio_put(GREEN,true);
                (status!=last_stt)?play_buzzer(LA):pwm_set_gpio_level(BUZZER,0);
            } else {
                ssd1306_draw_string(&ssd, "STATUS:DANGER",4, 52);    
                symbol('x');
                gpio_put(RED,true);gpio_put(GREEN,false);
                (status!=last_stt)?play_buzzer(RE):pwm_set_gpio_level(BUZZER,0);
            }
        }
        ssd1306_draw_string(&ssd, (mode==1) ? "H" : (mode==2) ? "C" : "I", 113, 4);
        ssd1306_send_data(&ssd);                        // Atualiza o display

        last_stt = status;
        sleep_ms(100);      // Reduz o uso da CPU
    }

    //Desliga a arquitetura CYW43.
    cyw43_arch_deinit();
    return 0;
}

// Função responsável por configurar e iniciar o servidor TCP. 
int setup_tcp_server(void) {
    // Cria o PCB (Protocol Control Block) para o servidor TCP
    struct tcp_pcb *server = tcp_new();
    if (!server) {
        printf("Falha ao criar servidor TCP\n");
        return -1;
    }

    // Vincula o PCB a uma porta específica (porta 80) e aceita conexões de qualquer endereço IP
    if (tcp_bind(server, IP_ADDR_ANY, 80) != ERR_OK) {
        printf("Falha ao associar servidor TCP à porta 80\n");
        return -1;
    }

    // Coloca o PCB em modo de escuta para aceitar conexões de entrada
    server = tcp_listen(server);

    // Define a função de callback que será chamada quando uma conexão de entrada for aceita
    tcp_accept(server, tcp_server_accept);
    printf("Servidor ouvindo na porta 80\n");

    return 0;
}

// Tratamento do request do usuário - digite aqui
void user_request(char **request){

    if (strstr(*request, "GET /+") != NULL)
    {
        pwm = (pwm + 25)%1250;
    }
    else if (strstr(*request, "GET /-") != NULL)
    {
        pwm = (pwm - 25 + 1250) % 1250;
    }
    else if (strstr(*request, "GET /s") != NULL)
    {
        status = status==0;
    }
    else if (strstr(*request, "GET /m") != NULL)
    {
        mode = (mode + 1)%3;
    }
};


// Função de callback ao aceitar conexões TCP
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    tcp_recv(newpcb, tcp_server_recv);
    return ERR_OK;
}

// Função de callback para processar requisições HTTP
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
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

    char str_mode[5], str_TA[6], str_TI[6], str_stt[7], stt_btn_color[5], stt_btn_sym[2], str_speed[5];

    printf("Request: %s", request);

    // Tratamento de request - Controle dos LEDs
    user_request(&request);

    if (mode == 1)
        strcpy(str_mode, "HEAT");
    else if (mode == 2)
        strcpy(str_mode, "COLD");
    else
        strcpy(str_mode, "IDLE");

    if (status == 1)
        strcpy(str_stt, "NORMAL");
    else if (status == 2)
        strcpy(str_stt, "ALERT");
    else if (status == 3)
        strcpy(str_stt, "DANGER");
    else
        strcpy(str_stt, "STOP");

    if(status) {
        sprintf(stt_btn_sym, "⏹");
        sprintf(stt_btn_color, "red");
    }
    else {
        sprintf(stt_btn_sym, "⟳");
        sprintf(stt_btn_color, "green");
    }

    sprintf(str_TI, "%.2f", tempI);
    sprintf(str_TA, "%.2f", tempA);
    sprintf(str_speed, "%i", pwm);

    // Cria a resposta HTML
    char html[1024];

    snprintf(html, sizeof(html),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "\r\n"
            "<!DOCTYPE html>"
            "<html>"
            "<script>"
            "setTimeout(function(){ location.href = '/'; }, 5000);"
            "</script>"
            "<head>"
            "<meta charset=\"UTF-8\">"
            "<title> Painel </title>"
            "<style>"
            "body { text-align: center; margin-top: 50px; }"
            );

    // Escreve dados para envio (mas não os envia imediatamente).
    tcp_write(tpcb, html, strlen(html), TCP_WRITE_FLAG_COPY);

    // Envia a mensagem
    tcp_output(tpcb);

    snprintf(html, sizeof(html),
        "button {font-size: 36px; margin: 10px; padding: 20px 40px; border-radius: 10px; color:white;}"
        "div {display: flex; justify-content: center; gap: 20px; margin-bottom: 20px;}"
        "</style>"
        "</head>"
        "<body>");

    // Escreve dados para envio (mas não os envia imediatamente).
    tcp_write(tpcb, html, strlen(html), TCP_WRITE_FLAG_COPY);

    // Envia a mensagem
    tcp_output(tpcb);

    snprintf(html, sizeof(html),
         "<div>"
         "<form action=\"./+\"><button style=\"background-color: blue;X\">↑</button></form>"
         "<form action=\"./-\"><button style=\"background-color: blue;X\">↓</button></form>"
         "</div>"
         "<form action=\"./s\"><button style=\"background-color: %s;X\">%s</button></form>"
         "<form action=\"./m\"><button style=\"background-color: #FF8C00;X\">%s</button></form>"
         "<table border=\"1\" style=\"margin:auto;margin-top: 30px;\">"
         "<tr><th> TI (°C) </th><th> TA (°C) </th><th> STATUS </th><th> MODE </th><th>SPEED</th></tr>"
         "<tr><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td></tr>"
         "</table>"
         "</body>"
         "</html>", stt_btn_color, stt_btn_sym, str_mode, str_TI, str_TA, str_stt, str_mode, str_speed);
    
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

// Função de interrupção com debouncing
void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());  // Obtém o tempo atual em microssegundos
  
    if (current_time - last_time > 2e5) { // 200 ms de debouncing
        last_time = current_time; 
        if(gpio == BTNA) { // Botão A alterna entre diferentes telas ou retorna para as telas de seleção
            mode = (mode + 1)%3;
        } else if(gpio == BTNB) {
            status = status==0;
        }
    }
}

