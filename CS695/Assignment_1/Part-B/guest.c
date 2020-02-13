#include <stddef.h>
#include <stdint.h>

#include "constants.h"
#include "flags.h"
#include "structures.h"

static inline uint32_t inl(uint16_t port) {
  uint32_t ret;
  asm volatile("inl %1, %0" : "=a"(ret) : "Nd"(port) : "memory");
  return ret;
}

static inline void outl(uint16_t port, uint32_t value) {
  asm volatile("outl %0,%1" : /* empty */ : "a"(value), "Nd"(port) : "memory");
}

void printVal(uint32_t val) { outl(PORT_PRINT_VALUE, val); }

void display(const char *str) {
  uint32_t strptr = (uintptr_t)str;
  outl(PORT_DISPLAY_STRING, (uint32_t)strptr);
}

uint32_t getNumExits(uint16_t port) { return inl(port); }

int file_open(char *pathname, int flags) {
  open_params parameters;
  parameters.pathname = pathname;
  parameters.flags = flags;
  parameters.mode = -1;
  outl(PORT_OPEN, (uint32_t)(uintptr_t)&parameters);
  return inl(PORT_OPEN);
}

void file_close(int fd) { outl(PORT_CLOSE, (uint32_t)fd); }

long file_read(int fd, void *buf, size_t count) {
  read_write_params parameters;
  parameters.fd = fd;
  parameters.count = count;
  parameters.buf = buf;
  outl(PORT_READ, (uint32_t)(uintptr_t)&parameters);
  return inl(PORT_READ);
}

long file_write(int fd, void *buf, size_t count) {
  read_write_params parameters;
  parameters.fd = fd;
  parameters.count = count;
  parameters.buf = buf;
  outl(PORT_WRITE, (uint32_t)(uintptr_t)&parameters);
  return inl(PORT_WRITE);
}

void __attribute__((noreturn)) __attribute__((section(".start"))) _start(void) {
  uint32_t numExits;
  numExits = getNumExits(PORT_GET_EXITS);
  display("Greetings from VM\n");
  numExits = getNumExits(PORT_GET_EXITS);
  printVal(numExits);

  char buff[4096] = "Fuck\n";
  int fd = file_open("test", O_CREAT | O_RDWR);
  file_read(fd, buff, 250);
  display("guest: fd: ");
  printVal(fd);
  display(buff);
  fd = fd * 3;

  *(long *)0x400 = 42;
  for (;;) asm("hlt" : /* empty */ : "a"(42) : "memory");
}
