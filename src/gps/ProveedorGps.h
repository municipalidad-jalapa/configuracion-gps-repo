#pragma once

#include "PosicionGps.h"

/*
 * Contrato de la fuente de ubicacion. El NEO-6M V2 es el proveedor de esta
 * iteracion; el hardware definitivo implementara la misma interfaz sin
 * tocar cola, serializacion ni gestor de telemetria.
 */
class ProveedorGps {
public:
    virtual ~ProveedorGps() = default;
    virtual void iniciar() = 0;
    virtual void actualizar() = 0;
    virtual bool hayPosicionValida() const = 0;
    virtual bool obtenerPosicion(PosicionGps& posicion) = 0;
    virtual unsigned long caracteresNmea() const { return 0; }
    virtual unsigned long nmeaCorrectas() const { return 0; }
    virtual unsigned long nmeaCorruptas() const { return 0; }
    virtual int satelites() const { return -1; }
    virtual bool ubicacionValida() const { return false; }
    virtual bool fechaValida() const { return false; }
    virtual bool horaValida() const { return false; }
};
