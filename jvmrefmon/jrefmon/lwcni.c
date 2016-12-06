#include "LWCNI.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include <pthread.h>
#include <sys/mman.h>

#ifndef LINUX_BUILD
#include <sys/capsicum.h>
#endif

#include <lwc.h>

#define NUM_SWITCH_ARGS 10

void do_lwc_syscall(int src_switch, register_t *src_arg, int *ret_value,
		int *ret_err)
{
	errno = 0;
	*ret_value = lwcsyscall(src_switch, LWCR_MEMORY | LWCR_FILES, src_arg[0],
			&src_arg[1]);
	*ret_err = errno;
}

void rm_loop(int appLwc)
{
	int src_switch;
	register_t src_arg[NUM_SWITCH_ARGS];
	int src_num;
	int ret_switch;
	register_t ret_args[NUM_SWITCH_ARGS];
	int ret_num, ret_err, ret_value;

	long thr_id = 0;

#ifndef LINUX_BUILD
	thr_self(&thr_id);
#endif

	src_switch = ret_switch = 0;
	src_num = ret_num = NUM_SWITCH_ARGS;
	memset(src_arg, 0, sizeof(void*) * NUM_SWITCH_ARGS);
	memset(ret_args, 0, sizeof(void*) * NUM_SWITCH_ARGS);

	if (lwcswitch(appLwc, NULL, 0, &src_switch, &src_arg[0],
			&src_num) == LWC_FAILED)
	{
		perror("LWC suspend in parent failed");
		exit(1);
	}

	while (1)
	{

		if (src_switch < 1)
		{
			printf("switch failed src_swtich %d\n", src_switch);
			perror("LWC failed in switch");
			exit(2);
		}

		int syscall_num = (int) ((long long*) src_arg)[0];
		void *arg1 = (void *) ((long long*) src_arg)[1];
		void *arg2 = (void *) ((long long*) src_arg)[2];
		void *arg3 = (void *) ((long long*) src_arg)[3];
		void *arg4 = (void *) ((long long*) src_arg)[4];

		printf("(%d-%d) syscall %d (arg1 %p) (arg2 %p) (arg3 %p) (arg4 %p)\n",
				(int) getpid(), (int) thr_id, syscall_num, arg1, arg2, arg3,
				arg4);

		do_lwc_syscall(src_switch, src_arg, &ret_value, &ret_err);

		printf("(%d-%d) return (%d), err (%d)\n", (int) getpid(), (int) thr_id,
				ret_value, ret_err);

		ret_args[0] = ret_err;
		ret_args[1] = (int) ret_value;

		ret_switch = src_switch;
		ret_num = 2;

		memset(src_arg, 0, sizeof(void*) * NUM_SWITCH_ARGS);
		src_num = NUM_SWITCH_ARGS - 1;
		if (lwcswitch(ret_switch, ret_args, ret_num, &src_switch, src_arg,
				&src_num) == LWC_FAILED)
		{
			perror("LWC failed in suspend");
			exit(2);
		}
	}
}

int refmon_fd = 0;
int app_fd = 0;
int inside_refmon = 0;
int num_tds = 1;
int confined = 0;
//int tds_done = 0;

pthread_mutex_t mutex_m;
pthread_mutex_t *__m;

//pthread_mutex_t *__m = NULL;
//pthread_cond_t *__c = NULL;

int register_number_threads(int num_threads)
{
	num_tds = num_threads;

	__m = &mutex_m;
	pthread_mutex_init(__m, NULL);

//	__r = &mutex_r;
//	pthread_mutex_init(__r, NULL);
//	//allocate the shared memory
//	int size_shared_mem = sizeof(pthread_mutex_t) + sizeof(pthread_cond_t);
//
//	void *shm = mmap(NULL, size_shared_mem, PROT_READ | PROT_WRITE,
//	MAP_ANON | MAP_SHARED, -1, 0);
//	assert(shm != (void * ) -1);
//
//	pthread_mutexattr_t __m_attr;
//	pthread_mutexattr_init(&__m_attr);
//	pthread_mutexattr_setpshared(&__m_attr, PTHREAD_PROCESS_SHARED);
//	__m = shm;
//	pthread_mutex_init(__m, &__m_attr);
//
//	pthread_condattr_t __c_attr;
//	pthread_condattr_init(&__c_attr);
//	pthread_condattr_setpshared(&__c_attr, PTHREAD_PROCESS_SHARED);
//	__c = ((char *) shm + sizeof(pthread_mutex_t));
//	pthread_cond_init(__c, &__c_attr);

	return 0;
}

