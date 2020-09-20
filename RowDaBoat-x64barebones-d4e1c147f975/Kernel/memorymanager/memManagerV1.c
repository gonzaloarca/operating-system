#include <memManager.h>

unsigned char *mem = MEM_BASE;

union header {
    struct {
        union header *next;
        unsigned int size;
    } s;

	unsigned char x[16];
};

typedef union header Header;

static Header base = {{0}};
static Header *freep = NULL;

//	malloc adaptado del K&R
void *sys_malloc(size_t size){
	Header *currp, *prevp;
	size_t nunits;
	
	if (freep == NULL){ //No hay lista todavia, creo el bloque de toda la memoria
		Header *new = (Header *) mem;

		new->s.size = PWRTWO(MEM_SIZE_POW)/sizeof(base);		//Recordar que el size es en cantidad de Headers
		new->s.next = &base;						//Vuelve a la base de la lista

		base.s.next = new;							//Apunta al bloque de toda la memoria
		freep = &base;
		base.s.size = 0;
	}

	nunits = (size+sizeof(base)-1)/sizeof(base) + 1;
	prevp = freep;

	for(currp = freep->s.next; ;prevp = currp, currp = currp->s.next){
		if(currp->s.size >= nunits){		//Si entra en el bloque
			if(currp->s.size == nunits)		//Si entra exacto
				prevp->s.next = currp->s.next;
			else{
				currp->s.size -= nunits;
				currp += currp->s.size;
				currp->s.size = nunits;
			}
			freep = prevp;
			break;
		}

		if(currp == freep){ //Si dio toda la vuelta a la lista
			return NULL;
		}
	}

	return (void *)(currp + 1);
}

//	free copiado textual del K&R
void sys_free(void *ptr){
	Header *bp, *p;

	bp = (Header *)ptr - 1; /* point to block header */

	for(p = freep; !(bp > p && bp < p->s.next); p = p->s.next)
		if(p >= p->s.next && (bp > p || bp < p->s.next))
  			break; /* freed block at start or end of arena */

	if(bp + bp->s.size == p->s.next){
		bp->s.size += p->s.next->s.size;
		bp->s.next = p->s.next->s.next;
	}else
  		bp->s.next = p->s.next;

	if(p + p->s.size == bp){
		p->s.size += bp->s.size;
		p->s.next = bp->s.next;
	}else
		p->s.next = bp;

	freep = p;
}

void sys_getMemStatus(MemStatus *stat){
	int freeCount = 0;
	int totalCount = PWRTWO(MEM_SIZE_POW);
	
	if(freep == NULL){
		stat->totalMem = totalCount;
		stat->occMem = 0;
		stat->freeMem = totalCount;
		return;
	}

	Header *curr = base.s.next;

	while(curr != &base){
		freeCount += curr->s.size * sizeof(base);
		curr = curr->s.next;
	}
	
	stat->totalMem = totalCount;
	stat->occMem = totalCount - freeCount;
	stat->freeMem = freeCount;
}
