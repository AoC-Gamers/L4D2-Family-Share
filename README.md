# L4D2-Family-Share

Plugin de SourceMod para Left 4 Dead 2 orientado a detectar jugadores que están usando una copia prestada del juego mediante Steam Family Sharing.

## Objetivo

El objetivo del plugin es dar visibilidad y control sobre este tipo de acceso en servidores dedicados. Según cómo se configure en cada servidor, el flujo puede:
- anunciar la detección al resto de los jugadores
- registrar el evento para auditoría
- aplicar enforcement sobre el jugador detectado

## Flujo general

1. El servidor detecta que un jugador está usando una copia prestada.
2. Se identifica al jugador que está conectado y al owner asociado a esa licencia compartida.
3. El plugin construye un evento de detección con los datos disponibles en ese momento.
4. Si existe integración con servicios auxiliares, se enriquece el evento con información adicional del owner.
5. Finalmente, el evento puede anunciarse, registrarse y/o usarse para enforcement.

## Integración con SteamWorks y SteamIDTools

Este plugin está pensado para usarse con estas implementaciones:
- `SteamWorks`: https://github.com/AoC-Gamers/SteamWorks
- `SteamIDTools`: https://github.com/AoC-Gamers/SteamIDTools

Rol de cada componente:
- `SteamWorks` entrega la señal principal de Family Share y el identificador base del owner.
- `SteamIDTools` resuelve en línea la conversión del identificador del owner a un formato más útil para perfiles, enlaces y enriquecimiento posterior.

## Enriquecimiento del owner

Cuando la integración auxiliar está disponible, el flujo puede:
- resolver el identificador público del owner
- construir una URL de perfil usable
- consultar datos complementarios del owner

Cuando esa integración no está disponible, el plugin sigue siendo útil:
- la detección de Family Share sigue ocurriendo
- el servidor puede seguir anunciando y/o aplicando enforcement
- solo se pierde el enriquecimiento adicional del owner

## Registro y auditoría

El plugin puede registrar eventos para dejar trazabilidad histórica de:
- quién estaba usando la copia prestada
- cuándo ocurrió
- qué datos del owner pudieron resolverse
- si el servidor estaba en modo de enforcement o solo en modo informativo

## Notas

- La detección principal no depende del backend auxiliar.
- El backend auxiliar mejora la calidad del dato del owner, pero no es requisito para detectar el evento.
- El proyecto está pensado para integrarse con el ecosistema público de `SteamWorks` y `SteamIDTools` enlazado arriba.

## Créditos

La idea original de este plugin está basada en:

- **Family Share Manager v1.5.5**
  - https://forums.alliedmods.net/showthread.php?t=293927