int create_ref_mon()
{
	int ret = 0;
	int creates_rm;

	long thr_id = 0;

#ifndef LINUX_BUILD
	thr_self(&thr_id);
#endif

	printf("(%d-%d) confining thread\n", (int) getpid(), (int) thr_id);
	pthread_mutex_lock(__m);
	confined++;
	creates_rm = (confined == num_tds) ? 1 : 0;
	pthread_mutex_unlock(__m);

	/* only one thread wakes up and notifies the rest
	 * after refmon context creation */
	if (!creates_rm)
	{
		assert(inside_refmon == 0);

		while (1)
		{
			// break if refmon context has been initialized
			pthread_mutex_lock(__m);
			if (refmon_fd > 0 || inside_refmon)
			{
				pthread_mutex_unlock(__m);
				break;
			}
			pthread_mutex_unlock(__m);

			//L: wakey wakey
			usleep(10);
		}

		if (!inside_refmon)
		{
			assert(refmon_fd > 0);

			// one at a time
			pthread_mutex_lock(__m);

			// to be executed only once per thread
			printf("(%d-%d) switching to refmon_fd %d\n", (int) getpid(),
					(int) thr_id, refmon_fd);

			// wake up at line L: wakey wakey with inside_refmon set to 1
			if (lwcswitch(refmon_fd, NULL, 0, NULL, NULL, NULL) == LWC_FAILED)
			{
				perror("LWC suspend in parent failed");
				exit(1);
			}

			printf("(%d-%d) return from switch/register\n", (int) getpid(),
					(int) thr_id);

			pthread_mutex_unlock(__m);

		}
		else
		{
			/* here is refmon */
			rm_loop(app_fd);
		}
	}
	else
	{
		/* create refmon context */
		struct lwc_resource_specifier specs[10];
		specs[0].flags = LWC_RESOURCE_FILES | LWC_RESOURCE_SHARE;
		specs[0].sub.descriptors.from = specs[0].sub.descriptors.to = -1;

		pthread_mutex_lock(__m);
		ret = Lwccreate(specs, 1, &app_fd, NULL, 0, LWC_TRAP_SYSCALL);
		pthread_mutex_unlock(__m);

		if (ret < 0)
		{
			/* here is refmon */
			printf("(%d-%d) starting refmon, app fd is %d\n", (int) getpid(),
					(int) thr_id, app_fd);

			pthread_mutex_lock(__m);
			inside_refmon = 1;
			pthread_mutex_unlock(__m);

			rm_loop(app_fd);
		}

		/* here is app */
		pthread_mutex_lock(__m);
		assert(refmon_fd == 0);

		// Sandbox
		printf("Sandbox app\n");
#ifndef LINUX_BUILD
		if (cap_enter() != 0)
		{
			perror("Cap enter");
		}
#endif

		refmon_fd = ret;

		printf("(%d-%d) switching to refmon_fd %d\n", (int) getpid(),
				(int) thr_id, refmon_fd);

		if (lwcswitch(refmon_fd, NULL, 0, NULL, NULL, NULL) == LWC_FAILED)
		{
			perror("LWC suspend in parent failed");
			exit(1);
		}

		printf("(%d-%d) return from switch\n", (int) getpid(), (int) thr_id);
		pthread_mutex_unlock(__m);
	}

	return refmon_fd;
}

int register_cur_thread()
{

	if (lwcswitch(refmon_fd, NULL, 0, NULL, NULL,
	NULL) == LWC_FAILED)
	{
		perror("LWC suspend in parent failed");
		exit(1);
	}

	return 0;
}

JNIEXPORT void JNICALL Java_LWCNI_lwCRegister
(JNIEnv *env, jclass c, jint num_threads)
{
	register_number_threads (num_threads);
}

JNIEXPORT jint JNICALL Java_LWCNI_lwCConfine(JNIEnv *env, jclass c)
{
	if (num_tds == 1 && __m == NULL)
	{
		register_number_threads(1);
	}

	return create_ref_mon();
}

JNIEXPORT void JNICALL Java_LWCNI_lwCCleanup
(JNIEnv *env, jclass c, jint rmfd)
{
	printf("lwcCleanup\n");
	/* cleanup the refmon fd */
	// worry about this later
}
