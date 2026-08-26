# Informe de pruebas — Prototipo de telemetría a bordo (HU-49)

**Proyecto:** EcoRuta / Seminario  
**Componente bajo prueba:** Firmware del módulo a bordo (ESP32 + GPS NEO-6M V2) y contrato de ingesta del backend  
**Fecha de ejecución:** 24–25 de agosto de 2026  
**Entorno de construcción:** PlatformIO (`env:esp32dev`, plataforma `espressif32@6.9.0`, framework Arduino)  
**Resultado global:** **APROBADO**

---

## 1. Objetivo

Verificar que el prototipo a bordo:

1. Adquiere posiciones GPS válidas.
2. Las almacena en cola persistente local.
3. Las envía por lotes al backend EcoRuta mediante el contrato acordado.
4. Que el backend las recibe, acepta y expone correctamente.

El alcance de esta iteración es el prototipo de desarrollo: hardware ESP32 + NEO-6M V2 e integración con la API existente, **sin modificar el backend**.

---

## 2. Alcance de las pruebas

| Incluido | Excluido |
|---|---|
| Compilación y flasheo del firmware | Pruebas de carga / estrés del servidor |
| Adquisición GPS (fix) | OTA, Traccar u otras plataformas |
| Persistencia de cola (LittleFS) | Cambios de API o frontend |
| Autenticación con credencial de equipo | Validación de criterios de red celular de Jira |
| Ingesta HTTP `POST /api/v1/telemetria/posiciones` | |
| Consulta de última posición en backend | |

---

## 3. Arquitectura ejercitada

```text
Neo6mGps (UART / TinyGPSPlus)
    → PosicionGps
        → RepositorioPosiciones (LittleFS)
            → GestorTelemetria
                → ClienteHttpTelemetria
                → Conectividad de red
                    → Backend EcoRuta (Docker local)
```

**Contrato de ingesta:**

- Método y ruta: `POST {BASE_URL}/api/v1/telemetria/posiciones`
- Cabeceras: `Content-Type: application/json`, `Authorization: Bearer <credencial-equipo>`
- Cuerpo: lote JSON con arreglo `posiciones` (`latitud`, `longitud`, `velocidadKmh` opcional, `timestamp` UTC)
- Éxito esperado: **HTTP 202**, con resumen `recibidas` / `aceptadas` / `descartadas`
- Atribución del vehículo: por la credencial del equipo, no por campos del JSON

---

## 4. Preparación del entorno

1. Instalación de **PlatformIO Core** y descarga de la plataforma ESP32, toolchain y librerías (`TinyGPSPlus`, `ArduinoJson`).
2. Compilación del proyecto con `pio run` — resultado: **SUCCESS**.
3. Configuración local de secretos de prueba en `include/secretos.local.h` (SSID/credenciales de red, `CREDENCIAL_EQUIPO`, `BASE_URL`).
4. Levantamiento del backend EcoRuta con Docker Compose (PostgreSQL/PostGIS + API en puerto `8080`).
5. Provisionamiento administrativo de vehículo y equipo a bordo (`POST /api/v1/admin/vehiculos`, `POST /api/v1/admin/equipos`) y uso de la credencial emitida en el firmware.
6. Flasheo al ESP32 (`pio run -t upload`) y monitor serie a 115200 baudios (`pio device monitor`).

---

## 5. Casos de prueba ejecutados

### PT-01 — Compilación del firmware

| Campo | Detalle |
|---|---|
| **Descripción** | Compilar el firmware completo para `esp32dev` con las dependencias declaradas en `platformio.ini`. |
| **Procedimiento** | `pio run` en el directorio `firmware/`. |
| **Resultado esperado** | Enlace correcto del binario sin errores de compilación. |
| **Resultado obtenido** | **PASS.** Build exitoso; uso aproximado reportado por PlatformIO: RAM ~13.3 %, Flash ~58.8 %. |

### PT-02 — Flasheo y arranque del módulo

| Campo | Detalle |
|---|---|
| **Descripción** | Cargar el firmware en el ESP32 y comprobar arranque estable. |
| **Procedimiento** | `pio run -t upload` y apertura de monitor serie. |
| **Resultado esperado** | Dispositivo operativo; mensajes de inicialización GPS y red. |
| **Resultado obtenido** | **PASS.** El módulo arranca, inicializa GPS y establece comunicación de datos con el backend. |

### PT-03 — Adquisición de posición GPS

| Campo | Detalle |
|---|---|
| **Descripción** | Verificar fix GPS válido desde el NEO-6M V2 (UART2, 9600 baudios, pines GPIO 16/17). |
| **Procedimiento** | Observar el monitor serie tras encendido con antena en condiciones de recepción. |
| **Resultado esperado** | Mensajes `Posicion GPS valida` con latitud, longitud y timestamp UTC. |
| **Resultado obtenido** | **PASS.** Ejemplo observado: `lat=14.636844 lon=-89.981800 ts=2026-08-25T00:04:18Z` (coordenadas coherentes con Jalapa). Intervalo de muestreo configurado: 5 s. |

