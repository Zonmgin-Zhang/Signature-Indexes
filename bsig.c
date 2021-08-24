// bsig.c ... functions on Tuple Signatures (bsig's)
// part of signature indexed files
// Written by John Shepherd, March 2019

#include "defs.h"
#include "reln.h"
#include "query.h"
#include "bsig.h"
#include "psig.h"

void findPagesUsingBitSlices(Query q)
{
	assert(q != NULL);
	//TODO
    //Qsig = makePageSig(Query)
    //Pages = AllOneBits
    //for each i in 0..pm-1 {
    //     if (Qsig bit[i] is 1) {
    //        Slice = get i'th bit slice from bsigFile
    //        zero bits in Pages which are zero in Slice
    //    }
    //}
    Reln r = q->rel;
    PageID p_id = 0;
    Bits qsig = makePageSig(r, q->qstring);
    Bits slice = newBits(bsigBits(r));
    int ppid = -1;
    Page p = getPage(bsigFile(r), 0);
    // Pages = AllOneBits
    setAllBits(q->pages);
    for (int i = 0; i < psigBits(r); i++) {
        if (bitIsSet(qsig, i) == TRUE) {
            // get the current page and its signature
            if (p_id != i/maxBsigsPP(r)) {
                putPage(r->bsigf, p_id, p);
                p = getPage(bsigFile(r), i/maxBsigsPP(r));
                p_id = q->curpage;
            }
            // avoid adding same signature twice
            if (i/maxBsigsPP(r) != ppid) {
                ppid = i/maxBsigsPP(r);
                q->nsigpages++;
            }
            getBits(p, i%maxBsigsPP(r), slice);
            // Zero bits in Pages which are zero in Slice
            for (int j = 0; j < nPages(r); j++) {
                if (bitIsSet(slice, j) == FALSE) {
                    unsetBit(q->pages, j);
                }
            }
            q->nsigs++;
        }
    }
    freeBits(slice);
}

