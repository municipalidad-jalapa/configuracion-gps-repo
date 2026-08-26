#pragma once

#include "Conectividad.h"

class ConectividadWifi : public Conectividad {
public:
    void iniciar() override;
    void mantenerConexion() override;
    bool estaConectado() const override;
};
