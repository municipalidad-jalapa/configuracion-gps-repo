#pragma once

/*
 * Configuracion central del prototipo HU-49.
 * Pines, intervalos y URL viven aqui. Secretos de prueba van en
 * secretos.local.h (ver secretos.ejemplo.h); no dispersarlos por el codigo.
 */

#if defined(__has_include)
#  if __has_include("secretos.local.h")
#    include "secretos.local.h"
#  endif
#endif

#ifndef WIFI_SSID
#define WIFI_SSID "CAMBIAR_SSID"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "CAMBIAR_PASSWORD"
#endif

#ifndef CREDENCIAL_EQUIPO
#define CREDENCIAL_EQUIPO "eq_CAMBIARXXXXXX.CAMBIAR_SECRETO_DE_43_CARACTERES_BASE64URL"
#endif

#ifndef BASE_URL
#define BASE_URL "http://CAMBIAR_HOST:8080"
#endif

/* Contrato confirmado en TelemetriaController: POST /api/v1/telemetria/posiciones */
#define RUTA_INGESTA_TELEMETRIA "/api/v1/telemetria/posiciones"

/*
 * UART hardware 2 del ESP32. GPS_RX_PIN recibe el TX del NEO-6M.
 * Sustituir pines aqui, no en el codigo del proveedor GPS.
 */
#define GPS_RX_PIN 16
#define GPS_TX_PIN 17
#define GPS_BAUD 9600

#define SERIAL_DIAGNOSTICO_BAUD 115200

#define INTERVALO_POSICION_MS 5000
#define INTERVALO_RECONEXION_WIFI_MS 10000
#define INTERVALO_REINTENTO_ENVIO_MS 3000
#define INTERVALO_LOG_ESPERANDO_FIX_MS 10000

#define HTTP_TIMEOUT_MS 8000

/*
 * El backend no impone @Size sobre el lote. 20 es un tope conservador del
 * firmware para no saturar RAM ni el POST en el ESP32.
 */
#define TAMANO_MAXIMO_LOTE 20

#define COLA_MAXIMA_POSICIONES 500

#define HTTP_CODIGO_LOTE_ACEPTADO 202

/*
 * 0 = no desactivar la verificacion TLS.
 * 1 = solo desarrollo local contra un HTTPS sin CA conocida.
 * El prototipo de esta iteracion usa HTTP hacia el backend local.
 */
#ifndef USAR_TLS_INSEGURO
#define USAR_TLS_INSEGURO 0
#endif
