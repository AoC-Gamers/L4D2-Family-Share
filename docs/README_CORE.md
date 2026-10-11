# Family Share Core

`l4d2_familyshare_core` registra la biblioteca del mismo nombre. Su contrato está
en `l4d2_familyshare_core.inc`; las utilidades de identidad y logging están en
`l4d2_familyshare_stock.inc` y no registran una biblioteca adicional. Cada plugin
conserva su propia instancia del logger.

## Detección

```sourcepawn
public void L4D2FamilyShareCore_OnDetected(int client, int borrowerAccountId, int ownerAccountId)
```

Se publica una vez por pareja borrower/owner en la conexión actual cuando
SteamWorks valida una licencia prestada. `client` sirve para el runtime actual;
los AccountIDs son identidades, representadas como celdas de 32 bits sin signo
(`%u` al escribirlas). El ID del owner no implica que esté conectado.

Los natives `L4D2FamilyShareCore_IsClientDetected`,
`L4D2FamilyShareCore_GetClientBorrowerAccountId` y
`L4D2FamilyShareCore_GetClientOwnerAccountId` consultan solo la conexión actual.
Devuelven false/0 para clientes inválidos o desconectados. Cargar tarde el core
no reconstruye validaciones previas; los consumidores capturan contexto al
recibir el forward. El core no publica un resultado de enforcement.

## Perfiles

`L4D2FamilyShareCore_RequestProfile(accountId)` devuelve un request ID positivo,
o 0 ante un ID inválido o descarga del core. El callback siempre es asíncrono,
incluso cuando solo se usa el fallback local:

```sourcepawn
public void L4D2FamilyShareCore_OnProfileResolved(int requestId, int accountId,
    const char[] steamId64, const char[] personaName, const char[] profileUrl)
```

Correlacionar por request ID **y AccountID**. El core nunca usa un client guardado
para resolver la respuesta. Las solicitudes pueden terminar después de una
desconexión o cambio de mapa. Un límite de 25 segundos devuelve los datos
locales; callbacks posteriores se ignoran. Al perder SteamIDTools, las
conversiones pendientes terminan con fallback. Al descargar el core se cierran
las solicitudes pendientes. Los consumidores deben tener también un fallback
propio si su dependencia se descarga.

SteamID64 y URL se calculan localmente. El nombre se captura si la cuenta está
conectada y se enriquece opcionalmente desde Steam; puede estar vacío si no se
conoce. Una respuesta de conversión no puede cambiar el AccountID solicitado.
Los formatos JSON de nombre que no puedan decodificarse conservan el nombre
capturado. Los perfiles no otorgan permisos.

## Configuración

Archivo: `cfg/sourcemod/l4d2_familyshare_core.cfg`.

| ConVar | Default | Uso |
|---|---|---|
| `l4d2_familyshare_steamidtools_backend` | `1` | Conversión online opcional mediante SteamIDTools. |
| `l4d2_familyshare_steam_api_key` | vacío | Clave protegida para summaries; no se registra. |
| `l4d2_familyshare_core_log_mode` | `0` | Apagado=0, normal=1, debug=2. |
| `l4d2_familyshare_core_debug_mask` | `0` | Identidad=1, async=2, lifecycle=4; todos=7. |

El archivo propio es `addons/sourcemod/logs/familyshare/core.log`, resuelto desde
`Path_SM`. El core no contiene SQL, listas de permitidos, chat ni expulsiones.
