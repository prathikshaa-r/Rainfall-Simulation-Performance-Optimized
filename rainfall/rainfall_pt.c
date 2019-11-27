#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include "rainfall_pt.h"

// Locks for calc trickle
// separate rows
// when i is at bounds
//
pthread_mutex_t *row_locks;

void init_row_locks(simulation *sim_data) {
  row_locks = (pthread_mutex_t *) malloc(sizeof(*row_locks)*sim_data->N);
  for (int i = 0; i < sim_data->N; i++) {
    pthread_mutex_init(&row_locks[i], NULL);
  }
}

void free_row_locks(simulation *sim_data){
  for (int i = 0; i < sim_data->N; i++) {
    pthread_mutex_destroy(&row_locks[i]);
  }
  free(row_locks);
}

struct timespec start_time, end_time;
double calc_time(struct timespec start, struct timespec end) {
   double start_sec = (double)start.tv_sec*1000000000.0 + (double)start.tv_nsec;
   double end_sec = (double)end.tv_sec*1000000000.0 + (double)end.tv_nsec;
   if (end_sec < start_sec) {
     return 0;
   } else {
     return end_sec - start_sec;
   }
 }


void print_data(FILE* stream, int N, float **data_struct){
	for (int i = 0; i < N; ++i){
		for (int j = 0; j < N; ++j){
		  fprintf (stream, "%8.6g ", data_struct[i][j]);
		}
		fprintf(stream, "\n");
	}
}

void usage(const char *prog_name){
	printf("Usage: %s <P> <M> <A> <N> <elevation_file> \n", prog_name);
	printf("-------------------------------------------------------------\n");
	printf("* P = # of parallel threads to use (Ignored in this version of the code). \n");
	printf("* M = # of simulation time steps during which a rain drop will fall"
		" on each landscape point. In other words, 1 rain drop falls on each"
		" point during the first M steps of the simulation. \n");
	printf("* A = absorption rate (specified as a floating point number)."
		" The amount of raindrops that are absorbed into the ground at a"
		" point during a timestep. \n");
	printf("* N = dimension of the landscape (NxN) \n");
	printf("* elevation_file = name of input file that specifies the elevation"
		" of each point. \n");
}

// string to number with error checking
// does not allow negative nummbers or strings
// will print error message and exit
size_t str_to_num(const char *str) {
  char *endptr;
  // check for -ve nos.
  if (str[0] == '-') {
    printf("str_to_num: Invalid Input:\t%s\n", str);
    exit(EXIT_FAILURE);
  }
  errno = 0; // reset errno before call
  size_t val = strtoul(str, &endptr, 10); // convert to num

  if (errno) {
    perror("str_to_num: Invalid Input: ");
    exit(EXIT_FAILURE);
  }

  if (endptr == str) {
    fprintf(stderr, "str_to_num: Invalid Input:\t%s\nNo digits were found.\n", str);
    exit(EXIT_FAILURE);
  }

  return val;
}

// string to float
float str_to_float(const char *str){
	char *endptr;
	errno = 0; // reset errno before call
	float val = strtof(str, &endptr); // convert to float
	if((endptr == NULL)||(errno)){
		fprintf(stderr, "str_to_float: Invalid Input: %s\n", str);
    	exit(EXIT_FAILURE);
	}
	return val;
}


/* void *print_the_arguments(void *arguments) */
/* { */
/*     printf("In print args...\n"); */
/*     struct arg_struct *args = arguments; // line */
/*     printf("In print args...\n"); */
/*     // print_data(stdout, args->sim_data->N, args->sim_data->current_rain); */
/*     printf("N: %d\n", args->sim_data->N); */
/*     printf("Thread ID:%d\n", args->thread_id); */
/*     pthread_exit(NULL); */
/*     return NULL; */
/* } */


int get_nums(int size, const char *line, int *landscape_row){
	char *endptr = (char *)line;
	for(int i = 0; i < size; i++){
		landscape_row[i] = strtoul(endptr, &endptr, 10);
		endptr++;
	}
}


// read landscape from elevation file
void read_landscape(simulation *sim_data){
	// open file sim_data->elevation_file
	// read N lines

	// for each line, getc non-space character
	// landscape[i][j] = str_to_num(char)
	FILE * fp;
	char * line = 0;
	size_t len = 0;
	ssize_t read = 0;

	fp = fopen(sim_data->elevation_file, "r");
	if (fp == NULL){
		printf("Error in opening file.\n");
		exit(EXIT_FAILURE);
	}

  	for (int i = 0; i < sim_data->N; i++){
		if((read = getline(&line, &len, fp) != 1)){
	      	get_nums(sim_data->N, line, sim_data->landscape[i]);
		}
	}

	fclose(fp);
	if(line)
	free(line);
}

