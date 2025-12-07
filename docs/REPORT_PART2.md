# Segunda parte del informe: configuración, análisis y validación

## (A) Configuración y ejecución de xv6-riscv con Docker en Windows
1. Instalar [Docker Desktop](https://www.docker.com/products/docker-desktop) y habilitar WSL2.
2. Abrir "Terminal de Windows" y clonar el repositorio modificado:
   ```bash
   git clone https://example.com/xv6-riscv-mlfq.git
   cd xv6-riscv-mlfq
   ```
3. Crear el contenedor de trabajo (usa la imagen oficial de RISC-V para enseñanza):
   ```bash
   docker run --rm -it -v $(pwd):/workspace -w /workspace ghcr.io/mit-pdos/xv6-riscv-build:latest bash
   ```
4. Compilar y ejecutar dentro del contenedor:
   ```bash
   make clean && make qemu
   ```
5. Verificar QEMU: la consola debe mostrar el prompt `xv6$`; si se abre una ventana gráfica, debe imprimirse la salida en la terminal del contenedor. La comunicación es correcta cuando los comandos (`ls`, `sh`, `ps`) responden de inmediato.
6. ¿Por qué RISC-V? RISC-V es una arquitectura limpia y minimalista que facilita el estudio del kernel y del scheduler, manteniendo fidelidad con el diseño original de xv6 del MIT.

## (B) Análisis del scheduler original de xv6
* Modelo: round-robin con prioridad fija, todos los RUNNABLE comparten el mismo quantum.
* Temporizador: la interrupción de reloj actualiza `ticks` y fuerza `yield()` cuando el proceso consumió su quantum.
* Funciones clave:
  * `scheduler()`: recorre la tabla de procesos y ejecuta el primero RUNNABLE.
  * `sched()`: guarda contexto y vuelve al scheduler.
  * `yield()`: marca RUNNABLE y llama a `sched()`.
  * `sleep()` / `wakeup()`: bloquean y reactivan procesos, manteniendo el orden round-robin.
* Estructura `struct proc`: contiene `state`, `pid`, `context`, `trapframe`, `kstack` y contadores de ticks para controlar la expulsión del CPU.

## (C) Justificación del cambio a MLFQ minimalista
* MLFQ mejora la latencia de procesos interactivos al dar prioridad alta con quanta cortos; los CPU-bound descienden gradualmente.
* Reduce la dominancia de tareas largas y mejora tiempos de respuesta sin reescrituras complejas.
* Se implementó con el mínimo de código: un campo `priority`, un contador de ticks usados y una cola virtual por nivel evaluada en `scheduler()`.
* Mantiene la claridad académica: sin estructuras adicionales ni macros complejas; la promoción ocurre sólo cuando el proceso cede CPU antes de agotar su quantum, la degradación cuando lo consume.

## (D) Instrucciones para reproducir y observar los cambios
1. Clonar y entrar al árbol del proyecto (ver paso A.2).
2. Construir y ejecutar en QEMU (paso A.4).
3. Observar el MLFQ en acción:
   * Lanzar `stressfs`, `yes > /dev/null` y `ls` en distintas consolas de xv6.
   * Ejecutar `ps` para ver `PRIORITY` y ticks por cola; los procesos interactivos se mantienen en prioridad alta.
4. Registro ligero de CPU/I/O:
   * Llamadas auxiliares `cpu_track_burst()` y `io_note_event()` guardan el último burst o evento; `cpu_last_burst()` e `io_last_event()` permiten leerlo si se añaden prints de depuración.
5. Reiniciar contenedores Docker si es necesario:
   ```bash
   docker ps -a
   docker stop <id>
   docker rm <id>
   docker rm -f <id>   # forzar
   ```

