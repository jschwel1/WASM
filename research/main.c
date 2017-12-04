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

#define HEIGHT 100 // height and width of the grid in pixels (max 1 cell per pixel)
#define WIDTH 100

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
char qualifies(float val, float compVal);

void EMSCRIPTEN_KEEPALIVE printMatrix(unit_size mat[HEIGHT][WIDTH]);
float** cellMap;//[HEIGHT][WIDTH];

int main(int argc, char** argv){
	setUp();
	quantize();
	DEBUG(printGrid());
	printf("Module loaded and set up\n");
	#ifdef NOWASM
	//printGrid();
	getPolygons();
	#endif
	return 0;
}

void EMSCRIPTEN_KEEPALIVE setUp(){
	cellMap = (float**)malloc(sizeof(float*)*HEIGHT);
	int r, c;
	for (r = 0; r < HEIGHT; r++){
		cellMap[r] = malloc(sizeof(float)*WIDTH);
	}
	
	for (r = 0; r < HEIGHT; r++){
		for (c = 0; c < WIDTH; c++){
            cellMap[r][c] = 0;
		}
	}

	for (r = HEIGHT/3; r < 2*HEIGHT/3; r++){
		for (c = WIDTH/4; c < 3*WIDTH/4; c++){
		    cellMap[r][c] = 25;
		}
	}

/*
	for (r = 0; r < HEIGHT; r++){
		for (c = 0; c < WIDTH; c++){

            if (r == 0 || c == 0 || r == HEIGHT-1 || c == WIDTH-1){
                cellMap[r][c] = 0;
                continue;
            }
			cellMap[r][c] += 5*(sin(r/30)+1)*(sin(c/30)+1);
			cellMap[r][c] += 5*(sin(r/15))*(sin(c/15));

		}
	}
*/
	for (r = 1; r < HEIGHT-1; r++){
		for (c = 1; c < WIDTH-1; c++){
            cellMap[r][c] = 0;
		}
	}
// 	for (r = HEIGHT/2; r < 2*HEIGHT/3; r++){
// 		for (c = WIDTH/2; c < 3*WIDTH/4; c++){
// 		    cellMap[r][c] = 3;
// 		}
// 	}
	
	for (r = HEIGHT/3; r < 2*HEIGHT/3; r++){
		for (c = WIDTH/4; c < 3*WIDTH/4; c++){
		    cellMap[r][c] = 25;
		}
	}
	
	for (r = 2; r < 5; r++){
	    for (c= 2; c < 9; c++){
	        cellMap[r][c] = 50;
	    }
	}
	cellMap[3][3] = 1;
	cellMap[3][4] = 1;
	cellMap[3][5] = 1;
	cellMap[3][6] = 1;
	cellMap[3][7] = 1;
	
	float angle;
	int radius;
	for (radius = 5; radius < 20; radius++){
    	for (angle = 0; angle < 2*M_PI; angle+=0.001){
    	    cellMap[(int)(radius*sin(angle))+HEIGHT/3][(int)(radius*cos(angle))+WIDTH/3] = 20-radius;
    	}
	}
    


}

void EMSCRIPTEN_KEEPALIVE quantize(){
    int r, c;
    for (r = 0; r < HEIGHT; r++){
        for (c = 0; c < WIDTH; c++){
            cellMap[r][c] = (int)((cellMap[r][c])/10);
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
    int rNew, cNew;
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
    if (qualifies(cellMap[rDir][cDir], compVal)) return 1;
    return 0;
}

point* EMSCRIPTEN_KEEPALIVE getPolygons(){
	int r,c;
	// Keep track of each point in the list.
	point* seqList = (point*)malloc(sizeof(point)*WIDTH*HEIGHT);
	point* seqListPtr = seqList;
	int labelCount = 0;
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
			if (gridCpy[r][c] != ALL_ONES){
			    prevCell = cellMap[r][c];
				continue;
			}
			// Does this cell qualify? What value is it encircling?
			thisCell = cellMap[r][c];
			
			if (thisCell != prevCell){
			    encirclingVal = thisCell;
			 //   DEBUG(printf("This cell"))
			}
			else{
			    prevCell = thisCell;
			    gridCpy[r][c] = 0;
			    continue;
			}
			
            DEBUG(printf("Starting at (r, c) = (%d, %d) -- (prev)%d!=%d(this) and gridCpy[%d][%d]==%d \n", r, c, prevCell, thisCell, r, c, gridCpy[r][c]));
			tracer* T = newTracer(r, c, E);
			int rR, cR, rL, cL, rRL, cRL;
			getCell(T, &rR, &cR, REAR_LEFT);
			getCell(T, &rL, &cL, REAR_LEFT);
			getCell(T, &rRL, &cRL, REAR_LEFT);
			// A starting assumption for the algorthm is that if the rear and left pixels are white,
			// the rear-left one must also be white for the stopping criteria to work.
			if (!qualifies(cellMap[rR][cR], encirclingVal) && !qualifies(cellMap[rL][cL], encirclingVal))
			    cellMap[rRL][cRL] = encirclingVal-1;
			gridCpy[r][c] = encirclingVal;
			char fullCircle = 0;
		    DEBUG(printf("Tracer(%d) is at %d, %d, dir=%d, enc=%d\n", labelCount, T->r, T->c, T->d, encirclingVal));
			while (!fullCircle){
			    int caseUsed = 0;
			    // Stage 1
				if (isQualified(T,REAR_LEFT, encirclingVal)){
				    if (isQualified(T, LEFT, encirclingVal)){
				        // Case 1: "Inner"
				        DEBUG(printf("Case 1: \n"));
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
			
			DEBUG(printf("Finished outline: (r, c) = (%d, %d)\n", T->r, T->c));
            DEBUG(printMatrix(gridCpy));
			addPoint(seqListPtr++, r, c, thisCell, 0);
			
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
			printf("%d,",(int)(cellMap[r][c]));
			#else
			printf("%2d ",(int)(cellMap[r][c]));
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
// 			printf("%d ", mat[r][c] == ALL_ONES?0:mat[r][c]);
			printf("%c", mat[r][c] == ALL_ONES?'x':mat[r][c]==0?'-':'#');
		}
		printf("\n");
	}
	printf("\n\n");
}

/********************** NOTES *************************/
/*

Make sure to check off used values in gridcpy












*/
