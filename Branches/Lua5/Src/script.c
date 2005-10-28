/**********************************************************************
 * Premake - script.c
 * Interface to the Lua scripting engine.
 *
 * Copyright (c) 2002-2005 Jason Perkins.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License in the file LICENSE.txt for details.
 **********************************************************************/

#include <stdlib.h>
#include <string.h>
#include "premake.h"
#include "script.h"
#include "arg.h"
#include "Lua/lua.h"
#include "Lua/lualib.h"
#include "Lua/lauxlib.h"
#include "Lua/ldebug.h"

static lua_State*  L;
static const char* currentScript = NULL;


static int         tbl_get(int from, const char* name);
static int         tbl_geti(int from, int i);
static int         tbl_getlen(int tbl);
static const char* tbl_getstring(int from, const char* name);
static const char* tbl_getstringi(int from, int i);

static int         addoption(lua_State* L);
static int         copyfile(lua_State* L);
static int         docommand(lua_State* L);
static int         findlib(lua_State* L);
static int         getcwd_lua(lua_State* L);
static int         initpkg(lua_State* L);
static int         matchfiles(lua_State* L);
static int         panic(lua_State* L);
static int         rmdir_lua(lua_State* L);

static void        buildOptionsTable();
static void        buildNewProject();
static void        buildNewPackage();


/**********************************************************************
 * The public script functions
 **********************************************************************/

int script_init()
{
	/* Create a script environment and install the standard libraries */
	L = lua_open();
	luaopen_base(L);
	luaopen_table(L);
	luaopen_io(L);
	luaopen_string(L);
	luaopen_math(L);
	luaopen_loadlib(L);

	lua_atpanic(L, panic);

	/* Register my extensions to the Lua environment */
	lua_register(L, "addoption", addoption);
	lua_register(L, "docommand", docommand);
	lua_register(L, "matchfiles", matchfiles);

	lua_getglobal(L, "os");

	lua_pushstring(L, "copyfile");
	lua_pushcfunction(L, copyfile);
	lua_settable(L, -3);

	lua_pushstring(L, "findlib");
	lua_pushcfunction(L, findlib);
	lua_settable(L, -3);

	lua_pushstring(L, "getcwd");
	lua_pushcfunction(L, getcwd_lua);
	lua_settable(L, -3);

	lua_pushstring(L, "rmdir");
	lua_pushcfunction(L, rmdir_lua);
	lua_settable(L, -3);

	lua_pop(L, 1);

	/* Create a list of option descriptions for addoption() */
	lua_getregistry(L);
	lua_pushstring(L, "options");
	lua_newtable(L);
	lua_settable(L, -3);
	lua_pop(L, 1);

	/* Create and populate a global "options" table */
	buildOptionsTable();

	/* Create an empty list of packages */
	lua_getregistry(L);
	lua_pushstring(L, "packages");
	lua_newtable(L);
	lua_settable(L, -3);
	lua_pop(L, 1);

	/* Create a default project object */
	buildNewProject();

	/* Set hook to intercept creation of globals, used to create packages */
	buildNewPackage();

	return 1;
}


int script_run(const char* filename)
{
	int result;

	result = lua_dofile(L, filename);
	if (result == LUA_ERRFILE)
	{
		char buffer[4096];
		strcpy(buffer, filename);
		strcat(buffer, ".lua");
		result = lua_dofile(L, buffer);
	}

	if (result != 0)
		lua_close(L);

	return (result == 0);
}


int script_export()
{
	int tbl, len, i;

	prj_open();

	/* Copy out the list of available options */
	tbl = tbl_get(LUA_REGISTRYINDEX, "options");
	len = tbl_getlen(tbl);
	project->options = (Option**)prj_newlist(len);
	for (i = 0; i < len; ++i)
	{
		int obj = tbl_geti(tbl, i + 1);

		Option* option = ALLOCT(Option);
		option->flag = tbl_getstringi(obj, 1);
		option->desc = tbl_getstringi(obj, 2);

		project->options[i] = option;
	}

	/* Copy out the project settings */
	tbl = tbl_get(LUA_GLOBALSINDEX, "project");
	project->name = tbl_getstring(tbl, "name");
	project->path = tbl_getstring(tbl, "path");
	return 1;
}


int script_docommand(const char* cmd)
{
	const char* arg;

	/* Look for a handler */
	lua_getglobal(L, "docommand");
	if (!lua_isfunction(L, -1))
	{
		lua_pop(L, 1);
		return 0;
	}

	/* Push the command and arguments onto the stack */
	lua_pushstring(L, cmd + 2);
	arg = arg_getflagarg();
	if (arg != NULL)
		lua_pushstring(L, arg);
	else
		lua_pushnil(L);

	lua_call(L, 2, 0);
	return 1;
}


int script_close()
{
	lua_close(L);
	return 1;
}



/**********************************************************************
 * These function assist with setup of the script environment
 **********************************************************************/

/* Creates a new global "options[]" table and populates it with the
 * flags from the command line */
static void buildOptionsTable()
{
	const char* flag;
	const char* arg;

	lua_newtable(L);

	arg_reset();
	flag = arg_getflag();
	while (flag != NULL)
	{
		lua_pushstring(L, flag);

		/* If the flag has an argument, push that too */
		arg = arg_getflagarg();
		if (arg != NULL)
			lua_pushstring(L, arg);
		else
			lua_pushnil(L);

		lua_settable(L, -3);
		flag = arg_getflag();
	}

	lua_setglobal(L, "options");
}


