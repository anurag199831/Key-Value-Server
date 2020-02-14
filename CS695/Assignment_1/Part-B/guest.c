#include <stddef.h>
#include <stdint.h>

#include "constants.h"
#include "flags.h"
#include "structures.h"

static inline int32_t inl(uint16_t port) {
  int32_t ret;
  asm volatile("inl %1, %0" : "=a"(ret) : "Nd"(port) : "memory");
  return ret;
}

static inline void outl(uint16_t port, int32_t value) {
  asm volatile("outl %0,%1" : /* empty */ : "a"(value), "Nd"(port) : "memory");
}

void printVal(int32_t val) { outl(PORT_PRINT_VALUE, val); }

void display(const char *str) {
  int32_t strptr = (intptr_t)str;
  outl(PORT_DISPLAY_STRING, (int32_t)strptr);
}

int32_t getNumExits(uint16_t port) { return inl(port); }

int file_open(char *pathname, int flags) {
  open_params parameters;
  parameters.pathname = pathname;
  parameters.flags = flags;
  parameters.mode = -1;
  outl(PORT_OPEN, (int32_t)(intptr_t)&parameters);
  return inl(PORT_OPEN);
}

int file_close(int fd) {
  outl(PORT_CLOSE, (int32_t)fd);
  return inl(PORT_CLOSE);
}

long file_read(int fd, void *buf, size_t count) {
  read_write_params parameters;
  parameters.fd = fd;
  parameters.count = count;
  parameters.buf = buf;
  outl(PORT_READ, (int32_t)(intptr_t)&parameters);
  return inl(PORT_READ);
}

long file_write(int fd, void *buf, size_t count) {
  read_write_params parameters;
  parameters.fd = fd;
  parameters.count = count;
  parameters.buf = buf;
  outl(PORT_WRITE, (int32_t)(intptr_t)&parameters);
  return inl(PORT_WRITE);
}

int file_seek(int fd, long offset, int whence) {
  seek_params parameters;
  parameters.fd = fd;
  parameters.offset = offset;
  parameters.whence = whence;
  outl(PORT_SEEK, (int32_t)(intptr_t)&parameters);
  return inl(PORT_SEEK);
}

void __attribute__((noreturn)) __attribute__((section(".start"))) _start(void) {
  uint32_t numExits;
  numExits = getNumExits(PORT_GET_EXITS);
  display("Greetings from VM\n");
  numExits = getNumExits(PORT_GET_EXITS);
  printVal(numExits);

  char buff[MAX_BUFF_SIZE];
  int fd, count_read, count_write, count_seek, closed_fd;

  fd = file_open("test", O_CREAT | O_RDWR);
  display("guest: fd: ");
  printVal(fd);

  count_read = file_read(fd, buff, 50);
  display("guest: count_read: ");
  printVal(count_read);
  display(buff);

  int i = 0;

  while (i < MAX_BUFF_SIZE) {
    buff[i++] = '\0';
  }

  i = 0;
  for (char *p = "Fuck this fake shit!\n"; *p != '\0'; p++, i++) {
    buff[i] = *p;
  }

  count_write = file_write(fd, buff, 21);
  display("guest: count_write: ");
  printVal(count_write);
  display(buff);

  count_seek = file_seek(fd, 0, SEEK_SET);
  display("guest: count_seek: ");
  printVal(count_seek);

  count_read = file_read(fd, buff, 50);
  display("guest: count_read: ");
  printVal(count_read);
  display(buff);

  closed_fd = file_close(fd);
  display("guest: close returned: ");
  printVal(closed_fd);
  *(long *)0x400 = 42;
  for (;;) asm("hlt" : /* empty */ : "a"(42) : "memory");
}
