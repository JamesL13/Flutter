#ifndef GRIM_PODALLOC_H
#define GRIM_PODALLOC_H

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "grim_podalloc_config.h"

#define _GPA_STRINGIFY_1(x) #x
#define _GPA_STRINGIFY(x) _GPA_STRINGIFY_1(x)

#ifndef GRIM_PODALLOC_VERBOSITY
    #define GRIM_PODALLOC_VERBOSITY 0
#endif

#define GRIM_PODALLOC_NEWRECORD_REPORT(T, _, __) {}
#define GRIM_PODALLOC_CLEANUP_REPORT(T, _)       {}
#define GRIM_PODALLOC_ALLOC_REPORT(T, _, __)     {}
#define GRIM_PODALLOC_FREE_REPORT(T, _, __)      {}

#if (GRIM_PODALLOC_VERBOSITY >= 1)
    #undef GRIM_PODALLOC_CLEANUP_REPORT
    #define GRIM_PODALLOC_CLEANUP_REPORT(T, pPod)                           \
        {                                                                   \
            GRIMMEMREC(T) *pRec;                                            \
            fprintf(GRIM_PODALLOC_OUTFILE,                                  \
                "GRIM_PODALLOC @%p (type:" _GPA_STRINGIFY(T)                \
                ") CLEANUP REPORT ::\n", (pPod));                           \
                                                                            \
            fprintf(GRIM_PODALLOC_OUTFILE,                                  \
                "  first=%p, last=%p, top=%p, bottom=%p\n"                  \
                "  allocCount=%u, freeCount=%u, recordLength=%u,\n"         \
                "  recordBlocks=%u, blockDatOffset=%u\n",                   \
                (pPod)->first, (pPod)->last,                                \
                (pPod)->top, (pPod)->bottom,                                \
                (pPod)->allocCount, (pPod)->freeCount,                      \
                (pPod)->recordLength, (pPod)->recordBlocks,                 \
                (unsigned)(pPod)->blockDatOffset);                          \
                                                                            \
            for (pRec = (pPod)->first; pRec; pRec = pRec->next)             \
            {                                                               \
                fprintf(GRIM_PODALLOC_OUTFILE,                              \
                    "  PRED @%p, SUCC @%p, BASE @%p, "                      \
                    "Alloc'd %u block(s), total %u.\n",                     \
                    pRec->prev, pRec->next, GRIMMEMRECBLOCK(T, pRec, 0, 0), \
                    pRec->nAlloc, pRec->nBlock);                            \
            }                                                               \
        }

    #if (GRIM_PODALLOC_VERBOSITY >= 2)
        #undef  GRIM_PODALLOC_NEWRECORD_REPORT
        #define GRIM_PODALLOC_NEWRECORD_REPORT(T, pPod, pRec)               \
            {                                                               \
                fprintf(GRIM_PODALLOC_OUTFILE,                              \
                    "GRIM_PODALLOC @%p (type:" _GPA_STRINGIFY(T) ") "       \
                    "has new record @%p"                                    \
                    " with %u blocks @%p\n",                                \
                    (pPod), (pRec), (pRec)->nBlock,                         \
                    GRIMMEMRECBLOCK(T, pRec, 0, 0));                        \
            }

        #if (GRIM_PODALLOC_VERBOSITY >= 3)
            #undef  GRIM_PODALLOC_ALLOC_REPORT
            #define GRIM_PODALLOC_ALLOC_REPORT(T, pPod, pBlk)               \
            {                                                               \
                fprintf(GRIM_PODALLOC_OUTFILE,                              \
                    "GRIM_PODALLOC @%p (type:" _GPA_STRINGIFY(T) ") "       \
                    " alloc'd block @%p, data @%p from record @%p\n"        \
                    "  with %u blocks now alloc'd in record.\n",            \
                    (pPod), (pBlk), &((pBlk)->_dat), (pBlk)->source,        \
                    (pBlk)->source->nAlloc);                                \
            }

            #undef  GRIM_PODALLOC_FREE_REPORT
            #define GRIM_PODALLOC_FREE_REPORT(T, pPod, pBlk)                \
            {                                                               \
                fprintf(GRIM_PODALLOC_OUTFILE,                              \
                    "GRIM_PODALLOC @%p (type:" _GPA_STRINGIFY(T) ") "       \
                    " free'd block @%p, data @%p from record @%p\n"         \
                    "  with %u blocks now alloc'd in record.\n",            \
                    (pPod), (pBlk), &((pBlk)->_dat), (pBlk)->source,        \
                    (pBlk)->source->nAlloc);                                \
            }
        #endif
    #endif
#endif

