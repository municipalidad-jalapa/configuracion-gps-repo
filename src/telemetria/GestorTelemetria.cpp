#include "GestorTelemetria.h"

#include "configuracion.h"

#include <Arduino.h>
#include <cstring>

GestorTelemetria::GestorTelemetria(ProveedorGps& gps,
                                   RepositorioPosiciones& repositorio,
                                   Conectividad& conectividad,
                                   ClienteHttpTelemetria& cliente)
    : _gps(gps),
      _repositorio(repositorio),
      _conectividad(conectividad),
      _cliente(cliente) {}

void GestorTelemetria::iniciar() {
    if (!credencialPareceConfigurada()) {
        Serial.println("Credencial de equipo no configurada");
    }
    _ultimaCapturaMs = millis();
}

void GestorTelemetria::procesar() {
    _gps.actualizar();
    _conectividad.mantenerConexion();
    capturarSiCorresponde();
    enviarSiCorresponde();
}

void GestorTelemetria::capturarSiCorresponde() {
    const unsigned long ahora = millis();
    if (!_gps.hayPosicionValida()) {
        if (!_huboFix && ahora - _ultimoAvisoFixMs >= INTERVALO_LOG_ESPERANDO_FIX_MS) {
            Serial.printf(
                "Esperando fix GPS nmea=%lu ok=%lu fail=%lu loc=%d fecha=%d hora=%d sat=%d\n",
                _gps.caracteresNmea(),
                _gps.nmeaCorrectas(),
                _gps.nmeaCorruptas(),
                _gps.ubicacionValida() ? 1 : 0,
                _gps.fechaValida() ? 1 : 0,
                _gps.horaValida() ? 1 : 0,
                _gps.satelites());
            _ultimoAvisoFixMs = ahora;
        }
        return;
    }
    if (ahora - _ultimaCapturaMs < INTERVALO_POSICION_MS) {
        return;
    }
    _ultimaCapturaMs = ahora;

    PosicionGps posicion{};
    if (!_gps.obtenerPosicion(posicion)) {
        return;
    }

    _huboFix = true;
    Serial.printf("Posicion GPS valida lat=%.6f lon=%.6f ts=%s\n",
                  posicion.latitud, posicion.longitud, posicion.timestampUtc);

    if (_repositorio.guardar(posicion)) {
        Serial.println("Posicion almacenada");
    } else {
        Serial.println("No se pudo almacenar la posicion");
    }
}

void GestorTelemetria::enviarSiCorresponde() {
    if (!_conectividad.estaConectado()) {
        return;
    }
    if (_repositorio.cantidadPendientes() == 0) {
        return;
    }
    if (!credencialPareceConfigurada()) {
        return;
    }

    const unsigned long ahora = millis();
    if (ahora - _ultimoIntentoEnvioMs < INTERVALO_REINTENTO_ENVIO_MS) {
        return;
    }
    _ultimoIntentoEnvioMs = ahora;

    PosicionGps lote[TAMANO_MAXIMO_LOTE];
    size_t lineasDelPrefijo = 0;
    const size_t cantidad = _repositorio.obtenerPendientes(lote, TAMANO_MAXIMO_LOTE, lineasDelPrefijo);
    if (cantidad == 0) {
        if (lineasDelPrefijo > 0) {
            _repositorio.eliminarConfirmadas(lineasDelPrefijo);
        }
        return;
    }

    Serial.printf("Enviando lote de %u posiciones\n", static_cast<unsigned>(cantidad));
    const ResultadoEnvio resultado = _cliente.enviarLote(lote, cantidad);

    if (resultado.tipo == TipoResultadoEnvio::ACEPTADO) {
        /*
         * 202 significa que el backend proceso el lote. Las descartadas lo
         * son por reloj fuera de ventana; reintentarlas empeora el desfase.
         * La respuesta no identifica cuales se descartaron: se retiran todas
         * las lineas del prefijo enviado.
         */
        Serial.printf("Lote aceptado recibidas=%d aceptadas=%d descartadas=%d\n",
                      resultado.recibidas, resultado.aceptadas, resultado.descartadas);
        if (!_repositorio.eliminarConfirmadas(lineasDelPrefijo)) {
            Serial.println("Lote aceptado pero no se pudo compactar la cola");
        }
        return;
    }

    if (resultado.tipo == TipoResultadoEnvio::ERROR_HTTP) {
        Serial.printf("Error HTTP al enviar lote codigo=%d\n", resultado.codigoHttp);
        return;
    }
    Serial.println("Error de conexion al enviar lote");
}

bool GestorTelemetria::credencialPareceConfigurada() const {
    return strstr(CREDENCIAL_EQUIPO, "CAMBIAR") == nullptr
           && strncmp(CREDENCIAL_EQUIPO, "eq_", 3) == 0;
}
