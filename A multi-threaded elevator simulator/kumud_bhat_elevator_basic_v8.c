#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h> /*header fiile for POSIX implementation of threads*/
#include<sys/types.h>
#include<sys/timeb.h>  /*defines timeb structure that is useful for performing a task for certain period of time*/

/*Data Structure for global information related to elevator simulation*/
struct gv{
	int num_elevators; /*Number of elevators*/
	int num_floors; /*Number of floors*/
	int arrival_time; /*Persons arrival time*/
	int elevator_speed; /*elevator speed*/
	int commence_time;/*Beginning of simulation*/
	int simulation_time;/*Total simulation time of the program*/
	int random_seed; /*pseudo random number generator initializer*/
	int num_people_started; /*statistics*/
	int num_people_finished; /*statistics*/
        int flag; /*This variable is used to keep track of whether person thread is generating persons or not*/
	pthread_mutex_t* lock;/*lock is used to ensure secured access to shared variables*/
	pthread_cond_t* cond;/*conditional variable*/
	struct personNode *person_head; /*this is used here because lock is present in this structure*/
};
struct gv *statistics;

/*Data Structure for storing information about an individual person arriving*/
struct person{
	int id; /*0,1,2,3....person identifier*/
	int from_floor; /*Persons source floor*/
	int to_floor; /*Persons destination floor*/
	double arrival_time; /*Persons arrival time*/
};

/*Data Structure for global doubly linked list*/
struct personNode{
	struct person data; /*This node contains persons information such as id, from floor, to floor etc*/
	struct personNode *left; /*left or previous pointer to a node*/
	struct personNode *right; /*right or next pointer to a node*/
};

/*Data Structure for elevator*/
struct elevator{
	int eid; /*0,1,2,3....elevator identifier*/
	int current_floor; /*Current location of the elevator*/
	pthread_mutex_t *elock;
	pthread_cond_t *econd;
	struct person *people; /*number of people in elevator*/
};

/*Function declaration*/
double calculate_passed_time(double);

/*Person generator thread routine*/
void* do_people_thread(void* ptid)
{
	int person_id, arriv_time;
	double total_simulation_time,commence_time;
	struct timeb t_present;
	struct personNode *temp;
	struct personNode *newNode;

	person_id = (*((int*)ptid))+1;
	total_simulation_time = (double) statistics->simulation_time;
	commence_time = (double) statistics->commence_time;
	arriv_time = statistics->arrival_time;

	while(calculate_passed_time(commence_time) <  total_simulation_time)
	{
		newNode = (struct personNode*) malloc(sizeof(struct personNode));
                /*Checking whether memory has been allocated properly or not*/
                if(newNode == NULL)
                {
                   printf("Error in allocating memory to new node\n");
                   exit(1);
                }


		newNode->data.id = person_id++;

		/*calculating person arrival time*/
		ftime(&t_present);
		newNode->data.arrival_time = (double) ((t_present.time) + (t_present.millitm/1000.0));

		/* comparing and ensuring source floor and destination floor are not same*/
		while(1)
		{
			newNode->data.from_floor = (rand()%statistics->num_floors)+1;
			newNode->data.to_floor = (rand()%statistics->num_floors)+1;

			if(newNode->data.from_floor != newNode->data.to_floor)
				break;
		}

		newNode->left = NULL;
		newNode->right = NULL;

		/*Using lock*/
		pthread_mutex_lock(statistics->lock);

		/*Checking whether the global doubly linked list is empty or not*/
		if(statistics->person_head==NULL)
		{
			statistics->person_head = newNode;
		}
		else
		{
			temp = statistics->person_head;

			while(temp->right != NULL)
				temp = temp->right;

			temp->right = newNode;
			newNode->left = temp;


		}

		statistics->num_people_started++;
		printf("[%lf] Person %d arrives on floor %d, waiting to go for floor %d\n", calculate_passed_time(commence_time), newNode->data.id, newNode->data.from_floor, newNode->data.to_floor);

		pthread_mutex_unlock(statistics->lock);
		/*conditional broadcasting*/
		pthread_cond_broadcast(statistics->cond);

		/*Sleep for p secs before generating next person because person arrives for every p seconds*/
		sleep(arriv_time);

	}

        pthread_mutex_lock(statistics->lock);  
        statistics->flag = 0; /*indicates that time has been passed and person generation thread task is over*/
        pthread_mutex_unlock(statistics->lock);
        pthread_cond_broadcast(statistics->cond);
}

