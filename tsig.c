// tsig.c ... functions on Tuple Signatures (tsig's)
// part of signature indexed files
// Written by John Shepherd, March 2019

#include <unistd.h>
#include <string.h>
#include "defs.h"
#include "tsig.h"
#include "reln.h"
#include "hash.h"
#include "bits.h"


// code words
Bits codewords(char *attr_value, int m, int k) {
    int nbits = 0;
    Bits cword = newBits(m);
    srandom(hash_any(attr_value, strlen(attr_value)));
    while (nbits < k) {
        int i = random() % m;
        if (bitIsSet(cword, i) == FALSE) {
            setBit(cword, i);
            nbits++;
        }
    }
    return cword;

}

// make a tuple signature

Bits makeTupleSig(Reln r, Tuple t)
{
	assert(r != NULL && t != NULL);
	//TODO
    int  tlen = tsigBits(r);
    int  tbits = codeBits(r);
    int attrs = nAttrs(r);
    Bits tsig = newBits(tlen);
    char **vals = tupleVals(r, t);
    unsetAllBits(tsig);

    if (sigType(r) == 's') {
       for (int i = 0; i < attrs; i++) {
           // skip sign "?"
           if (strcmp(vals[i], "?") == 0 ) {
               continue;
           }
           // do codewords for this attribute
           Bits cw = codewords(vals[i], tlen, tbits);
           // do 'or' operator
           orBits(tsig, cw);
           freeBits(cw);
       }
    }else {
        for (int i = 0; i < attrs; i++) {
            // 应该是地板除，这里不确定
            int len = tlen / attrs;
            int res = tlen % attrs;
            if (strcmp(vals[i], "?") == 0 ) {
                continue;
            }
            // do codewords for first attribute
            if (i == 0) {
                Bits tcopy = newBits(tlen);
                unsetAllBits(tcopy);
                Bits cw = codewords(vals[i], len+res, len/2);
                //Bits cw1 = codewords(vals[i], len+res, tbits);
                for (int j = 0; j < len+res; j++) {
                    if (bitIsSet(cw,j) == TRUE) {
                        setBit(tcopy,j);
                    }
                }
                //shiftBits(tcopy, (tlen - len - res));
                //printf("tcopy:"); showBits(tcopy); putchar('\n');
                //orBits(tsig, tcopy);
                //printf("cw:"); showBits(cw); putchar('\n');
                //printf("cw1:"); showBits(cw1); putchar('\n');
                //shiftBits(cw, (len + res));
                orBits(tsig, tcopy);
                //printf("tsig:"); showBits(tsig); putchar('\n');
                freeBits(cw);
                freeBits(tcopy);
            } else {
                Bits tcopy = newBits(tlen);
                unsetAllBits(tcopy);
                // do codewords for this attribute
                Bits cw = codewords(vals[i], tlen, len/2);
                for (int j = 0; j < len; j++) {
                    if (bitIsSet(cw,j) == TRUE) {
                        setBit(tcopy,j);
                    }
                }
                //orBits(tcopy, cw);
                shiftBits(tcopy, i*len);
                orBits(tsig, tcopy);
                //printf("tsig1:"); showBits(tsig); putchar('\n');
                freeBits(cw);
                freeBits(tcopy);
            }
        }
    }
    freeVals(vals, attrs);
    return tsig;
}

// find "matching" pages using tuple signatures

void findPagesUsingTupSigs(Query q)
{
	assert(q != NULL);
	//TODO

	Reln r = q->rel;
	// QuerySig = makeTupleSig(Query)
	Bits qsig = makeTupleSig(r, q->qstring);
    Page p = getPage(tsigFile(r), 0);
    PageID p_id = 0;
    // Pages = AllZeroBits
    unsetAllBits(q->pages);
    for (int pid = 0; pid < nTsigPages(r); pid++) {
        q->nsigpages++;
        if (p_id != pid) {
            putPage(r->tsigf, p_id, p);
            p = getPage(tsigFile(r), pid);
            p_id =pid;
        }
        for(int tid = 0; tid < pageNitems(p); tid++) {
            Bits tsig = newBits(tsigBits(r));
            // get the signature for each tuple store in the tsig
            getBits(p, tid, tsig);
            if (isSubset(qsig, tsig) == TRUE) {
                PageID index = q->nsigs / maxTupsPP(r);
                // include PID in Pages
                setBit(q->pages, index);
            }
            freeBits(tsig);
            q->nsigs++;
        }
    }

    freeBits(qsig);
	// The printf below is primarily for debugging
	// Remove it before submitting this function
	//printf("Matched Pages:"); showBits(q->pages); putchar('\n');
}
