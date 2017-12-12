#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef NOWASM
#include <emscripten/emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

#ifdef DEBUGGING
#define DEBUG(...) (__VA_ARGS__)
#else
#define DEBUG(...)
#endif

#define unit_size uint16_t
#define ALL_ONES 0xFFFF

#define HEIGHT 200 // height and width of the grid in pixels (max 1 cell per pixel)
#define WIDTH 200

#define DIFF_CONST .1

#define NONE 15
#define FRONT 0
#define FRONT_RIGHT 1
#define RIGHT 2
#define REAR_RIGHT 3
#define REAR 4
#define REAR_LEFT 5
#define LEFT 6
#define FRONT_LEFT 7
#define N 0
#define NE 1
#define E 2
#define SE 3
#define S 4
#define SW 5
#define W 6
#define NW 7

typedef struct {
    int r;
    int c;
    int d;
} tracer;

typedef struct {
    int x;
    int y;
    int val;
    int depth;
} point;

tracer* EMSCRIPTEN_KEEPALIVE newTracer(int r, int c, int d);
void EMSCRIPTEN_KEEPALIVE getCell(tracer* t, int* r, int* c, int dir);
void EMSCRIPTEN_KEEPALIVE turnTracer(tracer* t, int dir);
void EMSCRIPTEN_KEEPALIVE updateTracer(tracer* t, int moveDir, int turnDir, point** ptr, int enc, int d);


void EMSCRIPTEN_KEEPALIVE setUp();
void EMSCRIPTEN_KEEPALIVE quantize();
float** EMSCRIPTEN_KEEPALIVE getGrid();
int outOfBounds(int r, int c);
point* EMSCRIPTEN_KEEPALIVE getPolygons();
void EMSCRIPTEN_KEEPALIVE printGrid();
void EMSCRIPTEN_KEEPALIVE printQGrid();
char qualifies(float val, float compVal);
void EMSCRIPTEN_KEEPALIVE diffuse();

void EMSCRIPTEN_KEEPALIVE printMatrix(unit_size mat[HEIGHT][WIDTH]);
float** concentrationMap;//[HEIGHT][WIDTH];
int** quantizedCM;

int main(int argc, char** argv){
    setUp();
    quantize();
    DEBUG(printQGrid());
    printf("Module loaded and set up\n");
    #ifdef NOWASM
    printGrid();
    getPolygons();
    int i;
    for (i = 0; i < atoi(argv[1]); i++){
        diffuse();
        // printGrid();
    }
    printGrid();
    #endif
    return 0;
}

void EMSCRIPTEN_KEEPALIVE setUp(){
    // Set up concentrationMap and quantizeCM
    concentrationMap = (float**)malloc(sizeof(float*)*HEIGHT);
    quantizedCM = (int**)malloc(sizeof(int*)*HEIGHT);
    int r, c;
    for (r = 0; r < HEIGHT; r++){
        concentrationMap[r] = malloc(sizeof(float)*WIDTH);
        quantizedCM[r] = malloc(sizeof(int)*WIDTH);
    }
    // Clear the concentrationMap
    for (r = 0; r < HEIGHT; r++){
        for (c = 0; c < WIDTH; c++){
            concentrationMap[r][c] = 0;
        }
    }

    // 1/3 -> 2/3 vertically & 1/4 -> 3/4 horizontally rectangle of 25
    for (r = HEIGHT/3; r < 2*HEIGHT/3; r++){
        for (c = WIDTH/4; c < 3*WIDTH/4; c++){
            concentrationMap[r][c] = 25;
        }
    }



//     for (r = 1; r < HEIGHT-1; r++){
//         for (c = 1; c < WIDTH-1; c++){
//             concentrationMap[r][c] = 0;
//         }
//     }
//     for (r = HEIGHT/2; r < 2*HEIGHT/3; r++){
//         for (c = WIDTH/2; c < 3*WIDTH/4; c++){
//             concentrationMap[r][c] = 3;
//         }
//     }
    
//     for (r = HEIGHT/3; r < 2*HEIGHT/3; r++){
//         for (c = WIDTH/4; c < 3*WIDTH/4; c++){
//             concentrationMap[r][c] = 25;
//         }
//     }
    
//     for (r = 2; r < 5; r++){
//         for (c= 2; c < 9; c++){
//             concentrationMap[r][c] = 50;
//         }
//     }
//     concentrationMap[3][3] = 1;
//     concentrationMap[3][4] = 1;
//     concentrationMap[3][5] = 1;
//     concentrationMap[3][6] = 1;
//     concentrationMap[3][7] = 1;
    
    // float angle;
    // int radius;
    // for (radius = 5; radius < 20; radius++){
    //     for (angle = 0; angle < 2*M_PI; angle+=0.001){
    //         concentrationMap[(int)(radius*sin(angle))+HEIGHT/3][(int)(radius*cos(angle))+WIDTH/3] = 100-radius;
    //     }
    // }
    
//     for (r = 0; r < HEIGHT; r++){
//         for (c = 0; c < WIDTH; c++){

//             if (r == 0 || c == 0 || r == HEIGHT-1 || c == WIDTH-1){
//                 concentrationMap[r][c] = 0;
//                 continue;
//             }
//             concentrationMap[r][c] += 5*(sin(r/30)+1)*(sin(c/30)+1);
//             concentrationMap[r][c] += 5*(sin(r/15))*(sin(c/15));

//         }
//     }


}

