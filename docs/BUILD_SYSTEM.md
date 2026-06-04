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

- se compilan `l4d2_familyshare` y `l4d2_familyshare_ban_bridge`
- se publica solo el include público `l4d2_familyshare.inc`
- se incluyen traducciones y el árbol `configs/sql-init`

Las bibliotecas complementarias usadas solo para compilación quedan fuera del bundle final.

## CI

El workflow principal separa:

- `deps-smx`
- `build-smx`
- `release`

`release` absorbe el empaquetado liviano y publica el ZIP final.

## WSL

Si el repo se compila desde WSL sobre `/mnt/...`, el builder puede usar un workspace temporal Linux para evitar el costo de I/O sobre el filesystem montado de Windows.