int parallel_calculate_trickle(simulation *sim_data, int rain_drop){
  pthread_t threads[sim_data->P];
  for (int i = 0; i < sim_data->P; ++i)
    {
      // printf("Creating thread %d...\n", i);
      calc_trickle_args *thread_args = (calc_trickle_args *)malloc(sizeof(*thread_args));
      thread_args->sim_data = sim_data;
      thread_args->thread_id = (int *)malloc(sizeof(int));
      *(thread_args->thread_id) = i;
      thread_args->rain_drop = rain_drop;

      if (pthread_create(&threads[i], NULL, &thread_calc_trickle, (void *)thread_args) != 0){
	printf("Uh-oh!\n");
	return -1;
      }
    }

  for(int i = 0; i < sim_data->P; i++){
    pthread_join(threads[i], NULL); /* Wait until thread is finished */
  }
}

void *thread_calc_trickle(void *arguments){
  calc_trickle_args *args = arguments;
  simulation *sim_data = args->sim_data;
  int thread_id = *(args->thread_id);

  // printf("In threadid: %d| Before entering get_bounds()\n", thread_id);
  int *bounds = malloc(sizeof(int) * 2);
  get_bounds(sim_data, thread_id, bounds);
  // printf("Bounds before calc trickle...:%d, %d\n", bounds[0], bounds[1]);
  calculate_trickle(bounds, sim_data, args->rain_drop);

  free(args->thread_id);
  free(args);
  free(bounds);
}

int calculate_trickle(int * bounds, simulation *sim_data, int rain_drop){
  
	int N = sim_data->N;

	for (int i = bounds[0]; i < bounds[1]; i++){
		for (int j = 0; j < N; j++){

			// add rain_drop if raining
			sim_data->current_rain[i][j] += rain_drop;
			// absorb rain

			float new_absorbed = ((sim_data->A >= sim_data->current_rain[i][j]) ? sim_data->current_rain[i][j] : sim_data->A);
			sim_data->rain_absorbed[i][j] += new_absorbed;
			sim_data->current_rain[i][j] -= new_absorbed;

			
			// TRICKLE ONLY IF ONE FULL DROP IS AVAILABLE
			if(sim_data->current_rain[i][j] > 0){
				float trickle_amt = ((1 >= sim_data->current_rain[i][j]) ? sim_data->current_rain[i][j] : 1);

				// calculate trickle
				float *north_trickle, *south_trickle, *east_trickle, *west_trickle; // pointer to trickle arr
				int north, south, east, west, cur; // landscape values
				north = south = east = west = -1;

				north_trickle = south_trickle = east_trickle = west_trickle = NULL;
				cur = sim_data->landscape[i][j];


				if(i > 0){
					south = sim_data->landscape[i-1][j];
					south_trickle = &sim_data->trickle[i-1][j];
				}
				if(i < N-1){
					north = sim_data->landscape[i+1][j];
					north_trickle = &sim_data->trickle[i+1][j];
				}
				if(j > 0){
					west = sim_data->landscape[i][j-1];
					west_trickle = &sim_data->trickle[i][j-1];
				}
				if(j < N-1){
					east = sim_data->landscape[i][j+1];
					east_trickle = &sim_data->trickle[i][j+1];
				}
				
				// now check where to trickle
				// find min elevation
				int track_arr[4] = {0, 0, 0, 0}; // N S E W
				int smallest = cur;
				if (north_trickle)
				{
				 	smallest = (north > smallest) ? smallest : north;
				}
				if (south_trickle)
				{
					smallest = (south > smallest) ? smallest : south;
				}
				if (east_trickle)
				{
					smallest = (east > smallest) ? smallest : east;
				}
				if (west_trickle)
				{
					smallest = (west > smallest) ? smallest : west;
				}

			

				if(smallest == cur) continue;
				
				if (north == smallest) track_arr[0] = 1;
				if (south == smallest) track_arr[1] = 1;
				if (east == smallest) track_arr[2] = 1;
				if (west == smallest) track_arr[3] = 1;

			

				float div_count = 0; // count num of low lying points
				for (int k = 0; k < 4; ++k){
					if(track_arr[k]) div_count++;
				}
			
				// divide and trickle
				for (int k = 0; k < 4; ++k){
					if(track_arr[k]){
						switch(k){
							case 0:
							        /* if(i == (bounds[1] - 1)){ */
								/*   pthread_mutex_lock(&row_locks[bounds[1]]); */
								/* } */
							        pthread_mutex_lock(&row_locks[i+1]);
								*north_trickle += trickle_amt/div_count;
								pthread_mutex_unlock(&row_locks[i+1]);
								/* if(i == (bounds[1]-1)){ */
								/*   pthread_mutex_unlock(&row_locks[bounds[1]]); */
								/* } */
								break;

							case 1:
							        /* if(i == bounds[0]){ */
								/*   pthread_mutex_lock(&row_locks[bounds[0]-1]); */
								/* } */
							        pthread_mutex_lock(&row_locks[i-1]);
								*south_trickle += trickle_amt/div_count;
								pthread_mutex_unlock(&row_locks[i-1]);
								/* if(i == bounds[0]){ */
								/*   pthread_mutex_lock(&row_locks[bounds[0]-1]); */
								/* } */
								break;

							case 2:							
								/* if((i == bounds[0])||(i == bounds[1]-1)){ */
								/*   pthread_mutex_lock(&row_locks[i]); */
								/* } */
							        pthread_mutex_lock(&row_locks[i]);
								*east_trickle += trickle_amt/div_count;
								pthread_mutex_unlock(&row_locks[i]);
								/* if((i == bounds[0])||(i == bounds[1]-1)){ */
								/*   pthread_mutex_unlock(&row_locks[i]); */
								/* } */
								break;

							case 3:							
								/* if((i == bounds[0])||(i == bounds[1]-1)){ */
								/*   pthread_mutex_lock(&row_locks[i]); */
								/* } */
							        pthread_mutex_lock(&row_locks[i]);
								*west_trickle += trickle_amt/div_count;
								pthread_mutex_unlock(&row_locks[i]);
								/* if((i == bounds[0])||(i == bounds[1]-1)){ */
								/*   pthread_mutex_unlock(&row_locks[i]); */
								/* } */
								break;
						}
					}
				} 
				sim_data->current_rain[i][j] -= trickle_amt;

			} // END OF TRICKLE IF 1 DROP

		}
	}
	return 0;
}

