/*
 * HotPlate
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <omp.h>
#include <math.h>
#include <string.h>

/*
 * some macros to help us
 */
#define PLATE_SIZE 768
#define PLATE_AREA 589824 /* 768*768 */
#define FLOAT_ACCURACY 0.0001f /* for comparing floating point numbers */
#define STEADY_THRESHOLD 0.1f
#define PLATE1_NEW 1
#define PLATE1_OLD 0
#define HOT_START 100.0f
#define COLD_START 0.0f
#define WARM_START 50.0f
#define TRUE 1
#define FALSE 0
#define NUM_THREADS_FLAG "--nt="

/* magic numbers for the fixed hot dot and row */
#define DOT_X 199 /* row 200 */
#define DOT_Y 499 /* column 500 */
#define FIXED_ROW 399 /* row 400 */
#define FIXED_ROW_COL 329 /* last column of the fixed row */

/* macros for plate positioning */
#define LOC(x,y) ( y*PLATE_SIZE+x )
#define LEFT_LOC(x,y) ( y*PLATE_SIZE+x-1 )
#define RIGHT_LOC(x,y) ( y*PLATE_SIZE+x+1)
#define LOWER_LOC(x,y) ( (y-1)*PLATE_SIZE+x )
#define UPPER_LOC(x,y) ( (y+1)*PLATE_SIZE+x )

/* macros for chunk dividing in OMP */
#define CHUNK_START(x,y) ( (x==0)?1:((PLATE_SIZE/y)*x-1) )
#define CHUNK_END(x,y) ( (x==0)?(PLATE_SIZE/y):((PLATE_SIZE/y)*x) )

/*
 * some type definitions
 */
typedef int BOOL;

/*
 * stuff we need
 */
float *plate1;
float *plate2;
BOOL runOMP;
int numThreads;

/*
 * some utility functions
 */
double getTime();
BOOL updatePlate(float *curPlate, float *newPlate);
inline BOOL isFixed(int x, int y);

