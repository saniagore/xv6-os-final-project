#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/param.h"

// Tarea intensiva de CPU
void
long_loop(int cycles, int pid)
{
  int i, j, k;
  int total = 0;
  // Bucle largo que mantiene la CPU ocupada
  for(i = 0; i < cycles; i++) {
    for(j = 0; j < 1000; j++) {
      k = i * j;
      total += k;
    }
  }
  printf("Total calculated for PID %d: %d\n", pid, total);
}

// Función placeholder para demostrar la intención de prioridad
void
set_priority(int pid, int new_priority)
{
    // Esta función no existe como syscall, pero documenta la intención de la prueba.
    // El scheduler usará la prioridad asignada en allocproc.
    printf("Setting priority for PID %d to %d (Priority is hardcoded by scheduler).\n", pid, new_priority);
}

int
main(int argc, char *argv[])
{
  int pid_A, pid_B;
  int cycles = 500; // Ajusta este valor si la prueba es muy rápida o muy lenta
  
  printf("Starting Priority Test with %d cycles.\n", cycles);

  // --- Proceso de Alta Prioridad (A) ---
  pid_A = fork();
  if (pid_A == 0) {
    // El kernel asigna la prioridad por defecto (5). Para esta prueba, 
    // asumimos que el scheduler favorecerá al primero que ejecute.
    // Para una prueba REAL, tendríamos que cambiar la prioridad aquí con una syscall.
    set_priority(getpid(), 1); // ALTA PRIORIDAD (Conceptual)
    long_loop(cycles * 10, getpid()); 
    exit(0);
  }

  // --- Proceso de Baja Prioridad (B) ---
  pid_B = fork();
  if (pid_B == 0) {
    set_priority(getpid(), 10); // BAJA PRIORIDAD (Conceptual)
    long_loop(cycles * 10, getpid()); 
    exit(0);
  }

  // --- Proceso Padre (Espera) ---
  if (pid_A > 0 && pid_B > 0) {
    wait(0); 
    wait(0); 
  }

  printf("Priority Test complete.\n");
  exit(0);
}