void EMSCRIPTEN_KEEPALIVE quantize(){
    int r, c;
    for (r = 0; r < HEIGHT; r++){
        for (c = 0; c < WIDTH; c++){
            quantizedCM[r][c] = (int)((concentrationMap[r][c])/10);
        }
    }
}


int EMSCRIPTEN_KEEPALIVE outOfBounds(int r, int c){
    if ((r < 0) || (r >= HEIGHT) || (c < 0) || (c >= WIDTH)) return 1;
    return 0;
}

tracer* newTracer(int r, int c, int d){
    tracer* t = (tracer*)malloc(sizeof(tracer));
    t->r = r;
    t->c = c;
    t->d = d;
    return t;
}

void getCell(tracer* t, int* r, int* c, int dir){
    int abs = (dir + t->d)%8;
    switch(abs){
        case N:
            *r = t->r-1;
            *c = t->c;
            return;
        case NE:
            *r = t->r-1;
            *c = t->c+1;
            return;
        case E:
            *r = t->r;
            *c = t->c+1;
            return;
        case SE:
            *r = t->r+1;
            *c = t->c+1;
            return;
        case S:
            *r = t->r+1;
            *c = t->c;
            return;
        case SW:
            *r = t->r+1;
            *c = t->c-1;
            return;
        case W:
            *r = t->r;
            *c = t->c-1;
            return;
        case NW:
            *r = t->r-1;
            *c = t->c-1;
            return;
    }
}

void turnTracer(tracer* t, int dir){
    if (dir != NONE)
        t->d = (t->d+dir)%8;
}

void addPoint(point* ptr, int r, int c, int enc, int depth){
    ptr->x = c;
    ptr->y = r;
    ptr->val = enc;
    ptr->depth = depth;
}

void updateTracer(tracer* t, int moveDir, int turnDir, point** seqPtr, int enc, int d){
    int rNew = 0, cNew = 0;
    addPoint(*seqPtr, t->r, t->c, enc, d);
    if (moveDir != NONE){
        getCell(t, &rNew, &cNew, moveDir);
        t->r = rNew;
        t->c = cNew;
    }
    turnTracer(t, turnDir);
    (*seqPtr)++;
    DEBUG(printf("Updated to (r,c) = (%d, %d) dir = %d, seqPtr=%p\n",t->r, t->c, t->d, *seqPtr));
    return;
}

char isQualified(tracer* T, char d, int compVal){
    int rDir = 0, cDir = 0;
    getCell(T, &rDir, &cDir, d);
    if (outOfBounds(rDir, cDir)) return 0;
    if (qualifies(quantizedCM[rDir][cDir], compVal)) return 1;
    return 0;
}