#define GRIMMEMBLK(T)  struct _MBLK##T
#define GRIMMEMREC(T)  struct _MREC##T
#define GRIMMEMPOD(T)  struct _MPOD##T

#define GRIMMEMPODOBJBLK(T, pPod, p) \
    ((GRIMMEMBLK(T) *)(((char *)(p)) - (pPod)->blockDatOffset))

#define GRIMMEMBLKDEF(T)                    \
    GRIMMEMREC(T);                          \
    GRIMMEMBLK(T)                           \
    {                                       \
        GRIMMEMBLK(T) *next;                \
        GRIMMEMREC(T) *source;              \
        T _dat;                             \
    };

#define GRIMMEMRECDEF(T)                    \
    GRIMMEMREC(T)                           \
    {                                       \
        GRIMMEMREC(T) *prev, *next;         \
        unsigned nBlock, nAlloc;            \
        unsigned __base;                    \
    };

#define GRIMMEMRECBLOCK(T, pRec, _I, BLOCKSIZ) \
    ((GRIMMEMBLK(T) *)                         \
    (((char *)(pRec) + offsetof(GRIMMEMREC(T), __base)) + ((BLOCKSIZ)*_I)))

#define GRIMMEMPODMKREC(T, pPod)                                        \
    {                                                                   \
        GRIMMEMREC(T) *pRec;                                            \
                                                                        \
        register unsigned i;                                            \
        unsigned n = (pPod)->recordBlocks;                              \
        unsigned siz = (pPod)->blockSize;                               \
                                                                        \
        pRec = (GRIMMEMREC(T) *)malloc(                                 \
            sizeof(GRIMMEMREC(T)) + (pPod)->recordLength);              \
        memset(GRIMMEMRECBLOCK(T, pRec, 0, 0), 0, (pPod)->recordLength);\
        pRec->nBlock = n;                                               \
        pRec->nAlloc = 0;                                               \
                                                                        \
        if ((pPod)->top == NULL)                                        \
        {                                                               \
            (pPod)->top=(pPod)->bottom=GRIMMEMRECBLOCK(T, pRec, 0, 0);  \
            GRIMMEMRECBLOCK(T, pRec, 0, 0)->source = pRec;              \
            GRIMMEMRECBLOCK(T, pRec, 0, 0)->next = NULL;                \
            i = 1;                                                      \
        }                                                               \
        else                                                            \
            i = 0;                                                      \
                                                                        \
        for (; i < n; ++i)                                              \
        {                                                               \
            GRIMMEMRECBLOCK(T, pRec, i, siz)->source = pRec;            \
            GRIMMEMRECBLOCK(T, pRec, i, siz)->next   = NULL;            \
            (pPod)->bottom->next = GRIMMEMRECBLOCK(T, pRec, i, siz);    \
            (pPod)->bottom       = GRIMMEMRECBLOCK(T, pRec, i, siz);    \
        }                                                               \
                                                                        \
        if (!pPod->first)                                               \
        {                                                               \
            pRec->prev = pRec->next = NULL;                             \
            (pPod)->first = (pPod)->last = pRec;                        \
        }                                                               \
        else                                                            \
        {                                                               \
            (pPod)->first->prev = pRec;                                 \
            pRec->next = (pPod)->first;                                 \
            (pPod)->first = pRec;                                       \
        }                                                               \
                                                                        \
        GRIM_PODALLOC_NEWRECORD_REPORT(T, (pPod), pRec)                 \
    }

#define GRIMMEMPODDEF(T)                                                \
    GRIMMEMPOD(T)                                                       \
    {                                                                   \
        GRIMMEMREC(T) *first, *last;                                    \
        GRIMMEMBLK(T) *top, *bottom;                                    \
        unsigned allocCount;                                            \
        unsigned freeCount;                                             \
        unsigned blockSize;                                             \
        unsigned recordLength;                                          \
        unsigned recordBlocks;                                          \
        ptrdiff_t blockDatOffset;                                       \
    };                                                                  \
                                                                        \
    static inline T *_GRIM_MPODALLOC##T(GRIMMEMPOD(T) *pPod)            \
    {                                                                   \
        GRIMMEMBLK(T) *blk;                                             \
                                                                        \
        ++pPod->allocCount;                                             \
        if (pPod->top == NULL)                                          \
        {                                                               \
            if (((double)pPod->allocCount / (double)(pPod->freeCount+1))\
                >= GRIM_PODALLOC_GROWTH_HEURISTIC)                      \
                pPod->recordBlocks = (pPod->recordLength<<=1) /         \
                                     pPod->blockSize;                   \
            GRIMMEMPODMKREC(T, (pPod));                                 \
        }                                                               \
                                                                        \
        blk = pPod->top;                                                \
        pPod->top = blk->next;                                          \
        if (pPod->top == NULL) pPod->bottom = NULL;                     \
                                                                        \
        ++blk->source->nAlloc;                                          \
                                                                        \
        GRIM_PODALLOC_ALLOC_REPORT(T, pPod, blk)                        \
        return &blk->_dat;                                              \
    }


