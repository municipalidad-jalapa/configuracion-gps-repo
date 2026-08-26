#include "almacenamiento/RepositorioPosiciones.h"
#include "configuracion.h"
#include "gps/Neo6mGps.h"
#include "red/ClienteHttpTelemetria.h"
#include "red/ConectividadWifi.h"
#include "telemetria/GestorTelemetria.h"

#include <Arduino.h>

static Neo6mGps gps(Serial2);
static ConectividadWifi wifi;
static RepositorioPosiciones repositorio;
static ClienteHttpTelemetria cliente;
static GestorTelemetria gestor(gps, repositorio, wifi, cliente);

void setup() {
    Serial.begin(SERIAL_DIAGNOSTICO_BAUD);
    delay(200);

    gps.iniciar();
    repositorio.iniciar();
    wifi.iniciar();
    gestor.iniciar();
}

void loop() {
    gestor.procesar();
}
