// psig.c ... functions on page signatures (psig's)
// part of signature indexed files
// Written by John Shepherd, March 2019

#include "defs.h"
#include "reln.h"
#include "query.h"
#include "psig.h"
#include "hash.h"

// code word
Bits codeword(char *attr_value, int m, int k) {
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

Bits makePageSig(Reln r, Tuple t)
{
	assert(r != NULL && t != NULL);
	//TODO
    int  plen = psigBits(r);
    int attrs = nAttrs(r);
    Bits psig = newBits(plen);
    char **vals = tupleVals(r, t);
    int pbits = codeBits(r);
    unsetAllBits(psig);
    // iterate each attribute
    if (sigType(r) == 's') {
        for (int i = 0; i < attrs; i++) {
            // skip sign "?"
            if (strcmp(vals[i], "?") == 0) {
                continue;
            }
            // do codewords for this attribute
            Bits cw = codeword(vals[i], plen, pbits);
            // do 'or' operator
            orBits(psig, cw);
            freeBits(cw);
        }
    } else {
        for (int i = 0; i < attrs; i++) {
            // 应该是地板除，这里不确定
            int len = plen / attrs;
            int res = plen % attrs;
            if (strcmp(vals[i], "?") == 0 ) {
                continue;
            }
            // do codewords for first attribute
            if (i == 0) {
                Bits pcopy = newBits(plen);
                unsetAllBits(pcopy);
                Bits cw = codeword(vals[i], len+res, iceil(len+res,(2*maxTupsPP(r))));
                //orBits(pcopy, cw);
                for (int j = 0; j < len+res; j++) {
                    if (bitIsSet(cw,j) == TRUE) {
                        setBit(pcopy,j);
                    }
                }
                orBits(psig,pcopy);
                //printf("cw:"); showBits(cw); putchar('\n');
                //printf("psig1:"); showBits(psig); putchar('\n');
                freeBits(cw);
                freeBits(pcopy);
            } else {
                Bits pcopy = newBits(plen);
                unsetAllBits(pcopy);
                // do codewords for this attribute
                Bits cw = codeword(vals[i], len, iceil(len, (2*maxTupsPP(r))));
                //orBits(pcopy, cw);
                for (int j = 0; j < len+res; j++) {
                    if (bitIsSet(cw,j) == TRUE) {
                        setBit(pcopy,j);
                    }
                }
                shiftBits(pcopy, i*len);
                orBits(psig, pcopy);
                //printf("psig:"); showBits(psig); putchar('\n');
                freeBits(cw);
                freeBits(pcopy);
            }
        }
    }
    freeVals(vals, attrs);
    return psig;
}

void findPagesUsingPageSigs(Query q)
{
	assert(q != NULL);
	//TODO
    Reln r = q->rel;
    // QuerySig = makePageSig(Query)
    Bits qsig = makePageSig(r, q->qstring);
    Page p = getPage(psigFile(r), 0);
    PageID p_id = 0;
    int index = -1;
    // Pages = AllZeroBits
    unsetAllBits(q->pages);

    // iterate all pages
    for (int pid = 0; pid < nPsigPages(r); pid++) {
        q->nsigpages++;
        if (p_id != pid) {
            putPage(r->psigf, p_id, p);
            p = getPage(psigFile(r), pid);
            p_id = q->curpage;
        }
        // iteate each page
        for(int tid = 0; tid < pageNitems(p); tid++) {
            Bits psig = newBits(psigBits(r));
            index++;
            // get the signature for each tuple store in the psig
            getBits(p, tid, psig);
            if (isSubset(qsig, psig) == TRUE) {
                // include PID in Pages
                setBit(q->pages, index);
            }
            freeBits(psig);
            q->nsigs++;
        }
    }
    freeBits(qsig);
}

