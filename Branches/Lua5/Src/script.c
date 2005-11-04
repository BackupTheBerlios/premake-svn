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
static int         tbl_getlen_deep(int tbl);
static const char* tbl_getstring(int from, const char* name);
static const char* tbl_getstringi(int from, int i);

static int         addoption(lua_State* L);
static int         copyfile(lua_State* L);
static int         docommand(lua_State* L);
static int         dopackage(lua_State* L);
static int         findlib(lua_State* L);
static int         getcwd_lua(lua_State* L);
static int         getglobal(lua_State* L);
static int         matchfiles(lua_State* L);
static int         newpackage(lua_State* L);
static int         panic(lua_State* L);
static int         rmdir_lua(lua_State* L);
static int         setconfigs(lua_State* L);

static void        buildOptionsTable();
static void        buildNewProject();


/**********************************************************************
 * Initialize the Lua environment
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
	lua_register(L, "addoption",  addoption);
	lua_register(L, "docommand",  docommand);
	lua_register(L, "dopackage",  dopackage);
	lua_register(L, "matchfiles", matchfiles);
	lua_register(L, "newpackage", newpackage);

	/* Add some extensions to the built-in "os" table */
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
	lua_pushvalue(L, LUA_GLOBALSINDEX);
	lua_newtable(L);
	lua_pushstring(L, "__index");
	lua_pushcfunction(L, getglobal);
	lua_settable(L, -3);
	lua_setmetatable(L, -2);
	lua_pop(L, 1);

	return 1;
}


/**********************************************************************
 * Execute the specified premake script. Contains some logic for
 * locating the script file if an exact match isn't found
 **********************************************************************/

int script_run(const char* filename)
{
	char scriptname[8192];
	int result;

	strcpy(scriptname, filename);
	if (!io_fileexists(scriptname))
		strcat(scriptname, ".lua");
	if (!io_fileexists(scriptname))
		return 0;

	currentScript = scriptname;
	if (!script_init())
		return 0;

	result = lua_dofile(L, scriptname);
	return (result == 0);
}


/**********************************************************************
 * After the script has run, these functions pull the project data
 * out into local objects
 **********************************************************************/

static int export_list(int parent, int object, const char* name, const char*** list)
{
	int i;

	int parArr = tbl_get(parent, name);
	int parLen = tbl_getlen_deep(parArr);
	int objArr = tbl_get(object, name);
	int objLen = tbl_getlen_deep(objArr);

	*list = (char**)prj_newlist(parLen + objLen);

	for (i = 0; i < parLen; ++i)
		(*list)[i] = tbl_getstringi(parArr, i + 1);

	for (i = 0; i < objLen; ++i)
		(*list)[parLen + i] = tbl_getstringi(objArr, i + 1);

	return (parLen + objLen);
}

static const char* export_value(int parent, int object, const char* name)
{
	const char* value;
	value = tbl_getstring(object, name);
	if (value == NULL)
		value = tbl_getstring(parent, name);
	return value;
}

static int export_pkgconfig(Package* package, int tbl)
{
	int arr, obj;
	int len, i;

	arr = tbl_get(tbl, "config");
	len = tbl_getlen(arr);
	package->configs = (PkgConfig**)prj_newlist(len);
	for (i = 0; i < len; ++i)
	{
		PkgConfig* config = ALLOCT(PkgConfig);
		package->configs[i] = config;
		config->prjConfig = project->configs[i];

		obj = tbl_geti(arr, i + 1);
		config->objdir = tbl_getstring(obj, "objdir");

		config->extension = export_value(tbl, obj, "targetextension");
		config->prefix    = export_value(tbl, obj, "targetprefix");
		config->target    = export_value(tbl, obj, "target");

		export_list(tbl, obj, "buildflags",   &config->flags);
		export_list(tbl, obj, "buildoptions", &config->buildopts);
		export_list(tbl, obj, "defines",      &config->defines);
		export_list(tbl, obj, "files",        &config->files);
		export_list(tbl, obj, "includepaths", &config->incpaths);
		export_list(tbl, obj, "libpaths",     &config->libpaths);
		export_list(tbl, obj, "linkoptions",  &config->linkopts);
		export_list(tbl, obj, "links",        &config->links);

		if (config->target == NULL)
			config->target = package->name;
	}

	return 1;
}


