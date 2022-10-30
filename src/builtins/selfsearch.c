#include "../core.h"
#include "../utils/hash.h"
#include "../utils/talloc.h"

B not_c1(B t, B x);
B shape_c1(B t, B x);
B slash_c2(B t, B w, B x);

#define GRADE_UD(U,D) U
#include "radix.h"
u8 radix_offsets_2_u32(usz* c0, u32* v0, usz n) {
  usz rx = 256;
  usz* c1 = c0 + rx;
  // Count keys
  for (usz j=0; j<2*rx+1; j++) c0[j] = 0;
  for (usz i=0; i<n; i++) { u32 v=v0[i]; (c0+1)[(u8)(v>>16)]++; (c1+1)[(u8)(v>>24)]++; }
  u32 v=v0[0];
  // Inclusive prefix sum; note c offsets above
  if ((c1+1)[(u8)(v>>24)] < n) { c1[0]-=n; RADIX_SUM_2_u32; return 2; }
  if ((c0+1)[(u8)(v>>16)] < n) {           RADIX_SUM_1_u32; return 1; }
  return 0;
}
#undef GRADE_UD
#define RADIX_LOOKUP_32(INIT, SETTAB) \
  u8 bytes = radix_offsets_2_u32(c0, v0, n);                                                    \
  usz tim = tn/(64/sizeof(*tab)); /* sparse table init max */                                   \
  if (bytes==0) {                                                                               \
    if (n<tim) for (usz i=0; i< n; i++) tab[(u16)v0[i]]=INIT;                                   \
    else       for (usz j=0; j<tn; j++) tab[j]=INIT;                                            \
    for (usz i=0; i<n; i++) { u32 j=(u16)v0[i]; r0[i]=tab[j]; tab[j]SETTAB; }                   \
  } else {                                                                                      \
    if (bytes==1) { v1=v2; r1=r2; }                                                             \
    for (usz i=0; i<n; i++) { u32 v=v0[i]; u8 k=k0[i]=(u8)(v>>16); usz c=c0[k]++; v1[c]=v; }    \
    if (bytes==1) {                                                                             \
      /* Table lookup, getting radix boundaries from c0 */                                      \
      for (usz i=0; i<n; ) {                                                                    \
        usz l=c0[(u8)(v1[i]>>16)];                                                              \
        if (l-i < tim) for (usz ii=i; ii<l; ii++) tab[(u16)v1[ii]]=INIT;                        \
        else           for (usz  j=0; j<tn;  j++) tab[j]=INIT;                                  \
        for (; i<l; i++) { u32 j=(u16)v1[i]; r1[i]=tab[j]; tab[j]SETTAB; }                      \
      }                                                                                         \
    } else {                                                                                    \
      /* Radix move */                                                                          \
      for (usz i=0; i<n; i++) { u32 v=v1[i]; u8 k=k1[i]=(u8)(v>>24); usz c=c1[k]++; v2[c]=v; }  \
      /* Table lookup */                                                                        \
      u32 tv=v2[0]>>16; v2[n]=~v2[n-1];                                                         \
      for (usz l=0, i=0; l<n; ) {                                                               \
        for (;    ; l++) { u32 v=v2[l], t0=tv; tv=v>>16; if (tv!=t0) break; tab[(u16)v]=INIT; } \
        for (; i<l; i++) { u32 j=(u16)v2[i]; r2[i]=tab[j]; tab[j]SETTAB; }                      \
      }                                                                                         \
      /* Radix unmove; back up c0 to account for increments in radix step  */                   \
      *--c1=0; for (usz i=0; i<n; i++) { r1[i]=r2[c1[k1[i]]++]; }                               \
    }                                                                                           \
    *--c0=0; for (usz i=0; i<n; i++) { r0[i]=r1[c0[k0[i]]++]; }                                 \
  }                                                                                             \
  decG(x); TFREE(alloc);