void* do_elevator_thread(void* etid)
{
	int elevator_id,elev_speed;
	double total_simulation_time,commence_time;
	struct personNode *tmp, *delNode;
	struct elevator *elvtr;

	elevator_id = (*((int*)etid))+1;
	total_simulation_time = (double) statistics->simulation_time;
	commence_time = (double) statistics->commence_time;
	elev_speed = statistics->elevator_speed;

	elvtr = (struct elevator*) malloc(sizeof(struct elevator));
        /*Checking whether memory has been allocated or not*/
        if(elvtr  == NULL)
        {
          printf("Error in allocating memory\n");
          exit(1);
        }

	elvtr->eid = elevator_id;
	elvtr->current_floor = 0;

	while(calculate_passed_time(commence_time) <  total_simulation_time)
	{
		/*Check whether global linked list is empty or not, and check whether persons are getting generated or not....if its empty then block elevator on its cond variable until a new person arrives*/
		pthread_mutex_lock(statistics->lock);
		while(statistics->person_head == NULL && calculate_passed_time(commence_time) <  total_simulation_time && statistics->flag==1)
		{	
			pthread_mutex_unlock(statistics->lock);
			pthread_cond_wait(statistics->cond, statistics->lock);
		}

		if( statistics->flag==0)
		{	
			pthread_mutex_unlock(statistics->lock);
			break; /*because people generation task is over. No need to keep on waiting indefinitely*/
		}

		/*Removing the first person from global doubly linked list*/
		tmp = statistics->person_head;
		delNode = tmp;
		tmp = tmp->right;
		delNode->right = NULL;
		if(tmp!= NULL)
			tmp->left = NULL;
		statistics->person_head = tmp;


		pthread_mutex_unlock(statistics->lock);


		/*Checking whether elevator is in the same floor as the persons from_floor or not*/
		if(elvtr->current_floor != delNode->data.from_floor)
		{
			printf("[%lf] Elevator %d starts moving from %d to %d\n", calculate_passed_time(commence_time),elvtr->eid, elvtr->current_floor, delNode->data.from_floor);
		}

		while(elvtr->current_floor != delNode->data.from_floor)
		{
			if(delNode->data.from_floor < elvtr->current_floor)
				elvtr->current_floor--; /*Elevator comes down to pick a person*/

			else
				elvtr->current_floor++; /*Elevator goes up to pick a person */
			if(calculate_passed_time(commence_time) <  total_simulation_time)
				sleep(elev_speed); /*Elevator speed from one floor to other*/
			else
				return;
		}

		/*Now elevator has arrived to the floor where person is to pick him/her up */
		printf("[%lf] Elevator %d arrives at floor %d\n", calculate_passed_time(commence_time),elvtr->eid, delNode->data.from_floor);

		printf("[%lf] Elevator %d picks up Person %d\n", calculate_passed_time(commence_time), elvtr->eid, delNode->data.id);

		while(elvtr->current_floor != delNode->data.to_floor)
		{
			if(delNode->data.to_floor < elvtr->current_floor)
				elvtr->current_floor--; /*Drop person down*/
			else
				elvtr->current_floor++; /*Drop person up*/
			if(calculate_passed_time(commence_time) <  total_simulation_time)
				sleep(elev_speed); /*Elevator speed from one floor to other*/
			else
				return;
		}
		pthread_mutex_lock(statistics->lock);
                statistics->num_people_finished++;
		pthread_mutex_unlock(statistics->lock);
		elvtr->people++;
		printf("[%lf] Elevator %d drops Person %d\n", calculate_passed_time(commence_time), elvtr->eid, delNode->data.id);
                
	}

}

double calculate_passed_time(double commence_time)
{
	struct timeb t_present;
	double commenced_time,present_time, passed_time;

	ftime(&t_present);
	present_time = (double)((t_present.time) + (t_present.millitm/1000.0));

	commenced_time = commence_time;

	passed_time = present_time - commenced_time;

	return passed_time;

}


