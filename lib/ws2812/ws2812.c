#include "ws2812.h"

// Limpa todos os leds de um frame
void frame_cleaner(Led_Matrix *frame) {
    for(int i = 0; i < 5; i++) {
        for(int j = 0; j < 5; j++) {
            (*frame)[i][j].red = 0.0;
            (*frame)[i][j].blue = 0.0;
            (*frame)[i][j].green = 0.0;
        }
    }
}

//Definição da cor escolhida
uint32_t color(double b, double r, double g) {
  return (((unsigned char)(g * 255)) << 24) | (((unsigned char)(r * 255)) << 16) | (((unsigned char)(b * 255)) << 8);
}

//Imprime o frame na matriz de led
void printer(Led_Matrix *frame){
    for (int i = 4; i >= 0; i--){
        if(i % 2) {
            for (int j = 0; j < 5; j ++) {
                pio_sm_put_blocking((PIO)pio, sm, color((*frame)[i][j].blue, (*frame)[i][j].red, (*frame)[i][j].green));
            }
        }else {
            for (int j = 4; j >= 0; j --) {
                pio_sm_put_blocking((PIO)pio, sm, color((*frame)[i][j].blue, (*frame)[i][j].red, (*frame)[i][j].green));
            }
        }
    }
}

//Forma um frame na matriz de led
void framer(Led_Matrix *frame, float r, float g, float b, int row, int col, bool clear, bool print) {
    if(clear) frame_cleaner(frame);
    (*frame)[row][col].red = r;
    (*frame)[row][col].blue = b;
    (*frame)[row][col].green = g;
    if(print) printer(frame); 
}

// Desenha uma coluna na matriz de leds
void ws2812_draw_column(float r, float g, float b, int col, bool clear, bool print) {
    Led_Matrix frame = {
        {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}, // Linha 0
        {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}, // Linha 1
        {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}, // Linha 2
        {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}, // Linha 3
        {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}  // Linha 4
    };

    framer(&frame,r,g,b,0,col,clear,false);
    framer(&frame,r,g,b,1,col,false,false);
    framer(&frame,r,g,b,2,col,false,false);
    framer(&frame,r,g,b,3,col,false,false);
    framer(&frame,r,g,b,4,col,false,print);
}

// Limpa a matriz de leds
void ws2812_clean() {
    Led_Matrix frame = {
        {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}, // Linha 0
        {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}, // Linha 1
        {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}, // Linha 2
        {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}, // Linha 3
        {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}  // Linha 4
    };

    printer(&frame); 
}