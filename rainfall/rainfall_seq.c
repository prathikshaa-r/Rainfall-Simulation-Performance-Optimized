#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct simulation_struct
{
	int P; // num_threads
	int M; // rain steps
	int num_steps; // total simulation steps
	float A; // absorption
	int N; // landscape size
	int **landscape; // landscape array - input
	float **track_rain;
	float **rain_absorbed; // output
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

// read landscape from elevation file
void read_landscape(simulation *sim_data){
	// open file sim_data->elevation_file
	// read N lines

	// for each line, getc non-space character
	// landscape[i][j] = str_to_num(char)
}

// only function that is parallelized later
void run_simulation(simulation *sim_data){
	int num_rain_steps = sim_data->M;
	float absorption = sim_data->A;
	int N = sim_data->N;

	// loop over num_steps
	for(int i = 0; i<N; i++){
	  for(int j = 0; j<N; j++){
	    // absorb drops in current block

	    // check neighbours to flow the rest
	    // check i+1, j+1
	    
	  }
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

	sim_data->num_steps = 0;
	size_t size_landcape = sizeof(int)*(sim_data->N)*(sim_data->N);
	sim_data->landscape = malloc(size_landcape);
	memset(sim_data->landscape, 0, size_landcape);

	size_t size_absorbed = sizeof(float)*(sim_data->N)*(sim_data->N);
	sim_data->rain_absorbed = malloc(size_absorbed);
	memset(sim_data->rain_absorbed, 0, size_absorbed);

	read_landscape(sim_data);
	run_simulation(sim_data);
	write_result(sim_data);

	free(sim_data->rain_absorbed);
	free(sim_data->landscape);
	free(sim_data);
	return EXIT_SUCCESS;
}
