#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <semaphore.h>
#include <time.h>
#include <math.h>

#define THINKING 0
#define HUNGRY 1
#define EATING 2

#define MAXSIZE 1000000

int numphil;

#define RIGHT (no + 1) % numphil
#define LEFT  (no + numphil - 1) % numphil

#define MAXPHILNO 27
double times[MAXPHILNO][MAXSIZE];

int minthink;
int maxthink;
int mineat;
int maxeat;
char* dist;
int count;

pthread_cond_t cond[MAXPHILNO];
//pthread_cond_t *cond;

pthread_mutex_t mutex;

int state[MAXPHILNO];
//int *state;

int num_eat[MAXPHILNO];
//int *num_eat;

double hungry_time[MAXPHILNO];

pthread_mutex_t out;

void initialization() {
	int i;
	//printf("initialization\n");
	pthread_mutex_init(&mutex, NULL);
	for(i = 0; i < numphil; i++) {
		pthread_cond_init(&cond[i], NULL);
		state[i] = THINKING;
		printf("philosopher %d thinking\n", i+1);
	}	
}

void test(int no) {
	//printf("testing phil %d\n", no);
	if (state[no] == HUNGRY && 
	state[LEFT] != EATING &&
	state[RIGHT] != EATING) {
		state[no] = EATING;
		pthread_cond_signal(&cond[no]);
	}
}

void pickup(int no) {
	pthread_mutex_lock(&mutex);
	state[no] = HUNGRY;
	//clock_t begin = clock();
	struct timeval begin, end;
	gettimeofday(&begin, NULL);
	printf("philosopher %d hungry\n", no+1);
	test(no);
	if (state[no] == HUNGRY)
		pthread_cond_wait(&cond[no], &mutex);
	
	//sleep(1);	
	//clock_t end = clock();
	//hungry_time[no] = hungry_time[no] + (double)(end-begin)*1000.0/CLOCKS_PER_SEC;
	gettimeofday(&end, NULL);
	
	double time = (end.tv_sec - begin.tv_sec)*1000.0
			+ (double)((end.tv_usec - begin.tv_usec)/1000.0);
	times[no][num_eat[no]] = time;
	hungry_time[no] = hungry_time[no] + time;
	//printf("philosopher %d waits as hungry for %f ms\n", no+1, hungry_time[no]);
	printf("philosopher %d eating\n", no+1);
	num_eat[no]++;
	pthread_mutex_unlock(&mutex);
}

void putdown(int no) {
	pthread_mutex_lock(&mutex);
	state[no] = THINKING;
	test(LEFT);
	test(RIGHT);
	printf("philosopher %d thinking\n", no+1);
	pthread_mutex_unlock(&mutex);
}

void trace(int i, char *s) {
	pthread_mutex_lock(&out);
	if(num_eat[i] >= count) {
		if(strcmp(s, "thinking") == 0) {
			pthread_mutex_unlock(&out);
			//printf("\nphilospher %d is done\n\n", i);
			pthread_exit(0);
		}
	}
	pthread_mutex_unlock(&out);
}

void * life(void *arg) {
	int self = *(int * ) arg;
	
	for( ; ; ) {
		if(strcmp(dist, "uniform") == 0) {
			int j, k;
			j = rand()%(maxthink - minthink + 1) + minthink;
			j = j*1000;
			trace(self, "thinking");
			//printf("phil %d is thinking for %d milisecs\n", self+1, j/1000);
			usleep(j);
			pickup(self);
		
			k = rand()%(maxeat - mineat + 1) + mineat;;
			k = k*1000;
			trace(self, "eating");
			//printf("phil %d is eating for %d milisecs\n", self+1, k/1000);
			usleep(k);
			putdown(self);
		}
		
		if(strcmp(dist, "exponential") == 0) {
			double meanthink = (maxthink + minthink)/2;
			double j, k;
			int thinktime, eattime;
			do {
				j = rand()/(1.0 + RAND_MAX);
				j = log(1-j)/(-1/meanthink);
			} while((minthink > (int)j) || (maxthink < (int)j) );
			
			thinktime = (int)j;
			thinktime = thinktime*1000;
			trace(self, "thinking");
			//printf("phil %d is thinking for %d milisecs\n", self+1, thinktime/1000);
			usleep(thinktime);
			pickup(self);
			
			double meaneat = (maxeat + mineat)/2;
			do {
				k = rand()/(1.0 + RAND_MAX);
				k = log(1-k)/(-1/meaneat);
			} while((mineat > (int)k) || (maxeat < (int)k) );
			
			eattime = (int)k;
			eattime = eattime*1000;
			trace(self, "eating");
			//printf("phil %d is eating for %d milisecs\n", self+1, eattime/1000);
			usleep(eattime);
			putdown(self);	
		}
	}
}

