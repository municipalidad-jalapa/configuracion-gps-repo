#include "Neo6mGps.h"

#include "configuracion.h"

#include <Arduino.h>
#include <cstdio>

Neo6mGps::Neo6mGps(HardwareSerial& uart) : _uart(uart) {}

void Neo6mGps::iniciar() {
    _uart.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    Serial.println("GPS inicializado");
}

void Neo6mGps::actualizar() {
    while (_uart.available() > 0) {
        _parser.encode(static_cast<char>(_uart.read()));
    }
    if (intentarCapturar()) {
        _listaParaConsumir = true;
    }
}

bool Neo6mGps::hayPosicionValida() const {
    return _listaParaConsumir;
}

bool Neo6mGps::obtenerPosicion(PosicionGps& posicion) {
    if (!_listaParaConsumir) {
        return false;
    }
    copiarPosicion(posicion, _ultima);
    _listaParaConsumir = false;
    return true;
}

bool Neo6mGps::intentarCapturar() {
    if (!_parser.location.isValid() || !_parser.location.isUpdated()) {
        return false;
    }
    if (!_parser.date.isValid() || !_parser.time.isValid()) {
        return false;
    }

    const double latitud = _parser.location.lat();
    const double longitud = _parser.location.lng();
    if (latitud == 0.0 && longitud == 0.0) {
        return false;
    }
    if (latitud < -90.0 || latitud > 90.0 || longitud < -180.0 || longitud > 180.0) {
        return false;
    }

    _ultima.latitud = latitud;
    _ultima.longitud = longitud;
    _ultima.tieneVelocidad = _parser.speed.isValid();
    _ultima.velocidadKmh = _ultima.tieneVelocidad ? _parser.speed.kmph() : 0.0;

    /*
     * Timestamp del dispositivo: fecha y hora UTC del GPS en el momento de
     * la captura, no millis() ni la hora de envio posterior.
     */
    snprintf(_ultima.timestampUtc, sizeof(_ultima.timestampUtc),
             "%04d-%02d-%02dT%02d:%02d:%02dZ",
             _parser.date.year(),
             _parser.date.month(),
             _parser.date.day(),
             _parser.time.hour(),
             _parser.time.minute(),
             _parser.time.second());
    _ultima.timestampUtc[sizeof(_ultima.timestampUtc) - 1] = '\0';
    return true;
}