int script_export()
{
	int tbl, arr, obj;
	int len, i;

	prj_open();

	/* Copy out the list of available options */
	tbl = tbl_get(LUA_REGISTRYINDEX, "options");
	len = tbl_getlen(tbl);
	project->options = (Option**)prj_newlist(len);
	for (i = 0; i < len; ++i)
	{
		Option* option = ALLOCT(Option);
		project->options[i] = option;

		obj = tbl_geti(tbl, i + 1);
		option->flag = tbl_getstringi(obj, 1);
		option->desc = tbl_getstringi(obj, 2);
	}

	/* Copy out the project settings */
	tbl = tbl_get(LUA_GLOBALSINDEX, "project");
	project->name = tbl_getstring(tbl, "name");
	project->path = tbl_getstring(tbl, "path");
	project->script = tbl_getstring(tbl, "script");

	/* Copy out the project configurations */
	arr = tbl_get(tbl, "config");
	len = tbl_getlen(arr);
	project->configs = (PrjConfig**)prj_newlist(len);
	for (i = 0; i < len; ++i)
	{
		PrjConfig* config = ALLOCT(PrjConfig);
		project->configs[i] = config;

		obj = tbl_geti(arr, i + 1);
		config->name   = tbl_getstring(obj, "name");
		config->bindir = export_value(tbl, obj, "bindir");
		config->libdir = export_value(tbl, obj, "libdir");
	}

	/* Copy out the packages */
	tbl = tbl_get(LUA_REGISTRYINDEX, "packages");
	len = tbl_getlen(tbl);
	project->packages = (Package**)prj_newlist(len);
	for (i = 0; i < len; ++i)
	{
		Package* package = ALLOCT(Package);
		package->index = i;
		project->packages[i] = package;

		obj = tbl_geti(tbl, i + 1);
		package->name   = tbl_getstring(obj, "name");
		package->path   = tbl_getstring(obj, "path");
		package->script = tbl_getstring(obj, "script");
		package->lang   = tbl_getstring(obj, "language");
		package->kind   = tbl_getstring(obj, "kind");
		package->objdir = tbl_getstring(obj, "objdir");

		export_pkgconfig(package, obj);
	}

	return 1;
}


/**********************************************************************
 * Callback for commands pulled from the program arguments
 **********************************************************************/

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

static void buildNewProject()
{
	lua_newtable(L);

	lua_pushstring(L, "name");
	lua_pushstring(L, "MyProject");
	lua_settable(L, -3);

	lua_pushstring(L, "path");
	lua_pushstring(L, path_getdir(currentScript));
	lua_settable(L, -3);

	/* Hook "index" metamethod so I can tell when the config list changes */
	lua_newtable(L);
	lua_pushstring(L, "__newindex");
	lua_pushcfunction(L, setconfigs);
	lua_settable(L, -3);
	lua_setmetatable(L, -2);

	/* Set default values */
	lua_pushstring(L, "script");
	lua_pushstring(L, path_getname(currentScript));
	lua_settable(L, -3);

	lua_pushstring(L, "bindir");
	lua_pushstring(L, ".");
	lua_settable(L, -3);

	lua_pushstring(L, "libdir");
	lua_pushstring(L, ".");
	lua_settable(L, -3);

	lua_pushstring(L, "configs");
	lua_newtable(L);
	lua_pushstring(L, "Debug");
	lua_rawseti(L, -2, 1);
	lua_pushstring(L, "Release");
	lua_rawseti(L, -2, 2);
	lua_settable(L, -3);

	lua_setglobal(L, "project");
}

