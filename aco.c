/***************************************************************
* Title: CS325-400: ACO Implementation
* Author: Daniel Jarc (jarcd)
* Date: June 5, 2018
* Description: Program utilizes ACO to solve TSP.
***************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define TOKEN_DELIM " \t\r\n\a"	// Token delimiter characters
#define ITERATIONS		200		// Total iterations [Default: 200]
#define COLONY_SIZE		30		// Total ants [Default: 30]
#define ALPHA			1		// Influence of pheromones [Default: 1.0]
#define BETA			3		// Influence of visibility [Default: 3.0]
#define RHO				0.3		// Pheromone evaporation rate [Default: 0.1]
#define Q				1.0		// Used to calculate pheromone change [Default 1.0]
int n;							// Number of cities

/***************************************************************
* struct city [WORKING]
* Description: 
***************************************************************/
struct city {
	
	int id;
	int x;
	int y;
};

/***************************************************************
* loadFile() [WORKING]
* Description: 
***************************************************************/
struct city * loadFile(const char * filename) {

	// Input variables
	FILE *fp_in;
	char ch;
	char * line = NULL;
	char * token = NULL;
	size_t size = 0;
	size_t read;
	struct city * cities;
	int i = 0;
	
	// Open input file
	fp_in = fopen(filename,"r");
	if (fp_in == NULL) {
		fprintf(stderr, "Error opening input file\n");
		exit(1);  
	}
	
	// Get number of cities
	while((ch = fgetc(fp_in)) != EOF){
		if(ch == '\n'){
			n++;
		}	
	}

	// Reset file pointer
	rewind(fp_in);
	
	// Allocate space for cities
	cities = malloc(n * (sizeof(struct city)));
	if(cities == NULL){
		fprintf(stderr, "Error allocating memory for cities\n");
		exit(1); 
	}
	
	// Copy data
	for (i = 0; i < n; i++) {
		
		// Get line from file 
		if ((read = getline(&line, &size, fp_in)) == -1) {
			fprintf(stderr, "Error reading line");
			exit(1);
		}
		
		// Get id, x, y values
		token = strtok(line, TOKEN_DELIM); 
		cities[i].id = atoi(token);
		token = strtok(NULL, TOKEN_DELIM);
		cities[i].x = atoi(token);
		token = strtok(NULL, TOKEN_DELIM);
		cities[i].y = atoi(token);

		// Free line 
		if (line != NULL) {
			free(line);
			line = NULL;	
		}
	}
	
	// Close file
	fclose(fp_in);
	
	return cities; 
}

/***************************************************************
* init2DArray() [WORKING]
* Description: 
***************************************************************/
void init2DArray(double **arr, double val){
	
	int i, j;
	
	for (i = 0; i < n; i++){
		for (j = 0; j < n; j++){
			arr[i][j] = val;
		}
	}
}

/***************************************************************
* init1DArray() [WORKING]
* Description: 
***************************************************************/
void init1DArray(int * arr, int val){
	
	int i;
	
	for (i = 0; i < n; i++){
		arr[i] = val;
	}
}

/***************************************************************
* printArray() [WORKING]
* Description: 
***************************************************************/
void printArray(double **arr){
	
	int i, j;
	
	for (i = 0; i < n; i++) {
		for (j = 0; j < n; j++) {
			printf("%.4f ", arr[i][j]);
		}
		printf("\n");
	}
}

/***************************************************************
* printCities() [WORKING]
* Description:
***************************************************************/
void printCities(struct city * cities){

	int i;
	for(i = 0; i < n; i++){
		printf("cities[%d] id:%d, x:%d, y:%d\n", i, cities[i].id, cities[i].x, cities[i].y);
	}
}

/***************************************************************
* printPath() [WORKING]
* Description: 
***************************************************************/
void printPath(int * arr){
	
	int i;
	
	for (i = 0; i < n; i++) {
		printf("%d\n", arr[i]);
	}
}