point* EMSCRIPTEN_KEEPALIVE getPolygons(){
    quantize();
    int r,c;
    // Keep track of each point in the list.
    point* seqList = (point*)malloc(sizeof(point)*WIDTH*HEIGHT);
    point* seqListPtr = seqList;
    // create a table of labels to remap certain values if needed
    memset(seqList, 0, sizeof(point)*WIDTH*HEIGHT);
    // A new grid of the same size to keep track of what values are where. It is defaulted to all 1s (111...111)
    // but will be updated once touched.
    unit_size gridCpy[HEIGHT][WIDTH];
    memset(gridCpy, ALL_ONES, sizeof(unit_size)*HEIGHT*WIDTH);
    char prevCell, thisCell;
    int encirclingVal;
    // ignore edges while looping through all cells
    for (r = 1; r < HEIGHT-1; r++){
        prevCell = 0;
        for (c = 1; c < WIDTH-1; c++){
            // If this cell has already been seen (in a trace), ignore it
            if (gridCpy[r][c] != ALL_ONES){
                prevCell = quantizedCM[r][c];
                continue;
            }
            // Does this cell qualify? What value is it encircling?
            thisCell = quantizedCM[r][c];
            
            if (thisCell != prevCell){
                encirclingVal = thisCell;
            }
            else{
                prevCell = thisCell;
                gridCpy[r][c] = 0;
                continue;
            }
            
            // Save pointer incase this is an interior trace;
            point* startSeqListPtr = seqListPtr;
            int interior = 0;
            
            DEBUG(printf("Starting at (r, c) = (%d, %d) -- (prev)%d!=%d(this) and gridCpy[%d][%d]==%d \n", r, c, prevCell, thisCell, r, c, gridCpy[r][c]));
            tracer* T = newTracer(r, c, E);
            int rR, cR, rL, cL, rRL, cRL;
            getCell(T, &rR, &cR, REAR_LEFT);
            getCell(T, &rL, &cL, REAR_LEFT);
            getCell(T, &rRL, &cRL, REAR_LEFT);
            // A starting assumption for the algorthm is that if the rear and left pixels are white,
            // the rear-left one must also be white for the stopping criteria to work.
            if (!qualifies(quantizedCM[rR][cR], encirclingVal) && !qualifies(quantizedCM[rL][cL], encirclingVal))
                quantizedCM[rRL][cRL] = encirclingVal-1;
            gridCpy[r][c] = encirclingVal;
            char fullCircle = 0;
            DEBUG(printf("Tracer is at %d, %d, dir=%d, enc=%d\n", T->r, T->c, T->d, encirclingVal));
            while (!fullCircle){
                int caseUsed = 0;
                // Stage 1
                if (isQualified(T,REAR_LEFT, encirclingVal)){
                    if (isQualified(T, LEFT, encirclingVal)){
                        // Case 1: "Inner"
                        DEBUG(printf("Case 1: \n"));
                        interior++;
                        updateTracer(T, LEFT, LEFT, &seqListPtr, encirclingVal, 0);
                        // gridCpy[T->r][T->c] = encirclingVal;
                        if (T->r == r && T->c == c){
                            DEBUG(printf("FULL_CIRCLE-1"));
                            fullCircle = 1;
                            // continue;
                        }
                        updateTracer(T, LEFT, LEFT, &seqListPtr, encirclingVal, 0);
                        gridCpy[T->r][T->c] = encirclingVal;
                    }
                    else{
                        // Case 2: "Inner-outer"
                        DEBUG(printf("Case 2: \n"));
                        updateTracer(T, REAR_LEFT, REAR, &seqListPtr, encirclingVal, 0);
                        gridCpy[T->r][T->c] = encirclingVal;
                    }
                }
                else{
                    if (isQualified(T, LEFT, encirclingVal)){
                        // Case 3: "Straight"
                        DEBUG(printf("Case 3: \n"));
                        updateTracer(T, LEFT, LEFT, &seqListPtr, encirclingVal, 0);
                        gridCpy[T->r][T->c] = encirclingVal;
                    }
                    else{
                        // Case 4: "Outer"
                        interior--;
                        DEBUG(printf("Case 4: \n"));
                        caseUsed = 4;
                    }
                }
                if (fullCircle) break;
                // Double check the tracer didn't go full-circle
                if (caseUsed != 4 && T->r == r && T->c == c){
                    DEBUG(printf("FULL_CIRCLE-2"));
                    fullCircle = 1;
                    break;
                }
                
                // Stage 2
                if (isQualified(T, FRONT_LEFT, encirclingVal)){
                    if (isQualified(T, FRONT, encirclingVal)){
                        // Case 6: "Inner"
                        interior++;
                        DEBUG(printf("Case 6: \n"));
                        updateTracer(T, FRONT, LEFT, &seqListPtr, encirclingVal, 0);
                        // gridCpy[T->r][T->c] = encirclingVal;
                        if (T->r == r && T->c == c){
                            DEBUG(printf("FULL_CIRCLE-3"));
                            fullCircle = 1;
                        }
                        updateTracer(T, FRONT, RIGHT, &seqListPtr, encirclingVal, 0);
                        gridCpy[T->r][T->c] = encirclingVal;
                    }
                    else{
                        // Case 5: "Inner-Outer"x2
                        DEBUG(printf("Case 5: \n"));
                        updateTracer(T, FRONT_LEFT, NONE, &seqListPtr, encirclingVal, 0);
                        gridCpy[T->r][T->c] = encirclingVal;
                    }
                }
                else if (isQualified(T, FRONT, encirclingVal)){
                    // Case 7:
                    DEBUG(printf("Case 7: \n"));
                    updateTracer(T, FRONT, RIGHT, &seqListPtr, encirclingVal, 0);
                    gridCpy[T->r][T->c] = encirclingVal;
                }
                else {
                    // Case 8: "Outer"
                    interior--;
                    DEBUG(printf("Case 8: \n"));
                    updateTracer(T, NONE, REAR, &seqListPtr, encirclingVal, 0);
                    gridCpy[T->r][T->c] = encirclingVal;
                }
                // Check if the loop should break
                if ((T->r == r) && (T->c == c)){
                    DEBUG(printf("FULL_CIRCLE-4"));
                    fullCircle = 1;
                }
                prevCell = thisCell;
            }
            // If this was an interior trace, remove it
            if (interior > 0){
                point* temp = startSeqListPtr;
                while (temp < seqListPtr){
                    addPoint(temp++, 0, 0, 0, 0);
                }
                seqListPtr = startSeqListPtr;
            }
            else {
                DEBUG(printf("Finished outline: (r, c) = (%d, %d)\n", T->r, T->c));
                DEBUG(printMatrix(gridCpy));
                addPoint(seqListPtr++, r, c, thisCell, 0);
            }
            
        }
    }
    DEBUG(printMatrix(gridCpy));
    // int i;
    // for (i = 0; i < WIDTH*HEIGHT; i++) {printf("(%d, %d)\n", seqList[i].y, seqList[i].x);};
    return seqList;
}

