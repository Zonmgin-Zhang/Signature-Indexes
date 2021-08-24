void freeBits(Bits b)
{
    //TODO
    assert(b != NULL);
    free(b);
}

// check if the bit at position is 1

Bool bitIsSet(Bits b, int position)
{
    assert(b != NULL);
    assert(0 <= position && position < b->nbits);
    //TODO

    // set the position offset to 1
    Byte mask = (1 << (position % 8));
    if ((b->bitstring[position / 8] & mask) != 0) {
        return TRUE;
    } else {
        return FALSE;
    }
}

// check whether one Bits b1 is a subset of Bits b2

Bool isSubset(Bits b1, Bits b2)
{
    assert(b1 != NULL && b2 != NULL);
    assert(b1->nbytes == b2->nbytes);
    //TODO
    for (int i = 0; i < b1->nbytes; i++) {
        if ((b1->bitstring[i] & b2->bitstring[i]) != b1->bitstring[i]) {
            return FALSE;
        }
    }
    return TRUE;
}

// set the bit at position to 1

void setBit(Bits b, int position)
{
    assert(b != NULL);
    assert(0 <= position && position < b->nbits);
    //TODO
    b->bitstring[position / 8] |= (1 << (position % 8));
}

// set all bits to 1

void setAllBits(Bits b)
{
    assert(b != NULL);
    //TODO
    Byte mask = 0xFF;
    for (int i = 0; i < b->nbytes; i++) {
        b->bitstring[i] = b->bitstring[i] | mask;
    }
}

// set the bit at position to 0

void unsetBit(Bits b, int position) {
    assert(b != NULL);
    assert(0 <= position && position < b->nbits);
    //TODO

    // set the position offset to 0
    b->bitstring[position / 8] &= ~(1 << (position % 8));
}

// set all bits to 0

void unsetAllBits(Bits b)
{
    assert(b != NULL);
    //TODO
    for (int i = 0; i < b->nbytes; i++) {
        b->bitstring[i] = 0;
    }
}

// bitwise AND ... b1 = b1 & b2

void andBits(Bits b1, Bits b2)
{
    assert(b1 != NULL && b2 != NULL);
    assert(b1->nbytes == b2->nbytes);
    //TODO
    for (int i = 0; i < b1->nbytes; i++) {
        b1->bitstring[i] = b1->bitstring[i] & b2->bitstring[i];
    }
}

// bitwise OR ... b1 = b1 | b2

void orBits(Bits b1, Bits b2)
{
    assert(b1 != NULL && b2 != NULL);
    assert(b1->nbytes == b2->nbytes);
    //TODO
    for (int i = 0; i < b1->nbytes; i++) {
        b1->bitstring[i] = b1->bitstring[i] | b2->bitstring[i];
    }
}

// left-shift ... b1 = b1 << n
// negative n gives right shift

void shiftBits(Bits b, int n) {
    // TODO
    Bits new = newBits(b->nbits);
    unsetAllBits(new);
    if (n >= 0){
        for (int i = 0; i < b->nbits; i++) {
            if (bitIsSet(b, i) == TRUE) {
                if (i + n < b->nbits) {
                    setBit(new, i + n);
                    unsetBit(b, i);
                }
            } else {
                unsetBit(b, i);
            }
        }
    }else {
        for (int i = 0; i < b->nbits; i++) {
            if (bitIsSet(b, i) == TRUE) {
                if (i + n >= 0) {
                    setBit(new, i + n);
                    unsetBit(b, i);
                }
            }else {
                unsetBit(b, i);
            }
        }
    }
    unsetAllBits(b);
    orBits(b, new);
}

// get a bit-string (of length b->nbytes)
// from specified position in Page buffer
// and place it in a BitsRep structure

void getBits(Page p, Offset pos, Bits b)
{
    //TODO
    assert(b != NULL && p != NULL);
    Byte *addr = addrInPage(p, pos, b->nbytes);
    memcpy(b->bitstring, addr, b->nbytes);
}

// copy the bit-string array in a BitsRep
// structure to specified position in Page buffer

