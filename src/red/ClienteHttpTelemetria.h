#pragma once

#include "../gps/PosicionGps.h"

#include <cstddef>

enum class TipoResultadoEnvio {
    ACEPTADO,
    ERROR_HTTP,
    ERROR_CONEXION
};

struct ResultadoEnvio {
    TipoResultadoEnvio tipo = TipoResultadoEnvio::ERROR_CONEXION;
    int codigoHttp = 0;
    int recibidas = 0;
    int aceptadas = 0;
    int descartadas = 0;
};

/*
 * Transporte HTTP hacia el endpoint de ingesta. Encapsula HTTPClient para
 * que el gestor no dependa de WiFi ni de la libreria HTTP.
 */
class ClienteHttpTelemetria {
public:
    ResultadoEnvio enviarLote(const PosicionGps* posiciones, size_t cantidad);

private:
    bool serializarLote(const PosicionGps* posiciones, size_t cantidad,
                        char* destino, size_t capacidad, size_t& escrito) const;
};
