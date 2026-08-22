# configuracion-GPS-repo

Repositorio de configuración de GPS para la flota de buses de EcoRuta (Municipalidad de Jalapa).
Aún en fase inicial, sin contenido funcional — este README documenta el flujo de trabajo mientras
se define qué va a vivir aquí.

## Flujo de trabajo

Mismas reglas que `backend/api-buses-jalapa` y `frontend/app-movil-buses`:

- Ramas `main` y `develop` protegidas: no se permite push directo, todo entra por Merge Request.
- El pipeline debe pasar en verde antes de poder mergear.
- Las discusiones abiertas en la MR deben quedar resueltas antes del merge.
- La rama origen se elimina automáticamente al mergear.

## Crear una rama y abrir MR

```bash
git checkout develop
git pull
git checkout -b feature/hu-XXX-descripcion

# ... cambios ...

git push -o merge_request.create -o merge_request.target=develop \
  -o merge_request.title="feat: descripcion" \
  -o merge_request.remove_source_branch origin feature/hu-XXX-descripcion
```

## CI/CD

El pipeline (`.gitlab-ci.yml`) actualmente solo ejecuta **Secret Detection** (escaneo de secretos)
en cada push, igual que en backend. Se ampliará con jobs de build/deploy cuando se defina el
contenido real del repositorio (config de dispositivos, servicio, etc.).

## Equipos con acceso

Este repositorio hereda el mismo acceso por equipos que backend y frontend, vía grupos compartidos
a nivel de grupo `configuracion-gps`: `devs-backend`, `devs-frontend`, `backend-reviewers`,
`qa-team`, `devops-team`, `evaluador-curso`.
