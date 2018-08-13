/*
** $Id: ltests.h,v 2.49 2015/09/22 14:18:24 roberto Exp roberto $
** Internal Header for Debugging of the Lua Implementation
** See Copyright Notice in lua.h
*/


#ifndef ltests_h
#define ltests_h

/*
** The whole module only makes sense with LUA_DEBUG on
*/
#if defined(LUA_DEBUG)

#include <stdlib.h>


/* test Lua with no compatibility code */
#undef LUA_COMPAT_MATHLIB
#undef LUA_COMPAT_IPAIRS
#undef LUA_COMPAT_BITLIB
#undef LUA_COMPAT_APIINTCASTS
#undef LUA_COMPAT_FLOATSTRING
#undef LUA_COMPAT_UNPACK
#undef LUA_COMPAT_LOADERS
#undef LUA_COMPAT_LOG10
#undef LUA_COMPAT_LOADSTRING
#undef LUA_COMPAT_MAXN
#undef LUA_COMPAT_MODULE


/* turn on assertions */
#undef NDEBUG
#include <assert.h>
#define lua_assert(c)           assert(c)


/* to avoid warnings, and to make sure value is really unused */
#define UNUSED(x)       (x=0, (void)(x))


/* test for sizes in 'l_sprintf' (make sure whole buffer is available) */
#undef l_sprintf
#if !defined(LUA_USE_C89)
#define l_sprintf(s,sz,f,i)	(memset(s,0xAB,sz), snprintf(s,sz,f,i))
#else
#define l_sprintf(s,sz,f,i)	(memset(s,0xAB,sz), sprintf(s,f,i))
#endif


/* memory-allocator control variables */
typedef struct Memcontrol {
  lua_Alloc alloc_f;
  void * alloc_ud;

  unsigned long numblocks;
  unsigned long total;
  unsigned long maxmem;
  unsigned long memlimit;
  unsigned long objcount[LUA_NUMTAGS]; /* number of objects of each type */
  unsigned long objtotal[LUA_NUMTAGS]; /* memory used by all object of each type
                                        sum of elements must be equal to 'total' */
} Memcontrol;

/*
** Function to traverse and check all memory used by Lua
*/
int lua_checkmemory (lua_State *L);


/* test for lock/unlock */

struct L_EXTRA { int lock; int *plock; };

enum {LUA_OLDEXTRASPACE = LUA_EXTRASPACE };
#undef LUA_EXTRASPACE

struct COMP_L_EXTRA {
  char user_extraspace[LUA_OLDEXTRASPACE];
  struct L_EXTRA debug_extraspace;
  void *l_Trick;   /* generic variable for debug tricks */
};

#define LUA_EXTRASPACE	sizeof(struct COMP_L_EXTRA)

#define getlock(l) \
  (cast(struct COMP_L_EXTRA*, lua_getextraspace(l))->debug_extraspace)

#define gettrick(l) \
  (cast(struct COMP_L_EXTRA*, lua_getextraspace(l))->l_Trick)

#define luai_userstateopen(l)  \
	(gettrick(l) = 0, getlock(l).lock = 0, getlock(l).plock = &(getlock(l).lock))
#define luai_userstateclose(l)  \
  lua_assert(getlock(l).lock == 1 && getlock(l).plock == &(getlock(l).lock))
#define luai_userstatethread(l,l1) \
  lua_assert(getlock(l1).plock == getlock(l).plock)
#define luai_userstatefree(l,l1) \
  lua_assert(getlock(l).plock == getlock(l1).plock)
#define lua_lock(l)     lua_assert((*getlock(l).plock)++ == 0)
#define lua_unlock(l)   lua_assert(--(*getlock(l).plock) == 0)



/* Load the test library and assert that the intepreter is correctly set up
 * for testing.
 */
LUA_API int luaB_opentests (lua_State *L);

/* Initialize the control block for the test allocator.
 * The test allocator is a wrapper around the user supplied allocator that
 * records diagnostic and debug information.
 *
 * It uses a Memcontrol structure as the "ud" userdata pointer. Inside this
 * structure the "real" allocator is stored and will be called to perform the
 * actual memory operations.
 *
 * Set f and ud to your application's allocator.
 */
LUA_API void luaB_init_memcontrol(Memcontrol *mc, lua_Alloc f, void *ud);

/* Create a new state with a specially instrumented allocator.
 *
 * You must supply a properly initialized Memcontrol structure.
 * */
LUA_API lua_State * luaB_newstate(Memcontrol *mc);

/* Close the lua state and check that all memory has been freed.
 */
LUA_API void luaB_close(lua_State *L);

#define LUA_TESTLIBNAME "T"

/* Change some sizes to give some bugs a chance
 * Activate this macro to make tests harder.
 * This is not enabled my default because the user may want only the
 * functionality of the test module.
 */
#ifdef DEBUG_OVERRIDE_SIZES

#undef LUAL_BUFFERSIZE
#define LUAL_BUFFERSIZE		23
#define MINSTRTABSIZE		2
#define MAXINDEXRK		1


/* make stack-overflow tests run faster */
#undef LUAI_MAXSTACK
#define LUAI_MAXSTACK   50000


#undef LUAI_USER_ALIGNMENT_T
#define LUAI_USER_ALIGNMENT_T   union { char b[sizeof(void*) * 8]; }


#define STRCACHE_N	23
#define STRCACHE_M	5

#endif /* DEBUG_OVERRIDE_SIZES */

#endif /* LUA_DEBUG */

#endif /* ltests_h */
