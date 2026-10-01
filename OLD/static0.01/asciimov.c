#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/ioctl.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define MAX_X 800
#define MAX_Y 400

// paleta ascii
const char PALLETTE[] = " .,*:;-+=%#@";
const int PALLETTE_LEN = 12;

char preBuffer[MAX_Y][MAX_X];
char actBuffer[MAX_Y][MAX_X];

// obtener tamaño de ventana
void obtainSize(int *x, int *y, float *proportion){
    struct winsize w; // estructura de ioctl.h
    // ioctl devuelve 0 cuando no hay errores y -1 si si los hay
    // definimos 'x' y 'y' segun la cantidad de filas y columnas q da ioctl
    // usamos punteros pq las funciones solo pueden devolver valores con return
    // y asi cualquier cambio se recibe inmediatamente
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0){
        *x = w.ws_col; 
        *y = w.ws_row;

        // si nos da pixeles reales hacemos la cuenta exacta
        if(w.ws_xpixel > 0 && w.ws_ypixel > 0){
            float pixelXLen = (float)w.ws_xpixel/w.ws_col;
            float pixelYLen = (float)w.ws_ypixel/w.ws_row;
            *proportion = pixelXLen / pixelYLen;
        } else {
            *proportion = 0.5f;  // si no da pixeles usamos una proporcion de respaldo
        }

    } else {
        *x = 80;
        *y = 24;
        *proportion = 0.5f;
        // si algo sale mal definimos valores de terminales DOS viejas 
    }
}

int main(){
    // (demo) agarramos ruta de imagen con scanf
    char imgRoute[256];
    printf("enter img route: ");
    scanf("%255s", imgRoute);
    
    // control de errores
    // ruta vacia
    if (strlen(imgRoute) == 0){
        printf("ERR3: The route cant be empty. Exit...\n");
        return 1;
    }
    
    // ruta invalida
    if (access(imgRoute, F_OK) != 0){
        printf("ERR2: Invalid Route, Exit...\n");
        return 1;
    }

    int imgW, imgH, channels; // alto, ancho, rgba
    unsigned char *pixels = stbi_load(imgRoute, &imgW, &imgH, &channels, 1); // guardamos imagen en ram

    // imagen invalida/corrupta
    if (pixels == NULL){
        printf("ERR1: Invalid Image. Exit...\n");
        return 1;
    }

    // definimos tamaño de terminal (term*) y llamamos a obtainSize para recibir los valores
    int termX, termY;
    float termProp;
    obtainSize(&termX, &termY, &termProp);
    
    // por seguridad, si se superan los valores maximos, se igualan
    if (termX > MAX_X) termX = MAX_X;
    if (termY > MAX_Y) termY = MAX_Y;

    // calculamos las dimensiones adaptadas para ambos ejes
    int drawCols = termX;
    int drawRows = termY;

    // sacamos la relacion de aspecto de la foto original
    float imgRatio = (float)imgH / imgW;
    
    // proponemos los dos tamaños posibles
    int propH = (int)(termX * imgRatio * termProp);
    int propW = (int)(termY / (imgRatio * termProp));

    // elegimos que eje manda para que no se deforme
    if (propH <= termY) {
        drawRows = propH;
    } else {
        drawCols = propW;
    }

    // doble control por seguridad de las matrices globales
    if (drawRows > termY) drawRows = termY;
    if (drawCols > termX) drawCols = termX;

    // ok llego el dificil: EL FOR
    printf("\033[2J\033[?25l"); // limpiamos pantalla y ocultamos cursor

    // primer for: eje y 
    for(int y = 0; y < drawRows; y++){
        
        // regla de 3 directa vertical
        float pxRowYLen = (float)imgH / drawRows;
        int iY = (int)(y * pxRowYLen);
        
        // segundo for: eje x 
        for(int x = 0; x < drawCols; x++){
            
            // regla de 3 directa horizontal
            float pxColXLen = (float)imgW / drawCols;
            int iX = (int)(x * pxColXLen);
            
            // seguridad para evitar desbordes de la imagen
            if (iX >= imgW) iX = imgW - 1;
            if (iY >= imgH) iY = imgH - 1;
            if (iX < 0) iX = 0;
            if (iY < 0) iY = 0;

            // leemos el brillo con los indices corregidos
            unsigned char brightness = pixels[iY * imgW + iX];
            int iPall = (brightness * (PALLETTE_LEN - 1)) / 255;

            // guardamos el caracter generado en el buffer
            actBuffer[y][x] = PALLETTE[iPall];
        }
    }

    // bucle de renderizado acotado al tamaño real de dibujo
    for (int y = 0; y < drawRows; y++){
        for(int x = 0; x < drawCols; x++){
            if(actBuffer[y][x] != preBuffer[y][x]){
                printf("\033[%d;%dH%c", y + 1, x + 1, actBuffer[y][x]);
            }
        }
    }

    // mandamos todo de una
    fflush(stdout);

    // clonamos el lienzo actual para dejar la memoria guardada
    memcpy(preBuffer, actBuffer, sizeof(actBuffer));

    // limpiamos heap y devolvemos cursor
    stbi_image_free(pixels);
    printf("\033[?25h\033[%d;1H\n", termY + 1);

    return 0;
}
