#include "gc.h"
#include <sys/mman.h>

typedef struct EmptyValue EmptyValue;
struct EmptyValue { // needs set: mmInfo; type=t_empty; next; everything else can be garbage
  struct Value;
  EmptyValue* next;
};
extern u64 mm_heapAlloc;
extern u64 mm_heapMax;

#define  BSZ(X) (1ull<<(X))
#define BSZI(X) ((u8)(64-__builtin_clzl((X)-1ull)))
#define  MMI(X) X
#define   BN(X) mm_##X

#include "mm_buddyTemplate.h"

#ifdef OBJ_COUNTER
extern u64 currObjCounter;
#endif

static void* mm_allocN(usz sz, u8 type) {
  assert(sz>=16);
  onAlloc(sz, type);
  Value* r = mm_allocL(BSZI(sz), type);
  #ifdef OBJ_COUNTER
  r->uid = currObjCounter++;
  #endif
  return r;
}

static u64 mm_round(usz sz) {
  return BSZ(BSZI(sz));
}
static u64 mm_size(Value* x) {
  return BSZ(x->mmInfo&63);
}
void mm_forHeap(V2v f);

#undef BSZ
#undef BSZI