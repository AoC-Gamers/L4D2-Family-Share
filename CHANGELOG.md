# Changelog

Todos los cambios relevantes de este proyecto se documentarán en este archivo.

## [Unreleased]

### Added

- Publicación inicial del plugin en un repositorio público independiente.
- Detección de jugadores usando una copia prestada de Left 4 Dead 2 mediante Steam Family Sharing.
- Soporte para anuncios, registro y enforcement según la configuración del servidor.
- Integración con `SteamWorks` para recibir la señal principal de Family Share.
- Integración opcional con `SteamIDTools` para resolver en línea la identidad pública del owner.
- Modo degradado cuando el backend auxiliar no está disponible.
- Soporte para enriquecimiento del owner mediante perfil público y datos complementarios.
- Script SQL para persistencia de eventos.
- Traducciones en inglés y español.
- `README.md` centrado en el objetivo, flujo y dependencias del plugin.
- Créditos a la idea original basada en `Family Share Manager v1.5.5`.

### Changed

- none.

### Fixed

- La integración opcional con `SteamIDTools` ahora usa `SteamIDToolsProvider_Auto`, dejando la selección del provider HTTP centralizada en el plugin principal y evitando lógica duplicada en `l4d2_familyshare`.

## [1.3.0]

- Primera base pública del proyecto.
- Corresponde a la versión `1.3.0` mantenida previamente como desarrollo privado dentro del ecosistema de AoC.
- Se toma como punto de partida para el versionado abierto del plugin.
