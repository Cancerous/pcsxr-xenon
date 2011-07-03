#define PROT_WRITE 0
#define PROT_READ 0
#define MAP_PRIVATE 0
#define MAP_ANONYMOUS0


#define mmap(start, length, prot, flags, fd, offset) \
	((unsigned char *)malloc(length)

#define munmap(start, length) do { free(start); } while (0)