#include "ClienteHttpTelemetria.h"

#include "configuracion.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <cstdio>
#include <cstring>

namespace {

constexpr size_t CAPACIDAD_CUERPO = 4096;
constexpr size_t CAPACIDAD_AUTORIZACION = 80;

bool urlEsHttps(const char* url) {
    return strncmp(url, "https://", 8) == 0;
}

void aplicarAutorizacion(HTTPClient& http) {
    char autorizacion[CAPACIDAD_AUTORIZACION];
    snprintf(autorizacion, sizeof(autorizacion), "Bearer %s", CREDENCIAL_EQUIPO);
    autorizacion[sizeof(autorizacion) - 1] = '\0';
    http.addHeader("Authorization", autorizacion);
}

void interpretarRespuesta(HTTPClient& http, int codigo, ResultadoEnvio& resultado) {
    resultado.codigoHttp = codigo;
    if (codigo == HTTP_CODIGO_LOTE_ACEPTADO) {
        resultado.tipo = TipoResultadoEnvio::ACEPTADO;
        const String cuerpo = http.getString();
        JsonDocument resumen;
        if (!deserializeJson(resumen, cuerpo)) {
            resultado.recibidas = resumen["recibidas"] | 0;
            resultado.aceptadas = resumen["aceptadas"] | 0;
            resultado.descartadas = resumen["descartadas"] | 0;
        }
        return;
    }
    if (codigo < 0) {
        resultado.tipo = TipoResultadoEnvio::ERROR_CONEXION;
        return;
    }
    resultado.tipo = TipoResultadoEnvio::ERROR_HTTP;
}

}  // namespace

bool ClienteHttpTelemetria::serializarLote(const PosicionGps* posiciones, size_t cantidad,
                                           char* destino, size_t capacidad, size_t& escrito) const {
    JsonDocument documento;
    JsonArray lote = documento["posiciones"].to<JsonArray>();
    for (size_t i = 0; i < cantidad; ++i) {
        JsonObject item = lote.add<JsonObject>();
        item["latitud"] = posiciones[i].latitud;
        item["longitud"] = posiciones[i].longitud;
        if (posiciones[i].tieneVelocidad) {
            item["velocidadKmh"] = posiciones[i].velocidadKmh;
        }
        item["timestamp"] = posiciones[i].timestampUtc;
    }

    escrito = serializeJson(documento, destino, capacidad);
    return escrito > 0 && escrito < capacidad;
}

ResultadoEnvio ClienteHttpTelemetria::enviarLote(const PosicionGps* posiciones, size_t cantidad) {
    ResultadoEnvio resultado;
    if (posiciones == nullptr || cantidad == 0) {
        resultado.tipo = TipoResultadoEnvio::ERROR_HTTP;
        return resultado;
    }

    char cuerpo[CAPACIDAD_CUERPO];
    size_t longitudCuerpo = 0;
    if (!serializarLote(posiciones, cantidad, cuerpo, sizeof(cuerpo), longitudCuerpo)) {
        resultado.tipo = TipoResultadoEnvio::ERROR_HTTP;
        return resultado;
    }

    char url[160];
    snprintf(url, sizeof(url), "%s%s", BASE_URL, RUTA_INGESTA_TELEMETRIA);
    url[sizeof(url) - 1] = '\0';

    HTTPClient http;
    http.setTimeout(HTTP_TIMEOUT_MS);
    http.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);

    bool iniciado = false;
    WiFiClient cliente;
    WiFiClientSecure clienteTls;

    if (urlEsHttps(BASE_URL)) {
#if USAR_TLS_INSEGURO
        Serial.println("TLS inseguro activo (solo desarrollo)");
        clienteTls.setInsecure();
        iniciado = http.begin(clienteTls, url);
#else
        Serial.println("HTTPS sin CA configurada: envio omitido");
        resultado.tipo = TipoResultadoEnvio::ERROR_CONEXION;
        return resultado;
#endif
    } else {
        iniciado = http.begin(cliente, url);
    }

    if (!iniciado) {
        resultado.tipo = TipoResultadoEnvio::ERROR_CONEXION;
        return resultado;
    }

    http.addHeader("Content-Type", "application/json");
    aplicarAutorizacion(http);

    const int codigo = http.POST(reinterpret_cast<uint8_t*>(cuerpo), longitudCuerpo);
    interpretarRespuesta(http, codigo, resultado);
    http.end();
    return resultado;
}
