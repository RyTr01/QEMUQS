# QEMUQS

Interfaz CLI en C++ para crear, configurar y ejecutar máquinas virtuales QEMU
utilizando un archivo 'config.json'.

## Características
- Creación de discos qcow2
- Ejecución de VMs con QEMU
- Configuración persistente vía JSON
- Compatibilidad con Windows y Linux

## Requisitos
- QEMU (qemu-system-x86_64, qemu-img)
- Compilador C++17 o superior

## Compilación
### Linux
g++ main.cpp -std=c++17 -o qemuqs

## Windows (MinGW)
g++ main.cpp -std=c++17 -o qemuqs.exe

## Dependencias
- nlohmann/json (single-header, incluido en el repositorio)

## Próximas actualizaciones
- Diversas configuraciones de VMs con un menú
- Hacer más estético QEMUQS

## Licencia
MIT License
