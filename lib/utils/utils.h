#ifndef UTILS_H
#define UTILS_H

#include "../config/config.h"

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

    uint8_t act_2 = _tempI(status,mode,tempA);

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

#endif