#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <string>
#include <iostream>

#include <emscripten/emscripten.h>


int main(int argc, char ** argv) {
	printf("WebAssembly module loaded\n");
	return 0;
}

int EMSCRIPTEN_KEEPALIVE roll_dice() {
	srand ( time(NULL) );
	return rand()%6+1;
}

double EMSCRIPTEN_KEEPALIVE avgNRolls(int n) {
	srand ( time(NULL) );
	int i;
	int sum = 0;
	for (i = 0; i < n; i++)
		sum += rand()%6+1;
	return (double)(sum)/n;
}

double fact(int n) {
	if (n <= 1) return 1;
	return n*fact(n-1);
}

double EMSCRIPTEN_KEEPALIVE choose(int n, int k) {
	printf("%d!/(!%d * (%d))\n", n, k, (n-k));
	return fact(n)/(fact(k)*fact(n-k));
	
}

char* EMSCRIPTEN_KEEPALIVE reprint(char* str){
	printf("String (in c): %s\n", str);
	return str;
}
char* EMSCRIPTEN_KEEPALIVE vigenere(char* str, char* key){
	int kLength = strlen(key);
	int sLength = strlen(str);
	int i;
	int idx = 0;
	char* out = (char*)(malloc(sizeof(str)));
	for (i = 0; i < sLength; i++) {
		if (isalpha(str[i])){
			// get just the alphabet offset
			char c = ((0x1F & str[i]) + (0x1F & key[idx]))%26;
			if (c == 0) c = 26;
			out[i] = c | (str[i] & 0xE0);
			idx = (idx+1)%kLength;
		}
		else {
			out[i] = str[i];
		}
	}
//	printf("Encrypting: %s\nKey: %s\nOutput:%s\n", str, key, out);
	return out;
}

double EMSCRIPTEN_KEEPALIVE det(double** mat) {
	return mat[0][0]*mat[1][0]-mat[0][1]*mat[1][1];
}

int EMSCRIPTEN_KEEPALIVE arrSum(int* a, int length){
	int sum = 0;
	int i;
	printf("Starting arrSum...\nsizeof(a)=%d\nsizeof(int)=%d\n",
			sizeof(a), sizeof(int));
	printf("Received a length of %d\n", length);
	for (i = 0; i < length; i++) {
		printf("%d + %d = ", sum, a[i]);
		sum+=a[i];
		printf("%d\n", sum);
	}
	return sum;
}

double* EMSCRIPTEN_KEEPALIVE vSum(double* m1, double* m2, double* sum, int length) {
	int i;
	for (i = 0; i < length; i++) {
		printf("%f + %f\n", m1[i], m2[i]);
		sum[i] = m1[i] + m2[i];
		
	}
	return sum;
}

double* EMSCRIPTEN_KEEPALIVE dmVectors(double* v1, double* v2, double* product, int length) {
	int i;
	printf("Starting dot-multiply\n");
	for (i = 0; i < length; i++) {
//		printf("%f * %f\n", v1[i], v2[i]);
		product[i] = v1[i] * v2[i];
	}
	return product;

}

int EMSCRIPTEN_KEEPALIVE counter(char r){
	static int i = 0;
	if (r) i = 0;
	return i++;
}

/*********** Linked-List Test Stuff *************/
typedef struct node_t {
	int value;
	struct node_t* next;
} node;

node* EMSCRIPTEN_KEEPALIVE createNode(int val) {
	node* newNode = (node*)malloc(sizeof(node));
	if (newNode == NULL) {
		printf("Error: could not create node!\n");
		return NULL;
	}
	newNode->value = val;
	newNode->next = NULL;
	return newNode;
}
node* EMSCRIPTEN_KEEPALIVE addNode(node* head, int val) {
	if (head == NULL) {
		head = createNode(val);
		printf("Created new node with value %d", val);
		return head;
	}
	return addNode(head->next, val);
}
void EMSCRIPTEN_KEEPALIVE printList(node* head) {
	if (head == NULL){
		fflush(stdout);
		return;
	}
	printf("%d->", head->value);
	printList(head->next);
}

node* EMSCRIPTEN_KEEPALIVE addNodes(node* head, int* values, int length) {
	int i;
	node* nPtr = head;
	for (i = 0; i < length; i++) {
		addNode(nPtr, values[i]);
		nPtr = nPtr->next;
	}
	return head;
}

void EMSCRIPTEN_KEEPALIVE stringifyList(node* head) {
	int i;
	node* n = head;
	while (n != NULL) {
		printf("%d->", n->value);
		n = n->next;
	}
	printf("\n");
	fflush(stdout);
	return;
}

node* EMSCRIPTEN_KEEPALIVE getNext(node* nPtr) {
	printf("Current node: %d, next node at %p", nPtr->value, nPtr->next);
	return nPtr->next;
}

