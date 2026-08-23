#pragma once

/*
 * Contrato de conectividad. Esta iteracion usa Wi-Fi hacia el hotspot del
 * telefono. Un modulo celular futuro implementa la misma interfaz; no se
 * programan AT, APN ni TinyGSM en HU-49.
 */
class Conectividad {
public:
    virtual ~Conectividad() = default;
    virtual void iniciar() = 0;
    virtual void mantenerConexion() = 0;
    virtual bool estaConectado() const = 0;
};
