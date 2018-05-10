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

job_t *job_list_head = NULL;

typedef struct thread{
	pthread_t tid;

} thread_t; 

typedef struct threadpool {
	int thread_num_total;
	int thread_num_idle;
	job_t *job;
} threadpool_t;

threadpool_t *tp = NULL;

void insert_job(job_t *job)
{
	job_t *p_job_list = tp->job;
	if(p_job_list == NULL)
		p_job_list = job;	
	else {
		while(p_job_list->next)
			p_job_list->next = p_job_list->next->next;
		p_job_list->next = job;
		p_job_list->next->next = NULL;
	}

}

void threadpool_addjob(job_t *job)
{
//	insert_job(job);
	if(tp->job == NULL) {
	//	tp->job = malloc(sizeof(job_t));
		tp->job = job;
	} else {
		threadpool_t *tmp;
		while(tmp->job) {
			tmp->job = tmp->job->next;	
		}	
		tmp->job = malloc(sizeof(job_t));		
		assert(tmp->job);	
		tmp->job->next = NULL; 
	}
}

static void *thread_func(void *arg)
{
	DEBUG("Create thread %d\n", (int)pthread_self());
	(void*)arg;
	for(;;) {
		
		if(tp->job) {
			job_t *jobhandle = tp->job;	
			tp->job = tp->job->next;
			printf("%s\n thread_id: %d", (char*)(jobhandle->arg), (int)pthread_self());
			jobhandle->handler(jobhandle->arg);
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
	tp->job = job_list_head;

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
	sleep(2);
	job_t job[10];
	int i;
	for(i=0; i<10; i++) {
		job[i].handler = ( void*(*)(void*) )printf;
		char buf[128];	
		sprintf(buf, "job %d handle\n", i); 	
		job[i].arg = buf;
		threadpool_addjob(&job[i]);
	}
	return 0;
}