/***************************************************************
* calculateDistances() [WORKING]
* Description: 
***************************************************************/
void calculateDistances(struct city * cities, double ** distance){
	
	int i,j,d;
	double deltaX, deltaY;
	double dist;
	
	for(i = 0; i < n; i++){
		for(j = 0; j < n; j++) {	
			deltaX = cities[i].x - cities[j].x;
			deltaY = cities[i].y - cities[j].y;
			distance[i][j] = round(sqrt(pow(deltaX, 2) + pow(deltaY, 2)));
		}
	}
}

/***************************************************************
* calculateVisibility\ies() [WORKING]
* Description: 
***************************************************************/
void calculateVisibilities(double ** distance, double ** visibility){
	
	int i,j;
	
	for(i=0; i < n; i++){
		for(j=0; j < n; j++) {	
			if(distance[i][j] == 0.0){
				visibility[i][j] = 0.0;
			}else{
				visibility[i][j] = 1.0 / distance[i][j];
			}	
		}
	}	
}


/***************************************************************
* antSeek() [REFACTOR]
* Description: 
***************************************************************/
int antSeek(double ** distance, double ** visibility, double ** pheromone, double ** probability, int * visited, int location, int * path){
	
	int i, j; 
	int distanceTraveled = 0;
	double probabilitySummation;
	double maxProbability; 
	double randomProbability;
	int selectedCity;

	// Initialize arrays
	init2DArray(probability, 0.0);
	init1DArray(visited, 0);
	init1DArray(path, -1);
	
	// Iterate through all the cities
	for(i = 0; i < n; i++){

		visited[location] = 1;				// Mark current city as visited
		path[i] = location;					// Add current city to path
		probabilitySummation = 0.0;			// Reset probability summation value
		maxProbability = 0.0;				// Reset max probability value
		
		// If it's the last city, no need for calculations
		if(i == n - 1){
			break; 
		}
		
		// Calculate probabilites based on formula
		for(j=0; j < n; j++){
			
			if(visited[j] == 1){
				probability[location][j] = 0.0;
			}else{
				probability[location][j] = pow(pheromone[location][j], ALPHA) * pow(visibility[location][j], BETA);
			}
			probabilitySummation += probability[location][j];			
		}

		// Calculate the probability of the ant traveling from [location] to city [j]
		for(j=0; j < n; j++){
			probability[location][j] /= probabilitySummation;
			if(probability[location][j] >= maxProbability){
				maxProbability = probability[location][j];
			}
		}
		
		// Generate randomized probability influencing ant's choice
		randomProbability = ((double)rand()) / ((double)RAND_MAX);
		randomProbability *= maxProbability;
		
		// Select next city based on randomized probability, probability matrix and cities visited
		for(j=0; j < n; j++){
			if((randomProbability <= probability[location][j]) && (visited[j] == 0)){
				selectedCity = j;
			}
		}

		// Update distance traveled and location
		distanceTraveled += distance[location][selectedCity];
		location = selectedCity;
	}

	return distanceTraveled;
}

/***************************************************************
* antTrace(); [WORKING]
* Description: 
***************************************************************/
void antTrace(int * path, double ** pheromone, double pheromoneToAdd){

	int i, city1, city2;
	
	// Add pheromones between the cities in the path
	for(i = 0; i < n; i++){
		
		if(i == n-1){
			city1 = path[n-1];
			city2 = path[0];
		}else{
			city1 = path[i]; 
			city2 = path[i+1];
		}
		pheromone[city1][city2] += pheromoneToAdd;
	}
}

/***************************************************************
* updatePheromones() [WORKING]
* Description:
***************************************************************/
void evaporatePheromones(double ** pheromone){	
	
	int i, j;
	double delta = 1.0 - RHO;

	// Globally evaporate phereomones
	for (i = 0; i < n; i++){
		for (j = 0; j < n; j++) {
			pheromone[i][j] *= delta;
		}
	}
}

