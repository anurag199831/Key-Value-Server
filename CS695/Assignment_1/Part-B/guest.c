#include <stddef.h>
#include <stdint.h>

static inline uint32_t inl(uint16_t port) {
  uint32_t ret;
  asm("inl %1, %0" : "=a"(ret) : "Nd"(port) : "memory");
  return ret;
}

static inline void outl(uint16_t port, uint32_t value) {
  asm("outl %0,%1" : /* empty */ : "a"(value), "Nd"(port) : "memory");
}

void printVal(uint32_t val) { outl(0xE7, val); }

void display(const char *str) {
  uintptr_t strptr = (uintptr_t)str;
  outl(0xE8, (uint32_t)strptr);
}

uint32_t getNumExits(uint16_t port) {
  return inl(port);
  //   return port + 7;
}

void __attribute__((noreturn)) __attribute__((section(".start"))) _start(void) {
  uint32_t numExits;
  numExits = getNumExits(0xE9);
  printVal(numExits);
  display("Greetings from VM\n");
  numExits = getNumExits(0xE9);
  printVal(numExits);
  *(long *)0x400 = 42;
  for (;;) asm("hlt" : /* empty */ : "a"(42) : "memory");
}
