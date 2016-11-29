#define _GNU_SOURCE

#include <stdio.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdarg.h>

#include "IOTest.h"

__attribute__((constructor)) void
loader_init ()
{
	Java_IOTest_lwCConfine (NULL, NULL);
}
