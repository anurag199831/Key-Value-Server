#include <errno.h>
#include <fcntl.h>
#include <linux/kvm.h>
#include <linux/mman.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "constants.h"
#include "structures.h"

/* CR0 bits */
#define CR0_PE 1u
#define CR0_MP (1U << 1)
#define CR0_EM (1U << 2)
#define CR0_TS (1U << 3)
#define CR0_ET (1U << 4)
#define CR0_NE (1U << 5)
#define CR0_WP (1U << 16)
#define CR0_AM (1U << 18)
#define CR0_NW (1U << 29)
#define CR0_CD (1U << 30)
#define CR0_PG (1U << 31)

/* CR4 bits */
#define CR4_VME 1
#define CR4_PVI (1U << 1)
#define CR4_TSD (1U << 2)
#define CR4_DE (1U << 3)
#define CR4_PSE (1U << 4)
#define CR4_PAE (1U << 5)
#define CR4_MCE (1U << 6)
#define CR4_PGE (1U << 7)
#define CR4_PCE (1U << 8)
#define CR4_OSFXSR (1U << 8)
#define CR4_OSXMMEXCPT (1U << 10)
#define CR4_UMIP (1U << 11)
#define CR4_VMXE (1U << 13)
#define CR4_SMXE (1U << 14)
#define CR4_FSGSBASE (1U << 16)
#define CR4_PCIDE (1U << 17)
#define CR4_OSXSAVE (1U << 18)
#define CR4_SMEP (1U << 20)
#define CR4_SMAP (1U << 21)

#define EFER_SCE 1
#define EFER_LME (1U << 8)
#define EFER_LMA (1U << 10)
#define EFER_NXE (1U << 11)

/* 32-bit page directory entry bits */
#define PDE32_PRESENT 1
#define PDE32_RW (1U << 1)
#define PDE32_USER (1U << 2)
#define PDE32_PS (1U << 7)

/* 64-bit page * entry bits */
#define PDE64_PRESENT 1
#define PDE64_RW (1U << 1)
#define PDE64_USER (1U << 2)
#define PDE64_ACCESSED (1U << 5)
#define PDE64_DIRTY (1U << 6)
#define PDE64_PS (1U << 7)
#define PDE64_G (1U << 8)

struct vm {
  int sys_fd;
  int fd;
  char *mem;
};

static inline int hyper_open(const char *pathname, int flags) {
  int fd = open(pathname, flags, 0700);
  return fd;
}
static inline int hyper_write(int fd, void *buf, size_t count) {
  return write(fd, buf, count);
}

static inline long hyper_read(int fd, void *buf, size_t count) {
  return read(fd, buf, count);
}

static inline long hyper_seek(int fd, long offset, int whence) {
  return lseek(fd, offset, whence);
}

static inline int hyper_close(int fd) { return close(fd); }

