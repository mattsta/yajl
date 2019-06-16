#pragma once

#include "../../datakit/src/fibbuf.h"

#ifndef COUNT_ARRAY
#define COUNT_ARRAY(x) (sizeof(x) / sizeof(*(x)))
#endif

/* ====================================================================
 * Box Storage
 * ==================================================================== */
#define yajlDualStorageCount(storage) ((storage)->count)

/* Grow storage to at least 'newHighestExtent' count. */
#define yajlDualStorageGrow(storage, newHighestExtent)                         \
    do {                                                                       \
        if ((newHighestExtent) >= yajlDualStorageLocalSize(storage)) {         \
            const size_t allocateHigh =                                        \
                (newHighestExtent)-yajlDualStorageLocalSize(storage);          \
            const size_t allocatedCount = (storage)->totalCountOfAllocated;    \
                                                                               \
            /* Not enough allocated space, so grow it. */                      \
            if (allocateHigh >= allocatedCount) {                              \
                const size_t growCount = fibbufNextSizeBuffer(allocateHigh);   \
                const size_t growSize =                                        \
                    growCount * sizeof(*(storage)->allocated);                 \
                                                                               \
                (storage)->allocated =                                         \
                    YA_REALLOC((storage)->allocated, growSize);                \
                                                                               \
                /* Update account details... */                                \
                (storage)->totalCountOfAllocated = growCount;                  \
            }                                                                  \
        }                                                                      \
    } while (0)

/* Get element size of local array for storing elements before
 * needing external allocations. */
#define yajlDualStorageLocalSize(storage) (COUNT_ARRAY((storage)->local))

/* Append 'what' to 'storage' and maybe extend memory if necessary. */
#define yajlDualStorageAppend(storage, what)                                   \
    ({                                                                         \
        const size_t count = (storage)->count;                                 \
        void *wrote;                                                           \
                                                                               \
        /* If we have a free entry in default storage space, place there. */   \
        if (count < yajlDualStorageLocalSize(storage)) {                       \
            (storage)->local[count] = *(what);                                 \
            wrote = &(storage)->local[count];                                  \
            (storage)->count++;                                                \
        } else {                                                               \
            /* else, we need to consult the on-demand allocated storage. */    \
            /* +1 because "appending" */                                       \
            yajlDualStorageGrow(storage, count + 1);                           \
            (storage)->allocated[count - yajlDualStorageLocalSize(storage)] =  \
                *(what);                                                       \
            (storage)->count++;                                                \
            wrote =                                                            \
                &(storage)                                                     \
                     ->allocated[count - yajlDualStorageLocalSize(storage)];   \
        }                                                                      \
                                                                               \
        wrote;                                                                 \
    })

#define yajlDualStorageMinimize(storage)                                       \
    do {                                                                       \
        if ((storage)->allocated) {                                            \
            (storage)->totalCountOfAllocated =                                 \
                (storage)->count - yajlDualStorageLocalSize(storage);          \
            (storage)->allocated = YA_REALLOC(                                 \
                (storage)->allocated, (storage)->totalCountOfAllocated *       \
                                          sizeof(*(storage)->allocated));      \
        }                                                                      \
    } while (0)

/* Reset by free'ing any allocation then zeroing out entire struct. */
#define yajlDualStorageReset(storage)                                          \
    do {                                                                       \
        if (storage) {                                                         \
            YA_FREE((storage)->allocated);                                     \
            memset(storage, 0, sizeof(*storage));                              \
        }                                                                      \
    } while (0)

/* Retrieve entry (no overflow checking) */
#define yajlDualStorageGet(storage, i)                                         \
    ((i) < yajlDualStorageLocalSize(storage)                                   \
         ? (storage)->local[i]                                                 \
         : (storage)->allocated[(i)-yajlDualStorageLocalSize(storage)])

/* Retrieve pointer to entry (no overflow checking) */
#define yajlDualStorageGetPtr(storage, i)                                      \
    ((i) < yajlDualStorageLocalSize(storage)                                   \
         ? &(storage)->local[i]                                                \
         : &(storage)->allocated[(i)-yajlDualStorageLocalSize(storage)])

#define yajlDualStorageGetPtrLast(storage)                                     \
    yajlDualStorageGetPtr(storage, yajlDualStorageCount(storage) - 1)

/* Generate a local function to grow memory then return requested index */
#define yajlDualStorageGenerateHelperFunctions(structName, innerStructName)    \
    static innerStructName *yajlDualBoxGrow##structName(                       \
        structName *const storage, const size_t i) {                           \
        yajlDualStorageGrow(storage, i + 1);                                   \
        return yajlDualStorageGetPtr(storage, i);                              \
    }

/* This is GetPtr, but if 'i' is too big, we allocate more entries.
 * This requires pre-declaring the extend-allocation-then-return-pointer
 * function by using macro 'yajlDualBoxGenerateHelpers()' */
#define yajlDualStorageGetPtrAllocate(structName, storage, i)                  \
    ((i) < yajlDualStorageLocalSize(storage)                                   \
         ? &(storage)->local[i]                                                \
         : ((i) < yajlDualStorageLocalSize(storage) +                          \
                        (storage)->totalCountOfAllocated                       \
                ? &(storage)->allocated[(i)-yajlDualStorageLocalSize(storage)] \
                : yajlDualBoxGrow##structName(storage, i)))

/* Wrappers for known struct definitions */
#define yajlDualBoxGenerateHelpers()                                           \
    yajlDualStorageGenerateHelperFunctions(yajlDualAllocationBox, databox)
#define yajlDualBoxGetPtrAllocate(storage, i)                                  \
    yajlDualStorageGetPtrAllocate(yajlDualAllocationBox, storage, i)
