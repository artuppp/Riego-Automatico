<p align="center">
  <!-- Reemplaza YOUR_USERNAME/YOUR_REPO_NAME con tu usuario y repositorio de GitHub -->
  <a href="https://github.com/artuppp/Riego-automatico"><img src="https://img.shields.io/badge/ESP32_Riego_Inteligente-v1.0-blueviolet"/></a>
  <a href="https://github.com/artuppp/Riego-automatico"><img src="https://img.shields.io/badge/código-Fuente-yellowgreen"/></a>
  <a href="#%EF%B8%8F-licencia"><img src="https://img.shields.io/badge/licencia-MIT-green"/></a>
  <a href="https://github.com/artuppp/Riego-automatico/commits/main"><img src="https://img.shields.io/github/last-commit/YOUR_USERNAME/YOUR_REPO_NAME"/></a>
  <br>
  <a href="https://github.com/artuppp/Riego-automatico/stargazers"><img src="https://img.shields.io/github/stars/YOUR_USERNAME/YOUR_REPO_NAME?style=social"/></a>
  <a href="https://github.com/artuppp/Riego-automatico/network/members"><img src="https://img.shields.io/github/forks/YOUR_USERNAME/YOUR_REPO_NAME?style=social"/></a>
  <a href="https://github.com/artuppp/Riego-automatico/watchers"><img src="https://img.shields.io/github/watchers/YOUR_USERNAME/YOUR_REPO_NAME?style=social"/></a>
  <a href="https://github.com/artuppp/Riego-automatico/issues"><img src="https://img.shields.io/github/issues/YOUR_USERNAME/YOUR_REPO_NAME"/></a>
</p>

<p align="center">
  <img src="https://i.imgur.com/uSwn217.png" alt="Demo de la Interfaz Web" width="700"/>
</p>

<h1 align="center">🌿 Sistema de Riego Inteligente para ESP32 <sub>v1.0</sub></h1>

<p align="center">
  <i>Controla y monitoriza tus plantas con una interfaz web completa, gráficos históricos y lógica de riego flexible.</i>
</p>
<hr>

Un completo y robusto sistema de riego automático basado en la placa ESP32. Controla y monitoriza tus plantas a través de una intuitiva **interfaz web**, accesible desde cualquier dispositivo en tu red local. Ideal para huertos urbanos, macetas o pequeños jardines.

## ✨ Características Principales

*   **Interfaz Web Completa:** Panel de control moderno y responsive (adaptable a móvil) construido con Bootstrap y Chart.js.
*   **Monitorización en Tiempo Real:** Visualiza al instante la temperatura, la humedad ambiental y la humedad del suelo.
*   **Gráficos Históricos:** Gráficos interactivos que muestran las tendencias de las últimas 24 horas para todos los sensores, actualizados cada minuto.
*   **Control de Riego Flexible:**
    *   **Modo Manual:** Activa o desactiva el riego con un solo clic.
    *   **Modo Automático por Humedad:** Define un umbral de humedad del suelo; el sistema regará automáticamente cuando sea necesario.
    *   **Modo Automático por Horario:** Programa hasta dos riegos diarios a las horas que prefieras.
*   **Configuración Persistente:** Todos tus ajustes (umbral, duración, horarios) se guardan en la memoria no volátil del ESP32 y se conservan tras un reinicio.
*   **Código Eficiente:** El `loop()` principal es no bloqueante, permitiendo que el servidor web y la lógica de control se ejecuten de forma fluida y simultánea.
*   **Sincronización Horaria:** Obtiene la hora exacta de internet (NTP) para una programación precisa de los riegos.

---

## 🛠️ Componentes Necesarios

| Componente                | Descripción                                                   |
| :------------------------ | :------------------------------------------------------------ |
| **Placa de Desarrollo**   | ESP32 (cualquier variante con WiFi)                           |
| **Sensor de Temp/Hum.**   | DHT22 (o DHT11, ajustando `DHTTYPE` en el código)             |
| **Sensor de Humedad**     | Sensor de humedad de suelo capacitivo (recomendado)           |
| **Actuador**              | Módulo de relé de 1 canal (5V o 3.3V)                         |
| **Bomba / Electroválvula**| Una pequeña bomba de agua de 5V/12V o una electroválvula       |
| **Fuente de Alimentación**| Externa para la bomba/electroválvula.                         |
| **Cables y Protoboard**   | Cables de conexión (jumpers) y protoboard para el montaje.    |

