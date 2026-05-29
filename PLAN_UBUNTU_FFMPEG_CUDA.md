# Plan de Migración y Mejora: VirtualDub en Ubuntu con FFmpeg y CUDA

Este documento detalla la estrategia de alto nivel y la arquitectura necesaria para modernizar VirtualDub, llevándolo de una aplicación puramente ligada a Windows a una aplicación nativa de Ubuntu, con soporte robusto de formatos modernos mediante FFmpeg y aceleración de hardware mediante NVIDIA CUDA.

## Fase 1: Preparación y Nuevo Sistema de Compilación (CMake)
VirtualDub está fuertemente acoplado al ecosistema de Visual Studio. El primer paso para la portabilidad es modernizar la cadena de construcción.
*   **Adopción de CMake:** Reemplazar los archivos de solución y proyecto de Visual Studio (`.sln`, `.vcproj`/`.vcxproj`) por una estructura de `CMakeLists.txt`. CMake permitirá generar archivos de construcción para `make` o `ninja` en Linux.
*   **Configuración del Compilador:** Configurar directivas para GCC o Clang en Ubuntu.
*   **Gestión de Dependencias Básicas:** Identificar librerías estándar en desuso o propietarias y reemplazarlas por equivalentes estándar de C/C++ modernos (C++11/14 o superior) compatibles con POSIX.

## Fase 2: Desacoplamiento de la API Win32 y Abstracción del Core
El desafío principal de portabilidad es la extensa dependencia de la API de Windows (Win32, GDI, Video for Windows, DirectShow).
*   **Separación Lógica:** Identificar y separar la lógica "Core" (el motor de procesamiento de frames, gestión del flujo de video, procesamiento por lotes) de cualquier llamada a la interfaz gráfica o funciones del sistema operativo.
*   **Capa de Abstracción del Sistema (OSAL):** Crear una capa de abstracción para el manejo de archivos (file I/O), gestión de memoria, y operaciones multi-hilo (reemplazando hilos de Win32 por `std::thread` o pthreads).
*   **Desuso de Video for Windows (VfW):** Aislar completamente el antiguo sistema de captura y compresión VfW, ya que será reemplazado por soluciones modernas y multiplataforma en fases posteriores.

## Fase 3: Desarrollo de la Interfaz Gráfica (UI) Nativa
Para que la aplicación sea nativa en Ubuntu, se requiere abandonar los controles de Windows GDI/USER32.
*   **Selección de Framework:** Adoptar un framework de interfaz gráfica multiplataforma robusto, siendo **Qt** o **GTK** (GTK3/GTK4) las mejores opciones para una integración natural en el entorno de escritorio de Ubuntu (GNOME/KDE).
*   **Reescritura de la UI:** Replicar la interfaz clásica y eficiente de VirtualDub (línea de tiempo, paneles de video de origen y salida, ventanas de filtros) utilizando el nuevo framework.
*   **Conexión Core-UI:** Implementar el patrón MVC (Model-View-Controller) para conectar la nueva interfaz gráfica con el Core de procesamiento abstraído en la Fase 2.

## Fase 4: Integración de FFmpeg (C Libraries) para Importación/Exportación
VirtualDub históricamente dependía de codecs del sistema Windows. Para modernizarlo y darle soporte a la gran mayoría de formatos (MP4, MKV, H.264, HEVC, etc.), se integrará FFmpeg nativamente.
*   **Vinculación Directa:** Configurar el sistema de compilación (CMake) para enlazar estática o dinámicamente con las librerías de C de FFmpeg: `libavcodec`, `libavformat`, `libavutil` y `libswscale`.
*   **Motor de Lectura (Demuxing/Decoding):** Implementar un módulo de lectura de video que utilice `libavformat` para abrir contenedores y `libavcodec` para extraer y decodificar frames a un formato en memoria sin comprimir que el Core de VirtualDub pueda procesar.
*   **Motor de Exportación (Encoding/Muxing):** Desarrollar un módulo de salida que reciba frames procesados, utilice `libswscale` para conversiones de espacio de color si es necesario, los codifique utilizando `libavcodec` y los empaquete en el formato deseado usando `libavformat`.

## Fase 5: Aceleración por Hardware (Soporte CUDA Nativo)
Aprovechar la GPU es esencial para el procesamiento moderno de video de alta resolución.
*   **Integración del CUDA Toolkit:** Añadir soporte en CMake para compilar código CUDA (`.cu`) usando el compilador `nvcc`.
*   **Procesamiento de Filtros:** Identificar las operaciones más costosas en CPU (por ejemplo: reescalado, conversiones de espacio de color, filtros de desenfoque, reducción de ruido). Reescribir la lógica matemática de estos filtros utilizando "kernels" de CUDA, permitiendo que miles de hilos en la GPU procesen los píxeles de un frame simultáneamente.
*   **Aceleración de Codecs (NVENC/NVDEC):** Configurar la integración de FFmpeg de la Fase 4 para priorizar el uso de codecs acelerados por hardware de NVIDIA (como `h264_nvenc` y decodificadores `cuvid`/`nvdec`), reduciendo drásticamente la carga de la CPU durante la importación y exportación de video.
*   **Transferencia de Memoria Optimizada:** Implementar una gestión eficiente de la memoria de la GPU, minimizando las transferencias (copias de Host-a-Device y Device-a-Host) manteniendo los frames en la VRAM de la tarjeta gráfica mientras pasen a través de los distintos filtros de procesamiento de video.

## Fase 6: Opciones Básicas de Audio (Planificación)
Para que VirtualDubLin sea completamente funcional como editor, el audio debe ser tratado a la par del video.
*   **Interfaz Gráfica de Audio:**
    *   Implementar un menú `Audio` con las opciones clásicas: `No audio`, `Source audio` (por defecto) y `Audio from other file...`.
    *   Añadir opciones de procesamiento: `Direct stream copy` y `Full processing mode`.
    *   Añadir diálogo de compresión (`Compression...`) que permita seleccionar el formato (AAC, MP3, FLAC) y el bitrate (ej. 192kbps).
*   **Demuxing (FFmpegDecoder):**
    *   Modificar `FFmpegDecoder` para que no solo busque el stream de video, sino que detecte e inicialice el stream de audio (`AVMEDIA_TYPE_AUDIO`).
    *   Refactorizar el bucle de lectura (`av_read_frame`) para que entregue los paquetes (`AVPacket`) entrelazados (audio y video) a la lógica de procesamiento en lugar de descartarlos.
*   **Muxing y Encoding (FFmpegEncoder):**
    *   Ampliar el `FFmpegEncoder` para que acepte los parámetros de audio recopilados desde la UI.
    *   En modo `Direct stream copy`, rutear el paquete de audio crudo directamente al archivo contenedor de salida ajustando los sellos de tiempo (`pts`/`dts`).
    *   En modo `Full processing mode`, inicializar un codificador (`libfdk_aac` o el nativo de ffmpeg), enviar los frames PCM decodificados y empaquetarlos en el archivo de salida.