void vm_init(struct vm *vm, size_t mem_size) {
  int api_ver;
  struct kvm_userspace_memory_region memreg;

  vm->sys_fd = open("/dev/kvm", O_RDWR);
  if (vm->sys_fd < 0) {
    perror("open /dev/kvm");
    exit(1);
  }

  api_ver = ioctl(vm->sys_fd, KVM_GET_API_VERSION, 0);
  if (api_ver < 0) {
    perror("KVM_GET_API_VERSION");
    exit(1);
  }

  if (api_ver != KVM_API_VERSION) {
    fprintf(stderr, "Got KVM api version %d, expected %d\n", api_ver,
            KVM_API_VERSION);
    exit(1);
  }

  vm->fd = ioctl(vm->sys_fd, KVM_CREATE_VM, 0);
  if (vm->fd < 0) {
    perror("KVM_CREATE_VM");
    exit(1);
  }

  if (ioctl(vm->fd, KVM_SET_TSS_ADDR, 0xfffbd000) < 0) {
    perror("KVM_SET_TSS_ADDR");
    exit(1);
  }
  // Q1.2 Memory is allocated here using mmap.
  vm->mem = mmap(NULL, mem_size, PROT_READ | PROT_WRITE,
                 MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
  if (vm->mem == MAP_FAILED) {
    perror("mmap mem");
    exit(1);
  }
  // Q1.3  The virtual address where the memory mapped into the virtual
  // address space of this simple hypervisor is
  printf("Q1.3 - vm->mem starts at %p\n", vm->mem);

  madvise(vm->mem, mem_size, MADV_MERGEABLE);

  memreg.slot = 0;
  memreg.flags = 0;
  memreg.guest_phys_addr = 0;
  memreg.memory_size = mem_size;
  memreg.userspace_addr = (unsigned long)vm->mem;
  if (ioctl(vm->fd, KVM_SET_USER_MEMORY_REGION, &memreg) < 0) {
    perror("KVM_SET_USER_MEMORY_REGION");
    exit(1);
  }
}

struct vcpu {
  int fd;
  struct kvm_run *kvm_run;
};

void vcpu_init(struct vm *vm, struct vcpu *vcpu) {
  int vcpu_mmap_size;

  vcpu->fd = ioctl(vm->fd, KVM_CREATE_VCPU, 0);
  if (vcpu->fd < 0) {
    perror("KVM_CREATE_VCPU");
    exit(1);
  }

  vcpu_mmap_size = ioctl(vm->sys_fd, KVM_GET_VCPU_MMAP_SIZE, 0);
  if (vcpu_mmap_size <= 0) {
    perror("KVM_GET_VCPU_MMAP_SIZE");
    exit(1);
  }
  // Q2.2 Size of the VCPU struct
  printf("Q2.2 - Size of VCPU: %d\n", vcpu_mmap_size);

  // Q2.1 Memory for VCPU is allocated here.
  vcpu->kvm_run = mmap(NULL, vcpu_mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED,
                       vcpu->fd, 0);
  if (vcpu->kvm_run == MAP_FAILED) {
    perror("mmap kvm_run");
    exit(1);
  }
  // Q2.3 It is located in the virutal address space of the hypervisor at
  printf("Q2.3 - VCPU is located at: %p\n", vcpu->kvm_run);
}

// Function for assembly code for "out" instruction
static inline void outl(uint16_t port, uint32_t value) {
  asm("outl %0,%1" : /* empty */ : "a"(value), "Nd"(port) : "memory");
}

int run_vm(struct vm *vm, struct vcpu *vcpu, size_t sz) {
  struct kvm_regs regs;
  uint64_t memval = 0;
  uint32_t mNumOfExits = 0;

  int status = -1;
  bool fds[MAX_FDS];
  int open_fd_count = 0;

  for (int i = 0; i < MAX_FDS; i++) {
    fds[i] = false;
  }

  open_params *pointer_open_params = NULL;
  read_write_params *pointer_read_write_params = NULL;
  seek_params *pointer_seek_params = NULL;

  open_params recv_open_params = {0};
  read_write_params recv_read_write_params = {0};
  int recv_close_fd = 0;

  for (;;) {
    // Q6.1 The hypervisor to guest switch is done here.
    if (ioctl(vcpu->fd, KVM_RUN, 0) < 0) {
      perror("KVM_RUN");
      exit(1);
    }
    // Q6.2 The guest will return here after the switch is made to hypervisor
    // from the guest.

    switch (vcpu->kvm_run->exit_reason) {
      case KVM_EXIT_HLT:
        mNumOfExits++;
        goto check;

      case KVM_EXIT_IO:
        mNumOfExits++;
        if (vcpu->kvm_run->io.direction == KVM_EXIT_IO_OUT &&
            vcpu->kvm_run->io.port == 0xE9) {
          // Q7 Hello World is printed using 0xE9
          char *p = (char *)vcpu->kvm_run;
          fwrite(p + vcpu->kvm_run->io.data_offset, vcpu->kvm_run->io.size, 1,
                 stdout);
          fflush(stdout);
          continue;
        } else if (vcpu->kvm_run->io.direction == KVM_EXIT_IO_OUT &&
                   vcpu->kvm_run->io.port == PORT_PRINT_VALUE) {
          // handle printval function from guest.c here
          // printf("Host: printfval called: numOfExits: %u\n", mNumOfExits);
          char *ptr = (char *)vcpu->kvm_run + vcpu->kvm_run->io.data_offset;
          int32_t *numptr = (int32_t *)ptr;
          printf("%d\n", *numptr);

        } else if (vcpu->kvm_run->io.direction == KVM_EXIT_IO_OUT &&
                   vcpu->kvm_run->io.port == PORT_DISPLAY_STRING) {
          // handle display function from guest.c here
          // printf("Host: display called: numOfExits: %u\n", mNumOfExits);
          int32_t *ptr = (int32_t *)((intptr_t)vcpu->kvm_run +
                                     vcpu->kvm_run->io.data_offset);
          printf("%s", &vm->mem[*ptr]);

        } else if (vcpu->kvm_run->io.direction == KVM_EXIT_IO_IN &&
                   vcpu->kvm_run->io.port == PORT_GET_EXITS) {
          // handle getNumExits function from guest.c here
          // printf("Host: getNumExits called: numOfExits: %u\n", mNumOfExits);
          int32_t *ptr = (int32_t *)((char *)vcpu->kvm_run +
                                     vcpu->kvm_run->io.data_offset);
          *ptr = mNumOfExits;

        } else if (vcpu->kvm_run->io.direction == KVM_EXIT_IO_OUT &&
                   vcpu->kvm_run->io.port == PORT_OPEN) {
          // handle open::out function from guest.c here
          // printf("Host: open::out called\n");
          int32_t *ptr = (int32_t *)((char *)vcpu->kvm_run +
                                     vcpu->kvm_run->io.data_offset);
          pointer_open_params = (open_params *)&vm->mem[*ptr];
          // printf("pointer_open_params->pathname: %s\n",
          //        &vm->mem[(intptr_t)pointer_open_params->pathname]);
          // printf("pointer_open_params->flags:%d\n",
          // pointer_open_params->flags); printf("pointer_open_params->mode:
          // %d\n", pointer_open_params->mode);
          recv_open_params.pathname =
              &vm->mem[(intptr_t)pointer_open_params->pathname];
          recv_open_params.flags = pointer_open_params->flags;
          recv_open_params.mode = pointer_open_params->mode;

        } else if (vcpu->kvm_run->io.direction == KVM_EXIT_IO_IN &&
                   vcpu->kvm_run->io.port == PORT_OPEN) {
          // handle open::in function from guest.c here
          // printf("Host: open::in called\n");
          int32_t *ptr = (int32_t *)((char *)vcpu->kvm_run +
                                     vcpu->kvm_run->io.data_offset);
          int fd =
              hyper_open(recv_open_params.pathname, recv_open_params.flags);
          if (fd < 0) {
            perror("Host: OPEN");
          } else {
            // printf("Host: fd: %d\n", fd);
            open_fd_count++;
            fds[fd] = true;
          }
          *ptr = (int32_t)fd;

        } else if (vcpu->kvm_run->io.direction == KVM_EXIT_IO_OUT &&
                   vcpu->kvm_run->io.port == PORT_READ) {
          // handle read function from guest.c here
          // printf("Host: read::out called\n");
          int32_t *ptr = (int32_t *)((char *)vcpu->kvm_run +
                                     vcpu->kvm_run->io.data_offset);
          pointer_read_write_params = (read_write_params *)&vm->mem[*ptr];
          // printf("pointer_read_write_params->fd: %d\n",
          //        pointer_read_write_params->fd);
          // printf("pointer_read_write_params->count:%ld\n",
          //        pointer_read_write_params->count);
          // printf("pointer_read_write_params->buf: %p\n",
          //        pointer_read_write_params->buf);
          recv_read_write_params.fd = pointer_read_write_params->fd;
          recv_read_write_params.count = pointer_read_write_params->count;
          recv_read_write_params.buf =
              &vm->mem[(intptr_t)pointer_read_write_params->buf];

        } else if (vcpu->kvm_run->io.direction == KVM_EXIT_IO_IN &&
                   vcpu->kvm_run->io.port == PORT_READ) {
          // handle open::in function from guest.c here
          // printf("Host: read::in called\n");
          int32_t *ptr = (int32_t *)((char *)vcpu->kvm_run +
                                     vcpu->kvm_run->io.data_offset);
          status = read(recv_read_write_params.fd, recv_read_write_params.buf,
                        recv_read_write_params.count);
          if (fds[recv_read_write_params.fd] == true) {
            if (status < 0) {
              perror("Host: READ");
            }
            *ptr = status;
          } else {
            *ptr = -1;
          }

        } else if (vcpu->kvm_run->io.direction == KVM_EXIT_IO_OUT &&
                   vcpu->kvm_run->io.port == PORT_WRITE) {
          // handle write::out function from guest.c here
          // printf("Host: write::out called\n");
          int32_t *ptr = (int32_t *)((char *)vcpu->kvm_run +
                                     vcpu->kvm_run->io.data_offset);
          pointer_read_write_params = (read_write_params *)&vm->mem[*ptr];
          // printf("pointer_read_write_params->fd: %d\n",
          //        pointer_read_write_params->fd);
          // printf("pointer_read_write_params->count:%ld\n",
          //        pointer_read_write_params->count);
          // printf("pointer_read_write_params->buf: %p\n",
          //        pointer_read_write_params->buf);
          recv_read_write_params.fd = pointer_read_write_params->fd;
          recv_read_write_params.count = pointer_read_write_params->count;
          recv_read_write_params.buf =
              &vm->mem[(intptr_t)pointer_read_write_params->buf];

        } else if (vcpu->kvm_run->io.direction == KVM_EXIT_IO_IN &&
                   vcpu->kvm_run->io.port == PORT_WRITE) {
          // handle write::in function from guest.c here
          // printf("Host: write::in called\n");
          int32_t *ptr = (int32_t *)((char *)vcpu->kvm_run +
                                     vcpu->kvm_run->io.data_offset);
          if (fds[recv_read_write_params.fd] == true) {
            status = hyper_write(recv_read_write_params.fd,
                                 recv_read_write_params.buf,
                                 recv_read_write_params.count);
            if (status < 0) {
              perror("Host: WRITE");
            }
            *ptr = status;
          } else {
            *ptr = -1;
          }

        } else if (vcpu->kvm_run->io.direction == KVM_EXIT_IO_OUT &&
                   vcpu->kvm_run->io.port == PORT_SEEK) {
          // handle seek::out function from guest.c here
          // printf("Host: seek::out called\n");
          int32_t *ptr = (int32_t *)((char *)vcpu->kvm_run +
                                     vcpu->kvm_run->io.data_offset);
          pointer_seek_params = (seek_params *)&vm->mem[*ptr];
          // printf("pointer_seek_params->fd: %d\n", pointer_seek_params->fd);
          // printf("pointer_seek_params->whence:%d\n",
          //        pointer_seek_params->whence);
          // printf("pointer_seek_params->offset: %ld\n",
          //        pointer_seek_params->offset);

        } else if (vcpu->kvm_run->io.direction == KVM_EXIT_IO_IN &&
                   vcpu->kvm_run->io.port == PORT_SEEK) {
          // handle seek::in function from guest.c here
          // printf("Host: seek::in called\n");
          int32_t *ptr = (int32_t *)((char *)vcpu->kvm_run +
                                     vcpu->kvm_run->io.data_offset);
          if (fds[pointer_seek_params->fd] == true) {
            status =
                hyper_seek(pointer_seek_params->fd, pointer_seek_params->offset,
                           pointer_seek_params->whence);
            if (status < 0) {
              perror("Host: SEEK");
            }
            *ptr = status;
          } else {
            *ptr = -1;
          }

        } else if (vcpu->kvm_run->io.direction == KVM_EXIT_IO_OUT &&
                   vcpu->kvm_run->io.port == PORT_CLOSE) {
          // handle close function from guest.c here
          // printf("Host: close::out called\n");
          int32_t *ptr = (int32_t *)((char *)vcpu->kvm_run +
                                     vcpu->kvm_run->io.data_offset);
          recv_close_fd = *ptr;

        } else if (vcpu->kvm_run->io.direction == KVM_EXIT_IO_IN &&
                   vcpu->kvm_run->io.port == PORT_CLOSE) {
          // handle close function from guest.c here
          // printf("Host: close::in called for %d\n", recv_close_fd);
          int32_t *ptr = (int32_t *)((char *)vcpu->kvm_run +
                                     vcpu->kvm_run->io.data_offset);
          if (fds[recv_close_fd] == true) {
            status = hyper_close(recv_close_fd);
            if (status < 0) {
              perror("Host: CLOSE");
            } else {
            }
            *ptr = status;
            fds[recv_close_fd] = false;
            open_fd_count--;
          } else {
            *ptr = -1;
          }
        }
        break;
        /* fall through */
      default:
        mNumOfExits++;
        fprintf(stderr,
                "Got exit_reason %d,"
                " expected KVM_EXIT_HLT (%d)\n",
                vcpu->kvm_run->exit_reason, KVM_EXIT_HLT);
        if (open_fd_count != 0) {
          for (int i = 0; i < MAX_FDS; i++) {
            if (fds[i] == true) {
              hyper_close(fds[i]);
            }
          }
        }
        open_fd_count = 0;
        close(vm->fd);
        close(vm->sys_fd);
        exit(1);
    }
  }
check:
  if (ioctl(vcpu->fd, KVM_GET_REGS, &regs) < 0) {
    perror("KVM_GET_REGS");
    exit(1);
  }

  if (regs.rax != 42) {
    printf("Wrong result: {E,R,}AX is %lld\n", regs.rax);
    return 0;
  }

  // Q8.2 The value 42 is received here.
  memcpy(&memval, &vm->mem[0x400], sz);
  if (memval != 42) {
    printf("Wrong result: memory at 0x400 is %lld\n",
           (unsigned long long)memval);
    return 0;
  }
  if (open_fd_count != 0) {
    for (int i = 0; i < MAX_FDS; i++) {
      if (fds[i] == true) {
        hyper_close(fds[i]);
      }
    }
  }
  open_fd_count = 0;
  close(vm->fd);
  close(vm->sys_fd);
  return 1;
}

extern const unsigned char guest16[], guest16_end[];

int run_real_mode(struct vm *vm, struct vcpu *vcpu) {
  struct kvm_sregs sregs;
  struct kvm_regs regs;

  printf("Testing real mode\n");

  if (ioctl(vcpu->fd, KVM_GET_SREGS, &sregs) < 0) {
    perror("KVM_GET_SREGS");
    exit(1);
  }

  sregs.cs.selector = 0;
  sregs.cs.base = 0;

  if (ioctl(vcpu->fd, KVM_SET_SREGS, &sregs) < 0) {
    perror("KVM_SET_SREGS");
    exit(1);
  }

  memset(&regs, 0, sizeof(regs));
  /* Clear all FLAGS bits, except bit 1 which is always set. */
  regs.rflags = 2;
  regs.rip = 0;

  if (ioctl(vcpu->fd, KVM_SET_REGS, &regs) < 0) {
    perror("KVM_SET_REGS");
    exit(1);
  }

  memcpy(vm->mem, guest16, guest16_end - guest16);
  return run_vm(vm, vcpu, 2);
}

static void setup_protected_mode(struct kvm_sregs *sregs) {
  struct kvm_segment seg = {
      .base = 0,
      .limit = 0xffffffff,
      .selector = 1 << 3,
      .present = 1,
      .type = 11, /* Code: execute, read, accessed */
      .dpl = 0,
      .db = 1,
      .s = 1, /* Code/data */
      .l = 0,
      .g = 1, /* 4KB granularity */
  };

  sregs->cr0 |= CR0_PE; /* enter protected mode */

  sregs->cs = seg;

  seg.type = 3; /* Data: read/write, accessed */
  seg.selector = 2 << 3;
  sregs->ds = sregs->es = sregs->fs = sregs->gs = sregs->ss = seg;
}

extern const unsigned char guest32[], guest32_end[];

int run_protected_mode(struct vm *vm, struct vcpu *vcpu) {
  struct kvm_sregs sregs;
  struct kvm_regs regs;

  printf("Testing protected mode\n");

  if (ioctl(vcpu->fd, KVM_GET_SREGS, &sregs) < 0) {
    perror("KVM_GET_SREGS");
    exit(1);
  }

  setup_protected_mode(&sregs);

  if (ioctl(vcpu->fd, KVM_SET_SREGS, &sregs) < 0) {
    perror("KVM_SET_SREGS");
    exit(1);
  }

  memset(&regs, 0, sizeof(regs));
  /* Clear all FLAGS bits, except bit 1 which is always set. */
  regs.rflags = 2;
  regs.rip = 0;

  if (ioctl(vcpu->fd, KVM_SET_REGS, &regs) < 0) {
    perror("KVM_SET_REGS");
    exit(1);
  }

  memcpy(vm->mem, guest32, guest32_end - guest32);
  return run_vm(vm, vcpu, 4);
}

static void setup_paged_32bit_mode(struct vm *vm, struct kvm_sregs *sregs) {
  uint32_t pd_addr = 0x2000;
  uint32_t *pd = (void *)(vm->mem + pd_addr);

  /* A single 4MB page to cover the memory region */
  pd[0] = PDE32_PRESENT | PDE32_RW | PDE32_USER | PDE32_PS;
  /* Other PDEs are left zeroed, meaning not present. */

  sregs->cr3 = pd_addr;
  sregs->cr4 = CR4_PSE;
  sregs->cr0 = CR0_PE | CR0_MP | CR0_ET | CR0_NE | CR0_WP | CR0_AM | CR0_PG;
  sregs->efer = 0;
}

int run_paged_32bit_mode(struct vm *vm, struct vcpu *vcpu) {
  struct kvm_sregs sregs;
  struct kvm_regs regs;

  printf("Testing 32-bit paging\n");

  if (ioctl(vcpu->fd, KVM_GET_SREGS, &sregs) < 0) {
    perror("KVM_GET_SREGS");
    exit(1);
  }

  setup_protected_mode(&sregs);
  setup_paged_32bit_mode(vm, &sregs);

  if (ioctl(vcpu->fd, KVM_SET_SREGS, &sregs) < 0) {
    perror("KVM_SET_SREGS");
    exit(1);
  }

  memset(&regs, 0, sizeof(regs));
  /* Clear all FLAGS bits, except bit 1 which is always set. */
  regs.rflags = 2;
  regs.rip = 0;

  if (ioctl(vcpu->fd, KVM_SET_REGS, &regs) < 0) {
    perror("KVM_SET_REGS");
    exit(1);
  }

  memcpy(vm->mem, guest32, guest32_end - guest32);
  return run_vm(vm, vcpu, 4);
}

extern const unsigned char guest64[], guest64_end[];

static void setup_64bit_code_segment(struct kvm_sregs *sregs) {
  struct kvm_segment seg = {
      .base = 0,
      .limit = 0xffffffff,
      .selector = 1 << 3,
      .present = 1,
      .type = 11, /* Code: execute, read, accessed */
      .dpl = 0,
      .db = 0,
      .s = 1, /* Code/data */
      .l = 1,
      .g = 1, /* 4KB granularity */
  };

  sregs->cs = seg;

  seg.type = 3; /* Data: read/write, accessed */
  seg.selector = 2 << 3;
  sregs->ds = sregs->es = sregs->fs = sregs->gs = sregs->ss = seg;
}

static void setup_long_mode(struct vm *vm, struct kvm_sregs *sregs) {
  // Q4.1 There are 3 levels of Page Table
  // Q4.2 The page table occupies three 4Kb pages.
  // Q3.1 Page tables are setup here
  uint64_t pml4_addr = 0x2000;
  uint64_t *pml4 = (void *)(vm->mem + pml4_addr);

  uint64_t pdpt_addr = 0x3000;
  uint64_t *pdpt = (void *)(vm->mem + pdpt_addr);

  uint64_t pd_addr = 0x4000;
  uint64_t *pd = (void *)(vm->mem + pd_addr);

  printf("Q3.2 - Page Tables are setup as follows:\n");
  printf("--------------------------------------------------------\n");
  printf("|  Type\t|\tHPA(Start)\t|\tHPA(End)       |\n");
  printf("--------------------------------------------------------\n");
  printf("|  pml4\t|\t%p\t|\t%p |\n", pml4, pml4 + 0xFFF);
  printf("|  pdpt\t|\t%p\t|\t%p |\n", pdpt, pdpt + 0xFFF);
  printf("|  pd\t|\t%p\t|\t%p |\n", pd, pd + 0xFFF);
  printf("--------------------------------------------------------\n\n");

  printf("----------------------------------------\n");
  printf("|  Type\t|GPA(Start)\t|GPA(End)      |\n");
  printf("----------------------------------------\n");
  printf("|  pml4\t|\t0x%lx\t|\t0x%lx |\n", pml4_addr, pml4_addr + 0xFFF);
  printf("|  pdpt\t|\t0x%lx\t|\t0x%lx |\n", pdpt_addr, pdpt_addr + 0xFFF);
  printf("|  pd\t|\t0x%lx\t|\t0x%lx |\n", pd_addr, pd_addr + 0xFFF);
  printf("----------------------------------------\n");

  // Q4.4 The page table maps the first 2MB of the guest virtual address,
  // since there is only one page of 2MB size.

  // Q4.3 Guest virtual to physical page table mappings are here.
  pml4[0] = PDE64_PRESENT | PDE64_RW | PDE64_USER | pdpt_addr;
  pdpt[0] = PDE64_PRESENT | PDE64_RW | PDE64_USER | pd_addr;
  pd[0] = PDE64_PRESENT | PDE64_RW | PDE64_USER | PDE64_PS;

  sregs->cr3 = pml4_addr;
  sregs->cr4 = CR4_PAE;
  sregs->cr0 = CR0_PE | CR0_MP | CR0_ET | CR0_NE | CR0_WP | CR0_AM | CR0_PG;
  sregs->efer = EFER_LME | EFER_LMA;

  setup_64bit_code_segment(sregs);
}

int run_long_mode(struct vm *vm, struct vcpu *vcpu) {
  struct kvm_sregs sregs;
  struct kvm_regs regs;

  printf("Testing 64-bit mode\n");

  if (ioctl(vcpu->fd, KVM_GET_SREGS, &sregs) < 0) {
    perror("KVM_GET_SREGS");
    exit(1);
  }

  setup_long_mode(vm, &sregs);

  if (ioctl(vcpu->fd, KVM_SET_SREGS, &sregs) < 0) {
    perror("KVM_SET_SREGS");
    exit(1);
  }

  memset(&regs, 0, sizeof(regs));
  /* Clear all FLAGS bits, except bit 1 which is always set. */
  regs.rflags = 2;
  regs.rip = 0;  // Q5 The execution starts at 0 and is configured in rip.
  /* Create stack at top of 2 MB page and grow down. */
  regs.rsp = 2 << 20;  // Q3.1 Kernel stack is setup here
  printf(
      "Q3.2 - Kernel stack starts at 0x%llx i.e. 2MB and grows in the "
      "direction of 0x0.\n",
      regs.rsp);

  if (ioctl(vcpu->fd, KVM_SET_REGS, &regs) < 0) {
    perror("KVM_SET_REGS");
    exit(1);
  }
  // Q3.1 Guest code is setup here
  memcpy(vm->mem, guest64, guest64_end - guest64);
  // Q3.2 Guest range
  printf("Q3.2 - Guest range: GPA is 0x%llx to 0x%llx\n", sregs.cs.base,
         sregs.cs.base + (guest64_end - guest64));
  printf("Q3.2 - Guest range: HPA is %p to %p\n", vm->mem,
         vm->mem + (guest64_end - guest64));
  return run_vm(vm, vcpu, 8);
}

int main(int argc, char **argv) {
  struct vm vm;
  struct vcpu vcpu;
  enum {
    REAL_MODE,
    PROTECTED_MODE,
    PAGED_32BIT_MODE,
    LONG_MODE,
  } mode = REAL_MODE;
  int opt;

  while ((opt = getopt(argc, argv, "rspl")) != -1) {
    switch (opt) {
      case 'r':
        mode = REAL_MODE;
        break;

      case 's':
        mode = PROTECTED_MODE;
        break;

      case 'p':
        mode = PAGED_32BIT_MODE;
        break;

      case 'l':
        mode = LONG_MODE;
        break;

      default:
        fprintf(stderr, "Usage: %s [ -r | -s | -p | -l ]\n", argv[0]);
        return 1;
    }
  }
  // Q1.1 Memory of 2MB is allocated
  vm_init(&vm, 0x200000);
  vcpu_init(&vm, &vcpu);

  switch (mode) {
    case REAL_MODE:
      return !run_real_mode(&vm, &vcpu);

    case PROTECTED_MODE:
      return !run_protected_mode(&vm, &vcpu);

    case PAGED_32BIT_MODE:
      return !run_paged_32bit_mode(&vm, &vcpu);

    case LONG_MODE:
      return !run_long_mode(&vm, &vcpu);
  }
  return 1;
}
