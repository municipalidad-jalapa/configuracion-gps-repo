#pragma once

#include "../almacenamiento/RepositorioPosiciones.h"
#include "../gps/ProveedorGps.h"
#include "../red/ClienteHttpTelemetria.h"
#include "../red/Conectividad.h"

class GestorTelemetria {
public:
    GestorTelemetria(ProveedorGps& gps,
                     RepositorioPosiciones& repositorio,
                     Conectividad& conectividad,
                     ClienteHttpTelemetria& cliente);

    void iniciar();
    void procesar();

private:
    void capturarSiCorresponde();
    void enviarSiCorresponde();
    bool credencialPareceConfigurada() const;

    ProveedorGps& _gps;
    RepositorioPosiciones& _repositorio;
    Conectividad& _conectividad;
    ClienteHttpTelemetria& _cliente;
    unsigned long _ultimaCapturaMs = 0;
    unsigned long _ultimoIntentoEnvioMs = 0;
    unsigned long _ultimoAvisoFixMs = 0;
    bool _huboFix = false;
};
