# Changelog

Todos los cambios relevantes de este proyecto se documentarán en este archivo.

## [Unreleased]

### Added

- Core y manager 2.0.0 con APIs separadas; resolución de perfiles asíncrona por AccountID.
- Excepciones persistentes exclusivas por AccountID del borrower y comandos administrativos.
- Identidad del owner por AccountID y decisión `allowed` en el historial, con migración explícita del esquema 1.4.

### Changed

- Bibliotecas SteamIDTools sincronizadas con 2.5.0; el stock compartido delega la conversión uint32 y el parseo de AccountID, conservando la escritura canónica de la lista de permitidos.
- SQL, historial, anuncios y expulsiones pasan al manager; perfiles permanecen dentro del core.
- El paquete contiene únicamente los dos nuevos plugins. Se eliminan el bridge de BanSystem Access y su include de compilación.
- Se elimina la API combinada anterior: include, alias de biblioteca, natives y forward. Los consumidores deben migrar a las APIs de core y manager.

### Fixed

- Expulsiones diferidas protegidas por serial y AccountID ante desconexión o reutilización de slots.
- Solicitudes de perfiles con timeout y fallback, callbacks SQL protegidos por generación y pendientes conservados hasta confirmación.
- Argumentos de traducción sincronizados en inglés y español.

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
