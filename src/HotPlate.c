/*
 * implementation for HotPlate
 */

#include "HotPlate.h"

/* plate pointers... */
extern float *curPlate;
extern float *newPlate;
extern float *tPlate;

/* thread control */
extern int nproc;
/* linear barrier */
extern unsigned long GoCalc;
extern unsigned long GoCheck;
/* counting "semaphore" */
int calcWait;
int checkWait;
/* how to know if we're steady */
unsigned long notSteady;

int main(int argc, char *argv[]) {
    double startTime;
    double endTime;
    double elapsedTime;
    int x;
    int y;
    int numIterations = 0;
    notSteady = PLATE_UNSTEADY_START;

    startTime = getTime();
    nproc = 1;
    if(argc > 1) {
        if(strcasestr(argv[1],NUM_THREADS_FLAG)) {
            printf("Processing desired number of threads...\n");
            char *numPtr = argv[1] + strlen(NUM_THREADS_FLAG);
            nproc = atoi(numPtr);
            if(nproc > 1) {
            }
        }
    }
    printf("We'll be using %d threads...\n",nproc);

    /* allocate our arrays on the heap */
    curPlate = (float*) calloc(PLATE_AREA, sizeof(float));
    newPlate = (float*) calloc(PLATE_AREA, sizeof(float));

    /* parallelize the initialization TODO*/

    /* initialize our starting plate */
    for(y = 1; y < PLATE_SIZE - 1; y++) { /* iterate over the rows */
        for(x = 1; x < PLATE_SIZE - 1; x++) { /* iterate over the columns */
            curPlate[LOC(x,y)] = WARM_START;
            newPlate[LOC(x,y)] = WARM_START;
            /* printf("(%d,%d = %d)\t", x,y,LOC(x,y)); */
        }
        /*printf("\n");*/
    }

    /* initialize the edges */
    for(x = 0; x < PLATE_SIZE; x++) {
        /* do the bottom edge */
        curPlate[LOC(x,0)] = HOT_START;
        newPlate[LOC(x,0)] = HOT_START;
        /*printf("Column %d in row 0\n", LOC(x,0));*/
        /* do the left edge */
        curPlate[LOC(0,x)] = COLD_START;
        newPlate[LOC(0,x)] = COLD_START;
        /*printf("Row %d in column 0\n", LOC(x,0));*/
        /* do the right edge */
        curPlate[LOC(PLATE_SIZE-1,x)] = COLD_START;
        newPlate[LOC(PLATE_SIZE-1,x)] = COLD_START;
        /*printf("Row %d in column %d\n", LOC(x,0),PLATE_SIZE-1);*/
    }

    /* initialize our hot row */
    for(x = 0; x < FIXED_ROW_COL; x++) {
        curPlate[LOC(x,FIXED_ROW)] = HOT_START;
        newPlate[LOC(x,FIXED_ROW)] = HOT_START;
    }

    /* initialize our lonely hot dot */
    curPlate[LOC(DOT_X,DOT_Y)] = HOT_START;
    newPlate[LOC(DOT_X,DOT_Y)] = HOT_START;

    /* initialize our pthread attributes once... */
    int err;
    pthread_attr_t threadAttr;
    /* initialize... */
    err = pthread_attr_init(&threadAttr);
    /* check for errors */
    if(err) {
        perror("pthread_attr_init");
        return -1;
    }
    /* set detach state */
    err = pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);
    /* check for errors */
    if(err) {
        perror("pthread_attr_setdetachstate");
        return -1;
    }

    int num;
    for(num = 0; num < nproc; num++) {
        printf("make thread %d...\n", num);
    }


    /* finish up...*/
    endTime = getTime();
    elapsedTime = endTime - startTime;
    printf("We've taken %0.6f seconds and %d iterations\n", elapsedTime, numIterations);
    return 0;
}

/*
 * get a high-precision representation of the current time
 */
double getTime() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return (double)tp.tv_sec + (double)tp.tv_usec * 1e-6;
}

/*
 * loop on our chunk of the plate until we're all steady
 */

/*
 * update the current plate using the last one as reference
 */
BOOL updatePlate(ThreadArg *arg) {
    int x;
    int y;
    float me;
    float neighborAvg;
    BOOL isSteady = TRUE;

    PRINT_LINE;
    /* update the new plate with temps from the old plate */
    /*
    for(y = 1; y < PLATE_SIZE - 1; y++) {
        for(x = 1; x < PLATE_SIZE - 1; x++) {
            if(!isFixed(x,y)) {
                newPlate[LOC(x,y)] = (curPlate[LEFT_LOC(x,y)]
                        + curPlate[RIGHT_LOC(x,y)]
                        + curPlate[LOWER_LOC(x,y)]
                        + curPlate[UPPER_LOC(x,y)]
                        + 4 * curPlate[LOC(x,y)] ) / 8;
            }
        }
    }
    */

    /* check the new plate for steady state */
    /*
    for(y = 1; y < PLATE_SIZE - 1; y++) {
        for(x = 1; x < PLATE_SIZE - 1; x++) {
            if(!isFixed(x,y)) {
                me = newPlate[LOC(x,y)];
                neighborAvg = (newPlate[LEFT_LOC(x,y)]
                        + newPlate[RIGHT_LOC(x,y)]
                        + newPlate[LOWER_LOC(x,y)]
                        + newPlate[UPPER_LOC(x,y)])/4;
                if(fabsf(me - neighborAvg) >= STEADY_THRESHOLD) {
                    isSteady = FALSE;
                }
            }
        }
    }
    */

    return isSteady;
}

/*
 * check to see if the given (x,y) location is one of our fixed hot cells
 */
inline BOOL isFixed(int x, int y) {
    /* check for our fixed temperature hot row */
    if((x < FIXED_ROW_COL) && (y == FIXED_ROW)) {
        return TRUE;
    }
    /* check for our lonely dot */
    if((x == DOT_X) && (y == DOT_Y)) {
        return TRUE;
    }
    /* no need to check for the edges, because neither x nor y will ever be 0 */
    return FALSE;
}