/***************************************************************
* outputResults() [WORKING]
* Description:
***************************************************************/
void outputResult(int * path, int pathDistance, const char * filename){
	
	int i;
	FILE * fp_out;
	char * filename_out;
	int filenameLength;
	
	// Create output filename
	filenameLength = strlen(filename);
	filename_out = malloc(filenameLength + 6);
	sprintf(filename_out, "%s.tour", filename);
	
	// Open file
	fp_out = fopen(filename_out, "w");
	
	// Copy path distance and cities
	fprintf(fp_out, "%d\n", pathDistance);
	for(i=0; i < n; i++){
		fprintf(fp_out, "%d\n", path[i]);
	}
	
	// Close file
	fclose(fp_out);
	
	// Free memory
	free(filename_out);
	filename_out = NULL;
}

/***************************************************************
* aco() [REFACTOR]
* Description:
***************************************************************/
void aco(struct city * cities, const char * filename){
	
	int i, j; 
	double ** distance;
	double ** visibility;
	double ** pheromone;
	double ** probability;	
	int * visited;
	int * path;
	int location;
	int ant;
	int pathDistance; 
	double pheromoneToAdd;
	
	// Allocate arrays
	distance = malloc(n * sizeof(double *));
	visibility = malloc(n * sizeof(double *));
	pheromone = malloc(n * sizeof(double *));
	probability = malloc(n * sizeof(double *));
	visited = malloc(n * sizeof(int));
	path = malloc(n * sizeof(int));
	
	for(i=0; i< n; i++){
		distance[i] = malloc(n * sizeof(double));
		visibility[i] = malloc(n * sizeof(double));
		pheromone[i] = malloc(n * sizeof(double));
		probability[i] = malloc(n * sizeof(double));
	}
	
	// Init arrays
	init2DArray(distance, 0.0);
	init2DArray(visibility, 0.0);	
	init2DArray(pheromone, 1.0);
	
	// Calulate distances and visibilites between cities
	calculateDistances(cities, distance); 
	calculateVisibilities(distance, visibility);
	
	// Choose random starting city for ants
	location = rand() % n;
	
	// For desired iterations
	for(i = 0; i < ITERATIONS; i++){
		
		// For each ant in the colony
		for (ant = 0; ant < COLONY_SIZE; ant++) {
			
			pathDistance = antSeek(distance, visibility, pheromone, probability, visited, location, path);
			pheromoneToAdd = Q / pathDistance;
			antTrace(path, pheromone, pheromoneToAdd);
		}
		evaporatePheromones(pheromone);
	}
	
	// Process results and output to file..
	outputResult(path, pathDistance, filename);

	// Free memory
	if(distance != NULL){
		for(i=0; i < n; i++) {
			free(distance[i]);
		}
		free(distance);
		distance = NULL;
	}
	
	if(visibility != NULL){
		for(i=0; i < n; i++) {
			free(visibility[i]);
		}
		free(visibility);
		visibility = NULL;
	}
	
	if(pheromone != NULL){
		for(i=0; i < n; i++) {
			free(pheromone[i]);
		}
		free(pheromone);
		pheromone = NULL;
	}

	if(probability != NULL){
		for(i=0; i < n; i++) {
			free(probability[i]);
		}
		free(probability);
		probability = NULL;
	}

	if(visited != NULL){
		free(visited);
		visited = NULL;
	}
	
	if(path != NULL){
		free(path);
		path = NULL;
	}
}

/***************************************************************
* main()
* Description: 
***************************************************************/
int main(int argc, char* argv[]) {
	
	// filename and cities array
	const char * filename = argv[1];
	struct city * cities;

	// Error check - incorrect command line argument
	if(filename == NULL){
		fprintf(stderr, "Usage: aco [filename]\n");
		exit(1);
	}
	
	// Seed random
	srand(time(NULL));
	
	// Load cities, run ACO
	cities = loadFile(filename);
	aco(cities, filename);
	
	// Free memory
	if(cities != NULL){
		free(cities);
		cities = NULL;
	}
	
    return 0;
}