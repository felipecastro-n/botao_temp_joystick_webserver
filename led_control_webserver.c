#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "pico/cyw43_arch.h"
#include <stdio.h>
#include <string.h>
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/netif.h"

#define WIFI_SSID "SEU_SSID"
#define WIFI_PASSWORD "SUA_SENHA"
#define BUTTON_PIN 5
#define SERVER_IP "IP_DO_SERVIDOR" // IP do servidor

#define SERVER_PORT 5000 // Porta do servidor
#define JOYSTICK_X_PIN 26
#define JOYSTICK_Y_PIN 27
#define ADC_X_CHANNEL 0
#define ADC_Y_CHANNEL 1

// Valores de referência para o joystick
#define JOYSTICK_CENTER_X 2048
#define JOYSTICK_CENTER_Y 2048
#define JOYSTICK_THRESHOLD 500

// Definição das direções da rosa dos ventos
typedef enum {
    DIR_CENTRO,
    DIR_NORTE,
    DIR_NORDESTE,
    DIR_LESTE,
    DIR_SUDESTE,
    DIR_SUL,
    DIR_SUDOESTE,
    DIR_OESTE,
    DIR_NOROESTE
} direcao_t;

volatile bool button_pressed = false;
float current_temperature = 0.0f;
uint16_t joystick_x = 0;
uint16_t joystick_y = 0;
direcao_t direcao_atual = DIR_CENTRO;

// Função para converter a direção em string
const char* direcao_para_string(direcao_t dir) {
    switch (dir) {
        case DIR_CENTRO: return "Centro";
        case DIR_NORTE: return "Norte";
        case DIR_NORDESTE: return "Nordeste";
        case DIR_LESTE: return "Leste";
        case DIR_SUDESTE: return "Sudeste";
        case DIR_SUL: return "Sul";
        case DIR_SUDOESTE: return "Sudoeste";
        case DIR_OESTE: return "Oeste";
        case DIR_NOROESTE: return "Noroeste";
        default: return "Desconhecido";
    }
}

float read_internal_temperature() {
    adc_select_input(4);
    uint16_t raw_value = adc_read();
    const float conversion_factor = 3.3f / (1 << 12);
    return 27.0f - ((raw_value * conversion_factor - 0.706f) / 0.001721f);
}

void button_callback(uint gpio, uint32_t events) {
    if (gpio == BUTTON_PIN) {
        button_pressed = true;
    }
}

void setup_joystick() {
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);
}

// Determina a direção da rosa dos ventos com base nas leituras do joystick
direcao_t determinar_direcao(uint16_t x, uint16_t y) {
    int dx = x - JOYSTICK_CENTER_X;
    int dy = y - JOYSTICK_CENTER_Y;
    
    // Se estiver próximo do centro, retorna centro
    if (abs(dx) < JOYSTICK_THRESHOLD && abs(dy) < JOYSTICK_THRESHOLD) {
        return DIR_CENTRO;
    }
    
    // Determina a direção com base nos valores de x e y
    if (dy < -JOYSTICK_THRESHOLD) {
        if (dx < -JOYSTICK_THRESHOLD) return DIR_NOROESTE;
        else if (dx > JOYSTICK_THRESHOLD) return DIR_NORDESTE;
        else return DIR_NORTE;
    } 
    else if (dy > JOYSTICK_THRESHOLD) {
        if (dx < -JOYSTICK_THRESHOLD) return DIR_SUDOESTE;
        else if (dx > JOYSTICK_THRESHOLD) return DIR_SUDESTE;
        else return DIR_SUL;
    }
    else {
        if (dx < -JOYSTICK_THRESHOLD) return DIR_OESTE;
        else if (dx > JOYSTICK_THRESHOLD) return DIR_LESTE;
        else return DIR_CENTRO;
    }
}

void read_joystick() {
    adc_select_input(ADC_X_CHANNEL);
    sleep_us(2);
    joystick_x = adc_read();
    
    adc_select_input(ADC_Y_CHANNEL);
    sleep_us(2);
    joystick_y = adc_read();
    
    // Atualiza a direção atual
    direcao_atual = determinar_direcao(joystick_x, joystick_y);
}





void send_data_to_server(bool button_state, float temperature, uint16_t x, uint16_t y, direcao_t direcao) {
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("Falha ao criar PCB\n");
        return;
    }

    ip_addr_t server_addr;
    if (!ipaddr_aton(SERVER_IP, &server_addr)) {
        printf("Endereço IP inválido\n");
        return;
    }

    if (tcp_connect(pcb, &server_addr, SERVER_PORT, NULL) != ERR_OK) {
        printf("Erro ao conectar ao servidor\n");
        return;
    }

    char json[256];
    snprintf(json, sizeof(json),
             "{\"button\": %d, \"temperature\": %.2f, \"x\": %d, \"y\": %d, \"direcao\": \"%s\"}",
             button_state, temperature, x, y, direcao_para_string(direcao));
    int json_length = strlen(json);

    char request[512];
    snprintf(request, sizeof(request),
             "POST / HTTP/1.1\r\n"
             "Host: %s:%d\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %d\r\n"
             "Connection: close\r\n"
             "\r\n"
             "%s",
             SERVER_IP, SERVER_PORT, json_length, json);

    tcp_write(pcb, request, strlen(request), TCP_WRITE_FLAG_COPY);
    tcp_output(pcb);
    sleep_ms(500);
    tcp_close(pcb);
}

int main() {
    stdio_init_all();

    // Configuração do botão
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true, &button_callback);

    // Configuração do Wi-Fi
    if (cyw43_arch_init()) {
        printf("Erro ao iniciar Wi-Fi\n");
        return -1;
    }
    cyw43_arch_enable_sta_mode();

    printf("Conectando ao Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 20000)) {
        printf("Falha ao conectar\n");
        return -1;
    }
    printf("Conectado!\n");

    // Configuração dos sensores
    adc_init();
    adc_set_temp_sensor_enabled(true);
    setup_joystick();

    while (true) {
        // Leitura dos sensores
        current_temperature = read_internal_temperature();
        read_joystick();

        // Exibe os dados no console
        printf("Temp: %.2fC | Botão: %d | X: %d | Y: %d | Direção: %s\n", 
              current_temperature, button_pressed, joystick_x, joystick_y, 
              direcao_para_string(direcao_atual));

        // Envia os dados quando o botão é pressionado
        if (button_pressed) {
            send_data_to_server(true, current_temperature, joystick_x, joystick_y, direcao_atual);
            sleep_ms(1000);
            send_data_to_server(false, current_temperature, joystick_x, joystick_y, direcao_atual);
            button_pressed = false;
        }

        // Envio periódico a cada 3 segundos
        static absolute_time_t last_send = 0;
        if (absolute_time_diff_us(last_send, get_absolute_time()) > 1000000) {
            send_data_to_server(button_pressed, current_temperature, joystick_x, joystick_y, direcao_atual);
            last_send = get_absolute_time();
        }

        sleep_ms(1000);
        cyw43_arch_poll();
    }

    cyw43_arch_deinit();
    return 0;
}