/*
 * implementation for HotPlate
 */

#include "HotPlate.h"

extern float *plate1;
extern float *plate2;
extern int numThreads;

int main(int argc, char *argv[]) {
    double startTime;
    double endTime;
    double elapsedTime;
    BOOL steadyState;
    int threadID;
    int x;
    int y;
    int numIterations = 0;
    int plateStatus = PLATE1_NEW;

    startTime = getTime();
    numThreads = 1;
    if(argc > 1) {
        if(strcasestr(argv[1],NUM_THREADS_FLAG)) {
            printf("Processing desired number of threads...\n");
            char *numPtr = argv[1] + strlen(NUM_THREADS_FLAG);
            numThreads = atoi(numPtr);
            if(numThreads > 1) {
            }
        }
    }
    printf("We'll be using %d threads...\n",numThreads);

    /* allocate our arrays on the heap */
    plate1 = (float*) calloc(PLATE_AREA, sizeof(float));
    plate2 = (float*) calloc(PLATE_AREA, sizeof(float));

    /* parallelize the initialization TODO*/

    /* initialize our starting plate */
    for(y = 1; y < PLATE_SIZE - 1; y++) { /* iterate over the rows */
        for(x = 1; x < PLATE_SIZE - 1; x++) { /* iterate over the columns */
            plate1[LOC(x,y)] = WARM_START;
            plate2[LOC(x,y)] = WARM_START;
            /* printf("(%d,%d = %d)\t", x,y,LOC(x,y)); */
        }
        /*printf("\n");*/
    }

    /* initialize the edges */
    for(x = 0; x < PLATE_SIZE; x++) {
        /* do the bottom edge */
        plate1[LOC(x,0)] = HOT_START;
        plate2[LOC(x,0)] = HOT_START;
        /*printf("Column %d in row 0\n", LOC(x,0));*/
        /* do the left edge */
        plate1[LOC(0,x)] = COLD_START;
        plate2[LOC(0,x)] = COLD_START;
        /*printf("Row %d in column 0\n", LOC(x,0));*/
        /* do the right edge */
        plate1[LOC(PLATE_SIZE-1,x)] = COLD_START;
        plate2[LOC(PLATE_SIZE-1,x)] = COLD_START;
        /*printf("Row %d in column %d\n", LOC(x,0),PLATE_SIZE-1);*/
    }

    /* initialize our hot row */
    for(x = 0; x < FIXED_ROW_COL; x++) {
        plate1[LOC(x,FIXED_ROW)] = HOT_START;
        plate2[LOC(x,FIXED_ROW)] = HOT_START;
    }

    /* initialize our lonely hot dot */
    plate1[LOC(DOT_X,DOT_Y)] = HOT_START;
    plate2[LOC(DOT_X,DOT_Y)] = HOT_START;

    steadyState = FALSE;
    /* update over the plate until we reach a steady state */
    while(!steadyState) {
        /*printf("Hello from %d\n", __LINE__);*/
        switch(plateStatus) {
            case PLATE1_NEW:
                /*printf("Hello from %d\n", __LINE__);*/
                steadyState = updatePlate(plate1, plate2);
                plateStatus = PLATE1_OLD;
                break;
            case PLATE1_OLD:
                /*printf("Hello from %d\n", __LINE__);*/
                steadyState = updatePlate(plate2, plate1);
                plateStatus = PLATE1_NEW;
                break;
        }
        /*printf("Hello from %d\n", __LINE__);*/
        numIterations++;
        /*
        if(numIterations % 100 == 0) {
            printf("%d...\t", numIterations);
        }
        */
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
 * update the current plate using the last one as reference
 */
BOOL updatePlate(float *curPlate,
        float *newPlate) {
    int x;
    int y;
    float me;
    float neighborAvg;
    int threadID;
    BOOL isSteady = TRUE;

    /*printf("Hello from %d\n", __LINE__);*/
    /* update the new plate with temps from the old plate */
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
    /*printf("Hello from %d\n", __LINE__);*/

    /* check the new plate for steady state */
    for(y = 1; y < PLATE_SIZE - 1; y++) {
        for(x = 1; x < PLATE_SIZE - 1; x++) {
            if(!isFixed(x,y)) {
                me = newPlate[LOC(x,y)];
                /*printf("Hello from %d\n", __LINE__);*/
                neighborAvg = (newPlate[LEFT_LOC(x,y)]
                        + newPlate[RIGHT_LOC(x,y)]
                        + newPlate[LOWER_LOC(x,y)]
                        + newPlate[UPPER_LOC(x,y)])/4;
                /*printf("Hello from %d\n", __LINE__);*/
                if(fabsf(me - neighborAvg) >= STEADY_THRESHOLD) {
                    /*printf("Hello from %d, (x,y)=(%d,%d)\n", __LINE__,x,y);*/
                    isSteady = FALSE;
                }
                /*printf("Hello from %d\n", __LINE__);*/
            }
        }
    }
    /*printf("Hello from %d\n", __LINE__);*/

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

