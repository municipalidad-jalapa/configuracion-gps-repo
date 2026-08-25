#include "ConectividadWifi.h"

#include "configuracion.h"

#include <WiFi.h>

static volatile bool wifiTieneIp = false;
static unsigned long ultimoDiagnosticoMs = 0;
static unsigned long ultimoReintentoMs = 0;

static const char* nombreEstadoWifi(wl_status_t estado) {
    switch (estado) {
        case WL_IDLE_STATUS: return "idle";
        case WL_NO_SSID_AVAIL: return "ssid_no_visible";
        case WL_SCAN_COMPLETED: return "scan_ok";
        case WL_CONNECTED: return "conectado";
        case WL_CONNECT_FAILED: return "clave_o_auth_fallo";
        case WL_CONNECTION_LOST: return "perdido";
        case WL_DISCONNECTED: return "desconectado";
        default: return "otro";
    }
}

static void manejarEventoWifi(WiFiEvent_t evento, WiFiEventInfo_t /*info*/) {
    switch (evento) {
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            if (!wifiTieneIp) {
                Serial.println("WiFi conectado");
            }
            wifiTieneIp = true;
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            if (wifiTieneIp) {
                Serial.println("WiFi desconectado");
            }
            wifiTieneIp = false;
            break;
        default:
            break;
    }
}

void ConectividadWifi::iniciar() {
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.setAutoReconnect(true);
    WiFi.onEvent(manejarEventoWifi);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    ultimoReintentoMs = millis();
}

void ConectividadWifi::mantenerConexion() {
    const unsigned long ahora = millis();
    if (ahora - ultimoDiagnosticoMs >= INTERVALO_LOG_ESPERANDO_FIX_MS) {
        ultimoDiagnosticoMs = ahora;
        if (!wifiTieneIp) {
            Serial.printf("WiFi estado=%s rssi=%d\n",
                          nombreEstadoWifi(WiFi.status()), WiFi.RSSI());
        }
    }

    if (wifiTieneIp) {
        return;
    }
    if (ahora - ultimoReintentoMs < 20000) {
        return;
    }
    ultimoReintentoMs = ahora;
    const wl_status_t estado = WiFi.status();
    if (estado == WL_NO_SSID_AVAIL || estado == WL_CONNECT_FAILED) {
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }
}

bool ConectividadWifi::estaConectado() const {
    return wifiTieneIp;
}
