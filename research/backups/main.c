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

#define NONE 0
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
} point;

tracer* EMSCRIPTEN_KEEPALIVE newTracer(int r, int c, int d);
void EMSCRIPTEN_KEEPALIVE getCell(tracer* t, int* r, int* c, int dir);
void EMSCRIPTEN_KEEPALIVE turnTracer(tracer* t, int dir);
void EMSCRIPTEN_KEEPALIVE updateTracer(tracer* t, int moveDir, int turnDir);


void EMSCRIPTEN_KEEPALIVE setUp();
float** EMSCRIPTEN_KEEPALIVE getGrid();
int outOfBounds(int r, int c);
point* EMSCRIPTEN_KEEPALIVE getPolygons(float compVal);
void EMSCRIPTEN_KEEPALIVE printGrid();

void EMSCRIPTEN_KEEPALIVE printMatrix(unit_size mat[HEIGHT][WIDTH]);
float** cellMap;//[HEIGHT][WIDTH];

int main(int argc, char** argv){
	setUp();
	printf("Module loaded and set up\n");
	#ifdef NOWASM
	//printGrid();
	getPolygons(10);
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
		    
            if (r == 0 || c == 0 || r == HEIGHT-1 || c == WIDTH-1){
                cellMap[r][c] = 0;
                continue;
            }
			cellMap[r][c] = 5*(sin(r/20)+1)*(sin(c/30)+1);
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
	t->d = (t->d+dir)%8;
}

void updateTracer(tracer* t, int moveDir, int turnDir){
    int rNew, cNew;
    getCell(t, &rNew, &cNew, moveDir);
    t->r = rNew;
    t->c = cNew;
    turnTracer(t, turnDir);
    DEBUG(printf("Updated to (r,c) = (%d, %d) dir = %d\n",t->r, t->c, t->d));
}

