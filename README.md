# U1A2
Este repositório contém o desenvolvimento parcial de um servidor HTTP embarcado no Raspberry Pi Pico W, com conexão à rede Wi-Fi e controle básico via navegador de um celular ou computador.

__Aluno:__
Lucas Carneiro de Araújo Lima

## ATIVIDADE 

__Para este trabalho, os seguintes componentes e ferramentas se fazem necessários:__
1) Microcontrolador Raspberry Pi Pico W.
2) Ambiente de trabalho VSCode.
3) LEDs RGB.
4) _Display_ SSD1306.
5) 2 Botões Pull-Up.
7) Ferramenta educacional BitDogLab.
8) Matriz de LEDs 5x5.
9) 1 Protoboard.
10) 4 resistores de 330 0hms ou mais.
11) 5 _jumpers_.
12) Cabo USB.
13) 4 LEDs

__O resultado do projeto pode ser assistido através deste link: [Vídeo de Apresentação - Webserver HTTP.](https://youtu.be/zVdWSzDmSOE).__

## Instruções de Uso

### 1. Clone o repositório
Abra o terminal e execute o comando abaixo para clonar o repositório em sua máquina:
```bash
git clone https://github.com/LucasCarneiro3301/U1A2.git
```

### 2. Configure o ambiente de desenvolvimento
Certifique-se de que o [SDK do Raspberry Pi Pico](https://github.com/raspberrypi/pico-sdk) esteja instalado e configurado corretamente no seu sistema.

### 3. Instale o CMake
Certifique-se de que o [CMake](https://cmake.org/download/) esteja instalado e configurado corretamente no seu sistema.

### 4. Conexão com a Rapberry Pico
1. Conecte o Raspberry Pi Pico ao seu computador via USB.
2. Inicie o modo de gravação pressionando o botão **BOOTSEL** e **RESTART**.
3. O Pico será montado como um dispositivo de armazenamento USB.
4. Execute através do comando **RUN** a fim de copiar o arquivo `U1A2.uf2` para o Pico.
5. O Pico reiniciará automaticamente e executará o programa.

### 5. Observações (IMPORTANTE !!!)
2. Manuseie a placa com cuidado.

## Recursos e Funcionalidades

### 1. Botão

| BOTÃO                            | DESCRIÇÃO                                     | 
|:----------------------------------:|:---------------------------------------------:|
| A                                  | Modo de Operação              | 
| B                                  | Botão de Parada              | 

### 2. Display OLED
Exibe informações sobre temperatura interna e externa, velocidade da ventoinha, estado do sistema e modo de operação atual

### 3. Matriz de LEDs (5x5)
Representa visualmente os estados do sistema.

### 4. LED RGB
Representa visualmente os estados do sistema.

### 5. BUZZER
Representa através de efeitos sonoros os estados do sistema

### 6. Interrupções
As interrupções são usadas para detectar a borda de descida do Botão A e B de forma assíncrona, com tratamento de debounce para evitar leituras falsas por oscilações elétricas, eliminando a necessidade de polling contínuo.

### 7. Comunicação Serial
Envia mensagens do sistema via USB para o terminal do computador.

### 8. Módulo Wi-Fi CYW43
Fornece conectividade Wi-Fi ao Raspberry Pi Pico W.
