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
char qualifies(float val, float compVal);

void EMSCRIPTEN_KEEPALIVE printMatrix(unit_size mat[HEIGHT][WIDTH]);
float** cellMap;//[HEIGHT][WIDTH];

int main(int argc, char** argv){
	setUp();
	DEBUG(printGrid());
	printf("Module loaded and set up\n");
	#ifdef NOWASM
	//printGrid();
	float cv = atof(argv[1]);
	getPolygons(cv);
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
		    cellMap[r][c] = 5;
		}
	}


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

	for (r = 1; r < HEIGHT-1; r++){
		for (c = 1; c < WIDTH-1; c++){
            cellMap[r][c] = 0;
		}
	}
	for (r = HEIGHT/2; r < 2*HEIGHT/3; r++){
		for (c = WIDTH/2; c < 3*WIDTH/4; c++){
		    cellMap[r][c] = 3;
		}
	}
	for (r = 2; r < 5; r++){
	    for (c= 2; c < 9; c++){
	        cellMap[r][c] = 5;
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
	char thisCell, prevCell;
	// ignore edges while looping through all cells
	for (r = 1; r < HEIGHT-1; r++){
	    prevCell = 0;
		for (c = 1; c < WIDTH-1; c++){
			DEBUG(printf("{r=%d, c=%d} -> seqIdx = %d\n", r, c, seqIdx));
			// Probably won't need this anymore
			if (gridCpy[r][c] != ALL_ONES){
			//	DEBUG(printf("Skipping r, c = %d, %d\n", r, c));
				continue;
			}
			DEBUG(printf("Checking %d, %d\n", r, c));
			// Does this cell qualify?
			thisCell = qualifies(cellMap[r][c], compVal);
			char boundary = 0;
			tracer* T;
			switch((prevCell << 1)|thisCell){
			    case 0: // No change
			    case 3: //
			        boundary = 1;
			        gridCpy[r][c] = 0;
			        break;
			    case 1: // 01
			        T = newTracer(r,c,E); // Tracer will start off facing east
			        break;
			    case 2: // 10
			        prevCell = 0;
			        c--;
			        T = newTracer(r,c,W); // Tracer will start off facing west
			        break;
			}
			if (boundary) continue;
		    DEBUG(printf("\tFound->%f\n", cellMap[r][c]));
			gridCpy[r][c] = ++labelCount;
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
					if (qualifies(cellMap[rRL][cRL], compVal)){
						int rL, cL;
						getCell(T, &rL, &cL, LEFT);
						// If P_left = black
						if (qualifies(cellMap[rL][cL], compVal)){
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
						if (qualifies(cellMap[rL][cL], compVal)){
        					seqList[seqIdx].y = T->r;
        					seqList[seqIdx++].x = T->c;
						    updateTracer(T, LEFT, LEFT); // straight
				            gridCpy[T->r][T->c] = labelCount;
						}
						else{
							// Outer
						}
					}
					if (T->r == r && T->c == c) fullCircle = 1;
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
					if (qualifies(cellMap[rFL][cFL], compVal)){
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
					else if (qualifies(cellMap[rF][cF], compVal)){
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
				// Act as the last else case
				else{
					turnTracer(T, REAR);
					seqIdx--;
			//		printf("breaking out\n"); break;
				}
				
			    if (seqIdx >= WIDTH*HEIGHT){
			        break;
			    }
				// Check if the loop should break
				if ((T->r == r) && (T->c == c)) fullCircle = 1;
				prevCell = thisCell;
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
        		    else if (!qualifies(cellMap[subr][subc], compVal)){
        		        filling = 0;
        		    }
        		    else if (filling && gridCpy[subr][subc] == ALL_ONES){
        		      //  gridCpy[subr][subc] = labelCount;
        		        gridCpy[subr][subc] = 0;
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

inline char qualifies(float val, float compVal){
    float n = val - compVal;
    if (n <= 1 && n >= -1) return 1; // true
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
			printf("%2d ", mat[r][c] == ALL_ONES?0:mat[r][c]);
// 			printf("%c", mat[r][c] == ALL_ONES?'x':mat[r][c]==0?'-':'#');
		}
		printf("\n");
	}
	printf("\n\n");
}

