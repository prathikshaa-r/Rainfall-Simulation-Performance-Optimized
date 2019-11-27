#ifndef __RP_H
#define __RP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <pthread.h>

// Structs
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

struct calc_trickle_args_t {
         simulation *sim_data;
         int *thread_id;
         int rain_drop;
}typedef calc_trickle_args;


// Functions
// General Purpose Functions
double calc_time(struct timespec start, struct timespec end);
void print_data(FILE* stream, int N, float **data_struct);
void usage(const char *prog_name);
size_t str_to_num(const char *str);
float str_to_float(const char *str);
int get_nums(int size, const char *line, int *landscape_row);

// Special purpose Functions
void read_landscape(simulation *sim_data);
int parallel_calculate_trickle(simulation *sim_data, int rain_drop);
void *thread_calc_trickle(void *arguments);
int calculate_trickle(int * bounds, simulation *sim_data, int rain_drop);
void update_trickle(simulation *sim_data);
void get_bounds(simulation *sim_data, int thread_id, int *bounds);
int all_absorbed(simulation *sim_data);
void run_simulation(simulation * sim_data);
void write_result(simulation *sim_data);


#endif
