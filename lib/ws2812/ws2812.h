/*
    Aluno: Lucas Carneiro de Araújo Lima
*/

#ifndef WS2812_H
#define WS2812_H

#include "../config/config.h"
#include <string.h>

// Definição de tipo da estrutura que irá controlar a cor dos LED's
typedef struct {
    double red;
    double green;
    double blue;
} Color;

// Definição de tipo da matriz de leds
typedef Color Led_Matrix[5][5];

void symbol(char symbol);

#endif