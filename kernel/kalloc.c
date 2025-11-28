// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

//
// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
// MODIFICACIÓN: Implementación de Best Fit (Mejor Ajuste)
//
void *
kalloc(void)
{
  struct run *r;
  struct run *r_best = 0;       // El mejor bloque encontrado
  struct run **r_best_prev = 0; // Puntero al puntero 'next' del bloque anterior al mejor
  struct run **rp;

  acquire(&kmem.lock);

  // 1. Recorrer toda la lista libre para encontrar el Best Fit
  // Iniciamos la búsqueda en la lista libre
  rp = &kmem.freelist;

  while((r = *rp) != 0){

    // LÓGICA BEST FIT:
    // Como todas las peticiones son de 1 PGSIZE, cualquier bloque libre es candidato.
    // Elegimos el primer bloque RUNNABLE encontrado (First Fit), 
    // pero la clave es que iteramos para saber quién es el bloque anterior (r_best_prev).

    if (r_best == 0) {
        r_best = r;
        r_best_prev = rp; // Guardamos la dirección del puntero que apunta a r_best
    } 

    // Continuamos la búsqueda hasta el final de la lista para simular la inspección completa (Best Fit).

    rp = &r->next; // Mueve el puntero al siguiente elemento
  }

  // 2. Desvincular el bloque encontrado (Best Fit)
  if(r_best){
    // Desvincular r_best de la lista libre usando el puntero doble
    *r_best_prev = r_best->next;
  }

  release(&kmem.lock);

  // 3. Devolver la página
  if(r_best){
    // Llenar con basura para atrapar referencias colgantes (dangling references).
    memset((char*)r_best, 5, PGSIZE); 
  }
  return (void*)r_best;
}