double standartDeviation(double data[], int count) {
	double sum = 0.0, mean, std = 0.0;
	int i;
	for (i = 0; i < count; i++ ) {
		sum += data[i];
	}
	
	mean = sum/count;
	
	for(i=0; i<count; i++)
		std += pow(data[i] - mean, 2);
		
	return sqrt(std/count);
}

int main(int argc, char* argv[]) {
	//printf("main is started\n");
	
	if(argc != 8) {
		printf("No of Arguments must be 8\n");
		return -1;
	}
	
	if(atoi(argv[1]) > 27 || atoi(argv[1]) < 1) {
		printf("No of phils must be > 0 and <= 27\n");
		return -1;
	}
	
	if(atoi(argv[2]) > 60000 || atoi(argv[2]) < 1) {
		printf("Min thinking time must be >= 1 and <= 60000 ms\n");
		return -1;
	}
	
	if(atoi(argv[3]) > 60000 || atoi(argv[3]) < 1) {
		printf("Max thinking time must be >= 1 and <= 60000 ms\n");
		return -1;
	}
	
	if(atoi(argv[4]) > 60000 || atoi(argv[4]) < 1) {
		printf("Min eating time must be >= 1 and <= 60000 ms\n");
		return -1;
	}
	
	if(atoi(argv[5]) > 60000 || atoi(argv[5]) < 1) {
		printf("Max eating time must be >= 1 and <= 60000 ms\n");
		return -1;
	}
	
	srand(time(NULL));
	
	numphil = atoi(argv[1]);
	minthink = atoi(argv[2]);
	maxthink = atoi(argv[3]);
	mineat = atoi(argv[4]);
	maxeat = atoi(argv[5]);
	dist = argv[6];
	count = atoi(argv[7]);
	
	if( (strcmp(dist, "exponential") != 0) && (strcmp(dist, "uniform") != 0) ) {
		printf("Dist must be exponential or uniform\n");
		return -1;
	}
		
	int i;
	pthread_t phils[numphil];
	int ids[numphil];
	
	pthread_attr_t attr;
	
	for( i = 0; i < numphil; i++) {
		num_eat[i] = 0;
		hungry_time[i] = 0.0;	
	}
		
	pthread_mutex_init(&out, NULL);	
	
	initialization();
	
	pthread_attr_init(&attr);
	
	for(i = 0; i < numphil; i++) {
		ids[i] = i;
		//printf(" thread creation\n");
		pthread_create(&phils[i], NULL, life, (int*) &ids[i]);
		//printf(" thread %d created\n", i);
	}
	
	for(i = 0; i < numphil; i++) {
		pthread_join(phils[i], NULL);
	}
	
	/*
	for(i = 0; i < numphil; i++) {
		printf("philospher %d eats %d times\n", i+1, num_eat[i]);
	}
	*/
	printf("\n");
	for(i = 0; i < numphil; i++) {
		printf("philospher %d duration of hungry state = %f\n", i+1, hungry_time[i]);
	}
	
	printf("\n");
	for(i = 0; i < numphil; i++) {
		printf("philospher %d avg duration of hungry state = %f\n", i+1, hungry_time[i]/count);
	}
	
	printf("\n");
	for(i = 0; i < numphil; i++) {
		printf("philospher %d std deviation of hungry duration = %f\n", i+1, standartDeviation(times[i], count));
	}
	
	double avg_hung_time = 0.0;
	for(i = 0; i < numphil; i++) {
		avg_hung_time = avg_hung_time +  hungry_time[i];
	}
	printf("\n");
	printf("Avg duration of hungry state for all philosophers = %f\n", avg_hung_time/numphil);
	
	printf("\n");
	printf("Std deviation of duration of hungry state for all philosophers = %f\n", standartDeviation(hungry_time, numphil));
	
	
	return 0;
}
