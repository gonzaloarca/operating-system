
#include <memManager.h>

#define GET_BUDDY(base, size, offset) ((unsigned char *)(((long)(base - offset) ^ (1 << size)) + (long)offset))

#define FREE 0
#define OCCUPIED 1
#define MIN_SIZE_POW 8  //2^8 = 256 bytes
#define LIST_SIZE (PWRTWO(MEM_SIZE_POW - MIN_SIZE_POW) / 2)
#define LIST_QTY (MEM_SIZE_POW - MIN_SIZE_POW + 1)

static void createHeader(unsigned char *blockBase, unsigned char occupied, unsigned int sizePow);
static void partitionMem(unsigned char *blockBase);
static void addFreeList(unsigned char *blockBase);
static void removeFreeList(unsigned char *blockBase);
static void mergeBlocks(unsigned char **blockBase);

unsigned char *mem = MEM_BASE;
static unsigned char *freeLists[LIST_QTY][LIST_SIZE] = {{0}};
static int initMem = 0;

void *sys_malloc(size_t size){
    if(size == 0)
        return NULL;

    unsigned char *ret = NULL;

    if(initMem == 0){
        createHeader(mem, FREE, MEM_SIZE_POW);
        //addFreeList en vez de hacerlo a mano?
        freeLists[LIST_QTY - 1][0] = mem;
        initMem = 1;
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
            removeFreeList(ret);

            //hay que ver si algun bloque con menor tamano podria acomodarlo 
            while(PWRTWO(sizePow-1) - 8 >= size && sizePow > MIN_SIZE_POW){
                sizePow--;

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

    blockBase[0] = --currSizePow;
    unsigned char *buddyBase = GET_BUDDY(blockBase, currSizePow, mem); //aprovechando los tomanos de potencias de base 2, accedemos a la direccion del buddy
                                                                    //utilizando operaciones de bits
    createHeader(buddyBase, FREE, currSizePow);
    
    addFreeList(buddyBase);
}

static void addFreeList(unsigned char *blockBase){
    unsigned int sizePow = blockBase[0];
    int i;

    //busca la primer posicion vacia en la lista de su tamano y la asigna
    for(i = 0; freeLists[sizePow - MIN_SIZE_POW][i] != NULL && i < LIST_SIZE; i++);
    
    freeLists[sizePow - MIN_SIZE_POW ][i] = blockBase;
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

void sys_free(void *ptr){
    unsigned char *blockBase = ((unsigned char *)ptr) - 8;
    unsigned int currSizePow = blockBase[0] ^ (1 << 7);

    createHeader(blockBase, FREE, currSizePow);
    addFreeList(blockBase);

    //hay que ver si puede mergear con su buddy
    unsigned char* buddyBase = GET_BUDDY(blockBase, currSizePow, mem);

    //reviso que el buddy este libre y sea del mismo size para que puedan ser mergeados
    while((buddyBase[0] & 0x80) == 0 && buddyBase[0] == blockBase[0] ){
        mergeBlocks(&blockBase);

        currSizePow++;
        if(currSizePow == MEM_SIZE_POW)
            break;
            
        buddyBase = GET_BUDDY(blockBase, currSizePow, mem);
    }

}

//te deja en el puntero la direccion del buddy en direcciones mas bajas
static void mergeBlocks(unsigned char **blockBase){
    //ya esta garantizado que el primer bit es 0
    unsigned int sizePow = (*blockBase)[0];
    
    removeFreeList(*blockBase);
    removeFreeList(GET_BUDDY(*blockBase, sizePow, mem));
    
    //verifica si el bloque se corresponde al buddy de direcciones mas bajas
    if(((long)(*blockBase - mem) & (1 << sizePow)) != 0){
        *blockBase = GET_BUDDY(*blockBase, sizePow, mem);
    }
    
    (*blockBase)[0]++; 

    addFreeList(*blockBase);
}

void sys_getMemStatus(MemStatus *stat){
    stat->totalMem = PWRTWO(MEM_SIZE_POW);

    if(initMem == 0){
        stat->freeMem = stat->totalMem;
    }else{
        size_t freeCount = 0;
        for(int i = 0; i < LIST_QTY ; i++){
            for(int j = 0; j < LIST_SIZE && freeLists[i][j] != NULL; j++){
                freeCount += PWRTWO(i+MIN_SIZE_POW);
            }
        }
        stat->freeMem = freeCount;
    }
    stat->occMem = stat->totalMem - stat->freeMem;
}
