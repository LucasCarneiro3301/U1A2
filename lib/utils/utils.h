#ifndef UTILS_H
#define UTILS_H

#include "../config/config.h"

/*
    IDLE = 25 - 60 (20-25  e 60-80)
    HEAT = 80 - 120 (70-80 e 120-130)
    COLD = 5 - 20 (0-5 e 20-25)
*/

uint8_t _tempI(uint8_t status, uint8_t mode, float tempI) {
    if(!status)
        return 0;

    if (
        ((mode==0 && tempI >= 20.0 && tempI < 25.0) ||
        (mode==1 && tempI >= 70.0 && tempI < 80.0) ||
        (mode==2 && tempI >= 0.0 && tempI < 5.0))
    )
        return 2;
    if (
        ((mode==0 && tempI <= 80.0 && tempI > 60.0) ||
        (mode==1 && tempI <= 130.0 && tempI > 120.0) ||
        (mode==2 && tempI <= 25.0 && tempI > 20.0))
    )
        return 6;
    if (
        ((mode==0 && tempI > 80.0) ||
        (mode==1 && tempI > 130.0) ||
        (mode==2 && tempI > 25.0))
    )
        return 3;
    if (
        ((mode==0 && tempI < 20.0) ||
        (mode==1 && tempI < 70.0) ||
        (mode==2 && tempI < 0.0))
    )
        return 7;
    
    return 1;
}

uint8_t _tempA(uint8_t status, uint8_t mode, float tempA) {
    if(!status)
        return 0;

    if(
        ((mode==0 && tempA >= 16.0 && tempA < 20.0) ||
        (mode==1 && tempA >= 20.0 && tempA < 25.0) ||
        (mode==2 && tempA >= 12.0 && tempA < 16.0))
    )
        return status = 2;
    if(
        ((mode==0 && tempA <= 35.0  && tempA > 30.0) ||
        (mode==1 && tempA <= 40.0 && tempA > 35.0) ||
        (mode==2 && tempA <= 30.0 && tempA > 25.0))
    ) 
        return status = 6;
    if(
        ((mode==0 && tempA > 35.0) ||
        (mode==1 && tempA > 40.0) ||
        (mode==2 && tempA > 30.0))
    ) 
        return status = 3;
    if(
        ((mode==0 && tempA < 16.0) ||
        (mode==1 && tempA < 20.0) ||
        (mode==2 && tempA < 12.0))
    ) 
        return status = 7;

    return 1;
}

uint8_t action(uint8_t status, uint8_t mode, float tempI,float tempA, uint16_t pwm) {
    printf("%i\n",status);
    if(!status) {
        pwm_set_gpio_level(HEAT_RESIST, 0);
        pwm_set_gpio_level(FAN, 0);
        gpio_put(HEATER,false);
        gpio_put(AIR_CONDIT,false);

        return 0;
    }

    uint8_t act_1 = _tempI(status,mode,tempI);

    if(act_1==1) {
        pwm_set_gpio_level(HEAT_RESIST, 0);
        pwm_set_gpio_level(FAN, 0);
    }
    if(act_1==2) {
        pwm_set_gpio_level(HEAT_RESIST, 100);
        pwm_set_gpio_level(FAN, 0);
    } else if(act_1==6) {
        pwm_set_gpio_level(HEAT_RESIST, 0);
        pwm_set_gpio_level(FAN,pwm);
    } else if(act_1==3) {
        pwm_set_gpio_level(HEAT_RESIST, 0);
        pwm_set_gpio_level(FAN, pwm);
    } else if(act_1==7) {
        pwm_set_gpio_level(HEAT_RESIST, 625);
        pwm_set_gpio_level(FAN,0);
    }

    uint8_t act_2 = _tempA(status,mode,tempA);

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

#endif