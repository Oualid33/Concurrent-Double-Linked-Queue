#include <time.h> // for random numbers generation
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h> // POSIX Threads
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_THREAD 100000


/* data type definitions */
typedef struct node {
	int value; 
	struct node* prev;
	struct node* next;
} node;

typedef struct dq {
	node* head;
	node* tail;
} dq;

dq list; // global shared dynamic data structure

// associated mutexes
pthread_mutex_t mutex_head; // for locking head
pthread_mutex_t mutex_tail; // for locking tail

// help routines
void print_queue1() { // from tail
	node* ptr = list.tail;
	pthread_mutex_lock(&mutex_head);
	printf("[");
	while(ptr) {
		printf("%d,", ptr->value);
		ptr = ptr->prev;
	}
	printf("]\n");
	pthread_mutex_unlock(&mutex_head);
}

void print_queue2() { // from head
	node* ptr = list.head;
	pthread_mutex_lock(&mutex_tail);
	printf("[");
	while(ptr) {
		printf("%d,", ptr->value);
		ptr = ptr->next;
	}
	printf("]\n");
	pthread_mutex_unlock(&mutex_tail);
}

void print_queue() { // from head
	node* ptr = list.head;
	printf("[");
	while(ptr) {
		printf("%d,", ptr->value);
		ptr = ptr->next;
	}
	printf("]\n");
}

// thread routines
void* enqueue(void* ptr) { // 'ptr' points to 'value' to be enqueued
			   // STRATEGY:  first, create node with all necessary data
			   //            second, double-link node to existing list
	//sleep((float)rand()/RAND_MAX);
	node* new = (node*)malloc(sizeof(node));
	node* tmp = NULL;
	if (new) { // valid pointer
		new->value = (*(int*)ptr);
		new->next = NULL;

		//gettimeofday(, NULL); // start

		// access and update shared pointers: 'tail' and 'tail->next'
		pthread_mutex_lock(&mutex_tail);
		tmp = new->prev = list.tail; // read (tail)
		list.tail = new; // write (tail)
		if (tmp) {
			tmp->next = new; // write (tail->next)
		} else { // queue is empty: special case 2.1
			// access shared pointer 'head'
			// possible deadlock if a dequeuer has 
			// already acquired mutex_head
			pthread_mutex_lock(&mutex_head); 
			list.head = new; // write (head)
			assert(list.head == list.tail); // panic! if this assertion is violated
			pthread_mutex_unlock(&mutex_head);
		}

		printf("Enqueue: %d\n", (*(int*)ptr)); // this is not part of code; just for illust.
		//print_queue1();
		pthread_mutex_unlock(&mutex_tail);

		//gettimeofday(, NULL); // end
		// compute execution time of the thread
	} else 	printf("Error Enqueue: malloc()\n"); // print error
	
	return NULL;
}

void* dequeue() { // 'ptr' points to 'value' to be dequeued
		  // SAME STRATEGY as enqueue
	//sleep(((float)rand()/RAND_MAX));
	node *tmp = NULL;
	int v = -1; 
	// access shared pointers: 'head' and 'head->prev'
	pthread_mutex_lock(&mutex_head);
	tmp = list.head; // read (head)
	if (tmp) { // queue is not empty
		v = tmp->value; // read (head)
		tmp = tmp->next; // read (head->next)
		free(list.head); // read (head)
		list.head = tmp; // write (head)
		if (!tmp) { // queue was with one node: special case 2.2
			    // access to shared pointer 'tail'
			    // possible deadlock if an enqueuer has 
			    // already acquired mutex_tail
			pthread_mutex_lock(&mutex_tail);
			list.tail = NULL; // write (tail)
			assert(list.head == list.tail); // panic! if this assertion is violated
			pthread_mutex_unlock(&mutex_tail);
		} else tmp->prev = NULL; // write (head->prev)
	} else {
		printf("Error Dequeue: empty list\n"); // print error
	}

	printf("Dequeue: %d\n", v);
	//print_queue2();
	pthread_mutex_unlock(&mutex_head);

	return NULL;
}

int main() {
	int i, v[MAX_THREAD];
	//float exec_time[MAX_THREAD];
	// threads
	pthread_t enq[MAX_THREAD], deq[MAX_THREAD];
	
	// initialize mutex and condition variables
	pthread_mutex_init(&mutex_head, NULL);
	pthread_mutex_init(&mutex_tail, NULL);

	// init values
	for(i = 0; i < MAX_THREAD; i++) {
		v[i] = i;
		//exec_time[i] = 0;
	}

	//srand(time(NULL));

	// Assumption: NB_DEQ = NB_ENQ
	for(i = 0; i < MAX_THREAD; i++) {
		pthread_create(&enq[i], NULL, enqueue, &v[i]);
		pthread_create(&deq[i], NULL, dequeue, NULL);
	}

	// waiting for all threads to terminate
	for(i = 0; i < MAX_THREAD; i++) {
		pthread_join(enq[i], 0);
		pthread_join(deq[i], 0);
	}

	print_queue();

	return 0;
}

