#include "config.h"

PIO pio = pio0;
int sm = 0;

void btn_setup() {
    gpio_init(BTNA);
    gpio_set_dir(BTNA, GPIO_IN);
    gpio_pull_up(BTNA);

    gpio_init(BTNB);
    gpio_set_dir(BTNB, GPIO_IN);
    gpio_pull_up(BTNB);
}

void output_setup() {
    gpio_init(GREEN);
    gpio_set_dir(GREEN, GPIO_OUT);
    gpio_init(RED);
    gpio_set_dir(RED, GPIO_OUT);
    gpio_init(AIR_CONDIT);
    gpio_set_dir(AIR_CONDIT, GPIO_OUT);
    gpio_init(HEATER);
    gpio_set_dir(HEATER, GPIO_OUT);
}

// Inicializa e configura a comunicação serial I2C 
void i2c_setup() {
    i2c_init(I2C_PORT, 4e2 * 1e3);              // Inicialização I2C.
  
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);  // Define a função do pino GPIO para I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);  // Define a função do pino GPIO para I2C
    gpio_pull_up(I2C_SDA);                      // Pull up para linha de dados
    gpio_pull_up(I2C_SCL);                      // Pull up para linha de clock
}

// Limpa o display
void clear(ssd1306_t* ssd) {
    ssd1306_fill(ssd, false);
    ssd1306_send_data(ssd);
}

// Inicializa e configura o display
void ssd1306_setup(ssd1306_t* ssd) {
    ssd1306_init(ssd, WIDTH, HEIGHT, false, I2C_address, I2C_PORT); 
    ssd1306_config(ssd); 
    ssd1306_send_data(ssd);
    clear(ssd);

  
    ssd1306_draw_char(ssd, '#', 28, 60);  
    ssd1306_send_data(ssd);
}

// Inicializa e configura a matriz de LEDs 5x5 
void ws2812_setup(PIO pio, uint sm) {
    uint offset = pio_add_program(pio, &ws2812_program);
  
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW); //Inicializa a matriz de leds
}

// Inicializa e configura os pinos do joystick como periféricos ADC
void adc_setup() {
    adc_init();
    adc_gpio_init(JOY_Y);
    adc_gpio_init(JOY_X);
}

void pwm_setup() {
    gpio_set_function(FAN, GPIO_FUNC_PWM);
    gpio_set_function(HEAT_RESIST, GPIO_FUNC_PWM);

    uint slice_num_1 = pwm_gpio_to_slice_num(FAN);
    uint slice_num_2 = pwm_gpio_to_slice_num(HEAT_RESIST);

    uint32_t clock_freq = 125000000;
    uint32_t pwm_freq = 10000;
    uint32_t top = 1250;  // wrap = top

    float div = (float)clock_freq / (pwm_freq * (top + 1));
    pwm_set_clkdiv(slice_num_1, div);
    pwm_set_clkdiv(slice_num_2, div);

    pwm_set_wrap(slice_num_1, top);
    pwm_set_wrap(slice_num_2, top);

    pwm_set_chan_level(slice_num_1, PWM_CHAN_B, top / 2); // GPIO 4 → canal B
    pwm_set_chan_level(slice_num_2, PWM_CHAN_A, top / 2); // GPIO 9 → canal A

    pwm_set_enabled(slice_num_1, true);
    pwm_set_enabled(slice_num_2, true);
}


void init(PIO pio, uint sm, ssd1306_t* ssd) {
    btn_setup();
    output_setup();
    i2c_setup();
    ssd1306_setup(ssd);
    adc_setup();
    pwm_setup();
    ws2812_setup(pio, sm);
}