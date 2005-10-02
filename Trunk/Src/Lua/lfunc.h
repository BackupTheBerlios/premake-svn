/*
** $Id: lfunc.h,v 1.1 2003/01/24 16:26:04 jason379 Exp $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#ifndef lfunc_h
#define lfunc_h


#include "lobject.h"



Proto *luaF_newproto (lua_State *L);
void luaF_protook (lua_State *L, Proto *f, int pc);
Closure *luaF_newclosure (lua_State *L, int nelems);
void luaF_freeproto (lua_State *L, Proto *f);
void luaF_freeclosure (lua_State *L, Closure *c);

const char *luaF_getlocalname (const Proto *func, int local_number, int pc);


#endif
