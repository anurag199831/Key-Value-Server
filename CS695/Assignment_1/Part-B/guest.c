#include <stdarg.h>
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

static inline void outb(uint16_t port, int32_t value) {
  asm volatile("outl %0,%1" : /* empty */ : "a"(value), "Nd"(port) : "memory");
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

int file_open(char *pathname, int flags, ...) {
  va_list valist;
  va_start(valist, flags);
  long mode = va_arg(valist, long);
  switch (mode) {
    case S_IRWXU:
    case S_IRUSR:
    case S_IWUSR:
    case S_IXUSR:
    case S_IRWXG:
    case S_IRGRP:
    case S_IWGRP:
    case S_IXGRP:
    case S_IRWXO:
    case S_IROTH:
    case S_IWOTH:
    case S_IXOTH:
      break;
    default:
      mode = -1;
      break;
  }
  // display("guest: mode: ");
  // printVal(mode);
  va_end(valist);
  open_params parameters;
  parameters.pathname = pathname;
  parameters.flags = flags;
  parameters.mode = mode;
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

static inline void print_error(const char *tag) {
  display("guest:");
  display(tag);
  display(" : Error Occured");
}

static inline void reset_buffer(char *buff) {
  int i = 0;
  while (i < MAX_BUFF_SIZE) {
    buff[i++] = '\0';
  }
}

void __attribute__((noreturn)) __attribute__((section(".start"))) _start(void) {
  const char *p;

  int numExits_before = getNumExits(PORT_GET_EXITS);
  for (p = "Hello, world!\n"; *p; ++p) outb(0xE9, *p);
  int numExits_after = getNumExits(PORT_GET_EXITS);

  display("Exits required to print \"Hello World!\\n\" are ");
  printVal(numExits_after - numExits_before - 1);

  numExits_before = getNumExits(PORT_GET_EXITS);
  display("Greetings from VM\n");
  numExits_after = getNumExits(PORT_GET_EXITS);

  display(
      "Exits required to print \"Greetings form VM\\n\", using display() are ");
  printVal(numExits_after - numExits_before - 1);

  char buff[MAX_BUFF_SIZE];
  int fd1, fd2, count_read, closed_fd;
  int count_write;
  // int count_seek;
  int i = 0;

  reset_buffer(buff);

  fd1 = file_open("test1.txt", O_CREAT | O_RDWR);
  display("guest: fd: ");
  printVal(fd1);
  if (fd1 < 0) print_error("open");

  // count_read = file_read(fd1, buff, 17);
  // if (count_read < 0) print_error("read");
  // display("guest: count_read: ");
  // printVal(count_read);
  // display(buff);

  i = 0;
  for (char *p = "Pranav Chaudhary"; *p != '\0'; p++, i++) {
    buff[i] = *p;
  }

  count_write = file_write(fd1, buff, 16);
  display("guest: Writing File: count_write: ");
  printVal(count_write);

  reset_buffer(buff);
  file_seek(fd1, 0, SEEK_SET);
  count_read = file_read(fd1, buff, 20);
  if (count_read < 0) print_error("read");
  display("guest: Reading file :count_read: ");
  printVal(count_read);
  display(buff);
  display("\n");

  fd2 = file_open("test2.txt", O_CREAT | O_RDWR);
  if (fd2 < 0) print_error("open");
  display("guest: fd: ");
  printVal(fd2);

  i = 0;
  for (char *p = "May the force be with You!"; *p != '\0'; p++, i++) {
    buff[i] = *p;
  }

  count_write = file_write(fd2, buff, 26);
  display("guest:Writing File: count_write: ");
  printVal(count_write);

  reset_buffer(buff);
  file_seek(fd2, 0, SEEK_SET);
  count_read = file_read(fd2, buff, 30);
  if (count_read < 0) print_error("read");
  display("guest: Reading File: count_read: ");
  printVal(count_read);
  display(buff);
  display("\n");

  closed_fd = file_close(fd2);
  if (closed_fd < 0) print_error("close");
  display("guest: close returned with status code: ");
  printVal(closed_fd);

  closed_fd = file_close(fd1);
  if (closed_fd < 0) print_error("close");
  display("guest: close returned with status code: ");
  printVal(closed_fd);

  // Q8.1 The number 42 is written here to Guest virtual address 0x400.
  *(long *)0x400 = 42;
  for (;;) asm("hlt" : /* empty */ : "a"(42) : "memory");
}
