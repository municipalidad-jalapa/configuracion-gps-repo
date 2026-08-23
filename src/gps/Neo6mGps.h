#pragma once

#include "ProveedorGps.h"

#include <HardwareSerial.h>
#include <TinyGPSPlus.h>

class Neo6mGps : public ProveedorGps {
public:
    explicit Neo6mGps(HardwareSerial& uart);

    void iniciar() override;
    void actualizar() override;
    bool hayPosicionValida() const override;
    bool obtenerPosicion(PosicionGps& posicion) override;

private:
    bool intentarCapturar();

    HardwareSerial& _uart;
    TinyGPSPlus _parser;
    PosicionGps _ultima{};
    bool _listaParaConsumir = false;
};
