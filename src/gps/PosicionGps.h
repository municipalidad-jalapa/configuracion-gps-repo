#pragma once

#include <cstddef>

/*
 * Modelo comun de posicion. Independiente de TinyGPSPlus y del transporte.
 * Los nombres coinciden con PosicionRequest del backend.
 */
struct PosicionGps {
    double latitud;
    double longitud;
    double velocidadKmh;
    bool tieneVelocidad;
    char timestampUtc[21];
};

inline void copiarPosicion(PosicionGps& destino, const PosicionGps& origen) {
    destino.latitud = origen.latitud;
    destino.longitud = origen.longitud;
    destino.velocidadKmh = origen.velocidadKmh;
    destino.tieneVelocidad = origen.tieneVelocidad;
    for (size_t i = 0; i < sizeof(destino.timestampUtc); ++i) {
        destino.timestampUtc[i] = origen.timestampUtc[i];
    }
}