/* Creates an new project object with default settings */
static void buildNewProject()
{
	lua_newtable(L);

	lua_pushstring(L, "name");
	lua_pushstring(L, "MyProject");
	lua_settable(L, -3);

	lua_pushstring(L, "path");
	lua_pushstring(L, ".");
	lua_settable(L, -3);

	lua_setglobal(L, "project");
}


/* Creates an empty package */
static void buildNewPackage()
{
	lua_newtable(L);

	lua_getmetatable(L, -1);
	lua_pushstring(L, "_newindex");
	lua_pushcfunction(L, initpkg);
	lua_settable(L, -3);
	lua_pushstring(L, "_index");
	lua_pushcfunction(L, initpkg);
	lua_settable(L, -3);

	lua_setglobal(L, "package");
}


/**********************************************************************
 * These function help get data out of the Lua tables
 **********************************************************************/

static int tbl_get(int from, const char* name)
{
	int ref;

	if (from == LUA_REGISTRYINDEX || from == LUA_GLOBALSINDEX)
	{
		lua_pushvalue(L, from);
	}
	else
	{
		lua_getref(L, from);
	}

	lua_pushstring(L, name);
	lua_gettable(L, -2);

	if (!lua_istable(L, -1))
	{
		char msg[512];
		sprintf(msg, "'%s' should be a table.\nPlace value between brackets like: { values }", name);
		lua_pushstring(L, msg);
		lua_error(L);
	}

	ref = lua_ref(L, -1);
	lua_pop(L, 1);
	return ref;
}


static int tbl_geti(int from, int i)
{
	int ref;

	lua_getref(L, from);
	lua_rawgeti(L, -1, i);

	ref = lua_ref(L, -1);
	lua_pop(L, 1);
	return ref;
}


static int tbl_getlen(int tbl)
{
	int size;
	lua_getref(L, tbl);
	size = luaL_getn(L, -1);
	lua_pop(L, 1);
	return size;
}


static const char* tbl_getstring(int from, const char* name)
{
	const char* str;
	
	lua_getref(L, from);
	lua_pushstring(L, name);
	lua_gettable(L, -2);
	str = lua_tostring(L, -1);
	lua_pop(L, 2);

	return str;
}


static const char* tbl_getstringi(int from, int i)
{
	const char* str;
	
	lua_getref(L, from);
	lua_rawgeti(L, -1, i);
	str = lua_tostring(L, -1);
	lua_pop(L, 2);

	return str;
}


/**********************************************************************
 * These are new functions for the Lua environment
 **********************************************************************/

static int addoption(lua_State* L)
{
	const char* name = luaL_checkstring(L, 1);
	const char* desc = luaL_checkstring(L, 2);

	/* Retrieve the options list from the registry */
	lua_getregistry(L);
	lua_pushstring(L, "options");
	lua_gettable(L, -2);

	/* Create a new table for this new option */
	lua_newtable(L);
	lua_pushstring(L, name);
	lua_rawseti(L, -2, 1);
	lua_pushstring(L, desc);
	lua_rawseti(L, -2, 2);

	/* Add the option to the end of the registry list */
	lua_rawseti(L, -2, luaL_getn(L, -2) + 1);

	lua_pop(L, 2);
	return 0;
}



static int docommand(lua_State* L)
{
	const char* cmd = luaL_checkstring(L, 1);
	const char* arg = (!lua_isnil(L,2)) ? luaL_checkstring(L, 2) : NULL;
	if (!onCommand(cmd, arg))
		exit(1);
	return 0;
}


static int copyfile(lua_State* L)
{
	const char* src  = luaL_checkstring(L, 2);
	const char* dest = luaL_checkstring(L, 3);
	if (io_copyfile(src, dest))
		lua_pushnumber(L, 1);
	else
		lua_pushnil(L);
	return 1;
}


static int findlib(lua_State* L)
{
	const char* libname = luaL_check_string(L, 2);
	const char* result = io_findlib(libname);
	if (result)
		lua_pushstring(L, result);
	else
		lua_pushnil(L);
	return 1;
}


static int getcwd_lua(lua_State* L)
{
	const char* cwd = io_getcwd();
	lua_pushstring(L, cwd);
	return 1;
}


static int initpkg(lua_State* L)
{
	const char* name = luaL_checkstring(L, 1);
	puts(name);
	return 0;
}


static int matchfiles(lua_State* L)
{
	int numArgs, i;

	/* Create a table to hold the results */
	lua_newtable(L);

	/* Read and scan for each mask in turn */
	numArgs = lua_gettop(L) - 1;
	for (i = 1; i <= numArgs; ++i)
	{
		const char* mask = luaL_checkstring(L, i);

		io_mask_open(mask);
		while(io_mask_getnext())
		{
			if (io_mask_isfile())
			{
				lua_pushstring(L, io_mask_getname());
				lua_rawseti(L, -2, luaL_getn(L, -2) + 1);
			}
		}
		io_mask_close();
	}

	return 1;
}


static int panic(lua_State* L)
{
	lua_Debug ar;
	int stack;

	const char* msg = lua_tostring(L, 1);
	printf("\n** Error: %s\n", msg);

	for (stack = 0; lua_getstack(L, stack, &ar); ++stack)
	{
		lua_getinfo(L, "S1", &ar);
		if (ar.source && ar.currentline > 0)
		{
			printf("<%.70s: line %d>\n\n", ar.short_src, ar.currentline);
			break;
		}
	}

	return 0;
}

static int rmdir_lua(lua_State* L)
{
	const char* dir = luaL_check_string(L, 2);
	io_rmdir(".", dir);
	return 0;
}