/****************************************
			Array of objects
****************************************/
typedef struct creature_t {
	float nrg;
	float xloc;
	float yloc;
	float zloc;
	uint32_t cId;
	uint8_t type;
	struct creature_t* child;
} creature;

typedef struct creatureList_t{
	creature* cr;
	struct creatureList_t* next;
} creatureList;

creature* EMSCRIPTEN_KEEPALIVE createCreature(){
	static int cid = 0;
	creature* cr = (creature*)malloc(sizeof(creature));
	if (cr == NULL) {
		printf("NOT ENOUGH MEMORY TO CREATE ANOTHER CREATURE: CID=%d\n",cid);
		return NULL;
	}

	cr->nrg = 1;
	cr->xloc = 0;
	cr->yloc = 0;
	cr->zloc = 0;
	cr->cId = cid++;
	cr->type = 3;
	cr->child = NULL;
	return cr;
}

void EMSCRIPTEN_KEEPALIVE printCreatureDetails(creature* cr){
	printf("Creature #%d:\n"
		   "Energy: %f\n"
		   "Location (x,y,z): (%f,%f,%f)\n"
		   "Type: %d\n"
		   "Child: %p\n", cr->cId, cr->nrg, cr->xloc, cr->yloc, cr->zloc, cr->type, cr->child);
}

void EMSCRIPTEN_KEEPALIVE moveCreature(creature* cr, float x, float y, float z) {
	printf("Moving Creature:\n");
	printCreatureDetails(cr);
	cr->xloc = cr->xloc+x;
	cr->yloc = cr->yloc+y;
	cr->zloc = cr->zloc+z;
	printf("Now:\n");
	printCreatureDetails(cr);

	return;
}

creature* EMSCRIPTEN_KEEPALIVE replicate(creature* cr) {
	if (cr->child != NULL) {
		printf("This creature already has a child");
		return NULL;
	}
	creature* child = createCreature();
	cr->child = child;
	return child;
}

void EMSCRIPTEN_KEEPALIVE printLineage(creature* cr) {
	if (cr == NULL) {
		printf("Creature %p == NULL, end of line\n", cr);
		return;
	}
	printf("-----------------%p\n", cr);
	printCreatureDetails(cr);
	printLineage(cr->child);
}

const char* EMSCRIPTEN_KEEPALIVE getCreatureJSON(creature* cr) {
	creature* cur = cr;
	std::string str;
	str+="{\"creatures\":[";
	while(cur != NULL) {
		str+="{\"cid\":\""+std::to_string(cur->cId)+"\","
			 "\"nrg\":\""+std::to_string(cur->nrg)+"\","
		 	 "\"x\":\""+std::to_string(cur->xloc)+"\","
			 "\"y\":\""+std::to_string(cur->yloc)+"\","
			 "\"z\":\""+std::to_string(cur->zloc)+"\","
			 "\"type\":\""+std::to_string(cur->type)+"\","
			 "\"child\":\""+std::to_string((int)(&(cur->child)))+"\"},";
		cur = cur->child;
	}
	str+="]}\0";
	return str.c_str();
}
/*********************************
			BST
*********************************/
typedef struct bstNode_t {
	int value;
	int count;
	struct bstNode_t* left;
	struct bstNode_t* right;
} bstNode;

bstNode* EMSCRIPTEN_KEEPALIVE newBstNode(int val) {
	bstNode* newNode = (bstNode*)malloc(sizeof(bstNode));
	newNode->value = val;
	newNode->count = 0;
	newNode->left = NULL;
	newNode->right = NULL;
	return newNode;
}

bstNode* EMSCRIPTEN_KEEPALIVE addToBst(bstNode* root, int val) {
	if (root == NULL) {
		return newBstNode(val);
	}
	if (root->value == val) {
		root->count++;
		return root;
	}
	else if(val > root->value){
		if (root->right == NULL) {
			root->right = newBstNode(val);
			return root->right;
		}
		else {
			return addToBst(root->right, val);
		}
	}
	else {
		if (root->left == NULL) {
			root->left = newBstNode(val);
			return root->left;
		}
		else {
			return addToBst(root->left, val);
		}
	}
}

void EMSCRIPTEN_KEEPALIVE _printBst(bstNode* root, int depth) {
	int i;
	if (root->left != NULL) _printBst(root->left, depth+1);
	for (i = 0; i < depth; i++) {
		printf("          ");
	}
	printf("-------------%d\n", root->value);
	if (root->right != NULL) _printBst(root->right, depth+1);
	return;
}

void EMSCRIPTEN_KEEPALIVE printBst(bstNode* root) {
	_printBst(root, 0);
}
