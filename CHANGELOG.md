# Changelog

Todos los cambios relevantes de este proyecto se documentarán en este archivo.

## [Unreleased]

### Added

- none.

### Changed

- none.

### Fixed

- none.

## [1.4.0]

### Added

- API pública para `l4d2_familyshare` mediante librería, include, natives de estado por cliente y forward `L4D2FamilyShare_OnDetected`.
- Nuevo plugin `l4d2_familyshare_ban_bridge` para consumir detecciones de Family Share y delegar enforcement en BanSystem.
- Integración inicial con `bansystem_access` para consultar bans del owner por `accountid` y replicar la sanción sobre la cuenta prestada.
- Include local `bansystem_access.inc` dentro del repo para compilar integraciones contra BanSystem desde este proyecto.

### Changed

- El plugin principal pasa a versión `1.4.0`.
- El bridge de BanSystem replica la `ban_length` original del ban activo del owner al borrower.
- El CI ahora compila y empaqueta tanto `l4d2_familyshare` como `l4d2_familyshare_ban_bridge`.
- El artefacto de CI solo incluye la biblioteca propia `l4d2_familyshare.inc`; las bibliotecas complementarias copiadas para compilación quedan fuera del paquete final.

### Fixed

- La integración opcional con `SteamIDTools` ahora usa `SteamIDToolsProvider_Auto`, dejando la selección del provider HTTP centralizada en el plugin principal y evitando lógica duplicada en `l4d2_familyshare`.

## [1.3.0]

- Primera base pública del proyecto.
- Corresponde a la versión `1.3.0` mantenida previamente como desarrollo privado dentro del ecosistema de AoC.
- Se toma como punto de partida para el versionado abierto del plugin.