static void buildNewConfig(const char* name)
{
	/* Store the config name */
	if (name != NULL)
	{
		lua_pushstring(L, "name");
		lua_pushstring(L, name);
		lua_settable(L, -3);
	}

	/* Set defaults */
	lua_pushstring(L, "buildflags");
	lua_newtable(L);
	if (matches(name, "Release")) 
	{
		lua_pushstring(L, "no-symbols");
		lua_rawseti(L, -2, 1);
		lua_pushstring(L, "optimize");
		lua_rawseti(L, -2, 2);
	}
	lua_settable(L, -3);

	lua_pushstring(L, "buildoptions");
	lua_newtable(L);
	lua_settable(L, -3);

	lua_pushstring(L, "defines");
	lua_newtable(L);
	lua_settable(L, -3);

	lua_pushstring(L, "files");
	lua_newtable(L);
	lua_settable(L, -3);

	lua_pushstring(L, "includepaths");
	lua_newtable(L);
	lua_settable(L, -3);

	lua_pushstring(L, "libpaths");
	lua_newtable(L);
	lua_settable(L, -3);

	lua_pushstring(L, "linkoptions");
	lua_newtable(L);
	lua_settable(L, -3);

	lua_pushstring(L, "links");
	lua_newtable(L);
	lua_settable(L, -3);
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


static int tbl_getlen_deep(int tbl)
{
	int size, i;
	lua_getref(L, tbl);

	size = 0;
	for (i = 1; i <= luaL_getn(L, -1); ++i)
	{
		lua_rawgeti(L, -1, i);
		if (lua_istable(L, -1))
		{
			size += tbl_getlen_deep(lua_ref(L, 0));
		}
		else
		{
			lua_pop(L, 1);
			size++;
		}
	}

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


static int dopackage(lua_State* L)
{
	const char* oldScript;
	char filename[8192];
	int result;

	/* Clear the current global so included script can create a new one */
	lua_pushnil(L);
	lua_setglobal(L, "package");

	/* Search for the file */
	oldScript = currentScript;
	currentScript = filename;

	strcpy(filename, lua_tostring(L, 1));
	if (!io_fileexists(filename))
	{
		strcat(filename, ".lua");
	}
	if (!io_fileexists(filename))
	{
		strcpy(filename, lua_tostring(L, 1));
		strcat(filename, "/premake.lua");
	}
	if (!io_fileexists(filename))
	{
		lua_pushstring(L, "Unable to open package '");
		lua_pushvalue(L, 1);
		lua_pushstring(L, "'");
		lua_concat(L, 3);
		lua_error(L);
	}

	result = lua_dofile(L, filename);
	currentScript = oldScript;
	return 0;
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


static int getglobal(lua_State* L)
{
	const char* name = luaL_checkstring(L, 2);
	if (matches(name, "package"))
	{
		newpackage(L);
		lua_pushvalue(L, -1);
		lua_setglobal(L, "package");
		return 1;
	}
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


static int newpackage(lua_State* L)
{
	int count, i;

	lua_newtable(L);

	/* Add this package to the master list in the registry */
	lua_getregistry(L);
	lua_pushstring(L, "packages");
	lua_gettable(L, -2);
	count = luaL_getn(L, -1);

	lua_pushvalue(L, -3);
	lua_rawseti(L, -2, count + 1);

	lua_pop(L, 2);

	/* Set default values */
	if (count == 0)
	{
		lua_getglobal(L, "project");
		lua_pushstring(L, "name");
		lua_pushstring(L, "name");
		lua_gettable(L, -3);
		lua_settable(L, -4);
		lua_pop(L, 1);
	}
	else
	{
		lua_pushstring(L, "name");
		lua_pushstring(L, "Package");
		lua_pushnumber(L, count);
		lua_concat(L, 2);
		lua_settable(L, -3);
	}

	lua_pushstring(L, "script");
	lua_pushstring(L, currentScript);
	lua_settable(L, -3);

	lua_pushstring(L, "path");
	lua_pushstring(L, path_getdir(currentScript));
	lua_settable(L, -3);

	lua_pushstring(L, "language");
	lua_pushstring(L, "c++");
	lua_settable(L, -3);

	lua_pushstring(L, "kind");
	lua_pushstring(L, "exe");
	lua_settable(L, -3);

	lua_pushstring(L, "objdir");
	lua_pushstring(L, "obj");
	lua_settable(L, -3);

	buildNewConfig(NULL);

	/* Build list of configurations matching what is in the project, and
	 * which can be indexed by name or number */
	lua_pushstring(L, "config");
	lua_newtable(L);

	lua_getglobal(L, "project");
	lua_pushstring(L, "configs");
	lua_gettable(L, -2);
	count = luaL_getn(L, -1);
	
	for (i = 1; i <= count; ++i)
	{
		lua_rawgeti(L, -1, i);

		lua_newtable(L);

		buildNewConfig(lua_tostring(L, -2));
	
		lua_pushvalue(L, -1);
		lua_rawseti(L, -6, i);
		lua_settable(L, -5);
	}

	lua_pop(L, 2);
	lua_settable(L, -3);

	return 1;
}


static int panic(lua_State* L)
{
	lua_Debug ar;
	int stack;

	const char* msg = lua_tostring(L, 4);
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


static int setconfigs(lua_State* L)
{
	int i;

	const char* name = luaL_checkstring(L, 2);
	if (matches(name, "configs"))
	{
		if (!lua_istable(L, 3))
		{
			lua_pushstring(L, "Project configs must be a table of config names");
			lua_error(L);
		}

		lua_pushstring(L, "config");
		lua_newtable(L);
		for (i = 1; i <= luaL_getn(L, 3); ++i)
		{
			/* Set up the new config table to be added by name */
			lua_rawgeti(L, 3, i);

			/* Create the config and set the config name */
			lua_newtable(L);
			lua_pushstring(L, "name");
			lua_pushvalue(L, -3);
			lua_rawset(L, -3);

			/* Add the config by index */
			lua_pushvalue(L, -1);
			lua_rawseti(L, -4, i);

			/* Add the config by name */
			lua_rawset(L, -3);
		}

		/* Add the new config table to the project */
		lua_rawset(L, 1);
	}

	/* Add the initially requested item to the list */
	lua_rawset(L, 1);
	return 0;
}
