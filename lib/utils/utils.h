#ifndef UTILS_H
#define UTILS_H

#include "../config/config.h"

void stop_motor() {
    gpio_put(FAN_IN1,false);
    gpio_put(FAN_IN2,false);
    pwm_set_gpio_level(FAN_ENA, 0);
}

void motor_forward(uint16_t speed) {
    gpio_put(FAN_IN1, true);
    gpio_put(FAN_IN2, false);

    if (speed > 1250) speed = 1250;
    pwm_set_gpio_level(FAN_ENA, speed);
}

float conversion(uint16_t adc_value) {
    float voltage = (adc_value * 3.3f) / 4095.0f; // Converte ADC para volts
    return voltage * 100.0f;                      // Converte volts para °C (10mV/°C)
}

/*
    IDLE = 25 - 60 (20-25  e 60-80)
    HEAT = 80 - 120 (70-80 e 120-130)
    COLD = 5 - 20 (0-5 e 20-25)
*/

uint8_t _ti(uint8_t status, uint8_t mode, float ti) {
    if(!status)
        return 0;

    if (
        ((mode==0 && ti >= 20.0 && ti < 25.0) ||
        (mode==1 && ti >= 70.0 && ti < 80.0) ||
        (mode==2 && ti >= 0.0 && ti < 5.0))
    )
        return 2;
    if (
        ((mode==0 && ti <= 80.0 && ti > 60.0) ||
        (mode==1 && ti <= 130.0 && ti > 120.0) ||
        (mode==2 && ti <= 25.0 && ti > 20.0))
    )
        return 6;
    if (
        ((mode==0 && ti > 80.0) ||
        (mode==1 && ti > 130.0) ||
        (mode==2 && ti > 25.0))
    )
        return 3;
    if (
        ((mode==0 && ti < 20.0) ||
        (mode==1 && ti < 70.0) ||
        (mode==2 && ti < 0.0))
    )
        return 7;
    
    return 1;
}

uint8_t _ta(uint8_t status, uint8_t mode, float ta) {
    if(!status)
        return 0;

    if(
        ((mode==0 && ta >= 16.0 && ta < 20.0) ||
        (mode==1 && ta >= 20.0 && ta < 25.0) ||
        (mode==2 && ta >= 12.0 && ta < 16.0))
    )
        return status = 2;
    if(
        ((mode==0 && ta <= 35.0  && ta > 30.0) ||
        (mode==1 && ta <= 40.0 && ta > 35.0) ||
        (mode==2 && ta <= 30.0 && ta > 25.0))
    ) 
        return status = 6;
    if(
        ((mode==0 && ta > 35.0) ||
        (mode==1 && ta > 40.0) ||
        (mode==2 && ta > 30.0))
    ) 
        return status = 3;
    if(
        ((mode==0 && ta < 16.0) ||
        (mode==1 && ta < 20.0) ||
        (mode==2 && ta < 12.0))
    ) 
        return status = 7;

    return 1;
}

uint8_t action(uint8_t status, uint8_t mode, float ti,float ta, uint16_t pwm) {
    if(!status) {
        stop_motor();
        pwm_set_gpio_level(HEAT_RESIST, 0);
        gpio_put(HEATER,false);
        gpio_put(AIR_CONDIT,false);

        return 0;
    }

    uint8_t act_1 = _ti(status,mode,ti);

    if(act_1==1) {
        pwm_set_gpio_level(HEAT_RESIST, 0);
        stop_motor();
    }
    if(act_1==2) {
        pwm_set_gpio_level(HEAT_RESIST, 100);
        stop_motor();
    } else if(act_1==6) {
        pwm_set_gpio_level(HEAT_RESIST, 0);
        motor_forward(pwm);
    } else if(act_1==3) {
        pwm_set_gpio_level(HEAT_RESIST, 0);
        motor_forward(pwm);
    } else if(act_1==7) {
        pwm_set_gpio_level(HEAT_RESIST, 625);
        stop_motor();
    }

    uint8_t act_2 = _ta(status,mode,ta);

    if(act_2==1) {
        gpio_put(HEATER,false);
        gpio_put(AIR_CONDIT,false);
    }
    if(act_2==2) {
        gpio_put(HEATER,true);
        gpio_put(AIR_CONDIT,false);
    } else if(act_2==6) {
        gpio_put(HEATER,false);
        gpio_put(AIR_CONDIT,true);
    } else if(act_2==3) {
        gpio_put(HEATER,false);
        gpio_put(AIR_CONDIT,true);
    } else if(act_2==7) {
        gpio_put(HEATER,true);
        gpio_put(AIR_CONDIT,false);
    }

    return ((act_1)%4==3 || (act_2)%4==3) ? 3 : ((act_1)%4==2 || (act_2)%4==2) ? 2 : 1;
}

// Configura o PWM no pino do buzzer com uma frequência especificada
void set_buzzer_frequency(uint frequency) {
    uint slice_num = pwm_gpio_to_slice_num(BUZZER);
    gpio_set_function(BUZZER, GPIO_FUNC_PWM);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (frequency * 4096));
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(BUZZER, 0);
}

// Função para tocar o buzzer por um tempo especificado (em milissegundos)
void play_buzzer(uint frequency) {
    set_buzzer_frequency(frequency);
    pwm_set_gpio_level(BUZZER, 32768);
}

void ssd1306_screen(ssd1306_t *ssd, float ti, float ta, float speed, uint8_t status, uint8_t mode) {
    char str[16];

    if(!status) {
        ssd1306_draw_string(ssd, "TI:--" ,4, 4);
        ssd1306_draw_string(ssd, "TA:--", 4, 20);
        ssd1306_draw_string(ssd, "SPEED:--",4, 36);
        ssd1306_draw_string(ssd, "STATUS:STOP",4, 52);  
    } else {
        sprintf(str, "TI:%.2f", ti);
        ssd1306_draw_string(ssd, str ,4, 4);
        sprintf(str, "TA:%.2f", ta);
        ssd1306_draw_string(ssd, str,4, 20);
        sprintf(str, "SPEED:%.1f%%", speed);
        ssd1306_draw_string(ssd, str,4, 36);
        if(status==1)
            ssd1306_draw_string(ssd, "STATUS:NORMAL",4, 52);
        else if(status==2)
            ssd1306_draw_string(ssd, "STATUS:ALERT",4, 52);    
        else
            ssd1306_draw_string(ssd, "STATUS:DANGER",4, 52);
    }
    ssd1306_draw_string(ssd, (mode==1) ? "H" : (mode==2) ? "C" : "I", 113, 4);   
}

#endif