### PT-04 — Almacenamiento en cola local

| Campo | Detalle |
|---|---|
| **Descripción** | Cada posición válida debe persistirse en la cola LittleFS antes del envío. |
| **Procedimiento** | Correlacionar en monitor serie `Posicion GPS valida` con `Posicion almacenada`. |
| **Resultado esperado** | Confirmación de almacenamiento por cada fix válido. |
| **Resultado obtenido** | **PASS.** Secuencia consistente: fix → `Posicion almacenada` → preparación de lote. |

### PT-05 — Envío de lotes e aceptación por el backend

| Campo | Detalle |
|---|---|
| **Descripción** | El firmware agrupa posiciones (máx. 20 por lote) y las publica al endpoint de telemetría con la credencial de equipo. |
| **Procedimiento** | Dejar el módulo en operación continua y revisar el monitor serie. |
| **Resultado esperado** | Respuesta de aceptación del lote; conteos `recibidas` = `aceptadas`, `descartadas` = 0. |
| **Resultado obtenido** | **PASS.** Evidencia en monitor: |
| | `Enviando lote de 20 posiciones` |
| | `Lote aceptado recibidas=20 aceptadas=20 descartadas=0` |
| | También se observaron lotes parciales (p. ej. 17 posiciones) igualmente aceptados. |
| | **Los datos se comunican correctamente entre el módulo a bordo y el backend.** |

### PT-06 — Verificación en el backend (última posición)

| Campo | Detalle |
|---|---|
| **Descripción** | Comprobar que la API refleja la posición más reciente del vehículo asociado a la credencial. |
| **Procedimiento** | Consulta pública: `GET /api/v1/telemetria/posicion?vehiculoId=1` |
| **Resultado esperado** | Coordenadas recientes alineadas con el GPS del módulo; vehículo identificado. |
| **Resultado obtenido** | **PASS.** Respuesta de ejemplo: |
| | `latitud: 14.636857` |
| | `longitud: -89.981759` |
| | `velocidadKmh: 0.04` |
| | `timestamp: 2026-08-25T00:05:13Z` |
| | `vehiculo: BUS-01` |

### PT-07 — Salud del servicio backend

| Campo | Detalle |
|---|---|
| **Descripción** | Confirmar disponibilidad del API durante las pruebas de ingesta. |
| **Procedimiento** | Contenedores Docker en ejecución; sonda de salud del servicio. |
| **Resultado esperado** | Backend y base de datos en estado saludable; API alcanzable. |
| **Resultado obtenido** | **PASS.** Contenedores `api-buses-jalapa-backend-1` y `api-buses-jalapa-db-1` en estado healthy; endpoint de salud operativo. |

---

## 6. Evidencia resumida (monitor serie)

Durante la prueba exitosa se observó de forma reiterada el ciclo completo:

```text
Posicion GPS valida lat=14.636844 lon=-89.981800 ts=2026-08-25T00:04:18Z
Posicion almacenada
Enviando lote de 20 posiciones
Lote aceptado recibidas=20 aceptadas=20 descartadas=0
```

Esto demuestra:

- Fix GPS válido.
- Persistencia local.
- Transmisión del lote.
- Aceptación completa por el backend (sin descartes).

---

## 7. Criterios de aceptación vs. resultado

| Criterio | Estado |
|---|---|
| El firmware compila y se despliega en el ESP32 | Cumplido |
| El GPS entrega posiciones válidas con timestamp UTC | Cumplido |
| Las posiciones se encolan localmente | Cumplido |
| El módulo autentica con credencial de equipo (`Bearer`) | Cumplido |
| El backend acepta lotes con HTTP 202 y conteos coherentes | Cumplido |
| La última posición es consultable vía API | Cumplido |
| Comunicación de datos módulo ↔ backend correcta | Cumplido |

---

## 8. Conclusión

Las pruebas del prototipo a bordo se ejecutaron de extremo a extremo con resultado **aprobado**. El módulo obtiene posición GPS, la almacena, la envía por lotes al backend EcoRuta y este la recibe y registra correctamente, quedando disponible para consulta. La comunicación de datos entre el dispositivo a bordo y el servicio backend funciona de forma estable en las condiciones de prueba documentadas.

---

## 9. Referencias técnicas

- Firmware: `firmware/` (`platformio.ini`, `include/configuracion.h`)
- Backend: `backend/api-buses-jalapa/` (Docker Compose, puerto 8080)
- Documentación API: `http://localhost:8080/swagger-ui.html`
- Consulta de posición: `GET /api/v1/telemetria/posicion?vehiculoId={id}`
- Stream SSE (opcional): `GET /api/v1/telemetria/stream`
