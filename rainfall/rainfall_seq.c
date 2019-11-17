#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

double calc_time(struct timespec start, struct timespec end) {
   double start_sec = (double)start.tv_sec*1000000000.0 + (double)start.tv_nsec;
   double end_sec = (double)end.tv_sec*1000000000.0 + (double)end.tv_nsec;
   if (end_sec < start_sec) {
     return 0;
   } else {
     return end_sec - start_sec;
   }
 }


struct simulation_struct
{
	int P; // num_threads
	int M; // rain steps
	int num_steps; // total simulation steps
	float A; // absorption
	int N; // landscape size
	int **landscape; // landscape array - input
	float **current_rain; // keep track of rain through simulation
	float **trickle; // keep track of trickle in each time-step
	float **rain_absorbed; // rain absorbed in each tile output
	const char *elevation_file; // name of input file
} typedef simulation;

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
	char * line;
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


int calculate_trickle(simulation *sim_data, int rain_drop){
	int N = sim_data->N;

	if (!(rain_drop))
	{
		// if cur_rain is all zeros, return 1
		int keep_going = 0;
		for (int i = 0; i < N; ++i)
		{
			for (int j = 0; j < N; ++j)
			{
				if (sim_data->current_rain)
				{
					keep_going = 1;
					break;
				}
			}
			if (keep_going)
			{
				break;
			}
		}
		return 1;
	}

	printf("N in calculate_trickle = %d\n", N);
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{

			printf("N in calculate_trickle = %d\n", N);
			// add rain_drop if raining
			printf("Start\n");
			printf("Rain Drop: %d\n", rain_drop);
			printf("sim_data->current_rain: %f\n", sim_data->current_rain[i][j]);
			sim_data->current_rain[i][j] += rain_drop;
			printf("i, j, landscape: %d, %d, %d\n", i, j, sim_data->landscape[i][j]);
			printf("sim_data->current_rain after rain: %f\n", sim_data->current_rain[i][j]);

			// absorb rain
			sim_data->rain_absorbed[i][j] = (sim_data->A >= sim_data->current_rain[i][j]) ? sim_data->current_rain[i][j] : sim_data->A;
			sim_data->current_rain[i][j] -= sim_data->rain_absorbed[i][j];
			printf("Before trickle\n");

			// calculate trickle
			float *north_trickle, *south_trickle, *east_trickle, *west_trickle; // pointer to trickle arr
			int north, south, east, west, cur; // landscape values
			north = south = east = west = -1;

			north_trickle = south_trickle = east_trickle = west_trickle = NULL;

			cur = sim_data->landscape[i][j];

			printf("Step-0.5\n");

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
			printf("Step-1\n");
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

			printf("Step-2\n");

			if(smallest == cur) continue;
			
			if (north == smallest) track_arr[0] = 1;
			if (south == smallest) track_arr[1] = 1;
			if (east == smallest) track_arr[2] = 1;
			if (west == smallest) track_arr[3] = 1;

			printf("Step-3\n");

			float div_count = 0; // count num of low lying points
			for (int i = 0; i < 4; ++i)
			{
				if(track_arr[i]) div_count++;
			}
			printf("Div Count: %f\n", div_count);
			printf("Step-3.5\n");
			// divide and trickle
			for (int i = 0; i < 4; ++i)
			{
				if(track_arr[i]){
					switch(i){
						case 0:
							printf("North\n");
							*north_trickle += 1/div_count;
							break;

						case 1:
							printf("South\n");
							*south_trickle += 1/div_count;
							break;

						case 2:							
							printf("East\n");
							*east_trickle += 1/div_count;
							break;

						case 3:							
							printf("West\n");
							*west_trickle += 1/div_count;
							break;
					}
				}
			} // end of divide and trickle
			printf("Step-4\n");

		}
	}
	return 0;
}

void update_trickle(simulation *sim_data){

}

// only function that is parallelized later
void run_simulation(simulation *sim_data){
	int num_rain_steps = sim_data->M;
	int N = sim_data->N;
	printf("N in run_sim = %d\n", N);

	// loop over num_steps
	for(sim_data->num_steps = 0; ;sim_data->num_steps++){ // break when cur_rain is all 0
	    // absorb drops in current block
	    // check neighbours to flow the rest
	    // check i+1, j+1
	    int stop_true = calculate_trickle(sim_data, ((sim_data->num_steps < num_rain_steps)?1:0));
	    update_trickle(sim_data);
	    if (stop_true) break;
	}
}

// write the result of the simulation to output file
void write_result(simulation *sim_data){
	// write sim_data->rain_absorbed to elevation_file + ".out"
	// space seperated
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

	printf("N=%d\n", sim_data->N);

	sim_data->num_steps = 0;
	
	sim_data->landscape = (int**)malloc(sizeof(int*)*(sim_data->N));
	for (int i = 0; i < sim_data->N; ++i)
	{
		sim_data->landscape[i] = (int *)malloc(sizeof(int) * sim_data->N);
		memset(sim_data->landscape[i], 0, (sizeof(int) * sim_data->N));
	}

	sim_data->rain_absorbed = (float**)malloc(sizeof(int*)*(sim_data->N));
	for (int i = 0; i < sim_data->N; ++i)
	{
		sim_data->rain_absorbed[i] = (float *)malloc(sizeof(int) * sim_data->N);
		memset(sim_data->rain_absorbed[i], 0, (sizeof(int) * sim_data->N));
	}

	sim_data->current_rain = (float**)malloc(sizeof(int*)*(sim_data->N));
	for (int i = 0; i < sim_data->N; ++i)
	{
		sim_data->current_rain[i] = (float *)malloc(sizeof(int) * sim_data->N);
		memset(sim_data->current_rain[i], 0, (sizeof(int) * sim_data->N));
	}

	sim_data->trickle = (float**)malloc(sizeof(int*)*(sim_data->N));
	for (int i = 0; i < sim_data->N; ++i)
	{
		sim_data->trickle[i] = (float *)malloc(sizeof(int) * sim_data->N);
		memset(sim_data->trickle[i], 0, (sizeof(int) * sim_data->N));
	}

	printf("N before read: %d\n", sim_data->N);
	read_landscape(sim_data);
	printf("N after read: %d\n", sim_data->N);
	run_simulation(sim_data);
	printf("N after sim: %d\n", sim_data->N);
	write_result(sim_data);
	printf("N after write: %d\n", sim_data->N);

	free(sim_data->rain_absorbed);
	free(sim_data->landscape);
	free(sim_data->current_rain);
	free(sim_data->trickle);
	free(sim_data);
	return EXIT_SUCCESS;
}