inline char qualifies(float val, float compVal){
    if (val == compVal) return 1; // true
    return 0; // false
}

void EMSCRIPTEN_KEEPALIVE printGrid(){
    int r,c;
    for (r = 0; r < HEIGHT; r++){
        for (c = 0; c < WIDTH; c++){
            #ifdef DEBUGGING
            printf("%d,",(int)(concentrationMap[r][c]));
            #else
            printf("%2d ",(int)(concentrationMap[r][c]));
            #endif
        }
        printf("\n");
    }
    printf("\n\n");
}
void EMSCRIPTEN_KEEPALIVE printQGrid(){
    int r,c;
    for (r = 0; r < HEIGHT; r++){
        for (c = 0; c < WIDTH; c++){
            #ifdef DEBUGGING
            printf("%d,",(int)(quantizedCM[r][c]));
            #else
            printf("%2d ",(int)(quantizedCM[r][c]));
            #endif
        }
        printf("\n");
    }
    printf("\n\n");
}

void EMSCRIPTEN_KEEPALIVE printMatrix(unit_size mat[HEIGHT][WIDTH]){
    int r,c;
    for (r = 0; r < HEIGHT; r++){
        for (c = 0; c < WIDTH; c++){
//             printf("%d ", mat[r][c] == ALL_ONES?0:mat[r][c]);
            printf("%c", mat[r][c] == ALL_ONES?'x':mat[r][c]==0?'-':'#');
        }
        printf("\n");
    }
    printf("\n\n");
}
void EMSCRIPTEN_KEEPALIVE printfMatrix(float mat[HEIGHT][WIDTH]){
    int r,c;
    for (r = 0; r < HEIGHT; r++){
        for (c = 0; c < WIDTH; c++){
            printf("%2.2f ", mat[r][c]);
        }
        printf("\n");
    }
    printf("\n\n");
}

