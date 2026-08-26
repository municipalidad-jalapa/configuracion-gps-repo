# configuracion-GPS-repo

Firmware del modulo a bordo de EcoRuta (Municipalidad de Jalapa). El bus
reporta su posicion por si mismo, sin que nadie opere un telefono (HU-49 /
SCRUM-144).

Este repositorio es independiente de `backend/api-buses-jalapa` y
`frontend/app-movil-buses`. El backend existente es el contrato de ingesta;
este codigo no modifica la API.

## Prototipo de esta iteracion

Hardware de pruebas: ESP32 + GPS NEO-6M V2 + Wi-Fi hacia el hotspot del
telefono. Ese Wi-Fi sustituye al enlace celular solo durante desarrollo; no
valida el criterio celular de Jira.

## Entorno

Unica herramienta de construccion: PlatformIO.

```text
pio run
pio run -t upload
pio device monitor
```

## Cableado NEO-6M V2

Pines definidos en `include/configuracion.h`.

```text
NEO-6M TX  ->  ESP32 GPIO 16 (UART2 RX)
NEO-6M RX  ->  ESP32 GPIO 17 (UART2 TX, opcional)
NEO-6M GND ->  GND
NEO-6M VCC ->  3.3 V o 5 V segun el modulo (el GPIO del ESP32 es 3.3 V)
```

UART a 9600 baudios. No se usa SoftwareSerial.

## Configuracion local

Copiar `include/secretos.ejemplo.h` a `include/secretos.local.h` y completar:

* SSID y password del hotspot de prueba
* `CREDENCIAL_EQUIPO` emitida por `POST /api/v1/admin/equipos` (HU-47)
* `BASE_URL` del backend, sin barra final, por ejemplo `http://HOST:8080`

`secretos.local.h` no se versiona. No poner credenciales reales en archivos
que se suban al repositorio. La credencial viaja solo como
`Authorization: Bearer ...`; nunca en la URL ni en el JSON.

## Contrato que consume

```text
POST {BASE_URL}/api/v1/telemetria/posiciones
Content-Type: application/json
Authorization: Bearer <credencial-equipo>

{"posiciones":[{"latitud":14.6335,"longitud":-89.9885,"velocidadKmh":18,"timestamp":"2026-08-17T10:00:00Z"}]}
```

Exito: HTTP 202. El vehiculo se atribuye por la credencial, no por el cuerpo.

## Arquitectura sustituible

```text
Neo6mGps (TinyGPSPlus, UART)
    -> PosicionGps
        -> RepositorioPosiciones (LittleFS)
            -> GestorTelemetria
                -> ClienteHttpTelemetria
                -> ConectividadWifi
```

Para cambiar el GPS, reemplazar `Neo6mGps` por otro `ProveedorGps`.
Para pasar de Wi-Fi a red celular, reemplazar `ConectividadWifi` y, si hace
falta, el cliente HTTP. Cola, timestamp, lote y autenticacion no se reescriben.

## Informe de pruebas

Las pruebas del prototipo (compilacion, flasheo, GPS, cola local, ingesta por
lotes y consulta de ultima posicion en el backend) se ejecutaron con resultado
**APROBADO**. Los datos se comunican correctamente entre el modulo a bordo y
el backend EcoRuta.

Informe detallado (casos PT-01 a PT-07, evidencia y conclusion):

* [docs/informe-pruebas-prototipo-a-bordo.md](docs/informe-pruebas-prototipo-a-bordo.md)

## Fuera de alcance (HU-49)

Traccar, GeoLinker, OsmAnd, SIM800/SIM7600, TinyGSM, APN, CSQ/RSSI, TK103A,
cambios de backend, frontend, SSE u OTA.

## Flujo de trabajo

Mismas reglas que `backend/api-buses-jalapa` y `frontend/app-movil-buses`:

- Ramas `main` y `develop` protegidas: no se permite push directo, todo entra por Merge Request.
- El pipeline debe pasar en verde antes de poder mergear.
- Las discusiones abiertas en la MR deben quedar resueltas antes del merge.
- La rama origen se elimina automaticamente al mergear.

## Crear una rama y abrir MR

```bash
git checkout develop
git pull
git checkout -b SCRUM-XXX-hu-YY-descripcion

# ... cambios ...

git push -o merge_request.create -o merge_request.target=develop \
  -o merge_request.title="feat: descripcion" \
  -o merge_request.remove_source_branch origin SCRUM-XXX-hu-YY-descripcion
```

## CI/CD

El pipeline (`.gitlab-ci.yml`) ejecuta **Secret Detection** en cada push, igual
que en backend. La compilacion del firmware se hace en local con `pio run`
hasta que se agregue un job de build.

## Equipos con acceso

Este repositorio hereda el mismo acceso por equipos que backend y frontend, via grupos compartidos
a nivel de grupo `configuracion-gps`: `devs-backend`, `devs-frontend`, `backend-reviewers`,
`qa-team`, `devops-team`, `evaluador-curso`.
