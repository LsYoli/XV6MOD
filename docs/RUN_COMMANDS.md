# Comandos rápidos para ejecutar xv6-riscv modificado

```bash
# 1) Clonar el repositorio
git clone https://example.com/xv6-riscv-mlfq.git
cd xv6-riscv-mlfq

# 2) Abrir contenedor con toolchain RISC-V y QEMU
# (usa volumen para compartir el código con el contenedor)
docker run --rm -it -v $(pwd):/workspace -w /workspace ghcr.io/mit-pdos/xv6-riscv-build:latest bash

# 3) Compilar y ejecutar en QEMU
make clean && make qemu
```

Dentro de QEMU, deberías ver el prompt `xv6$`. Puedes probar con `ls`, `ps`, `yes > /dev/null` y `stressfs` para observar el scheduler MLFQ en acción.