B memberOf_c1(B t, B x) {
  if (isAtm(x) || RNK(x)==0) thrM("∊: Argument cannot have rank 0");
  usz n = *SH(x);
  if (n<=1) { decG(x); return n ? taga(arr_shVec(allOnes(1))) : emptyIVec(); }
  
  u8 lw = cellWidthLog(x);
  void* xv = tyany_ptr(x);
  if (lw == 0) {
    usz i = bit_find(xv, n, 1 &~ *(u64*)xv); decG(x);
    B r = taga(arr_shVec(allZeroes(n)));
    u64* rp = tyany_ptr(r);
    rp[0]=1; if (i<n) bitp_set(rp, i, 1);
    return r;
  }
  #define BRUTE(T) \
      i##T* xp = xv;                                                   \
      u64* rp; B r = m_bitarrv(&rp, n); bitp_set(rp, 0, 1);            \
      for (usz i=1; i<n; i++) {                                        \
        bool c=1; i##T xi=xp[i];                                       \
        for (usz j=0; j<i; j++) c &= xi!=xp[j];                        \
        bitp_set(rp, i, c);                                            \
      }                                                                \
      decG(x); return r;
  #define LOOKUP(T) \
    usz tn = 1<<T;                                                     \
    u##T* xp = (u##T*)xv;                                              \
    i8* rp; B r = m_i8arrv(&rp, n);                                    \
    TALLOC(u8, tab, tn);                                               \
    if (T>8 && n<tn/64) for (usz i=0; i<n;  i++) tab[xp[i]]=1;         \
    else                for (usz j=0; j<tn; j++) tab[j]=1;             \
    for (usz i=0; i<n; i++) { u##T j=xp[i]; rp[i]=tab[j]; tab[j]=0; }  \
    decG(x); TFREE(tab);                                               \
    return num_squeeze(r)
  if (lw == 3) { if (n<8) { BRUTE(8); } else { LOOKUP(8); } }
  if (lw == 4) { if (n<8) { BRUTE(16); } else { LOOKUP(16); } }
  #undef LOOKUP
  if (lw == 5) {
    if (n<=32) { BRUTE(32); }
    // Radix-assisted lookup
    usz rx = 256, tn = 1<<16; // Radix; table length
    u32* v0 = (u32*)xv;
    i8* r0; B r = m_i8arrv(&r0, n);
    
    TALLOC(u8, alloc, 6*n+(4+(tn>3*n?tn:3*n)+(2*rx+1)*sizeof(usz)));
    //                                         timeline
    // Allocations               len  count radix hash deradix     bytes  layout:
    usz *c0 = (usz*)(alloc)+1; // rx   [+++................]     c0   rx  #
    usz *c1 = (usz*)(c0+rx);   // rx    [++................]     c1   rx   #
    u8  *k0 = (u8 *)(c1+rx);   //  n        [+.............]     k0    n    ##
    u32 *v2 = (u32*)(k0+n);    //  n+1       [+.......]          v2  4*n+4    ########
    u8  *k1 = (u8 *)(v2+n+1);  //  n         [+............]     k1    n              ##
    u32 *v1 = (u32*)(k1);      //  n        [+-]                 v1  4*n              ########
    u8  *r2 = (u8 *)(v2);      //  n              [+.....]       r2    n      ##
    u8  *r1 = (u8 *)(k1+n);    //  n                   [+..]     r1    n                ##
    u8  *tab= (u8 *)(r1);      // tn              [+]            tab  tn                #####
   
    RADIX_LOOKUP_32(1, =0)
    return num_squeeze(r);
  }
  #undef BRUTE
  
  if (RNK(x)>1) x = toCells(x);
  u64* rp; B r = m_bitarrv(&rp, n);
  H_Sb* set = m_Sb(64);
  SGetU(x)
  for (usz i = 0; i < n; i++) bitp_set(rp, i, !ins_Sb(&set, GetU(x,i)));
  free_Sb(set); decG(x);
  return r;
}

B count_c1(B t, B x) {
  if (isAtm(x) || RNK(x)==0) thrM("⊒: Argument cannot have rank 0");
  usz n = *SH(x);
  if (n<=1) { decG(x); return n ? taga(arr_shVec(allZeroes(1))) : emptyIVec(); }
  if (n>(usz)I32_MAX+1) thrM("⊒: Argument length >2⋆31 not supported");
  
  u8 lw = cellWidthLog(x);
  if (lw==0) { x = toI8Any(x); lw = cellWidthLog(x); }
  void* xv = tyany_ptr(x);
  #define BRUTE(T) \
      i##T* xp = xv;                                           \
      i8* rp; B r = m_i8arrv(&rp, n); rp[0]=0;                 \
      for (usz i=1; i<n; i++) {                                \
        usz c=0; i##T xi=xp[i];                                \
        for (usz j=0; j<i; j++) c += xi==xp[j];                \
        rp[i] = c;                                             \
      }                                                        \
      decG(x); return r;
  #define LOOKUP(T) \
    usz tn = 1<<T;                                             \
    u##T* xp = (u##T*)xv;                                      \
    i32* rp; B r = m_i32arrv(&rp, n);                          \
    TALLOC(i32, tab, tn);                                      \
    if (T>8 && n<tn/16) for (usz i=0; i<n;  i++) tab[xp[i]]=0; \
    else                for (usz j=0; j<tn; j++) tab[j]=0;     \
    for (usz i=0; i<n;  i++) rp[i]=tab[xp[i]]++;               \
    decG(x); TFREE(tab);                                       \
    return num_squeeze(r)
  if (lw==3) { if (n<12) { BRUTE(8); } else { LOOKUP(8); } }
  if (lw==4) { if (n<12) { BRUTE(16); } else { LOOKUP(16); } }
  #undef LOOKUP
  if (lw==5) {
    if (n<=32) { BRUTE(32); }
    // Radix-assisted lookup
    usz rx = 256, tn = 1<<16; // Radix; table length
    u32* v0 = (u32*)xv;
    i32* r0; B r = m_i32arrv(&r0, n);
    
    TALLOC(u8, alloc, 6*n+(4+4*(tn>n?tn:n)+(2*rx+1)*sizeof(usz)));
    //                                         timeline
    // Allocations               len  count radix hash deradix     bytes  layout:
    usz *c0 = (usz*)(alloc)+1; // rx   [+++................]    c0    rx  #
    usz *c1 = (usz*)(c0+rx);   // rx    [++................]    c1    rx   #
    u8  *k0 = (u8 *)(c1+rx);   //  n        [+.............]    k0     n    ##
    u8  *k1 = (u8 *)(k0+n);    //  n         [+............]    k1     n      ##
    u32 *v2 = (u32*)(k1+n);    //  n+1       [+....-]           v2   4*n        ########
    u32 *v1 = (u32*)(v2+n+1);  //  n        [+..]               v1   4*n                ########
    u32 *r2 = (u32*)v2;        //  n              [+.....]      r2   4*n        ########
    u32 *r1 = (u32*)v1;        //  n                   [+..]    r1   4*n                ########
    u32 *tab= (u32*)v1;        // tn              [+]           tab 4*tn                ###########
    
    RADIX_LOOKUP_32(0, ++)
    return num_squeeze(r);
  }
  #undef BRUTE
  
  if (RNK(x)>1) x = toCells(x);
  i32* rp; B r = m_i32arrv(&rp, n);
  H_b2i* map = m_b2i(64);
  SGetU(x)
  for (usz i = 0; i < n; i++) {
    bool had; u64 p = mk_b2i(&map, GetU(x,i), &had);
    rp[i] = had? ++map->a[p].val : (map->a[p].val = 0);
  }
  decG(x); free_b2i(map);
  return r;
}

B indexOf_c1(B t, B x) {
  if (isAtm(x) || RNK(x)==0) thrM("⊐: 𝕩 cannot have rank 0");
  usz n = *SH(x);
  if (n<=1) { decG(x); return n ? taga(arr_shVec(allZeroes(1))) : emptyIVec(); }
  if (n>(usz)I32_MAX+1) thrM("⊐: Argument length >2⋆31 not supported");
  
  u8 lw = cellWidthLog(x);
  void* xv = tyany_ptr(x);
  if (lw == 0) {
    B r = 1&*(u64*)xv ? not_c1(m_f64(0), x) : x;
    return shape_c1(m_f64(0), r);
  }
  #define BRUTE(T) \
      i##T* xp = xv;                                           \
      i8* rp; B r = m_i8arrv(&rp, n); rp[0]=0;                 \
      TALLOC(i##T, uniq, n); uniq[0]=xp[0];                    \
      for (usz i=1, u=1; i<n; i++) {                           \
        bool c=1; usz s=0; i##T xi=uniq[u]=xp[i];              \
        for (usz j=0; j<u; j++) s += (c &= xi!=uniq[j]);       \
        rp[i]=s; u+=u==s;                                      \
      }                                                        \
      decG(x); TFREE(uniq); return r;
  #define DOTAB(T) \
    i32 u=0;                                                   \
    for (usz i=0; i<n; i++) {                                  \
      T j=xp[i]; i32 t=tab[j];                                 \
      if (t==n) tab[j]=u++;                                    \
      rp[i]=tab[j];                                            \
    }
  #define LOOKUP(T) \
    usz tn = 1<<T;                                             \
    u##T* xp = (u##T*)xv;                                      \
    i32* rp; B r = m_i32arrv(&rp, n);                          \
    TALLOC(i32, tab, tn);                                      \
    if (T>8 && n<tn/16) for (usz i=0; i<n;  i++) tab[xp[i]]=n; \
    else                for (usz j=0; j<tn; j++) tab[j]=n;     \
    DOTAB(u##T)                                                \
    decG(x); TFREE(tab);                                       \
    return num_squeeze(r)
  if (lw==3) { if (n<12) { BRUTE(8); } else { LOOKUP(8); } }
  if (lw==4) { if (n<12) { BRUTE(16); } else { LOOKUP(16); } }
  #undef LOOKUP
  
  if (lw==5) {
    if (n<32) { BRUTE(32); }
    i32* xp = tyany_ptr(x);
    i32 min=I32_MAX, max=I32_MIN;
    for (usz i = 0; i < n; i++) {
      i32 c = xp[i];
      if (c<min) min = c;
      if (c>max) max = c;
    }
    i64 dst = 1 + (max-(i64)min);
    if (dst<n*5 || dst<50) {
      i32* rp; B r = m_i32arrv(&rp, n);
      TALLOC(i32, tmp, dst); i32* tab = tmp-min;
      for (i64 i = 0; i < dst; i++) tmp[i] = n;
      DOTAB(i32)
      decG(x); TFREE(tmp);
      return r;
    }
  }
  #undef BRUTE
  #undef DOTAB
  
  if (RNK(x)>1) x = toCells(x);
  i32* rp; B r = m_i32arrv(&rp, n);
  H_b2i* map = m_b2i(64);
  SGetU(x)
  i32 ctr = 0;
  for (usz i = 0; i < n; i++) {
    bool had; u64 p = mk_b2i(&map, GetU(x,i), &had);
    if (had) rp[i] = map->a[p].val;
    else     rp[i] = map->a[p].val = ctr++;
  }
  free_b2i(map); decG(x);
  return r;
}

B find_c1(B t, B x) {
  if (isAtm(x) || RNK(x)==0) thrM("⍷: Argument cannot have rank 0");
  usz n = *SH(x);
  if (n<=1) return x;
  return slash_c2(m_f64(0), memberOf_c1(m_f64(0), inc(x)), x);
}
