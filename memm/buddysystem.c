#include <stddef.h>
#include <stdio.h>
#define PWRTWO(x) (1 << (x))
#define GET_BUDDY(base, size) ( (unsigned char *)((long) base ^ (1 << size)) )
#define FREE 0
#define OCCUPIED 1
#define MEM_SIZE_POW 12 //2^12 bytes
#define MIN_SIZE_POW 6  //2^6 bytes
#define LIST_SIZE (PWRTWO(MEM_SIZE_POW - MIN_SIZE_POW) / 2)
#define LIST_QTY (MEM_SIZE_POW - MIN_SIZE_POW + 1)

void *malloc(size_t size);
void free(void *ptr);
static void createHeader(unsigned char *blockBase, unsigned char occupied, unsigned int sizePow);
static void partitionMem(unsigned char *blockBase);
static void addFreeList(unsigned char *blockBase);
static void removeFreeList(unsigned char *blockBase);
static void mergeBlocks(unsigned char **blockBase);

unsigned char mem[PWRTWO(MEM_SIZE_POW)] = {0};
static unsigned char *freeLists[LIST_QTY][LIST_SIZE] = {{0}};
static int initMem = 0;

void *malloc(size_t size){
    unsigned char *ret = NULL;

    if(initMem == 0){
        createHeader(mem, FREE, MEM_SIZE_POW);
        freeLists[LIST_QTY - 1][0] = mem;
    }

    //revisa la lista de bloques libres para ver si hay alguno que le sirva
    for(int i = 0; i < LIST_QTY && ret == NULL; i++){
        int sizePow = i + MIN_SIZE_POW;

        //hay 8 bytes reservados para el header en cada bloque
        //si la lista esta vacia se garantiza que su primer elemento es NULL
        if(PWRTWO(sizePow) - 8 >= size && freeLists[i][0] != NULL){ 
            //ahora debo encontrar el ultimo bloque libre en la lista
            int j;
            for(j = 0; j < LIST_SIZE && freeLists[i][j + 1] != NULL; j++);
            
            ret = freeLists[i][j];
            //quedaria mejor si se llama a removeFreeList aca? se rompe todo?
            freeLists[i][j] = NULL;

            //hay que ver si algun bloque con menor tamano podria acomodarlo 
            while(PWRTWO(sizePow-1) - 8 >= size){
                sizePow--;
                printf("sizePow = %d\n", sizePow);
                partitionMem(ret);
            }
            
            createHeader(ret, OCCUPIED, sizePow);

            ret += 8; //apunta despues del header
        }
    }

    return (void *)ret; //si no encontro ninguno, no habian mas bloques libres, devuelve NULL
}

static void createHeader(unsigned char *blockBase, unsigned char occupied, unsigned int sizePow){
    //utilizamos 1 bit para indicar si el bloque esta ocupado y el resto para indicar la potencia del tamano del bloque
    blockBase[0] = (occupied << 7) + sizePow;

    //rellenamos con 0 el resto de los bits para que el bloque quede alineado a palabra
    for(int i = 1; i < 8; i++)
        blockBase[i] = 0;
}

//particiona un bloque libre en 2, asigna los headers y modifica la free list como corresponda
static void partitionMem(unsigned char *blockBase){
    unsigned int currSizePow = blockBase[0];

    removeFreeList(blockBase);

    blockBase[0] = --currSizePow;

    unsigned char *buddyBase = GET_BUDDY(blockBase, currSizePow); //aprovechando los tomanos de potencias de base 2, accedemos a la direccion del buddy
                                                                    //utilizando operaciones de bits
    printf("currSizePow = %d\n", currSizePow);
    createHeader(buddyBase, FREE, currSizePow);
    
    addFreeList(buddyBase);
}

static void addFreeList(unsigned char *blockBase){
    unsigned int sizePow = blockBase[0];
    int i;

    //busca la primer posicion vacia en la lista de su tamano y la asigna
    for(i = 0; freeLists[sizePow - MIN_SIZE_POW][i] == NULL && i < LIST_SIZE; i++);
    
    freeLists[sizePow][i] = blockBase;
}

static void removeFreeList(unsigned char *blockBase){
    unsigned int sizePow = blockBase[0];

    //si el bloque a eliminar no es el ultimo en la lista, hay que swapearlo con el ultimo asi se garantiza que no hayan
    //NULLs en el medio de la lista
    int i, j;
    for(i = 0; i < LIST_SIZE - 1; i++){
        if(freeLists[sizePow - MIN_SIZE_POW][i] == blockBase){

            //encuentra el ultimo elemento en la lista
            for(j = i; j < LIST_SIZE && freeLists[sizePow - MIN_SIZE_POW][j + 1] != NULL; j++);
           
            freeLists[sizePow - MIN_SIZE_POW][i] = freeLists[sizePow - MIN_SIZE_POW][j];
            freeLists[sizePow - MIN_SIZE_POW][j] = NULL;
            break;
        } 
    }

    if(i == LIST_SIZE - 1){
        freeLists[sizePow - MIN_SIZE_POW][i] = NULL;
    }
}

void free(void *ptr){
    unsigned char *blockBase = ((unsigned char *)ptr) - 8;
    unsigned int currSizePow = blockBase[0] ^ (1 << 7);

    createHeader(blockBase, FREE, currSizePow);
    addFreeList(blockBase);

    //hay que ver si puede mergear con su buddy
    blockBase = GET_BUDDY(blockBase, currSizePow);

    while((blockBase[0] & 0x80) == 0){
        mergeBlocks(&blockBase);

        currSizePow++;
        blockBase = GET_BUDDY(blockBase, currSizePow);
    }

}

//te deja en el puntero la direccion del buddy en direcciones mas bajas
static void mergeBlocks(unsigned char **blockBase){
    //ya esta garantizado que el primer bit es 0
    unsigned int sizePow = (*blockBase)[0];
    
    removeFreeList(*blockBase);
    removeFreeList(GET_BUDDY(*blockBase, sizePow));
    
    //verifica si el bloque se corresponde al buddy de direcciones mas bajas
    if(((long)*blockBase & (1 << sizePow)) != 0){
        *blockBase = GET_BUDDY(*blockBase, sizePow);
    }
    
    (*blockBase)[0]++; 

    addFreeList(*blockBase);
}

int main(){
    unsigned char *array = malloc(128);
    // array -= 8;

    
    // for(int i = 0; i < 128; i++){
    //     printf("%d ", (int)array[i]);
    // }
    // fflush(stdout);
    
    int j = 0;
    for(int i = 0; i < 4096; i++){
        if( i % 8 == 0 ){
            printf("\n");
        }
        if(i % 128 == 0){
            printf("nuevo bloque de 128, j = %d, i = %d\n", j, i);
            j++;
        }

        printf("%d ", mem[i]);
    }
	if(array == NULL)
	{
		printf("F el primer if\n");
		return 0;
	}

	free(array);

	if (malloc(1) == NULL)
		printf("LA CAGAMOS\n");

	return 0;
}