#define GRIM_PODALLOCDEF(T) \
    GRIMMEMBLKDEF(T)        \
    GRIMMEMRECDEF(T)        \
    GRIMMEMPODDEF(T)

typedef struct _dynPodAllocBlk { unsigned _; } _dynPAB_;

GRIMMEMBLKDEF(_dynPAB_)
GRIMMEMRECDEF(_dynPAB_)
GRIMMEMPODDEF(_dynPAB_)

#define GRIM_PODALLOCTYPE(T) GRIMMEMPOD(T)
#define GRIM_DYNPODALLOCTYPE GRIMMEMPOD(_dynPAB_)

#define GRIM_PODALLOCINIT(T, pPod, recordLen)                           \
{                                                                       \
    (pPod)->first  = NULL;                                              \
    (pPod)->last   = NULL;                                              \
    (pPod)->top    = NULL;                                              \
    (pPod)->bottom = NULL;                                              \
    (pPod)->allocCount = 0;                                             \
    (pPod)->freeCount  = 0;                                             \
    (pPod)->blockSize  = sizeof(GRIMMEMBLK(T));                         \
    (pPod)->recordLength = (recordLen)*(pPod)->blockSize;               \
    (pPod)->recordBlocks = (pPod)->recordLength/(pPod)->blockSize;      \
    (pPod)->blockDatOffset = offsetof(GRIMMEMBLK(T), _dat);             \
}

#define GRIM_DYNPODALLOCINIT(_BLKSIZ, pPod, recordLen)                  \
{                                                                       \
    (pPod)->first  = NULL;                                              \
    (pPod)->last   = NULL;                                              \
    (pPod)->top    = NULL;                                              \
    (pPod)->bottom = NULL;                                              \
    (pPod)->allocCount = 0;                                             \
    (pPod)->freeCount  = 0;                                             \
    (pPod)->blockSize  = sizeof(GRIMMEMBLK(_dynPAB_))+(_BLKSIZ);        \
    (pPod)->recordLength = (recordLen)*(pPod)->blockSize;               \
    (pPod)->recordBlocks = (pPod)->recordLength/(pPod)->blockSize;      \
    (pPod)->blockDatOffset = offsetof(GRIMMEMBLK(_dynPAB_), _dat);      \
}

#define GRIM_PODALLOCCLEAN(T, pPod)                                     \
{                                                                       \
    GRIMMEMREC(T) *_pRec, *_pNext;                                      \
                                                                        \
    GRIM_PODALLOC_CLEANUP_REPORT(T, (pPod))                             \
    for (_pRec = (pPod)->first; _pRec; _pRec = _pNext)                  \
    {                                                                   \
        _pNext = _pRec->next;                                           \
        free((void *)_pRec);                                            \
    }                                                                   \
                                                                        \
    GRIM_PODALLOCINIT(T, (pPod), (pPod)->recordLength)                  \
} 

#define GRIM_PODALLOCALLOC(T, pPod) (_GRIM_MPODALLOC##T((pPod)))
#define GRIM_PODALLOCFREE(T, pPod, p)                                   \
{                                                                       \
    GRIMMEMBLK(T) *_pBlk = GRIMMEMPODOBJBLK(T, (pPod), (p));            \
                                                                        \
    --_pBlk->source->nAlloc;                                            \
                                                                        \
    if ((pPod)->bottom)                                                 \
    {                                                                   \
        (pPod)->bottom->next = _pBlk;                                   \
        (pPod)->bottom = _pBlk;                                         \
    }                                                                   \
    else                                                                \
        (pPod)->top = (pPod)->bottom = _pBlk;                           \
                                                                        \
    ++(pPod)->freeCount;                                                \
    _pBlk->next = NULL;                                                 \
                                                                        \
    GRIM_PODALLOC_FREE_REPORT(T, (pPod), _pBlk)                         \
}                                                               

#define GRIM_DYNPODALLOCALLOC(pPod)   _GRIM_MPODALLOC_dynPAB_((pPod))
#define GRIM_DYNPODALLOCFREE(pPod, p) GRIM_PODALLOCFREE(_dynPAB_,(pPod),(p))
#define GRIM_DYNPODALLOCCLEAN(pPod)   GRIM_PODALLOCCLEAN(_dynPAB_,(pPod))

#endif


