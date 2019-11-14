#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct simulation_struct
{
	int P;
	int M;
	float A;
	int N;
	const char * elevation_file;
} typedef simulation;

void usage(const char *prog_name){
	printf("Usage: %s <P> <M> <A> <N> <elevation_file> \n", prog_name);
	printf("-------------------------------------------------------------\n");
	printf("* P = # of parallel threads to use. \n");
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
  size_t val = strtoul(str, &endptr, 10);

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
	float val = strtof(str, &endptr);
	if((endptr == NULL)||(errno)){
		fprintf(stderr, "str_to_float: Invalid Input: %s\n", str);
    	exit(EXIT_FAILURE);
	}
	return val;
}

void read_landscape(int ***landscape_ptr, int N, const char* elevation_file){
	int ** landscape = *landscape_ptr; // recheck this syntax 
	// read N lines

	// for each line, getc non-space character
	// landscape[i][j] = str_to_num(char)
}

void run_simulation(simulation* sim_data){
	int num_threads = sim_data->P;
	int num_steps = sim_data->M;
	float absorption = sim_data->A;
	int N = sim_data->N;
	const char *elevation_file = sim_data->elevation_file;

	// create a 2D landscape array
	int **landscape = malloc(sizeof(int)*N*N);
	read_landscape(&landscape, N, elevation_file);
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
	sim_data->M = str_to_num(argv[2]); // num_steps = M
	sim_data->A = str_to_float(argv[3]); // absorption = A
	sim_data->N = str_to_num(argv[4]); // N dimensional landscape
	sim_data->elevation_file = argv[5]; // elevation filename

	run_simulation(sim_data);

	free(sim_data);
	return EXIT_SUCCESS;
}