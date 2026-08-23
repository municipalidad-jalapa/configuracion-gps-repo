#include "ConectividadWifi.h"

#include "configuracion.h"

#include <WiFi.h>

void ConectividadWifi::iniciar() {
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    _ultimoIntentoMs = millis();
}

void ConectividadWifi::mantenerConexion() {
    const bool ahoraConectado = WiFi.status() == WL_CONNECTED;
    if (ahoraConectado && !_conectado) {
        Serial.println("WiFi conectado");
    } else if (!ahoraConectado && _conectado) {
        Serial.println("WiFi desconectado");
    }
    _conectado = ahoraConectado;

    if (_conectado) {
        return;
    }

    const unsigned long ahora = millis();
    if (ahora - _ultimoIntentoMs < INTERVALO_RECONEXION_WIFI_MS) {
        return;
    }
    _ultimoIntentoMs = ahora;
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

bool ConectividadWifi::estaConectado() const {
    return _conectado;
}
