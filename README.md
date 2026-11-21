# Wii-DSi-Link

Proyecto experimental para convertir una Nintendo DSi XL en una segunda pantalla inalámbrica de una Wii.

## Objetivo inicial

Primero compilaremos una aplicación de prueba para Wii y otra para Nintendo DS/DSi. Después verificaremos la conectividad de red y desarrollaremos un protocolo de mensajes bidireccional.

## Estructura

- `wii/`: aplicación homebrew para Wii.
- `dsi/`: aplicación homebrew para Nintendo DSi XL, ejecutable desde TWiLight Menu++.
- `.github/workflows/build.yml`: compilación automática mediante GitHub Actions.

## Estado

Fase 1: pruebas de compilación independientes. La comunicación Wi-Fi todavía no está implementada.
