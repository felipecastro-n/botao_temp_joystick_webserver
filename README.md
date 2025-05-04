# Projeto de Monitoramento com Joystick e Interface Web

Sistema integrado para monitoramento de direção via joystick, temperatura e estados de botão, com visualização em tempo real através de interface web. Desenvolvido para Raspberry Pi Pico W usando SDK em C e servidor Flask.

## Funcionalidades Principais

🎮 **Controle por Joystick Analógico:**
- Detecção de 9 direções da rosa dos ventos
- Leitura precisa dos eixos X/Y (12-bit ADC)
- Threshold ajustável para precisão do movimento

🌡 **Monitoramento de Temperatura:**
- Leitura do sensor interno do RP2040
- Precisão de ±1°C com calibração

📶 **Comunicação Sem Fio:**
- Conexão Wi-Fi segura (WPA2)
- Envio periódico de dados via TCP
- Botão de trigger para envio imediato

🖥 **Interface Web Integrada:**
- Rosa dos ventos interativa com indicador visual
- Exibição de coordenadas X/Y em tempo real
- Histórico das últimas 100 leituras
- Atualização automática a cada 2 segundos

## Componentes Utilizados

Componente | Especificações
---|---
Raspberry Pi Pico W | Microcontrolador RP2040 + CYW43439
Joystick Analógico | 2 eixos + botão, 10KΩ
Botão Tactile | GPIO5 com pull-up interno
Servidor Flask | Python 3.10+, 500MB RAM

## Esquema de Conexões

Componente | Pino no Pico | Descrição
---|---|---
Joystick X | GPIO26 | Canal ADC0
Joystick Y | GPIO27 | Canal ADC1
Botão | GPIO5 | Entrada com pull-up
LED Wi-Fi | CYW43_LED_PIN | Status de conexão

## Instalação e Uso

### Pré-requisitos
1. **Python:**
   ```bash
   sudo apt update
   sudo apt install python3 python3-pip python3-venv
   ```

2. **Ambiente Virtual:**
   ```bash
   python3 -m venv venv
   ```

3. **Ativação do Ambiente Virtual:**
   - No Linux/macOS:
   ```bash
   source venv/bin/activate
   ```
   - No Windows:
   ```bash
   venv\Scripts\activate
   ```

### Configuração do Hardware
1. Conecte o joystick:
   - Eixo X → GPIO26 (ADC0)
   - Eixo Y → GPIO27 (ADC1)
   - Botão → GPIO5 com pull-up
2. Alimente via USB 5V

### Programação do Pico
```bash
git clone https://github.com/felipecastro-n/botao_temp_joystick_webserver.git
cd pico-firmware
mkdir build && cd build
cmake .. -DPICO_BOARD=pico_w
make -j4
```

### Configuração do Servidor
1. Instale as dependências no ambiente virtual:
```bash
pip install flask
pip install -r requirements.txt
```

2. Inicie o servidor Flask:
```bash
export FLASK_APP=server/app.py
flask run --host=0.0.0.0
```

3. Acesse a interface web em:
```
http://[IP_DO_SERVIDOR]:5000
```

### Personalização
Atualize credenciais em main.c:

```c
#define WIFI_SSID "SUA_REDE"
#define WIFI_PASSWORD "SENHA"
#define SERVER_IP "IP_LOCAL"
```

## Diagrama de Operação
```
[Pico W] → (Leitura Analógica) → (Processamento) 
 → [Wi-Fi] → [Servidor Flask] 
 → [Interface Web] ← (Atualização em Tempo Real)
```

## Otimizações
- Amostragem ADC com média móvel (5 leituras)
- Timeout de reconexão Wi-Fi (10 tentativas)
- Compressão de pacotes JSON
- Cache de templates no servidor
- Ambiente virtual para isolamento de dependências

## Solução de Problemas
- Se o servidor não iniciar, verifique se o ambiente virtual está ativado
- Para problemas de permissão no Linux, use `sudo` ao executar o Flask
- Verifique as configurações de firewall se não conseguir acessar a interface web

## Licença
GPLv3 - Livre para uso acadêmico e modificações, com obrigação de atribuição.
