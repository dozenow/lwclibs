#include "IOTest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <sys/capsicum.h>

#include <lwc.h>

#define NUM_SWITCH_ARGS 10

void
do_lwc_syscall (int src_switch, register_t *src_arg, int *ret_value,
		int *ret_err)
{
	errno = 0;
	*ret_value = lwcsyscall (src_switch, LWCR_MEMORY | LWCR_FILES, src_arg[0],
			   &src_arg[1]);
	*ret_err = errno;
}

void
rm_loop (int appLwc)
{
	int src_switch;
	register_t src_arg[NUM_SWITCH_ARGS];
	int src_num;
	int ret_switch;
	register_t ret_args[NUM_SWITCH_ARGS];
	int ret_num, ret_err, ret_value;

	src_switch = ret_switch = 0;
	src_num = ret_num = NUM_SWITCH_ARGS;
	memset (src_arg, 0, sizeof(void*) * NUM_SWITCH_ARGS);
	memset (ret_args, 0, sizeof(void*) * NUM_SWITCH_ARGS);

	if (lwcswitch (appLwc, NULL, 0, &src_switch, &src_arg[0],
			&src_num) == LWC_FAILED)
	{
		perror ("LWC suspend in parent failed");
		exit (1);
	}

	while (1)
	{

		if (src_switch < 1)
		{
			printf ("switch failed src_swtich %d\n", src_switch);
			perror ("LWC failed in switch");
			exit (2);
		}

		int syscall_num = (int) ((long long*) src_arg)[0];
		void *arg1 = (void *) ((long long*) src_arg)[1];
		void *arg2 = (void *) ((long long*) src_arg)[2];
		void *arg3 = (void *) ((long long*) src_arg)[3];
		void *arg4 = (void *) ((long long*) src_arg)[4];

		printf ("syscall %d (arg1 %p) (arg2 %p) (arg3 %p) (arg4 %p)\n", 
			syscall_num, arg1, arg2, arg3, arg4);

		do_lwc_syscall (src_switch, src_arg, &ret_value, &ret_err);

		printf("return (%d), err (%d)\n", ret_value, ret_err);

		ret_args[0] = ret_err;
		ret_args[1] = (int) ret_value;

		ret_switch = src_switch;
		ret_num = 2;

		memset (src_arg, 0, sizeof(void*) * NUM_SWITCH_ARGS);
		src_num = NUM_SWITCH_ARGS - 1;
		if (lwcswitch (ret_switch, ret_args, ret_num, &src_switch, src_arg,
			    &src_num) == LWC_FAILED)
		{
			perror ("LWC failed in suspend");
			exit (2);
		}
	}
}

int refmon_fd = 0;

int
create_ref_mon ()
{
	int ret = 0;
	int src;

	struct lwc_resource_specifier specs[10];

	specs[0].flags = LWC_RESOURCE_FILES | LWC_RESOURCE_SHARE;
	specs[0].sub.descriptors.from = specs[0].sub.descriptors.to = -1;

	/* create refmon context */
	ret = Lwccreate (specs, 1, &src, NULL, 0, LWC_TRAP_SYSCALL);

	if (ret < 0)
	{
		/* here is refmon */
		printf("Starting refmon, app fd is %d\n", src);
		rm_loop (src);
	}

	/* here is app */

	// this code is assumed to execute only once
	assert (refmon_fd == 0);

	refmon_fd = ret;

	// Sandbox
	printf("Sandbox app\n");
	if (cap_enter () != 0)
	{
		perror ("Cap enter");
	}

	return 0;
}

int
register_cur_thread ()
{

        if (lwcswitch (refmon_fd, NULL, 0, NULL, NULL,
                        NULL) == LWC_FAILED)
        {
                perror ("LWC suspend in parent failed");
                exit (1);
        }

	return 0;
}

JNIEXPORT void JNICALL Java_IOTest_lwCConfine
  (JNIEnv *env, jclass c)
{
	create_ref_mon ();
}

JNIEXPORT void JNICALL Java_IOTest_lwCRegister
  (JNIEnv *env, jclass c)
{
        register_cur_thread ();
}

JNIEXPORT void JNICALL Java_IOTest_lwCCleanup
  (JNIEnv *env, jclass c)
{
	printf("lwcCleanup\n");
	/* cleanup the refmon fd */
	// worry about this later
}

