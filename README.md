# L4D2-Family-Share

Suite SourceMod para detectar Steam Family Sharing en Left 4 Dead 2.

## Componentes

| Plugin | Responsabilidad |
|---|---|
| `l4d2_familyshare_core` | Señal de SteamWorks, estado de la conexión, AccountIDs y resolución opcional de perfiles. |
| `l4d2_familyshare_manager` | Cuentas permitidas, excepciones, anuncios, expulsiones e historial; SQL es un módulo interno. |

El core funciona por sí solo. Instalar manager cuando se necesite política o
historial. La detección entrega `client`, `borrowerAccountId` y `ownerAccountId`;
AccountID identifica la cuenta aunque se desconecte. El core no conserva
historial ni decide expulsiones.

Una excepción permite **únicamente el AccountID del borrower**. Permitir al owner
no permite a otras cuentas que usen su licencia. Los permitidos siguen generando
detecciones e historial, pero manager no los expulsa por Family Sharing.

## Dependencias

- [SteamWorks](https://github.com/AoC-Gamers/SteamWorks): extensión requerida para la validación de licencias y HTTP opcional de perfiles.
- [SteamIDTools](https://github.com/AoC-Gamers/SteamIDTools): plugin opcional para enriquecimiento de identidad.

SteamID64 y el enlace de perfil tienen conversión local como fallback. Una clave
Steam Web API permite consultar el nombre del owner; su ausencia o un fallo de
red no impiden detectar ni aplicar la política.

## Configuración y API

- [Core y contratos públicos](docs/README_CORE.md).
- [Manager, permitidos e historial](docs/README_MANAGER.md).
- [Migración desde 1.4](docs/MIGRATION.md).
- [Sistema de build](docs/BUILD_SYSTEM.md).

El antiguo `l4d2_familyshare_ban_bridge` y la API combinada se eliminan. Las
integraciones externas consumen la detección y los perfiles desde el core; las
consultas de política pertenecen a manager. Los consumidores antiguos requieren
la migración descrita en la guía.

## Build local

```sh
make deps-smx
make build-smx
make package-smx
make release
```

[plugin-package-map.json](plugin-package-map.json) selecciona exactamente dos
entrypoints y los archivos del paquete. Se incluye `steamidtools_stock.inc` para
las utilidades compartidas; las demás dependencias externas de compilación no se
distribuyen. Linux y CI usan el mismo builder Python.

## Créditos

Basado en [Family Share Manager v1.5.5](https://forums.alliedmods.net/showthread.php?t=293927).