void update_trickle(simulation *sim_data){
	for (int i = 0; i < sim_data->N; ++i){
		for (int j = 0; j < sim_data->N; ++j){
			sim_data->current_rain[i][j] += sim_data->trickle[i][j];
		}
	}
}

void get_bounds(simulation * sim_data, int thread_id, int *bounds){
  int P = sim_data->P;
  int N = sim_data->N;

  bounds[0] = thread_id * (N/P); // min - inclusive
  bounds[1] = (thread_id+1) * (N/P); // max - exclusive
  if(thread_id == (P-1)) {bounds[1] += (N%P);}
  // printf("Thread id: %d, P=%d, N=%d, min:%d, max=%d\n", thread_id, P, N, bounds[0], bounds[1]);
}

int all_absorbed(simulation *sim_data){
  int N = sim_data->N;
  if(sim_data->num_steps < sim_data->M){
    return 0;
  }
  for (int i = 0; i < N; ++i){ // rows
    for (int j = 0; j < N; ++j){ // columns
      if (sim_data->current_rain[i][j]){ // if value non-zero
	return 0; // return false
      } // end if
    } // end cols
  } // end rows
  return 1;
}

// only function that is parallelized later
void run_simulation(simulation * sim_data){
	int num_rain_steps = sim_data->M;
	int N = sim_data->N;
	// loop over num_steps
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	for(sim_data->num_steps = 0; ;sim_data->num_steps++){ // break when cur_rain is all 0
	    // absorb drops in current block
	    // check neighbours to flow the rest
	    // check i+1, j+1
	    if(all_absorbed(sim_data)) break;
	    parallel_calculate_trickle(sim_data, ((sim_data->num_steps < num_rain_steps)?1:0));
	    update_trickle(sim_data);
	    for (int i = 0; i < sim_data->N; ++i){
	      if (!(sim_data->trickle[i])){
		sim_data->trickle[i] = (float *)malloc(sizeof(float) * sim_data->N);
	      }
	      memset(sim_data->trickle[i], 0, (sizeof(float) * sim_data->N));
	    }
	}
	clock_gettime(CLOCK_MONOTONIC, &end_time);
}

/* void *run_simulation(void *arguments){ */
/*         struct arg_struct *args = arguments; // line */
/*         simulation * sim_data = args->sim_data; */
/* 	int thread_id = args->thread_id; */
/* 	printf("Sim Data:N: %d\n", sim_data->N); */
/* 	printf("Thread ID %d\n", thread_id); */
	
/* 	int num_rain_steps = sim_data->M; */
/* 	int N = sim_data->N; */

/* 	int * bounds = malloc(sizeof(int) * 2); */
/* 	get_bounds(sim_data, thread_id, bounds); */

