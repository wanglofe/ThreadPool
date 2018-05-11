#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>

#define DEBUG(fmt, arg...) printf(fmt, ##arg)

typedef struct job{
	void *arg;
	void *(*handler)(void *);
	struct job *next;
} job_t;

typedef struct thread{
	pthread_t tid;

} thread_t; 

typedef struct threadpool {
	int thread_num_total;
	int thread_num_idle;
	job_t *job_header;
} threadpool_t;


threadpool_t *tp = NULL;
static void *thread_func(void *arg);


pthread_mutex_t mutex;


static void print_jobarg(job_t *job)
{
	job_t *tmp = job;
	while(tmp) {
		printf("job arg: %s\n", (char*)tmp->arg);
		tmp = tmp->next;
	}
}

void threadpool_addjob(job_t *job)
{
	DEBUG("Add job: %s\n", (char*)job->arg);
	job_t *newjob = tp->job_header;
	newjob->arg = job->arg;
	newjob->handler = job->handler;
	newjob->next = NULL;

	pthread_mutex_lock(&mutex);
	if(tp->job_header == NULL) {
		printf("111111111\n");
		tp->job_header = newjob;
	} else {
		printf("9999999999\n");
		job_t *tmp = tp->job_header;
		while(tmp->next) {
			tmp->next = tmp->next->next;	
		}	
		tmp->next = newjob;
	}
	pthread_mutex_unlock(&mutex);
}

static void *thread_func(void *arg)
{
	DEBUG("Create thread #%x\n", (int)pthread_self());
	(void*)arg;
	for(;;) {
		
		if(tp->job_header) {
			pthread_mutex_lock(&mutex);
			job_t *jobhandle = tp->job_header;	
			tp->job_header = tp->job_header->next;
			pthread_mutex_unlock(&mutex);
			jobhandle->handler(jobhandle->arg);
			free(jobhandle);
		}
	}
	return NULL;
}

void threadpool_create(int thread_num)
{
	tp = malloc(sizeof(threadpool_t));
	assert(tp);
	tp->thread_num_total = thread_num;
	tp->thread_num_idle = thread_num;
	tp->job_header = NULL;
	
	pthread_mutex_init(&mutex, NULL);

	int i;
	pthread_t tid;
	for(i=0; i<tp->thread_num_total; i++)	
		pthread_create(&tid, NULL, thread_func, NULL);
}

void threadpool_destory(void)
{

}

int main(void)
{
	threadpool_create(5);	
	job_t job[10];
	int i;
	for(i=0; i<8; i++) {
		job[i].handler = ( void*(*)(void*) )printf;
		char buf[128];	
		sprintf(buf, "job %d handle\n", i); 	
		job[i].arg = buf;
		threadpool_addjob(&job[i]);
	}

	printf("----------------\n");
	print_jobarg(tp->job_header);

	sleep(3);
	return 0;
}
