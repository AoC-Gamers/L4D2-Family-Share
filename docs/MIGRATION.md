# Migration from 1.4

## Plugins y configuración

1. Retirar `l4d2_familyshare.smx` y `l4d2_familyshare_ban_bridge.smx` antes de instalar
   `l4d2_familyshare_core.smx` y, si se necesita política o historial,
   `l4d2_familyshare_manager.smx`. No ejecutar el plugin antiguo junto con la suite.
2. Trasladar desde `cfg/sourcemod/l4d2_familyshare.cfg` las ConVars de perfiles al
   archivo `l4d2_familyshare_core.cfg`; trasladar anuncios, enforcement y SQL a
   `l4d2_familyshare_manager.cfg`. Los nombres de las ConVars existentes se conservan.
3. Retirar la configuración del bridge `l4d2_familyshare_ban.cfg`. La suite ya no
   replica sanciones del owner mediante BanSystem Access.
4. Instalar el archivo vacío de permitidos solo en una instalación nueva;
   preservar `configs/l4d2_familyshare_allowed.kv` en actualizaciones posteriores.
   Agregar excepciones únicamente por AccountID del borrower.

La entrada existente en `databases.cfg`, la tabla y el KV de pendientes se
conservan. El log de auditoría pasa de `logs/FamilyShare.log` a
`logs/familyshare/manager.log`; el archivo anterior no se elimina automáticamente.

## Esquema del historial

Para una tabla MySQL 1.4 existente, ejecutar **una vez**
`addons/sourcemod/configs/sql-init/mysql/upgrade_1_4_to_2_0.sql`, adaptando el nombre
si se configuró otra tabla. Añade `owner_accountid`, `allowed` y el índice de owner.
Las filas anteriores conservan `owner_accountid=0` y `allowed=0`; no se infiere
una excepción a partir de datos históricos. Una instalación nueva usa
`l4d2_familyshare.sql` directamente. No ejecutar el ALTER en una tabla nueva.

SQLite crea la tabla nueva automáticamente. Para una tabla SQLite antigua que
reproduzca el esquema 1.4, agregar `owner_accountid INTEGER NOT NULL DEFAULT 0`
y `allowed INTEGER NOT NULL DEFAULT 0` antes de habilitar el historial.

Si faltan columnas, manager conserva sus pendientes y emite un error de esquema;
la detección y la política siguen funcionando. Esta migración no se ejecuta desde
el plugin ni cambia una base de datos durante la instalación del paquete.

## Consumidores

Se retiran `l4d2_familyshare.inc`, la biblioteca `l4d2_familyshare`, sus cuatro
natives y el forward `L4D2FamilyShare_OnDetected`. Manager registra únicamente
`l4d2_familyshare_manager`; el core registra `l4d2_familyshare_core`. Eliminar el
include antiguo de las instalaciones de desarrollo y recompilar los consumidores
con las APIs correspondientes.

| API anterior | API actual |
|---|---|
| `L4D2FamilyShare_IsClientDetected` | `L4D2FamilyShareCore_IsClientDetected` |
| `L4D2FamilyShare_GetClientBorrowerAccountId` | `L4D2FamilyShareCore_GetClientBorrowerAccountId` |
| `L4D2FamilyShare_GetClientOwnerAccountId` | `L4D2FamilyShareCore_GetClientOwnerAccountId` |
| `L4D2FamilyShare_IsClientEnforced` | `L4D2FamilyShareManager_IsClientEnforced` |
| `L4D2FamilyShare_OnDetected(client, borrowerAccountId, ownerAccountId, enforced)` | `L4D2FamilyShareCore_OnDetected(client, borrowerAccountId, ownerAccountId)` |

Incluir `l4d2_familyshare_core.inc` para detección, identidad y perfiles, y
`l4d2_familyshare_manager.inc` para consultas de política. Si la dependencia es
opcional, verificar la biblioteca y disponibilidad de natives antes de llamarlos.

El forward del core tiene tres parámetros y no entrega enforcement. Se emite
antes de que todos sus consumidores hayan procesado la detección: consultar
manager desde este callback puede devolver una decisión anterior según el orden
de ejecución. Una integración que necesite la intención de manager debe consultar
su native después del despacho del forward y validar la conexión mediante serial.
La intención capturada no certifica que una expulsión haya terminado.

El consumidor externo `bansystem_familyshare` `1.1.1` ya incorpora en su checkout
la migración al include y forward del core. Para despliegues que aún usen una
versión anterior, actualizar también ese consumidor: el callback antiguo deja
de recibir detecciones. La integración nueva elimina el cuarto parámetro
`enforced`, que no utilizaba, y funciona sin Family Share Manager. La migración
local no acredita instalación ni smoke SRCDS. Ningún plugin de este repositorio
llama a BanSystem Access.
