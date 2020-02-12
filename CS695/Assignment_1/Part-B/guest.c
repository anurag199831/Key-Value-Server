#include <stddef.h>
#include <stdint.h>

#include "constants.h"
#include "structures.h"

static inline uint32_t inl(uint16_t port) {
  uint32_t ret;
  asm("inl %1, %0" : "=a"(ret) : "Nd"(port) : "memory");
  return ret;
}

static inline void outl(uint16_t port, uint32_t value) {
  asm("outl %0,%1" : /* empty */ : "a"(value), "Nd"(port) : "memory");
}

void printVal(uint32_t val) { outl(PORT_PRINT_VALUE, val); }

void display(const char *str) {
  uintptr_t strptr = (uintptr_t)str;
  outl(PORT_DISPLAY_STRING, (uint32_t)strptr);
}

uint32_t getNumExits(uint16_t port) {
  return inl(port);
  //   return port + 7;
}

int open(const char *pathname, int flags) {
  uint32_t *addr = malloc(sizeof(open_params));
  open_params *params = (open_params *)(addr);
  params->pathname = pathname;
  params->flags = flags;
  params->mode = -1;
  outl(PORT_OPEN, (uint32_t)(uintptr_t)addr);
  return inl(PORT_OPEN);
}

void close(int fd) { outl(PORT_CLOSE, (uint32_t)fd); }

void __attribute__((noreturn)) __attribute__((section(".start"))) _start(void) {
  uint32_t numExits;
  numExits = getNumExits(PORT_GET_EXITS);
  printVal(numExits);
  display("Greetings from VM\n");
  numExits = getNumExits(PORT_GET_EXITS);
  printVal(numExits);
  *(long *)0x400 = 42;
  for (;;) asm("hlt" : /* empty */ : "a"(42) : "memory");
}
