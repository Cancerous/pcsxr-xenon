#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

int stat(const char * __restrict  path, struct stat * __restrict  buf){
	int fd = -1;
	fd = open(path, O_RDONLY);
	
	if(fd){
		return fstat(fd, buf);
	}
	return ENOENT;// file doesn't exist
}