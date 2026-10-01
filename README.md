# asciimov: Reproductor de video ASCII 

Un __*reproductor de video*__ de arte ascii escrito en C  
Funciona obteniendo el video en blanco y negro mas su audio atraves de `ffmpeg`, para asi ejecutar un bucle de dibujado y renderizado basandose en una paleta de 70 caracteres  
Tiene soporte para URLS gracias a `yt-dlp` mas `node.js` para poder descargar videos.  
Estos se guadan en `/tmp/asciimov_temp.mp4`, pero tras la ejecucion del programa se borran. 

## Dependencias
Van a necesitar una lista de 5 cosas para la ejecucion e instalacion:  
- `make`: Para el script de instalacion automatizado
- `gcc`: Para compilar el .c
- `ffmpeg`: Para el video y audio
- `yt-dlp`: Para soporte de las URL
- `node.js`: Para soporte de las URL con JavaScript

## Instalacion
Para instalar el programa, como ya se esclarecio, se necesita `make`.  
Pueden montarlo en una carpeta o instalarlo globalmente con los siguientes comandos:  
- `make`: Montaje local
- `make clean`: Borrado del montaje
- `make && sudo make install`: Instalacion global (`make` se ejecuta antes para el copiado de los binarios)
- `sudo make uninstall`: Desinstalacion

## Detalles
Esta en una version primitiva todavia, no tiene pausa ni rebobinado, y las rutas se manejan con un `scanf` en vez de con flags.  
Ante cualquier bug, saca una pull request y avisa! No me hare problema en buscar el error  

Gracias por leer el README! Espero que disfruten del reproductor.  PEACE -ac15
