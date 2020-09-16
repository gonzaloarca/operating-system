#include <stddef.h>

#include <stdio.h>
#include <string.h>

#define sizeof(type) ((char *)(&type+1)-(char*)(&type))
#define MIN_UNITS 128 
#define MEM_SIZE 4096

char mem[MEM_SIZE];
char *baseMem = mem;
char *heapPtr = mem;

union header {
    struct {
        union header *next;
        unsigned size;
    } s;

	char x[16];
};

typedef union header Header;

static Header *morecore(size_t nunits);

static Header base = {{0}};
static Header *freep = NULL;

void *malloc(size_t nbytes){
	Header *currp, *prevp;
	size_t nunits;

	nunits = (nbytes+sizeof(base)-1)/sizeof(base) + 1;
	
	if (freep == NULL){ //No hay lista todavia
		base.s.next = &base;
		freep = &base;
		base.s.size = 0;
	}

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
			//Solicitar espacio
			Header *aux = morecore(nunits);
			if(aux != NULL){
				aux->s.next = currp->s.next;
				currp->s.next = aux;
			} else
				return NULL;
		}
	}

	return (void *)(currp + 1);
}

static Header *morecore(size_t nunits){
	Header *new;

	if(nunits < MIN_UNITS)
		nunits = MIN_UNITS;

	if(heapPtr + nunits*sizeof(base) > baseMem + MEM_SIZE)	//Si no hay más memoria fisica
		return NULL;

	new = (Header *) heapPtr;
	heapPtr += nunits*sizeof(base);

	new->s.size = nunits;

	return new;
}

void free(void *ap){
	Header *bp, *p;

	bp = (Header *)ap - 1; /* point to block header */

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

int main(int argc, char const *argv[]){
	char *array = malloc(MEM_SIZE-16);

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
