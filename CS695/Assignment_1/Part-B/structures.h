typedef struct {
  int flags;
  char *pathname;
  unsigned int mode;
} open_params;

typedef struct {
  int fd;
  char *buf;
  size_t count;
} read_write_params;

typedef struct {
  int fd;
  long offset;
  int origin;
} seek_params;