int main(int argc, char *argv[])
{
	/*Elevator Simulation Using Pthreads*/

	/*Variables declaration*/
	int i,j,n;
	int *ptids,*etids;
	pthread_t *pthrds,*ethrds;
	pthread_attr_t *pattrs,*eattrs;
	void *pretval,*eretval;
	struct timeb t_commence;

	/*Error checking*/
	if(argc !=7)
	{
		fprintf(stderr, "Usage: main [# of elevators], [# of floors], [people arrival time], [elevator speed], [simulation time], [random function seed] \n");
		exit(1);
	}

	/*Error checking before allocating memory*/
	if(atoi(argv[1])<=0 || atoi(argv[2])<=1 || atoi(argv[3])<=0 || atoi(argv[4])<=0 || atoi(argv[5])<=0 || atoi(argv[6])<=0)
	{
		fprintf(stderr, "Usage: : all arguments must be greater than zero and number of floors must be greater than 1\n");
		exit(1);
	}

	/*Allocating memory*/
	statistics = (struct gv*) malloc(sizeof(struct gv));

        /*Checking whether memory has been allocated properly or  not */
        if(statistics == NULL)
        {
           printf("Error in memory allocation\n");
           exit(1);
        }

	/*Initializing members of struct gv*/
	statistics->num_elevators = atoi(argv[1]);
	statistics->num_floors = atoi(argv[2]);
	statistics->arrival_time = atoi(argv[3]);
	statistics->elevator_speed = atoi(argv[4]);
	statistics->simulation_time = atoi(argv[5]);
	statistics->random_seed = atoi(argv[6]);
	statistics->num_people_started = 0;
	statistics->num_people_finished = 0;
        statistics->flag = 1; /*indicator whether task of people generation thread is done or not, used later*/
	statistics->lock = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
	statistics->cond = (pthread_mutex_t*) malloc(sizeof(pthread_cond_t));
	/*Initialize the pseudo random number generator with input argumeny seed*/
	srand(statistics->random_seed);

	n = 1; /*counter variable for person thread*/

	/*Allocating memory*/
	pthrds = (pthread_t*) malloc(sizeof(pthread_t)*n);
	pattrs = (pthread_attr_t*) malloc(sizeof(pthread_attr_t)*n);
	ptids = (int *) malloc(sizeof(int)*n);

	ethrds = (pthread_t*) malloc(sizeof(pthread_t)*statistics->num_elevators);
	eattrs = (pthread_attr_t*) malloc(sizeof(pthread_attr_t)*statistics->num_elevators);
	etids = (int *) malloc(sizeof(int)*statistics->num_elevators);

        /*checking whether memory is allocated properly or not */
        if(pthrds==NULL || pattrs==NULL || ptids==NULL || ethrds==NULL || eattrs==NULL || etids==NULL)
        {
             printf("Error in allocating memory to threads\n");
             exit(1);
        }

	/*Beginning of simulation*/
	ftime(&t_commence);
	statistics->commence_time = (int) ((t_commence.time) + (t_commence.millitm/1000.0));

	/*Creating people threads*/
	for(i=0; i<n; i++)
	{
		if(pthread_attr_init(pattrs+i))
		{
			perror("attr_init()");
		}

		ptids[i] = i;

		if(pthread_create(pthrds+i, pattrs+i, do_people_thread, ptids+i) != 0)
		{
			perror("pthread_create()");
			exit(1);
		}

	}

	/*Creating elevator threads*/
	for(j=0; j<statistics->num_elevators; j++)
	{
		if(pthread_attr_init(eattrs+j))
		{
			perror("attr_init()");
		}

		etids[j] = j;

		if(pthread_create(ethrds+j, eattrs+j, do_elevator_thread, etids+j) != 0)
		{
			perror("pthread_create()");
			exit(1);
		}
	}


	/*Joining people threads*/
	for(i=0; i<n; i++)
	{
		pthread_join(pthrds[i],&pretval);
	}

	free(pattrs);
	free(pthrds);
	free(ptids);

	/*Joining elevator threads*/
	for(j=0; j<statistics->num_elevators; j++)
	{
		pthread_join(ethrds[j],&eretval);
	}

	free(eattrs);
	free(ethrds);
	free(etids);

        printf("Simulation result: %d people have started , %d people have finished during %d seconds\n", statistics->num_people_started, statistics->num_people_finished, statistics->simulation_time);
	return 0;
}
