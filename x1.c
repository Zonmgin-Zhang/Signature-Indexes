// Test Bits ADT

#include <stdio.h>
#include "defs.h"
#include "reln.h"
#include "tuple.h"
#include "bits.h"

int main(int argc, char **argv)
{
	Bits b = newBits(60);
	printf("t=0: "); showBits(b); printf("\n");
	setBit(b, 5);
    setBit(b, 4);
    setBit(b, 3);
    setBit(b, 2);
    setBit(b, 1);
    setBit(b, 0);
	//shiftBits(b, 5);
	printf("t=1: "); showBits(b); printf("\n");

	return 0;
}
