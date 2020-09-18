#include <stddef.h>

#include <stdio.h>
#include <string.h>

#define sizeof(type) ((char *)(&type+1)-(char*)(&type))

#define MEM_SIZE 4096

char mem[MEM_SIZE];			//AUXILIAR PARA TESTEAR: cambiar por el inicio de la memoria

union header {
    struct {
        union header *next;
        unsigned size;
    } s;

	char x[16];
};

typedef union header Header;

static Header base = {{0}};
static Header *freep = NULL;

//	malloc adaptado del K&R
void *malloc(size_t nbytes){
	Header *currp, *prevp;
	size_t nunits;
	
	if (freep == NULL){ //No hay lista todavia, creo el bloque de toda la memoria
		Header *new = (Header *) mem;

		new->s.size = MEM_SIZE/sizeof(base);		//Recordar que el size es en cantidad de Headers
		new->s.next = &base;						//Vuelve a la base de la lista

		base.s.next = new;							//Apunta al bloque de toda la memoria
		freep = &base;
		base.s.size = 0;
	}

	nunits = (nbytes+sizeof(base)-1)/sizeof(base) + 1;
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

//	PROGRAMA DE TESTEO BORRAR
int main(int argc, char const *argv[]){
	char *array = malloc(MEM_SIZE-16);

	if(array == NULL)
	{
		printf("F el primer if\n");
		return 0;
	}

	free(array);

	if (malloc(1) == NULL)
		printf("No pude alocar más\n");

	return 0;
}