void putBits(Page p, Offset pos, Bits b)
{
    //TODO
    assert(b != NULL && p != NULL);
    Byte *addr = addrInPage(p, pos, b->nbytes);
    memcpy(addr, b->bitstring, b->nbytes);
}

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
}
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
            // do codewords for first attribute
            if (i == 0) {
                Bits cw = codewords(vals[i], tlen, tbits);
                //orBits(tcopy, cw);
                shiftBits(cw, (len + res));
                orBits(tsig, cw);
                freeBits(cw);
            } else {
                // do codewords for this attribute
                Bits cw = codewords(vals[i], tlen, tbits);
                //orBits(tcopy, cw);
                shiftBits(cw, len);
                orBits(tsig, cw);
                freeBits(cw);
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
            p_id = q->curpage;
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


    // The printf below is primarily for debugging
    // Remove it before submitting this function
    //printf("Matched Pages:"); showBits(q->pages); putchar('\n');
}
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
            // do codewords for first attribute
            if (i == 0) {
                Bits cw = codeword(vals[i], plen, pbits/(maxTupsPP(r)));
                //orBits(pcopy, cw);
                shiftBits(cw, (len + res));
                orBits(psig,cw);
                freeBits(cw);
            } else {
                // do codewords for this attribute
                Bits cw = codeword(vals[i], plen, pbits/(maxTupsPP(r)));
                //orBits(pcopy, cw);
                shiftBits(cw, (len));
                orBits(psig, cw);
                freeBits(cw);
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
}
void scanAndDisplayMatchingTuples(Query q)
{
    assert(q != NULL);
    //TODO
    Reln r = q->rel;
    Page p = getPage(dataFile(r) , 0);
    PageID p_id = 0;
    // iterate the pages
    for (int pid = 0; pid < nPages(r); pid++) {
        Bool flag = FALSE;
        // if the page is not set to 1
        if (bitIsSet(q->pages, pid) == FALSE) {
            continue;
        }
        if (p_id != pid) {
            putPage(r->dataf, p_id, p);
            p = getPage(dataFile(r) , pid);
            p_id = q->curpage;
        }
        // iterate the pageItems(tuples)
        for(int tid = 0; tid < pageNitems(p); tid++) {

            Tuple t = getTupleFromPage(r, p, tid);
            q->ntuples++;

            // show the matched tuples
            if (tupleMatch(r, q->qstring, t) == TRUE) {
                flag = TRUE;
                showTuple(r,t);
            }
        }
        if (flag == FALSE) {
            q->nfalse++;
        }
        q->ntuppages++;
    }
}
// compute tuple signature and add to tsigf
//TODO
Bits tsig = makeTupleSig(r, t);
PageID t_pid = rp->tsigNpages - 1;
Page tsigpage = getPage(r->tsigf, t_pid);
// check if room on last page; if not add one new page
if (pageNitems(tsigpage) == rp->tsigPP) {
addPage(r->tsigf);
rp->tsigNpages++;
t_pid++;
free(tsigpage);
tsigpage = newPage();
if (tsigpage == NULL) return NO_PAGE;
}
putBits(tsigpage, pageNitems(tsigpage), tsig);
// add one item
addOneItem(tsigpage);
rp->ntsigs++;
// write into file
putPage(r->tsigf, t_pid, tsigpage);

// compute page signature and add to psigf
//TODO
Bits psig = makePageSig(r, t);
PageID p_pid = nPsigPages(r) - 1;
Page psigpage = getPage(r->psigf, p_pid);
if (nPsigs(r) == nPages(r)) {
Bits psig1 = newBits(psigBits(r));
// merge old psig and a psig which add a new tuple
getBits(psigpage, pageNitems(psigpage)-1, psig1);
orBits(psig, psig1);
// update this new psig to the psigfile
putBits(psigpage, pageNitems(psigpage)-1, psig);
putPage(r->psigf, p_pid, psigpage);

} else {
// if the page space is full
if (pageNitems(psigpage) == rp->psigPP) {
addPage(r->psigf);
rp->psigNpages++;
p_pid++;
free(psigpage);
psigpage = newPage();
if (psigpage == NULL) return NO_PAGE;
}
// create new page
putBits(psigpage, pageNitems(psigpage), psig);
// add one item
addOneItem(psigpage);
rp->npsigs++;
// write into file
putPage(r->psigf, p_pid, psigpage);
}

// use page signature to update bit-slices
//TODO

// get the length of page
int b_id = nPsigs(r) - 1;
for (int i = 0; i < psigBits(r); i++) {
if (bitIsSet(psig, i) == TRUE) {
Bits slice = newBits(bsigBits(r));
// get the page number
Page p1 = getPage(bsigFile(r), i/maxBsigsPP(r));
// get the bits and then set 1
getBits(p1, i%maxBsigsPP(r), slice);
setBit(slice, b_id);
putBits(p1, i%maxBsigsPP(r), slice);
// write updated Slice back to bsigFile
putPage(r->bsigf, i/maxBsigsPP(r), p1);
freeBits(slice);
}
}
return nPages(r)-1;
}
// Create a file containing "pm" all-zeroes bit-strings,
// each of which has length "bm" bits
//TODO

for (int i = 0; i < pm; i++) {
PageID p_pid = p->bsigNpages - 1;
Page bpage = getPage(bsigFile(r), p_pid);
Bits bsig = newBits(bsigBits(r));
if (pageNitems(bpage) == maxBsigsPP(r)) {
addPage(bsigFile(r));
p->bsigNpages++;
p_pid++;
free(bpage);
bpage = newPage();
if (bpage == NULL) return NO_PAGE;
}
putBits(bpage, pageNitems(bpage), bsig);
addOneItem(bpage);
p->nbsigs++;
putPage(bsigFile(r), p_pid, bpage);
freeBits(bsig);
}
closeRelation(r);
return 0;
}