---

## 🔌 Diagrama de Conexiones

Asegúrate de conectar los componentes a los pines correctos del ESP32 según se define en el código.

| Componente             | Pin del Componente | Pin del ESP32 |
| :--------------------- | :----------------- | :------------ |
| **Sensor DHT22**       | DATA               | `GPIO 4`      |
| **Módulo de Relé**     | IN                 | `GPIO 12`     |
| **Sensor de Suelo**    | AOUT (Analógico)   | `GPIO 34`     |

**Notas Importantes:**
*   Conecta el `VCC` y `GND` de los sensores y el relé a los pines `3.3V` (o `5V`, según el módulo) y `GND` del ESP32 respectivamente.
*   **¡Nunca alimentes la bomba o electroválvula directamente desde el ESP32!** Usa el relé como un interruptor y una fuente de alimentación externa para la carga.

---

## 🚀 Puesta en Marcha

1.  **Configura tu Entorno:**
    *   Abre el proyecto en el [IDE de Arduino](https://www.arduino.cc/en/software) o [PlatformIO](https://platformio.org/).
    *   Asegúrate de tener instalado el soporte para placas ESP32.
    *   Instala la librería **"DHT sensor library" de Adafruit** desde el gestor de librerías. Las demás librerías (`WiFi`, `WebServer`, etc.) ya vienen incluidas con el core de ESP32.

2.  **Modifica las Credenciales WiFi:**
    *   Abre el archivo de código y modifica las siguientes líneas con el nombre y la contraseña de tu red WiFi:
    ```cpp
    // --- CONFIGURACIÓN DE RED ---
    #define WIFI_SSID "TU_RED_WIFI"
    #define WIFI_PASSWORD "TU_CONTRASEÑA"
    ```

3.  **Compila y Sube el Código:**
    *   Selecciona tu placa ESP32 y el puerto COM correcto.
    *   Pulsa el botón de subir.

4.  **Encuentra la Dirección IP:**
    *   Abre el **Monitor Serie** (baud rate: 115200).
    *   Una vez conectado al WiFi, el ESP32 imprimirá su dirección IP.
    ```
    Conectando a WiFi...
    ...
    WiFi conectado. IP: 192.168.1.123
    ```

5.  **¡A Controlar!**
    *   Abre un navegador web en tu ordenador o móvil (conectado a la misma red WiFi) y navega a la dirección IP que obtuviste. ¡Ya estás en el panel de control!

---

## ⚙️ ¿Cómo Funciona la Interfaz?

La interfaz web está dividida en varias secciones claras:

*   **Gráficos de 24h:** Tres gráficos muestran la evolución de la temperatura, humedad ambiental y humedad del suelo. Se actualizan automáticamente cada minuto.
*   **Estado Actual:** Muestra los valores más recientes de los sensores y el estado del relé (riego `ACTIVO` o `APAGADO`). Si el riego está activo, mostrará una cuenta atrás.
*   **Control Manual:** Botones para forzar el encendido (por la duración configurada) o el apagado inmediato del riego.
*   **Configuración:**
    *   **Umbral de Humedad:** Establece el valor del sensor de suelo por debajo del cual se activará el riego automático.
    *   **Duración del Riego:** Define cuántos minutos durará cada ciclo de riego (manual o automático).
*   **Programación de Horarios:** Configura las dos horas del día en las que quieres que se active un ciclo de riego.

Todos los cambios en la configuración se guardan automáticamente en la memoria del ESP32.

---

## 📄 Licencia

Este proyecto se distribuye bajo la **Licencia MIT**. Siéntete libre de usarlo, modificarlo y distribuirlo.

---

<p align="center">
  Creado con ❤️ para plantas más felices.
</p>