point* EMSCRIPTEN_KEEPALIVE getPolygons(float compVal){
	int r,c;
	point* seqList = (point*)malloc(sizeof(point)*WIDTH*HEIGHT);
	DEBUG(printf("seqList starts at: %p\n", seqList));
	int seqIdx = 0;
	int labelCount = 0;
	// create a table of labels to remap certain values if needed
    memset(seqList, 0, sizeof(point)*WIDTH*HEIGHT);
	// A new grid of the same size to keep track of what values are where. It is defaulted to all 1s (111...111)
	// but will be updated once touched.
	unit_size gridCpy[HEIGHT][WIDTH];
	memset(gridCpy, ALL_ONES, sizeof(unit_size)*HEIGHT*WIDTH);
	
	for (r = 0; r < HEIGHT; r++){
		for (c = 0; c < WIDTH; c++){
			DEBUG(printf("{r=%d, c=%d} -> seqIdx = %d\n", r, c, seqIdx));
			if (gridCpy[r][c] != ALL_ONES){
				DEBUG(printf("Skipping r, c = %d, %d\n", r, c));
				continue;
			}
			DEBUG(printf("Checking %d, %d\n", r, c));
			// Does this cell qualify?
			if (cellMap[r][c] < compVal){
				gridCpy[r][c] = 0;
			    continue;
			}
			    
		    DEBUG(printf("\tFound->%f\n", cellMap[r][c]));
			gridCpy[r][c] = ++labelCount;
			tracer* T = newTracer(r,c,E); // Tracer will start off facing east
			char fullCircle = 0;
			while (!fullCircle && seqIdx < WIDTH*HEIGHT){
			    DEBUG(printf("Tracer(%d) is at %d, %d, dir=%d\n", labelCount, T->r, T->c, T->d));
			    char oob = 0; // Out of Bounds
				// Stage 1
				// Get rear-left
				int rRL = 0, cRL = 0;
				getCell(T, &rRL, &cRL, REAR_LEFT);
				if (!outOfBounds(rRL, cRL)){
					// If P_left-rear = black
					if (cellMap[rRL][cRL] >= compVal){
						int rL, cL;
						getCell(T, &rL, &cL, LEFT);
						// If P_left = black
						if (cellMap[rL][cL] >= compVal){
        					seqList[seqIdx].y = T->r;
        					seqList[seqIdx++].x = T->c;
							updateTracer(T, LEFT, LEFT); // inner
							gridCpy[T->r][T->c] = labelCount;
							if (T->r == r && T->c == c) fullCircle = 1;
        					seqList[seqIdx].y = T->r;
        					seqList[seqIdx++].x = T->c;
							updateTracer(T, LEFT, LEFT);
				            gridCpy[T->r][T->c] = labelCount;
						}
						else{
        					seqList[seqIdx].y = T->r;
        					seqList[seqIdx++].x = T->c;
						    updateTracer(T, REAR_LEFT, REAR); // inner-outer
				            gridCpy[T->r][T->c] = labelCount;
						}
					}
					else{
						int rL, cL;
						getCell(T, &rL, &cL, LEFT);
						// If P_left = black
						if (cellMap[rL][cL] >= compVal){
        					seqList[seqIdx].y = T->r;
        					seqList[seqIdx++].x = T->c;
						    updateTracer(T, LEFT, LEFT); // straight
				            gridCpy[T->r][T->c] = labelCount;
						}
						else{
							// Outer
						}
					}
				}
				else {
				    oob++;
				}
				if (T->r == r && T->c == c) fullCircle = 1;
				// Stage 2
				// get front-left and front cells
				int rFL = 0, cFL = 0, rF = 0, cF = 0;
				getCell(T, &rFL, &cFL, FRONT_LEFT);
				getCell(T, &rF, &cF, FRONT);
				// Ensure the points are in table
				if (!outOfBounds(rFL, cFL)){
					// If the front-left cell is black
					if (cellMap[rFL][cFL] >= compVal){
						// If front is also black
						if (cellMap[rF][cF] >= compVal){
        					seqList[seqIdx].y = T->r;
        					seqList[seqIdx++].x = T->c;
							updateTracer(T, FRONT, LEFT); // inner
							gridCpy[T->r][T->c] = labelCount;
							if (T->r == r && T->c == c) fullCircle = 1;
        					seqList[seqIdx].y = T->r;
        					seqList[seqIdx++].x = T->c;
							updateTracer(T, FRONT, RIGHT);
				            gridCpy[T->r][T->c] = labelCount;
						}
						else{
        					seqList[seqIdx].y = T->r;
        					seqList[seqIdx++].x = T->c;
						    updateTracer(T, FRONT_LEFT, NONE); // inner-outer
				            gridCpy[T->r][T->c] = labelCount;
						}
						
					}
					// else if the front cell is black
					else if (cellMap[rF][cF] >= compVal){
    					seqList[seqIdx].y = T->r;
    					seqList[seqIdx++].x = T->c;
					    updateTracer(T, FRONT, RIGHT);
				        gridCpy[T->r][T->c] = labelCount;
					}
					else{
						turnTracer(T, REAR); // outer
						seqIdx--;
					}
				}
				else if (oob){ printf("breaking out\n"); break; }
				
			    if (seqIdx >= WIDTH*HEIGHT){
			        break;
			    }
				// Check if the loop should break
				if ((T->r == r) && (T->c == c)) fullCircle = 1;
			}
			seqList[seqIdx].y = T->r;
			seqList[seqIdx++].x = T->c;
			DEBUG(printf("Sequence:%d\n", labelCount));
			
			/******* FILL IN POLYGON  WITH LABELCOUNT *****/
			int i;
			int minx = seqList[0].x;
			int miny = seqList[0].y;
			int maxx = seqList[0].x;
			int maxy = seqList[0].y;
			for (i = 1; i < seqIdx; i++){
			    if (seqList[i].x < minx) minx = seqList[i].x;
			    if (seqList[i].x > maxx) maxx = seqList[i].x;
			    if (seqList[i].y < miny) miny = seqList[i].y;
			    if (seqList[i].y > maxy) maxy = seqList[i].y;
			}
			
			int subr,subc;
        	for (subr = miny; subr < maxy; subr++){
        	    char filling = 0;
        		for (subc = minx; subc < maxx; subc++){
        		    if (gridCpy[subr][subc] == labelCount){
        		        filling = !filling;
        		    }
        		    else if (filling && gridCpy[subr][subc] == ALL_ONES){
        		        gridCpy[subr][subc] = labelCount;
        		    }
        		}
        	}
		}
	}
    DEBUG(printMatrix(gridCpy));
    /*
    if (seqIdx >= WIDTH*HEIGHT){
        int i;
        printf("ERROR seqIdx went too far:\n");
        for (i = WIDTH*HEIGHT-100; i < WIDTH*HEIGHT; i++){
            printf("%10d: {x=%3d,y=%3d}\n", i, seqList[i].x, seqList[i].y);
        }
    }*/
	return seqList;
}

void EMSCRIPTEN_KEEPALIVE printGrid(){
	int r,c;
	for (r = 0; r < HEIGHT; r++){
		for (c = 0; c < WIDTH; c++){
			printf("%2d ",(int)(cellMap[r][c]));
		}
		printf("\n");
	}
	printf("\n\n");
}

void EMSCRIPTEN_KEEPALIVE printMatrix(unit_size mat[HEIGHT][WIDTH]){
	int r,c;
	for (r = 0; r < HEIGHT; r++){
		for (c = 0; c < WIDTH; c++){
			printf("%2d ", mat[r][c]);
// 			printf("%c", mat[r][c] == ALL_ONES?'x':mat[r][c]==0?'-':'#');
		}
		printf("\n");
	}
	printf("\n\n");
}

