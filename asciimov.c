#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define MAX_X 800
#define MAX_Y 400

// paleta ascii
const char PALLETTE[] = " .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
const int PALLETTE_LEN = 70;

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

        if(w.ws_xpixel > 0 && w.ws_ypixel > 0){
            float pixelXLen = (float)w.ws_xpixel/w.ws_col;
            float pixelYLen = (float)w.ws_ypixel/w.ws_row;
            *proportion = pixelXLen / pixelYLen;
        } else {
            *proportion = 0.5f;
        }
    } else {
        *x = 80;
        *y = 24;
        *proportion = 0.5f;
    }
}

int main(){
    // borramos si el temp esta ocupado
    system("rm -f /tmp/asciimov_temp.mp4");
    // definimos la resolucion estandar a la que ffmpeg nos va a mandar los frames
    int imgW = 640;
    int imgH = 480;
    
    // (demo) variable para guardar el nombre o la url de youtube del video
    char videoRoute[256];
    printf("enter video route or url: ");
    scanf("%255s", videoRoute);

    // control de errores de la entrada
    if (strlen(videoRoute) == 0){
        printf("ERR3: The route cant be empty. Exit...\n");
        return 1;
    }

    // ultimo valor de x y y
    int lastCols = 0;
    int lastRows = 0;

    // calculamos el tamaño del frame
    int frameSize = imgH * imgW;
    // reservamos un bloque en la heap para guardar el frame
    unsigned char *videoPixels = (unsigned char *)malloc(frameSize); 

    // reservamos buffer para el comando de ffmpeg
    char codecCommand[1024]; // lo agrandamos a 1024 por seguridad de internet
    
        // controlamos si es una url de internet o un archivo local
    if (strncmp(videoRoute, "http", 4) == 0) {
        // buffer temporal para armar el comando de yt-dlp
        char downloadCommand[1024];

        // armamos el comando inyectando la variable videoRoute al final
        sprintf(downloadCommand, "yt-dlp -f \"b\" --js-runtimes node --remote-components ejs:github --cookies-from-browser firefox -o \"/tmp/asciimov_temp.mp4\" \"%s\"", videoRoute);
        
        // descargamos el video
        system(downloadCommand);
       
        // ejecutamos ffmpeg para reproducirlo
        sprintf(codecCommand, "ffmpeg -i \"/tmp/asciimov_temp.mp4\" -f alsa default -vn -loglevel quiet & ffmpeg -i \"/tmp/asciimov_temp.mp4\" -f image2pipe -vcodec rawvideo -pix_fmt gray -s 640x480 -loglevel quiet -");

       } else {
        // es un video local comun: llamamos a ffmpeg directo al archivo
        sprintf(codecCommand, "ffmpeg -i \"%s\" -f alsa default -vn -loglevel quiet & ffmpeg -i \"%s\" -f image2pipe -vcodec rawvideo -pix_fmt gray -s 640x480 -loglevel quiet -", videoRoute, videoRoute);
    }


    // ejecutamos el comando con popen en modo lectura
    FILE *pipeIn = popen(codecCommand, "r");
    // si ffmpeg no esta instalado o si falla el descriptor
    if (pipeIn == NULL){
        printf("ERR4: Failed to open FFMPEG. Please check dependencies. Exit...");
        free(videoPixels);
        return 1;
    }

    // ocultamos cursor al arrancar
    printf("\033[?25l");

    // bucle de video
    while(1){
        // leemos un frame desde el pipe de ffmpeg
        // fread devuelve cuantos elementos leyo con exito
        if(fread(videoPixels, 1, frameSize, pipeIn) != frameSize){
            break; // si lee menos bytes q framesize significa q termino, entonces terminamos
        }

        // miramos cuanto mide la terminal en cada frame para adaptarse al tamaño actual
        int termX, termY;
        float termProp;
        obtainSize(&termX, &termY, &termProp);

        // proteccion de desborde
        if (termX > MAX_X) termX = MAX_X;
        if (termY > MAX_Y) termY = MAX_Y;

        // calculamos las dimensiones adaptadas para el frame exacto
        int drawCols = termX;
        int drawRows = termY;

        float imgRatio = (float)imgH / imgW;
        int propH = (int)(termX * imgRatio * termProp);
        int propW = (int)(termY / (imgRatio * termProp));

        if (propH <= termY) {
            drawRows = propH;
        } else {
            drawCols = propW;
        }

        if (drawRows > termY) drawRows = termY;
        if (drawCols > termX) drawCols = termX;
        
        // controlamos redimension
        // si se estira la pantalla o se achica se procede a redibujar todo
        if (drawCols != lastCols || drawRows != lastRows) {
            printf("\033[2J"); 
            memset(preBuffer, 0, sizeof(preBuffer)); 
            lastCols = drawCols;
            lastRows = drawRows;
        }
        // bucles de renderizado
        // primer for: eje y
        for(int y = 0; y < drawRows; y++){
            // regla de 3
            float pxRowYLen = (float)imgH / drawRows;
            int iY = (int)(y * pxRowYLen);

            // segundo for: eje x
            for(int x = 0;  x < drawCols; x++){
                // regla de 3
                float pxColXLen = (float)imgW / drawCols;
                int iX = (int)(x * pxColXLen);

                // seguridad para evitar desbordes de la imagen
                if (iX >= imgW) iX = imgW - 1;
                if (iY >= imgH) iY = imgH - 1;
                if (iX < 0) iX = 0;
                if (iY < 0) iY = 0;

                // leemos en vivo desde el buffer del video actual
                unsigned char brightness = videoPixels[iY * imgW + iX];
                int iPall = (brightness * (PALLETTE_LEN - 1)) / 255;

                // guardamos el caracter generado en el buffer
                actBuffer[y][x] = PALLETTE[iPall];
            }
        }

        // bucle de dibujado
        for (int y = 0; y < drawRows; y++){
            for(int x = 0; x < drawCols; x++){
                if(actBuffer[y][x] != preBuffer[y][x]){
                    printf("\033[%d;%dH%c", y + 1, x + 1, actBuffer[y][x]);
                }
            }
        }

        // mandamos todo a la consola
        fflush(stdout);

        // clonamos el lienzo para dejar la memoria guardada ante la proxima vuelta
        memcpy(preBuffer, actBuffer, sizeof(actBuffer));

        // dejamos en 30fps
        usleep(33333);
    }

    // matamos a ffmpeg cuando termine el while
    pclose(pipeIn);
    // matamos todo lo q sobre de ffmpeg
    system("killall -q ffmpeg");
    usleep(10000);
    // borramos el video si viene de url
    system("rm -f /tmp/asciimov_temp.mp4");
    // vaciamos el buffer
    free(videoPixels);
    // devolvemos el cursor al terminar
    printf("\033[?25h\n");
    return 0;
}