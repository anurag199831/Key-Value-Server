#include <fcntl.h>

typedef struct {
  int flags;
  char *pathname;
  mode_t mode;
} open_params;

typedef struct {
  int fd;
  void *buf;
  size_t count;
} read_write_params;

typedef struct {
  int fd;
  long offset;
  int origin;
} seek_params;