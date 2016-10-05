#include <libgen.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pwd.h>
#include <grp.h>
#include <fnmatch.h>
#include <ctype.h>
#include <spawn.h>

#ifndef HOST_NAME_MAX
	#define HOST_NAME_MAX 256
#endif
#define MAX_ALLOC_SIZE (16 * 1024 * 1024)




//Malloc Allocation Handler
static void *set_malloc(size_t size) {
	void* ptr;

	assert(size < MAX_ALLOC_SIZE);

	ptr = malloc(size);
	if (ptr == NULL) {
		static const char* OOM_MSG = "Out of memory\n";
		if ( write(STDOUT_FILENO, OOM_MSG, sizeof(OOM_MSG)-1) < 0) {
		/* Do nothing on write failure, we are torched anyway */
		}
		abort();
	}
	return ptr;
}

char* getHostName () {
	char *host = set_malloc(HOST_NAME_MAX + 1);
	gethostname(host, HOST_NAME_MAX);
	return &host;
}

int main() {
	char host = getHostName();
	
	printf("%s\n", host);

	return 0;
}