/* 	// loop over num_steps */
/* 	clock_gettime(CLOCK_MONOTONIC, &start_time); */
/* 	for(sim_data->num_steps = 0; ;sim_data->num_steps++){ // break when cur_rain is all 0 */
/* 	    // absorb drops in current block */
/* 	    // check neighbours to flow the rest */
/* 	    // check i+1, j+1 */
/* 	    calculate_trickle(sim_data, ((sim_data->num_steps < num_rain_steps)?1:0)); */
/* 	    update_trickle(sim_data); */
/* 	    for (int i = 0; i < sim_data->N; ++i){ */
/* 	      if (!(sim_data->trickle[i])){ */
/* 		sim_data->trickle[i] = (float *)malloc(sizeof(float) * sim_data->N); */
/* 	      } */
/* 	      memset(sim_data->trickle[i], 0, (sizeof(float) * sim_data->N)); */
/* 	    } */
/* 	    if(all_absorbed(sim_data)) break; */
/* 	} */
/* 	clock_gettime(CLOCK_MONOTONIC, &end_time); */
/* 	free(bounds); */
/* } */

// write the result of the simulation to output file
void write_result(simulation *sim_data){
	// write sim_data->rain_absorbed to elevation_file + ".out"
	// space seperated

	double elapsed_s = calc_time(start_time, end_time) / 1000000000.0;
	fprintf(stderr, "Rainfall simulation took %d time steps to complete.\n", sim_data->num_steps);
	fprintf(stderr, "Runtime = %f seconds.\n", elapsed_s);
	fprintf(stderr, "\n");
	fprintf(stderr, "The following grid shows the number of raindrops absorbed at each point:\n");
	print_data(stderr, sim_data->N, sim_data->rain_absorbed);
}

int main(int argc, char const *argv[])
{
	if (argc !=6)
	{
		usage(argv[0]);
		return EXIT_SUCCESS;
	}
	simulation *sim_data = malloc(sizeof(simulation));
	sim_data->P = str_to_num(argv[1]); // num_threads = P
	sim_data->M = str_to_num(argv[2]); // num_rain_steps = M
	sim_data->A = str_to_float(argv[3]); // absorption = A
	sim_data->N = str_to_num(argv[4]); // N dimensional landscape
	sim_data->elevation_file = argv[5]; // elevation filename

	sim_data->num_steps = 0;

	sim_data->landscape = (int**)malloc(sizeof(int*)*(sim_data->N));
	for (int i = 0; i < sim_data->N; ++i){
		sim_data->landscape[i] = (int *)malloc(sizeof(int) * sim_data->N);
		memset(sim_data->landscape[i], 0, (sizeof(int) * sim_data->N));
	}

	sim_data->rain_absorbed = (float**)malloc(sizeof(int*)*(sim_data->N));
	for (int i = 0; i < sim_data->N; ++i){
		sim_data->rain_absorbed[i] = (float *)malloc(sizeof(int) * sim_data->N);
		memset(sim_data->rain_absorbed[i], 0, (sizeof(int) * sim_data->N));
	}

	sim_data->current_rain = (float**)malloc(sizeof(int*)*(sim_data->N));
	for (int i = 0; i < sim_data->N; ++i){
		sim_data->current_rain[i] = (float *)malloc(sizeof(int) * sim_data->N);
		memset(sim_data->current_rain[i], 0, (sizeof(int) * sim_data->N));
	}

	sim_data->trickle = (float**)malloc(sizeof(int*)*(sim_data->N));
	for (int i = 0; i < sim_data->N; ++i){
		sim_data->trickle[i] = (float *)malloc(sizeof(int) * sim_data->N);
		memset(sim_data->trickle[i], 0, (sizeof(int) * sim_data->N));
	}

	init_row_locks(sim_data);
	read_landscape(sim_data);

	// Don't create more threads than rows in the matrix
	if(sim_data->P > sim_data->N){
	  sim_data->P = sim_data->N;
	}

	// run for each thread
	/* for (int i = 0; i < sim_data->P; ++i) */
	/* { */
	/*        printf("Creating thread %d...\n", i); */
	/* 	pthread_t some_thread; */
   	/* 	struct arg_struct args; */
	/* 	args.sim_data = sim_data; */
	/* 	args.thread_id = i; // shared */

	/* 	if (pthread_create(&some_thread, NULL, &run_simulation, (void *)&args) != 0)                    { */
	/* 	  printf("Uh-oh!\n"); */
	/* 	  return -1; */
	/* 	} */
	/* 	pthread_join(some_thread, NULL); /\* Wait until thread is finished *\/ */
	/* } */

	run_simulation(sim_data);
	write_result(sim_data);

	free_row_locks(sim_data);

	for(int i = 0; i < sim_data->N; i++){
	  if(sim_data->landscape[i]) free(sim_data->landscape[i]);
	  if(sim_data->rain_absorbed[i]) free(sim_data->rain_absorbed[i]);
	  if(sim_data->current_rain[i]) free(sim_data->current_rain[i]);
	  if(sim_data->trickle[i]) free(sim_data->trickle[i]);
	}
	
	free(sim_data->rain_absorbed);
	free(sim_data->landscape);
	free(sim_data->current_rain);
	free(sim_data->trickle);
	free(sim_data);
	return EXIT_SUCCESS;
}
