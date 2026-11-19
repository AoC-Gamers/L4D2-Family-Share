# L4D2-Family-Share

Suite SourceMod para detectar el uso de Steam Family Sharing en Left 4 Dead 2. El [core](docs/core/README.md) detecta el préstamo de una licencia con SteamWorks. El [manager](docs/manager/README.md) agrega excepciones, avisos, expulsiones e historial.

## Instalación

Instala [SteamWorks](https://github.com/AoC-Gamers/SteamWorks) y extrae el ZIP de [GitHub Releases](https://github.com/AoC-Gamers/L4D2-Family-Share/releases) en `left4dead2/`. Carga el core y, si necesitas reglas o historial, el manager. Una clave de Steam Web API permite resolver el nombre del dueño de la licencia.

Las excepciones se aplican al AccountID de quien toma prestada la licencia. Si vienes de la versión 1.4, sigue la [guía de migración](docs/migration/README.md). Consulta el [índice de documentación](docs/README.md) para integrar otros plugins.

## Créditos

Basado en [Family Share Manager v1.5.5](https://forums.alliedmods.net/showthread.php?t=293927).

## Licencia

El código y los aportes originales de lechuga se distribuyen bajo [GPL-3.0-only](LICENSE). Las atribuciones y los términos de terceros se recogen en [NOTICES.md](NOTICES.md).
