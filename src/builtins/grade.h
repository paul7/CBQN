#define GRADE_CAT(N) CAT(GRADE_UD(gradeUp,gradeDown),N)
#define GRADE_NEG GRADE_UD(,-)

// Timsort
#define SORT_CMP(W, X) GRADE_NEG compare(W, X)
#define SORT_NAME GRADE_UD(bA,bD)
#define SORT_TYPE B
#include "sortTemplate.h"

#define SORT_CMP(W, X) GRADE_NEG compare((W).k, (X).k)
#define SORT_NAME GRADE_CAT(BP)
#define SORT_TYPE BI32p
#include "sortTemplate.h"

#define SORT_CMP(W, X) (GRADE_NEG ((W).k - (i64)(X).k))
#define SORT_NAME GRADE_CAT(IP)
#define SORT_TYPE I32I32p
#include "sortTemplate.h"


#define LT GRADE_UD(<,>)
#define FOR(I,MAX) GRADE_UD(for (usz I=0; I<MAX; I++), \
                            for (usz I=MAX; I--; ))

#define INSERTION_SORT(T) \
  rp[0] = xp[0];                                             \
  for (usz i=0; i<n; i++) {                                  \
    usz j=i, jn; T xi=xp[i], rj;                             \
    while (0<j && xi LT (rj=rp[jn=j-1])) { rp[j]=rj; j=jn; } \
    rp[j] = xi;                                              \
  }

#if SINGELI
extern void (*const avx2_scan_max8)(int8_t* v0,int8_t* v1,uint64_t v2);
extern void (*const avx2_scan_min8)(int8_t* v0,int8_t* v1,uint64_t v2);
extern void (*const avx2_scan_max16)(int16_t* v0,int16_t* v1,uint64_t v2);
extern void (*const avx2_scan_min16)(int16_t* v0,int16_t* v1,uint64_t v2);
#define COUNT_THRESHOLD 32
#define WRITE_SPARSE_i8 \
  for (usz i=0; i<n; i++) rp[i]=j;                       \
  while (ij<n) { rp[ij]=GRADE_UD(++j,--j); ij+=c0o[j]; } \
  GRADE_UD(avx2_scan_max8,avx2_scan_min8)(rp,rp,n);
#define WRITE_SPARSE_i16 \
  usz b = 1<<10;                                              \
  for (usz k=0; ; ) {                                         \
    usz e = b<n-k? k+b : n;                                   \
    for (usz i=k; i<e; i++) rp[i]=j;                          \
    while (ij<e) { rp[ij]=GRADE_UD(++j,--j); ij+=c0o[j]; }    \
    GRADE_UD(avx2_scan_max16,avx2_scan_min16)(rp+k,rp+k,e-k); \
    if (e==n) {break;}  k=e;                                  \
  }
#define WRITE_SPARSE(T) WRITE_SPARSE_##T
#else
#define COUNT_THRESHOLD 16
#define WRITE_SPARSE(T) \
  for (usz i=0; i<n; i++) rp[i]=0;                       \
  usz js = j;                                            \
  while (ij<n) { rp[ij]GRADE_UD(++,--); ij+=c0o[GRADE_UD(++j,--j)]; } \
  for (usz i=0; i<n; i++) js=rp[i]+=js;
#endif

#define COUNTING_SORT(T) \
  usz C=1<<(8*sizeof(T));                              \
  TALLOC(usz, c0, C); usz *c0o=c0+C/2;                 \
  for (usz j=0; j<C; j++) c0[j]=0;                     \
  for (usz i=0; i<n; i++) c0o[xp[i]]++;                \
  if (n/(COUNT_THRESHOLD*sizeof(T)) <= C) { /* Scan-based */ \
    T j=GRADE_UD(-C/2,C/2-1);                          \
    usz ij; while ((ij=c0o[j])==0) GRADE_UD(j++,j--);  \
    WRITE_SPARSE(T)                                    \
  } else { /* Branchy */                               \
    FOR(j,C) for (usz c=c0[j]; c--; ) *rp++ = j-C/2;   \
  }                                                    \
  TFREE(c0)

// Radix sorting
#define INC(P,I) GRADE_UD((P+1)[I]++,P[I]--)
#define ROFF GRADE_UD(1,0) // Radix offset

#define CHOOSE_SG_SORT(S,G) S
#define CHOOSE_SG_GRADE(S,G) G

