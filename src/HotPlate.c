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
extern int calcWaitCount;
extern int checkWaitCount;
/* how to know if we're steady */
extern unsigned int notSteady;
extern int numIterations;

int main(int argc, char *argv[]) {
    double startTime;
    double endTime;
    double elapsedTime;
    int x;
    int y;
    numIterations = 0;

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

    /* initialize notSteady to fill all bits with 1 */
    notSteady = 1;
    int i;
    for(i = 1; i < nproc; i++) {
        notSteady <<= 1;
        notSteady += 1;
    }
    printf("not steady 0x%x\n", notSteady);

    /* set our counting "semaphores" to 0 initially */
    calcWaitCount = 0;
    checkWaitCount = 0;
    swapWaitCount = 0;

    /* set our barriers to 0 initially */
    GoCalc = 0;
    GoCheck = 0;
    GoSwap = 0;

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
    pthread_t threads[nproc];
    threadArgs = calloc(nproc, sizeof(ThreadArg));
    for(num = 0; num < nproc; num++) {
        printf("making thread %d...\n", num);
        threadArgs[num].iproc = num;
        threadArgs[num].nproc = nproc;
        err = pthread_create(&threads[num], /* pthread_t */
                &threadAttr,                /* thread attributes */
                findSteadyState,            /* thread main loop */
                &(threadArgs[num]));          /* argument struct */
        if(err) {
            printf("error %d returned on thread %d\n", err, num);
        }
    }

    /* free our attribute struct and wait for the workers to finish */
    pthread_attr_destroy(&threadAttr);
    void *returnStatus;
    for(num = 0; num < nproc; num++) {
        err = pthread_join(threads[num], &returnStatus);
        if(err) {
            printf("error %d joining thread %d\n", err, num);
            exit(-1);
        }
        printf("joined thread %d with status %d\n", num, (int)returnStatus);
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
    /*
    int x;
    int y;
    float me;
    float neighborAvg;
    */
    BOOL isSteady = TRUE;

    /*PRINT_LINE;*/
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
 * find the steady state of the plate
 */
void* findSteadyState(void *arg) {
    ThreadArg *myArg = arg;
    int iproc = (*myArg).iproc;
    int nproc = (*myArg).nproc;
    BOOL isSteady;
    int myBit;
    int x;
    int y;
    float me;
    float neighborAvg;
    printf("thread %d checking in...\n", iproc);
    /* figure out the rows that we start and end on */
    int start = CHUNK_START(iproc,nproc);
    int end = CHUNK_END(iproc,nproc);
    /* figure out which bit we'll be watching in the barrier */
    myBit = 1 << iproc;
    printf("thread %d says my bit is 0x%04x\n", iproc,myBit);

    /* work until everything is steady */
    while(notSteady) {
        /* thread 0 will keep track of our iteration count */
        if(iproc == 0) {
            numIterations++;
            if(numIterations % 100 == 0) {
                printf("%d says numIterations is %d\n", iproc, numIterations);
            }
        }
        /* calculate the changes in our chunk of the plate */
        printf("%d starting calcuations...\n", iproc);
        for(y = start; y < end; y++) {
            for(x = 1; x < PLATE_SIZE; x++) {
                if(!isFixed(x,y)) {
                    newPlate[LOC(x,y)] = (curPlate[LEFT_LOC(x,y)]
                            + curPlate[RIGHT_LOC(x,y)]
                            + curPlate[LOWER_LOC(x,y)]
                            + curPlate[UPPER_LOC(x,y)]
                            + 4 * curPlate[LOC(x,y)] ) / 8;
                }
            }
        }
        /* wait for everyone before we check steadiness */
        checkWaitCount++;
        printf("%d waiting to check...\n", iproc);
        while((myBit & GoCheck) == 0) {
            /* if I'm the last one in, tell everyone they can go */
            if(checkWaitCount == nproc) {
                int go = 1;
                int i;
                for(i = 1; i < nproc; i++) {
                    go <<= 1;
                    go += 1;
                }
                /* reset GoSwap and swapWaitCount to 0 so we wait after checking */
                GoSwap = 0;
                /*swapWaitCount = 0;*/
                checkWaitCount = 0;
                printf("telling everyone they can go check for steady...\n");
                GoCheck = go;
            }
        }

        /* check for steady state... */
        isSteady = TRUE;
        printf("%d starting check...\n", iproc);
        for(y = start; y < end; y++) {
            for(x = 1; x < PLATE_SIZE; x++) {
                if(!isFixed(x,y)) {
                    me = newPlate[LOC(x,y)];
                    neighborAvg = (newPlate[LEFT_LOC(x,y)]
                            + newPlate[RIGHT_LOC(x,y)]
                            + newPlate[LOWER_LOC(x,y)]
                            + newPlate[UPPER_LOC(x,y)])/4;
                    if(fabsf(me - neighborAvg) >= STEADY_THRESHOLD) {
                        isSteady = FALSE;
                        break; /* no need to keep checking... */
                    }
                }
            }
            if(!isSteady) {
                break; /* no need to keep checking */
            }
        }
        /* if my chunk is steady, set my bit to 0 */
        if(isSteady) {
            notSteady ^= myBit;
        }
        /* otherwise, make sure my bit still indicates not steady */
        else {
            notSteady |= myBit;
        }

        /* wait for everyone before we swap plate pointers */
        swapWaitCount++;
        printf("%d waiting for the plate pointers to be swapped...\n", iproc);
        while((myBit & GoSwap) == 0) {
            /* if we're the last one here, swap the pointers, reset GoCheck to
               0, and tell everyone they can go */
            if(swapWaitCount == nproc) {
                printf("swapping plate pointers...\n");
                tPlate = curPlate;
                curPlate = newPlate;
                newPlate = tPlate;
                int go = 1;
                int i;
                for(i = 1; i < nproc; i++) {
                    go <<= 1;
                    go += 1;
                }
                /* reset GoCheck and checkWaitCount to 0 so we wait
                   after calculating */
                GoCheck = 0;
                /*checkWaitCount = 0;*/
                swapWaitCount = 0;
                printf("telling everyone they can go calculate...\n");
                GoSwap = go;
            }
        }

    } /* end of while notSteady loop */

    pthread_exit(NULL);
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

