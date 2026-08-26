#include "RepositorioPosiciones.h"

#include "configuracion.h"

#include <ArduinoJson.h>
#include <LittleFS.h>
#include <cstdio>
#include <cstring>

namespace {

constexpr const char* RUTA_COLA = "/posiciones.jsonl";
constexpr const char* RUTA_TEMPORAL = "/posiciones.tmp";
constexpr size_t CAPACIDAD_LINEA = 192;

void copiarTimestamp(char* destino, size_t capacidad, const char* origen) {
    if (origen == nullptr) {
        destino[0] = '\0';
        return;
    }
    strncpy(destino, origen, capacidad - 1);
    destino[capacidad - 1] = '\0';
}

}  // namespace

bool RepositorioPosiciones::iniciar() {
    if (!LittleFS.begin(true)) {
        Serial.println("Error al montar almacenamiento");
        return false;
    }
    if (LittleFS.exists(RUTA_TEMPORAL) && !LittleFS.exists(RUTA_COLA)) {
        LittleFS.rename(RUTA_TEMPORAL, RUTA_COLA);
    } else if (LittleFS.exists(RUTA_TEMPORAL)) {
        LittleFS.remove(RUTA_TEMPORAL);
    }
    reconstruirConteo();
    Serial.printf("Cola persistente: %u pendientes\n", static_cast<unsigned>(_pendientes));
    return true;
}

bool RepositorioPosiciones::guardar(const PosicionGps& posicion) {
    if (_pendientes >= COLA_MAXIMA_POSICIONES) {
        if (!eliminarConfirmadas(1)) {
            Serial.println("Cola llena: no se pudo rotar la posicion mas antigua");
            return false;
        }
    }
    if (!escribirLinea(posicion)) {
        return false;
    }
    _pendientes++;
    return true;
}

size_t RepositorioPosiciones::cantidadPendientes() const {
    return _pendientes;
}

size_t RepositorioPosiciones::obtenerPendientes(PosicionGps* destino, size_t maximo,
                                                size_t& lineasDelPrefijo) {
    lineasDelPrefijo = 0;
    if (destino == nullptr || maximo == 0) {
        return 0;
    }
    File archivo = LittleFS.open(RUTA_COLA, "r");
    if (!archivo) {
        return 0;
    }

    size_t leidas = 0;
    while (archivo.available() && leidas < maximo) {
        String linea = archivo.readStringUntil('\n');
        linea.trim();
        if (linea.isEmpty()) {
            continue;
        }
        lineasDelPrefijo++;
        PosicionGps posicion{};
        if (!parsearLinea(linea.c_str(), posicion)) {
            Serial.println("Posicion persistida ilegible, se omite");
            continue;
        }
        copiarPosicion(destino[leidas], posicion);
        leidas++;
    }
    archivo.close();
    return leidas;
}

bool RepositorioPosiciones::eliminarConfirmadas(size_t cantidad) {
    if (cantidad == 0) {
        return true;
    }
    return compactarDesde(cantidad);
}

bool RepositorioPosiciones::compactarDesde(size_t omitir) {
    File origen = LittleFS.open(RUTA_COLA, "r");
    if (!origen) {
        _pendientes = 0;
        return true;
    }

    File temporal = LittleFS.open(RUTA_TEMPORAL, "w");
    if (!temporal) {
        origen.close();
        Serial.println("No se pudo abrir temporal de cola");
        return false;
    }

    size_t saltadas = 0;
    size_t conservadas = 0;
    while (origen.available()) {
        String linea = origen.readStringUntil('\n');
        linea.trim();
        if (linea.isEmpty()) {
            continue;
        }
        if (saltadas < omitir) {
            saltadas++;
            continue;
        }
        temporal.print(linea);
        temporal.print('\n');
        conservadas++;
    }
    origen.close();
    temporal.close();

    LittleFS.remove(RUTA_COLA);
    if (!LittleFS.rename(RUTA_TEMPORAL, RUTA_COLA)) {
        Serial.println("No se pudo reemplazar la cola persistente");
        return false;
    }
    _pendientes = conservadas;
    return true;
}

bool RepositorioPosiciones::parsearLinea(const char* linea, PosicionGps& posicion) const {
    JsonDocument documento;
    if (deserializeJson(documento, linea)) {
        return false;
    }
    if (documento["latitud"].isNull() || documento["longitud"].isNull()
        || documento["timestamp"].isNull()) {
        return false;
    }
    posicion.latitud = documento["latitud"].as<double>();
    posicion.longitud = documento["longitud"].as<double>();
    posicion.tieneVelocidad = !documento["velocidadKmh"].isNull();
    posicion.velocidadKmh = posicion.tieneVelocidad ? documento["velocidadKmh"].as<double>() : 0.0;
    copiarTimestamp(posicion.timestampUtc, sizeof(posicion.timestampUtc),
                    documento["timestamp"].as<const char*>());
    return posicion.timestampUtc[0] != '\0';
}

bool RepositorioPosiciones::escribirLinea(const PosicionGps& posicion) {
    File archivo = LittleFS.open(RUTA_COLA, "a");
    if (!archivo) {
        Serial.println("No se pudo abrir la cola para escritura");
        return false;
    }

    char linea[CAPACIDAD_LINEA];
    int escritos;
    if (posicion.tieneVelocidad) {
        escritos = snprintf(linea, sizeof(linea),
                            "{\"latitud\":%.6f,\"longitud\":%.6f,\"velocidadKmh\":%.2f,\"timestamp\":\"%s\"}\n",
                            posicion.latitud, posicion.longitud, posicion.velocidadKmh,
                            posicion.timestampUtc);
    } else {
        escritos = snprintf(linea, sizeof(linea),
                            "{\"latitud\":%.6f,\"longitud\":%.6f,\"timestamp\":\"%s\"}\n",
                            posicion.latitud, posicion.longitud, posicion.timestampUtc);
    }
    if (escritos <= 0 || static_cast<size_t>(escritos) >= sizeof(linea)) {
        archivo.close();
        return false;
    }
    const size_t guardados = archivo.print(linea);
    archivo.close();
    return guardados == static_cast<size_t>(escritos);
}

void RepositorioPosiciones::reconstruirConteo() {
    _pendientes = 0;
    File archivo = LittleFS.open(RUTA_COLA, "r");
    if (!archivo) {
        return;
    }
    while (archivo.available()) {
        String linea = archivo.readStringUntil('\n');
        linea.trim();
        if (!linea.isEmpty()) {
            _pendientes++;
        }
    }
    archivo.close();
}
