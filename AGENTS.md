# Trabajo en L4D2-Family-Share

Consulta [el índice de documentación](docs/README.md) y
[la guía de compilación](docs/build/README.md) antes de editar.

- Conserva los nombres de binarios, APIs, traducciones, datos y rutas de
  instalación. Usa `scripts/package/package-map.json` para seleccionar productos.
- `addons/sourcemod/` contiene fuentes y recursos de SourceMod.
  `scripts/deps/` descarga el compilador y `scripts/build/` lo ejecuta.
  `scripts/package/` contiene el mapa, la validación y el instalador.
  `scripts/ci/` mantiene los adaptadores y metadatos de release.
- Usa el Makefile raíz en local y en CI. Ejecuta `make deps-smx`,
  `make build-smx`, `make package-smx`, `make release` y
  `make validate-artifact` en orden para verificar cambios de distribución.
- Las salidas locales quedan en `.deps/`, `.build/` y `dist/`. Conserva la
  autoría y los avisos de terceros. Incluye las fuentes de los plugins que se
  distribuyen y mantén las herramientas internas fuera del ZIP.
- Conserva las pruebas y los controles de empaquetado. Distingue compilación,
  validación offline y prueba en SRCDS. Los cambios remotos requieren autorización.
