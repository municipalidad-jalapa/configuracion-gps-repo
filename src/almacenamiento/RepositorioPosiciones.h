#pragma once

#include "../gps/PosicionGps.h"

#include <cstddef>

class RepositorioPosiciones {
public:
    bool iniciar();
    bool guardar(const PosicionGps& posicion);
    size_t cantidadPendientes() const;
    size_t obtenerPendientes(PosicionGps* destino, size_t maximo, size_t& lineasDelPrefijo);
    bool eliminarConfirmadas(size_t cantidad);

private:
    bool compactarDesde(size_t omitir);
    bool parsearLinea(const char* linea, PosicionGps& posicion) const;
    bool escribirLinea(const PosicionGps& posicion);
    void reconstruirConteo();

    size_t _pendientes = 0;
};