float EMSCRIPTEN_KEEPALIVE linReg(float a, float b, float c){
    // sumX = 0 (= -1 + 0 + 1)
    // sumXX = 2
    // n = 3;
    // Equation from: https://www.mathportal.org/calculators/statistics-calculator/correlation-and-regression-calculator.php
    float sumXY = (c - a);
    return (3*sumXY)/(6); // the slope ONLY of the linear regression
}

void EMSCRIPTEN_KEEPALIVE diffuse(){
    // create a copies of concentrationMap for 2nd derrivatives in x & y
    float dr1[HEIGHT][WIDTH];
    float dc1[HEIGHT][WIDTH];
    float dr2[HEIGHT][WIDTH];
    float dc2[HEIGHT][WIDTH];
    int r,c;
    // Use line of best fit to estimate derrivative (2->3 pts if edge or not)
    // (Linear Regression). Do twice to get second derrivative, then plug into
    // Eq 10 to calculate diff in concentration/time
    // 1st derrivatives
    for (r = 0; r < HEIGHT; r++){
        for (c= 0; c < WIDTH; c++){
            // Edge cases use simple slope equation
            if (r == 0) {
                // rise/run
                dr1[r][c] = (concentrationMap[r+1][c] - concentrationMap[r][c])/2;
            }
            else if (r == HEIGHT-1) {
                // rise/run
                dr1[r][c] = (concentrationMap[r-1][c] - concentrationMap[r][c])/2;
            }
            else {
                // Linear regression
                dr1[r][c] = linReg(concentrationMap[r-1][c], concentrationMap[r][c], concentrationMap[r+1][c]);
            }
            
            if (c == 0) {
                // rise/run
                dc1[r][c] = (concentrationMap[r][c+1] - concentrationMap[r][c])/2;
            }
            else if (c == WIDTH-1) {
                // rise/run
                dc1[r][c] = (concentrationMap[r][c-1] - concentrationMap[r][c])/2;
            }
            else {
                // Linear regression
                dc1[r][c] = linReg(concentrationMap[r][c-1], concentrationMap[r][c], concentrationMap[r][c+1]);
            }
        }
    }
    // 2nd derrivatives
    for (r = 0; r < HEIGHT; r++){
        for (c= 0; c < WIDTH; c++){
            // Edge cases use simple slope equation
            if (r == 0) {
                // rise/run
                dr2[r][c] = (dr1[r+1][c] - dr1[r][c])/2;
            }
            else if (r == HEIGHT-1) {
                // rise/run
                dr2[r][c] = (dr1[r-1][c] - dr1[r][c])/2;
            }
            else {
                // Linear regression
                dr2[r][c] = linReg(dr1[r-1][c], dr1[r][c], dr1[r+1][c]);
            }
            
            if (c == 0) {
                // rise/run
                dc2[r][c] = (dc1[r][c+1] - dc1[r][c])/2;
            }
            else if (c == WIDTH-1) {
                // rise/run
                dc2[r][c] = (dc1[r][c-1] - dc1[r][c])/2;
            }
            else {
                // Linear regression
                dc2[r][c] = linReg(dc1[r][c-1], dc1[r][c], dc1[r][c+1]);
            }
        }
    }
    // printfMatrix(dc2);
    // printfMatrix(dr2);
    // Fick's Law: Flux = V/A, D = Diffusion const, delta(concentation)
    // dC/dt = D * (d2C_x/dx2 + d2C_y/dy2)
    for (r = 1; r < HEIGHT-2; r++){
        for (c= 1; c < WIDTH-2; c++){
            // concentrationMap[r][c] -= DIFF_CONST * (dr2[r][c] + dc2[r][c]);
            // concentrationMap[r+1][c+1] += DIFF_CONST * (dr2[r][c] + dc2[r][c]);
            concentrationMap[r][c] += DIFF_CONST*(dr2[r][c]+dc2[r][c]);
        }
    }
    float sum = 0.0;
    for (r = 1; r < HEIGHT-1; r++){
        for (c= 1; c < WIDTH-1; c++){
            sum+=concentrationMap[r][c];
        }
    }
    // printGrid();
}