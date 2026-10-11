# Build System

Este repositorio usa un flujo SourceMod unificado para desarrollo local y CI.

## Objetivo

El mismo proceso debe servir para:

- compilación local en Windows
- compilación local en Linux o WSL
- compilación en GitHub Actions

La fuente real del flujo es Python. `make` actúa como interfaz corta y el CI reutiliza esos mismos comandos.

## Targets

- `make deps-smx`
- `make build-smx`
- `make package-smx`
- `make release`

## Manifiesto

[plugin-package-map.json](../plugin-package-map.json) define:

- qué plugins se compilan
- qué archivos runtime entran al artifact

En este repo:

- se compilan exactamente `l4d2_familyshare_core` y `l4d2_familyshare_manager`
- se publican los includes de core, manager y stocks
- se incluyen sus módulos internos, traducciones, el archivo inicial de permitidos y `configs/sql-init`
- no se genera un plugin SQL, de perfiles o de bridge

`l4d2_familyshare_stock.inc` reúne las utilidades de identidad y el logger compartido. Cada
plugin declara e inicializa su propio `g_Log`, con ConVars y archivo independientes.

`steamidtools_stock.inc` se distribuye porque los helpers compartidos reutilizan
su conversión decimal. Este include no exige cargar el plugin SteamIDTools. Las
demás bibliotecas complementarias de compilación quedan fuera del bundle final.

## CI

El workflow principal separa:

- `deps-smx`
- `build-smx`
- `release`

`release` absorbe el empaquetado liviano y publica el ZIP final.

## WSL

Si el repo se compila desde WSL sobre `/mnt/...`, el builder puede usar un workspace temporal Linux para evitar el costo de I/O sobre el filesystem montado de Windows.