#define RADIX_SORT_i8(T, TYP) \
  TALLOC(T, c0, 256+ROFF); T* c0o=c0+128;    \
  for (usz j=0; j<256; j++) c0[j]=0;       \
  GRADE_UD(,c0[0]=n;)                      \
  for (usz i=0; i<n; i++) INC(c0o,xp[i]);  \
  RADIX_SUM_1_##T;                         \
  for (usz i=0; i<n; i++) { i8 xi=xp[i];   \
    rp[c0o[xi]++]=CHOOSE_SG_##TYP(xi,i); } \
  TFREE(c0)

#define RADIX_SORT_i16(T, TYP, I) \
  TALLOC(u8, alloc, (2*256+ROFF)*sizeof(T) + n*(2 + CHOOSE_SG_##TYP(0,sizeof(I)))); \
  T* c0=(T*)alloc; T* c1=c0+256; T* c1o=c1+128;                              \
  for (usz j=0; j<2*256; j++) c0[j]=0;                                       \
  c1[0]=GRADE_UD(-n,c0[0]=n);                                                \
  for (usz i=0; i<n; i++) { i16 v=xp[i]; INC(c0,(u8)v); INC(c1o,(i8)(v>>8)); } \
  RADIX_SUM_2_##T;                                                           \
  i16 *r0 = (i16*)(c0+2*256);                                                \
  CHOOSE_SG_##TYP(                                                           \
  for (usz i=0; i<n; i++) { i16 v=xp[i]; r0[c0 [(u8)v     ]++]=v; }          \
  for (usz i=0; i<n; i++) { i16 v=r0[i]; rp[c1o[(i8)(v>>8)]++]=v; }          \
  ,                                                                          \
  I *g0 = (i32*)(r0+n);                                                      \
  for (usz i=0; i<n; i++) { i16 v=xp[i]; T c=c0[(u8)v     ]++; r0[c]=v; g0[c]=i; } \
  for (usz i=0; i<n; i++) { i16 v=r0[i]; rp[c1o[(i8)(v>>8)]++]=g0[i]; }      \
  )                                                                          \
  TFREE(alloc)

#define RADIX_SORT_i32(T, TYP, I) \
  TALLOC(u8, alloc, (4*256+ROFF)*sizeof(T) + n*(4 + CHOOSE_SG_##TYP(0,4+sizeof(I)))); \
  T *c0=(T*)alloc, *c1=c0+256, *c2=c1+256, *c3=c2+256, *c3o=c3+128;          \
  for (usz j=0; j<4*256; j++) c0[j]=0;                                       \
  c1[0]=c2[0]=c3[0]=GRADE_UD(-n,c0[0]=n);                                    \
  for (usz i=0; i<n; i++) { i32 v=xp[i];                                     \
    INC(c0 ,(u8)v      ); INC(c1 ,(u8)(v>> 8));                              \
    INC(c2 ,(u8)(v>>16)); INC(c3o,(i8)(v>>24)); }                            \
  RADIX_SUM_4_##T;                                                           \
  i32 *r0 = (i32*)(c0+4*256);                                                \
  CHOOSE_SG_##TYP(                                                           \
  for (usz i=0; i<n; i++) { i32 v=xp[i]; T c=c0 [(u8)v      ]++; r0[c]=v; }  \
  for (usz i=0; i<n; i++) { i32 v=r0[i]; T c=c1 [(u8)(v>> 8)]++; rp[c]=v; }  \
  for (usz i=0; i<n; i++) { i32 v=rp[i]; T c=c2 [(u8)(v>>16)]++; r0[c]=v; }  \
  for (usz i=0; i<n; i++) { i32 v=r0[i]; T c=c3o[(i8)(v>>24)]++; rp[c]=v; }  \
  ,                                                                          \
  i32 *r1 = r0+n; I *g0 = (i32*)(r1+n);                                      \
  for (usz i=0; i<n; i++) { i32 v=xp[i]; T c=c0 [(u8)v      ]++; r0[c]=v; g0[c]=i;     } \
  for (usz i=0; i<n; i++) { i32 v=r0[i]; T c=c1 [(u8)(v>> 8)]++; r1[c]=v; rp[c]=g0[i]; } \
  for (usz i=0; i<n; i++) { i32 v=r1[i]; T c=c2 [(u8)(v>>16)]++; r0[c]=v; g0[c]=rp[i]; } \
  for (usz i=0; i<n; i++) { i32 v=r0[i]; T c=c3o[(i8)(v>>24)]++;          rp[c]=g0[i]; } \
  )                                                                          \
  TFREE(alloc)

#define PRE(K) s##K=c##K[j]+=s##K
#define RADIX_SUM_1(T)                                  T s0=0;                   for(usz j=0;j<256;j++) { PRE(0); }
#define RADIX_SUM_2(T)  GRADE_UD(c1[0]=0;,)             T s0=0, s1=0;             for(usz j=0;j<256;j++) { PRE(0); PRE(1); }
#define RADIX_SUM_4(T)  GRADE_UD(c1[0]=c2[0]=c3[0]=0;,) T s0=0, s1=0, s2=0, s3=0; for(usz j=0;j<256;j++) { PRE(0); PRE(1); PRE(2); PRE(3); }

#if SINGELI
extern void (*const avx2_scan_pluswrap_u8)(uint8_t* v0,uint8_t* v1,uint64_t v2,uint8_t v3);
extern void (*const avx2_scan_pluswrap_u32)(uint32_t* v0,uint32_t* v1,uint64_t v2,uint32_t v3);
#define RADIX_SUM_1_u8   avx2_scan_pluswrap_u8 (c0,c0,  256,0);
#define RADIX_SUM_2_u8   avx2_scan_pluswrap_u8 (c0,c0,2*256,0);
#define RADIX_SUM_2_u32  avx2_scan_pluswrap_u32(c0,c0,2*256,0);
#define RADIX_SUM_4_u8   avx2_scan_pluswrap_u8 (c0,c0,4*256,0);
#define RADIX_SUM_4_u32  avx2_scan_pluswrap_u32(c0,c0,4*256,0);
#else
#define RADIX_SUM_1_u8   RADIX_SUM_1(u8)
#define RADIX_SUM_2_u8   RADIX_SUM_2(u8)
#define RADIX_SUM_2_u32  RADIX_SUM_2(u32)
#define RADIX_SUM_4_u8   RADIX_SUM_4(u8)
#define RADIX_SUM_4_u32  RADIX_SUM_4(u32)
#endif

#if SINGELI && !USZ_64
#define RADIX_SUM_1_usz  avx2_scan_pluswrap_u32(c0,c0,  256,0);
#define RADIX_SUM_2_usz  avx2_scan_pluswrap_u32(c0,c0,2*256,0);
#define RADIX_SUM_4_usz  avx2_scan_pluswrap_u32(c0,c0,4*256,0);
#else
#define RADIX_SUM_1_usz  RADIX_SUM_1(usz)
#define RADIX_SUM_2_usz  RADIX_SUM_2(usz)
#define RADIX_SUM_4_usz  RADIX_SUM_4(usz)
#endif

#define SORT_C1 CAT(GRADE_UD(and,or),c1)
B SORT_C1(B t, B x) {
  if (isAtm(x) || RNK(x)==0) thrM(GRADE_UD("∧","∨")": Argument cannot have rank 0");
  if (RNK(x)!=1) return bqn_merge(SORT_C1(t, toCells(x)));
  usz n = IA(x);
  if (n <= 1) return x;
  u8 xe = TI(x,elType);
  B r;
  if (xe==el_bit) {
    u64* xp = bitarr_ptr(x);
    u64* rp; r = m_bitarrv(&rp, n);
    usz sum = bit_sum(xp, n);
    u64 n0 = GRADE_UD(n-sum, sum);
    u64 ones = -1ull;
    u64 v0 = GRADE_UD(0, ones);
    usz i=0, e=(n+63)/64;
    for (; i<n0/64; i++) rp[i]=v0;
    if (i<e) rp[i++]=v0^(ones<<(n0%64));
    for (; i<e; i++) rp[i]=~v0;
  } else if (xe==el_i8) {
    i8* xp = i8any_ptr(x);
    i8* rp; r = m_i8arrv(&rp, n);
    if (n < 16) {
      INSERTION_SORT(i8);
    } else if (n < 256) {
      RADIX_SORT_i8(u8, SORT);
    } else {
      COUNTING_SORT(i8);
    }
  } else if (xe==el_i16) {
    i16* xp = i16any_ptr(x);
    i16* rp; r = m_i16arrv(&rp, n);
    if (n < 20) {
      INSERTION_SORT(i16);
    } else if (n < 256) {
      RADIX_SORT_i16(u8, SORT,);
    } else if (n < 1<<15) {
      RADIX_SORT_i16(u32, SORT,);
    } else {
      COUNTING_SORT(i16);
    }
  } else if (xe==el_i32) {
    i32* xp = i32any_ptr(x);
    i32* rp; r = m_i32arrv(&rp, n);
    if (n < 32) {
      INSERTION_SORT(i32);
    } else if (n < 256) {
      RADIX_SORT_i32(u8, SORT,);
    } else {
      RADIX_SORT_i32(u32, SORT,);
    }
  } else {
    B xf = getFillQ(x);
    HArr* r0 = cpyHArr(inc(x));
    CAT(GRADE_UD(bA,bD),tim_sort)(r0->a, n);
    r = withFill(taga(r0), xf);
  }
  decG(x);
  return FL_SET(r, CAT(fl,GRADE_UD(asc,dsc)));
}
#undef SORT_C1
#undef INSERTION_SORT
#undef COUNTING_SORT
#if SINGELI
#undef WRITE_SPARSE_i8
#undef WRITE_SPARSE_i16
#endif


#define GRADE_CHR GRADE_UD("⍋","⍒")
B GRADE_CAT(c1)(B t, B x) {
  if (isAtm(x) || RNK(x)==0) thrM(GRADE_CHR": Argument cannot be a unit");
  if (RNK(x)>1) x = toCells(x);
  usz ia = IA(x);
  if (ia>I32_MAX) thrM(GRADE_CHR": Argument too large");
  if (ia==0) { decG(x); return emptyIVec(); }
  
  u8 xe = TI(x,elType);
  i32* rp; B r = m_i32arrv(&rp, ia);
  if (xe==el_bit) {
    u64* xp = bitarr_ptr(x);
    u64 sum = bit_sum(xp, ia);
    u64 r0 = 0;
    u64 r1 = GRADE_UD(ia-sum, sum);
    for (usz i = 0; i < ia; i++) {
      if (bitp_get(xp,i)^GRADE_UD(0,1)) rp[r1++] = i;
      else                              rp[r0++] = i;
    }
    decG(x); return r;
  } else if (xe==el_i8 && ia>8) {
    i8* xp = i8any_ptr(x); usz n=ia;
    RADIX_SORT_i8(usz, GRADE);
    decG(x); return r;
  } else if (xe==el_i16 && ia>16) {
    i16* xp = i16any_ptr(x); usz n = ia;
    RADIX_SORT_i16(usz, GRADE, i32);
    decG(x); return r;
  }
  if (xe==el_i32 || xe==el_c32) { // safe to use the same comparison for i32 & c32 as c32 is 0≤x≤1114111
    i32* xp = tyany_ptr(x);
    i32 min=I32_MAX, max=I32_MIN;
    i32 sum=0;
    for (usz i = 0; i < ia; i++) {
      i32 c = xp[i];
      sum += c;
      if (c<min) min=c;
      if (c>max) max=c;
    }
    u64 range = max - (i64)min + 1;
    if (range/2 < ia) {
      // First try to invert it as a permutation
      if (range==ia && sum==(i32)((i64)ia*(min+max)/2)) {
        for (usz i = 0; i < ia; i++) rp[i]=ia;
        for (usz i = 0; i < ia; i++) { i32 v=xp[i]; GRADE_UD(rp[v-min],rp[max-v])=i; }
        bool done=1; for (usz i = 0; i < ia; i++) done &= rp[i]!=ia;
        if (done) { decG(x); return r; }
      }
      TALLOC(usz, c0, range); usz *c0o=c0-min;
      for (usz i = 0; i < range; i++) c0[i] = 0;
      for (usz i = 0; i < ia; i++) c0o[xp[i]]++;
      usz s=0; FOR (i, range) { usz p=s; s+=c0[i]; c0[i]=p; }
      for (usz i = 0; i < ia; i++) rp[c0o[xp[i]]++] = i;
      TFREE(c0); decG(x);
      return r;
    }
    if (ia > 40) {
      usz n=ia;
      RADIX_SORT_i32(usz, GRADE, i32);
      decG(x); return r;
    }
    
    TALLOC(I32I32p, tmp, ia);
    for (usz i = 0; i < ia; i++) {
      tmp[i].v = i;
      tmp[i].k = xp[i];
    }
    CAT(GRADE_CAT(IP),tim_sort)(tmp, ia);
    for (usz i = 0; i < ia; i++) rp[i] = tmp[i].v;
    TFREE(tmp); decG(x);
    return r;
  }
  
  SLOW1(GRADE_CHR"𝕩", x);
  TALLOC(BI32p, tmp, ia);
  SGetU(x)
  for (usz i = 0; i < ia; i++) {
    tmp[i].v = i;
    tmp[i].k = GetU(x,i);
  }
  CAT(GRADE_CAT(BP),tim_sort)(tmp, ia);
  for (usz i = 0; i < ia; i++) rp[i] = tmp[i].v;
  TFREE(tmp); decG(x);
  return r;
}


B GRADE_CAT(c2)(B t, B w, B x) {
  if (isAtm(w) || RNK(w)==0) thrM(GRADE_CHR": 𝕨 must have rank≥1");
  if (isAtm(x)) x = m_atomUnit(x);
  ur wr = RNK(w);
  ur xr = RNK(x);
  
  if (wr > 1) {
    if (wr > xr+1) thrM(GRADE_CHR": =𝕨 cannot be greater than =𝕩");
    i32 nxr = xr-wr+1;
    x = toKCells(x, nxr); xr = nxr;
    w = toCells(w);       xr = 1;
  }
  
  u8 we = TI(w,elType); usz wia = IA(w);
  u8 xe = TI(x,elType); usz xia = IA(x);
  
  if (wia>I32_MAX-10) thrM(GRADE_CHR": 𝕨 too big");
  i32* rp; B r = m_i32arrc(&rp, x);
  
  u8 fl = GRADE_UD(fl_asc,fl_dsc);
  
  if (LIKELY(we<el_B & xe<el_B)) {
    if (elNum(we)) { // number
      if (elNum(xe)) {
        if (!elInt(we) | !elInt(xe)) goto gen;
        w=toI32Any(w); x=toI32Any(x);
      } else {
        for (u64 i=0; i<xia; i++) rp[i]=wia;
        goto done;
      }
    } else { // character
      if (elNum(xe)) {
        Arr* ra=allZeroes(xia); arr_shVec(ra);
        decG(r); r=taga(ra); goto done;
      } else {
        w=toC32Any(w); x=toC32Any(x);
      }
    }
    i32* wi = tyany_ptr(w);
    i32* xi = tyany_ptr(x);
    if (CHECK_VALID && !FL_HAS(w,fl)) {
      for (i64 i = 0; i < (i64)wia-1; i++) if ((wi[i]-wi[i+1]) GRADE_UD(>,<) 0) thrM(GRADE_CHR": 𝕨 must be sorted"GRADE_UD(," in descending order"));
      FL_SET(w, fl);
    }
    
    for (usz i = 0; i < xia; i++) {
      i32 c = xi[i];
      i32 *s = wi-1;
      for (usz l = wia+1, h; (h=l/2)>0; l-=h) s += h * !(c LT s[h]);
      rp[i] = s - (wi-1);
    }
  } else {
gen:
    SGetU(x)
    SLOW2("𝕨"GRADE_CHR"𝕩", w, x);
    B* wp = arr_bptr(w);
    if (wp==NULL) {
      HArr* a = toHArr(w);
      wp = a->a;
      w = taga(a);
    }
    if (CHECK_VALID && !FL_HAS(w,fl)) {
      for (i64 i = 0; i < (i64)wia-1; i++) if (compare(wp[i], wp[i+1]) GRADE_UD(>,<) 0) thrM(GRADE_CHR": 𝕨 must be sorted"GRADE_UD(," in descending order"));
      FL_SET(w, fl);
    }
    
    for (usz i = 0; i < xia; i++) {
      B c = GetU(x,i);
      usz s = 0, e = wia+1;
      while (e-s > 1) {
        usz m = (s+e) / 2;
        if (compare(c, wp[m-1]) LT 0) e = m;
        else s = m;
      }
      rp[i] = s;
    }
  }
done:
  decG(w);decG(x);
  return r;
}
#undef GRADE_CHR

#undef LT
#undef FOR
#undef PRE
#undef INC
#undef ROFF
#undef PRE64
#undef CHOOSE_SG_SORT
#undef CHOOSE_SG_GRADE
#undef RADIX_SORT_i8
#undef RADIX_SORT_i16
#undef RADIX_SORT_i32
#undef RADIX_SUM_1
#undef RADIX_SUM_2
#undef RADIX_SUM_4
#undef RADIX_SUM_1_u8
#undef RADIX_SUM_1_usz
#undef RADIX_SUM_2_u8
#undef RADIX_SUM_2_usz
#undef RADIX_SUM_2_u32
#undef RADIX_SUM_4_u8
#undef RADIX_SUM_4_usz
#undef RADIX_SUM_4_u32
#undef GRADE_CAT
#undef GRADE_NEG
#undef GRADE_UD
