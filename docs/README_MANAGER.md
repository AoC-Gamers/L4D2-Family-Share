# Family Share Manager

`l4d2_familyshare_manager` consume el core y registra su propia biblioteca.
El core es necesario para nuevas detecciones; su dependencia dinámica permite
que manager complete snapshots e historial si el core se descarga.
Consume la detección y captura un snapshot de AccountIDs, nombres, SteamID64,
serial de conexión y decisión de política. AccountID es la identidad persistente;
client y serial se utilizan únicamente para actuar sobre la conexión actual.

## Permitidos y excepciones

La fuente es `addons/sourcemod/configs/l4d2_familyshare_allowed.kv`:

```text
"FamilyShareAllowed"
{
    "123456789"
    {
        "allowed" "1"
    }
}
```

Cada entrada autoriza solo a ese borrower. No hay herencia desde el owner,
expiraciones automáticas ni autorización por SteamID64. `allowed 0` desactiva una
entrada. El paquete incluye un archivo inicialmente vacío: preservar las entradas
locales al actualizar. Los cambios se aplican a futuras detecciones; no expulsan
retroactivamente a clientes conectados.

Comandos con permiso Root, utilizables también desde la consola del servidor:

- `sm_familyshare_allow <AccountID>`: agrega o habilita una excepción persistente.
- `sm_familyshare_disallow <AccountID>`: elimina esa excepción.
- `sm_familyshare_reload`: valida y recarga el archivo externo.

Se aceptan AccountIDs decimales de 1 a 4294967295, sin signos ni ceros iniciales.
Un archivo inválido conserva la última política válida. Si la primera carga
falla, las expulsiones quedan suspendidas hasta cargar una política válida.
La escritura usa un archivo temporal y reemplazo del destino; un fallo conserva
la política activa.

## Expulsiones y anuncios

`l4d2_familyshare_enforce 1` expulsa borrowers no permitidos. La expulsión se
difiere un frame para que los consumidores capturen sus datos. Antes de ejecutarla
se comprueban serial, AccountID, owner y política; reutilizar un slot nunca
permite expulsar a otra cuenta.

Los anuncios y el historial siguen disponibles para cuentas permitidas. Los
perfiles se correlacionan por generación del core, request ID y AccountID y se procesan incluso si
el borrower ya se desconectó. Un timeout propio de 30 segundos o la descarga del
manager completa la detección con el snapshot local; el resultado tardío se ignora.

## Configuración

Archivo: `cfg/sourcemod/l4d2_familyshare_manager.cfg`.

| ConVar | Default | Uso |
|---|---|---|
| `l4d2_familyshare_enforce` | `1` | Expulsar borrowers sin excepción. |
| `l4d2_familyshare_announce` | `1` | Anunciar detecciones y perfil del owner. |
| `l4d2_familyshare_sql` | `0` | Activar historial SQL opcional. |
| `l4d2_familyshare_sql_config` | `default` | Entrada de `databases.cfg`. |
| `l4d2_familyshare_sql_table` | `l4d2_familyshare` | Tabla de historial. |
| `l4d2_familyshare_sql_debug` | `0` | Diagnóstico SQL si el log propio está habilitado. |
| `l4d2_familyshare_manager_log_mode` | `1` | Apagado=0, normal=1, debug=2. |
| `l4d2_familyshare_manager_debug_mask` | `0` | Política=1, SQL=2, lifecycle=4; todos=7. |

Log propio: `addons/sourcemod/logs/familyshare/manager.log`. Registra AccountIDs,
SteamID64 y la decisión `allowed/enforced`; `enforced` expresa la intención
capturada, no certifica que una expulsión haya terminado. Las credenciales Steam
no pertenecen al manager.

## Historial SQL

SQL es un módulo interno, sin plugin ni biblioteca independientes. Conserva
borrower y owner por AccountID, SteamID64, nombres, excepción e intención de
expulsión. `created_at` corresponde a la inserción en la base de datos.

MySQL requiere el esquema de `configs/sql-init/mysql/`. Una instalación 1.4
requiere la [migración](MIGRATION.md); SQLite crea automáticamente una tabla nueva.
No se modifica automáticamente un esquema existente.

La conexión hace hasta dos intentos, separados por 30 segundos. Cambiar
configuración o ejecutar la configuración de un mapa reconcilia la conexión;
callbacks de generaciones anteriores no reemplazan la conexión actual. Los
errores de escritura conservan la fila para una reconciliación posterior.

Los pendientes usan `data/l4d2_familyshare_pending.kv`, compatible con las filas
antiguas. Cada fila permanece en el outbox hasta recibir confirmación del INSERT;
deshabilitar SQL o descargar manager conserva los pendientes. Un archivo corrupto
se preserva para reparación manual. La entrega es al menos una vez: un cierre
abrupto entre el commit SQL y la confirmación local puede duplicar una fila.

La biblioteca de manager publica únicamente `L4D2FamilyShareManager_IsAccountAllowed`
y `L4D2FamilyShareManager_IsClientEnforced`, documentados en su include. La detección
y los perfiles se consumen desde la API del core. La API combinada anterior se
retira; los consumidores deben seguir la [guía de migración](MIGRATION.md).
