/*
 * HotPlate
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <math.h>
#include <string.h>
#include <limits.h>

/*
 * some macros to help us
 */
#define PLATE_SIZE 768
#define PLATE_AREA 589824 /* 768*768 */
#define FLOAT_ACCURACY 0.0001f /* for comparing floating point numbers */
#define STEADY_THRESHOLD 0.1f
#define HOT_START 100.0f
#define COLD_START 0.0f
#define WARM_START 50.0f
#define TRUE 1
#define FALSE 0
#define NUM_THREADS_FLAG "--nt="
#define PRINT_LINE ( printf("Thread %d, at line %d\n", iproc, __LINE__) )

/* magic numbers for the fixed hot dot and row */
#define DOT_X 199 /* row 200 */
#define DOT_Y 499 /* column 500 */
#define FIXED_ROW 399 /* row 400 */
#define FIXED_ROW_COL 329 /* last column of the fixed row */

/* macros for plate positioning */
#define LOC(x,y) ( y*PLATE_SIZE+x )
#define LEFT_LOC(x,y) ( (y*PLATE_SIZE)+(x-1) )
#define RIGHT_LOC(x,y) ( (y*PLATE_SIZE)+(x+1))
#define LOWER_LOC(x,y) ( ((y-1)*(PLATE_SIZE))+x )
#define UPPER_LOC(x,y) ( ((y+1)*(PLATE_SIZE))+x )

/* macros for chunk dividing in OMP */
#define CHUNK_START(iproc,nproc) ( (iproc==0)?1:((PLATE_SIZE/nproc)*(iproc)) )
#define CHUNK_END(iproc,nproc) ( (iproc==0)?:(iproc==(nproc-1))?(PLATE_SIZE/nproc):((PLATE_SIZE/nproc)*(iproc+1)) )

/* macros for steady and not steady */
#define PLATE_UNSTEADY_START ULONG_MAX
#define PLATE_STEADY 0

/*
 * some type definitions
 */
/* a BOOL for simplicity's sake */
typedef int BOOL;

/* our thread argument struct */
typedef struct {
    int iproc;
    int nproc;
} ThreadArg;
/* array to store them */
ThreadArg *threadArgs;

/*
 * stuff we need
 */
/* plate pointers */
float *curPlate;
float *newPlate;
float *tPlate;

/* thread control */
int nproc;
/* linear barrier */
volatile unsigned int GoCalc;
volatile unsigned int GoCheck;
/* mutexes */
pthread_mutex_t checkLock;
pthread_mutex_t steadyLock;
pthread_mutex_t calcLock;
/* counting "semaphore" */
int calcWaitCount;
int checkWaitCount;
int swapWaitCount;
pthread_mutex_t checkMutex;
pthread_mutex_t calcMutex;
/* keep track of whether we're all steady yet... */
unsigned int notSteady;
int finalIterations;

/*
 * some utility functions
 */
double getTime();
BOOL updatePlate(ThreadArg *arg);
void* findSteadyState(void *arg);
void maskToOnes(int nproc, volatile unsigned int *mask);
inline BOOL isFixed(int x, int y);

