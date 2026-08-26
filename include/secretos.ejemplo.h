#pragma once

/*
 * Copiar este archivo como include/secretos.local.h y rellenar los valores
 * de prueba. secretos.local.h no se versiona: no incluir credenciales reales
 * en el repositorio.
 *
 * CREDENCIAL_EQUIPO es la credencial emitida por HU-47
 * (POST /api/v1/admin/equipos), formato eq_<codigoPublico>.<secreto>.
 * No viaja en la URL ni en el JSON de posiciones.
 */

#define WIFI_SSID "CAMBIAR_SSID"
#define WIFI_PASSWORD "CAMBIAR_PASSWORD"
#define CREDENCIAL_EQUIPO "eq_CAMBIARXXXXXX.CAMBIAR_SECRETO_DE_43_CARACTERES_BASE64URL"
#define BASE_URL "http://CAMBIAR_HOST:8080"
