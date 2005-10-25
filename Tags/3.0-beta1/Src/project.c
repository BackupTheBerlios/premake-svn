//-----------------------------------------------------------------------------
// Premake - project.c
//
// Functions to load and access the project data, along with all of the helper
// functions used by the project scripts.
//
// Copyright (C) 2002-2004 by Jason Perkins
// Source code licensed under the GPL, see LICENSE.txt for details.
//
// $Id: project.c,v 1.32 2005/09/22 21:03:13 jason379 Exp $
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Lua/lua.h"
#include "Lua/lualib.h"
#include "Lua/lauxlib.h"
#include "Lua/ldebug.h"
#include "project.h"
#include "util.h"

extern void defaultCommand(int argc, const char** argv);

Project* project = NULL;
char* os = NULL;

static lua_State* L;
static int   projConfigTag;
static int   fileConfigTag;
static const char* currentScript = NULL;

static int  finishProject();

static int  getArray(int ref, const char* name);
static int  getArraySize(int ref);
static int  getDeepArraySize(int ref);
static int  getObjectFromArray(int ref, int i);
static int  getObjectByName(int ref, const char* name);
static const char* getString(int ref, char* name);
static const char* getStringFromArray(int ref, int index);

static int  addoption(lua_State* L);
static int  chdir_(lua_State* L);
static int  copyfile(lua_State* L);
static int  docommand(lua_State* L);
static int  dopackage(lua_State* L);
static int  ERRORMESSAGE(lua_State* L);
static int  getglobal(lua_State* L);
static int  findlib(lua_State* L);
static int  matchfiles(lua_State* L);
static int  newprojconfig(lua_State* L);
static int  newfileconfig(lua_State* L);
static int  newpackage(lua_State* L);
static int  rmdir(lua_State* L);


void createProject(char* argv[])
{
	int argIndex, i;

	/* Create a script environment and install the standard libraries */
	L = lua_open(2048);

	lua_baselibopen(L);
	lua_iolibopen(L);
	lua_strlibopen(L);
	lua_mathlibopen(L);

	projConfigTag = lua_newtag(L);
	fileConfigTag = lua_newtag(L);

	/* Install the helper functions for the scripts */
	lua_pushcfunction(L, addoption);
	lua_setglobal(L, "option");

	lua_pushcfunction(L, addoption);
	lua_setglobal(L, "addoption");

	lua_pushcfunction(L, chdir_);
	lua_setglobal(L, "chdir");

	lua_pushcfunction(L, copyfile);
	lua_setglobal(L, "copyfile");

	lua_pushcfunction(L, docommand);
	lua_setglobal(L, "docommand");

	lua_pushcfunction(L, dopackage);
	lua_setglobal(L, "dopackage");

	lua_pushcfunction(L, ERRORMESSAGE);
	lua_setglobal(L, "_ERRORMESSAGE");

	lua_pushcfunction(L, getglobal);
	lua_settagmethod(L, LUA_TNIL, "getglobal");

	lua_pushcfunction(L, dopackage);
	lua_setglobal(L, "include");

	lua_pushcfunction(L, findlib);
	lua_setglobal(L, "findlib");

	lua_pushcfunction(L, matchfiles);
	lua_setglobal(L, "matchfiles");

	lua_pushcfunction(L, newprojconfig);
	lua_settagmethod(L, projConfigTag, "index");

	lua_pushcfunction(L, newfileconfig);
	lua_settagmethod(L, fileConfigTag, "index");

	lua_pushcfunction(L, newpackage);
	lua_setglobal(L, "newpackage");

	lua_pushcfunction(L, rmdir);
	lua_setglobal(L, "rmdir");

	/* Set up the registry */
	lua_getregistry(L);

	lua_pushstring(L, "packages");
	lua_newtable(L);
	lua_settable(L, -3);

	lua_pushstring(L, "options");
	lua_newtable(L);
	lua_settable(L, -3);

	lua_pop(L, 1);

	/* Set the OS variable */
	if (os == NULL)
		os = OS_IDENT;

	lua_pushstring(L, os);
	lua_setglobal(L, "OS");

	/* Create and populate the options table */
	lua_newtable(L);
	argIndex = 0;
	for (i = 1; argv[i] != NULL; ++i)
	{
		if (strncmp(argv[i], "--", 2) == 0)
		{
			if (argIndex > 0) lua_settable(L, -3);
			lua_pushstring(L, argv[i] + 2);
			lua_newtable(L);
			argIndex = 1;
		}
		else if (argIndex > 0)
		{
			lua_pushnumber(L, argIndex++);
			lua_pushstring(L, argv[i]);
			lua_settable(L, -3);
		}
	}
	if (argIndex > 0) lua_settable(L, -3);
	lua_setglobal(L, "options");

	/* Create an empty project object */
	lua_newtable(L);

	lua_pushstring(L, "name");
	lua_pushstring(L, "MyProject");
	lua_settable(L, -3);

	lua_pushstring(L, "path");
	lua_pushstring(L, getDirectory(currentScript, 0));
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

	/* Mark the config table so I can dynamically add file configs later */
	lua_pushstring(L, "config");
	lua_newtable(L);
	lua_settag(L, projConfigTag);
	lua_settable(L, -3);

	lua_setglobal(L, "project");
}


int loadProject(char* project)
{
	int result;
	result = lua_dofile(L, project);
	if (result == LUA_ERRFILE)
	{
		/* Try "${project}.lua" */
		char buffer[4096];
		strcpy(buffer, project);
		strcat(buffer, ".lua");
		result = lua_dofile(L, buffer);
	}

	if (result == 0)
		result = finishProject();

	if (result != 0)
		lua_close(L);

	return (result == 0);
}


void handleCommand(char* args[], int i)
{
	int j;

	/* Try to find a command processor */
	lua_getglobal(L, "docommand");
	if (!lua_isfunction(L, -1))
		return;

	/* Push the command and it's arguments onto the stack */
	lua_pushstring(L, args[i] + 2);
	for (j = i + 1; args[j] != NULL && strncmp(args[j], "--", 2) != 0; ++j)
		lua_pushstring(L, args[j]);

	/* Call the command processor */
	lua_call(L, j - i, 0);
}


int getProject()
{
	lua_getglobal(L, "project");
	return lua_ref(L, 0);
}


Package* getPackage(const char* name)
{
	int i;
	for (i = 0; i < project->numPackages; ++i)
	{
		if (strcmp(project->package[i]->name, name) == 0)
			return project->package[i];
	}
	return NULL;
}


FileConfig* getFileConfig(Package* package, const char* name)
{
	int i;
	for (i = 0; i < package->numFiles; ++i)
	{
		if (strcmp(package->files[i], name) == 0)
			return package->fileConfigs[i];
	}
	return NULL;
}


static void getConfigList(int package, int config, const char* list, const char*** array, int* length)
{
	const char** buffer;
	int size, i;

	int pkgRef = getArray(package, list);
	int pkgLen = getDeepArraySize(pkgRef);
	int cfgRef = getArray(config, list);
	int cfgLen = getDeepArraySize(cfgRef);

	size = pkgLen + cfgLen;
	buffer = (const char**)malloc(sizeof(char**) * (size + 1));
	buffer[size] = NULL;

	for (i = 0; i < pkgLen; ++i)
		buffer[i] = getStringFromArray(pkgRef, i);
	for (i = 0; i < cfgLen; ++i)
		buffer[i + pkgLen] = getStringFromArray(cfgRef, i);

	*array = buffer;
	*length = size;
}


static int finishProject()
{
	int prj, i, optlist, packages, configs;
	const char* bindir;
	const char* libdir;

	project = (Project*)malloc(sizeof(Project));

	/* Populate the project */
	prj = getProject();
	project->name = getString(prj, "name");
	if (strchr(project->name, ' ') != NULL)
		puts("** Warning: not all generators allow spaces in the project name.");

	project->path = getString(prj, "path");
	bindir = getString(prj, "bindir");
	libdir = getString(prj, "libdir");

	/* Set up the project-level configurations */
	configs = getArray(prj, "configs");
	project->numConfigs = getArraySize(configs);
	project->config = (ProjectConfig**)malloc(sizeof(ProjectConfig*) * (project->numConfigs + 1));
	project->config[project->numConfigs] = NULL;
	for (i = 0; i < project->numConfigs; ++i)
	{
		int prjConfigs, cfg;
		const char* configName;

		ProjectConfig* config = (ProjectConfig*)malloc(sizeof(ProjectConfig));
		project->config[i] = config;

		prjConfigs = getArray(prj, "config");
		configName = getStringFromArray(configs, i);
		cfg = getObjectByName(prjConfigs, configName);

		config->name = getString(cfg, "name");
		if (config->name == NULL)
			config->name = configName;

		config->bindir = getString(cfg, "bindir");
		if (config->bindir == NULL)
			config->bindir = bindir;

		config->libdir = getString(cfg, "libdir");
		if (config->libdir == NULL)
			config->libdir = libdir;
	}

	/* Populate the list of options */
	optlist = getArray(-1, "options");
	project->numOptions = getArraySize(optlist);
	project->option = (Option**)malloc(sizeof(Option*) * (project->numOptions + 1));
	project->option[project->numOptions] = NULL;
	for (i = 0; i < project->numOptions; ++i)
	{
		int opt = getObjectFromArray(optlist, i);
		Option* option = (Option*)malloc(sizeof(Option));
		project->option[i] = option;
		option->flag = (char*)getStringFromArray(opt, 0);
		option->description = (char*)getStringFromArray(opt, 1);
	}

	/* Populate the list of packages */
	packages = getArray(-1, "packages");
	project->numPackages = getArraySize(packages);
	project->package = (Package**)malloc(sizeof(Package**) * (project->numPackages + 1));
	project->package[project->numPackages] = NULL;

	for (i = 0; i < project->numPackages; ++i)
	{
		int pkg, pkgConfigs, files, j;

		/* Create the package and load the easy fields */
		Package* package = (Package*)malloc(sizeof(Package));
		project->package[i] = package;

		pkg = getObjectFromArray(packages, i);
		pkgConfigs = getArray(pkg, "config");

		package->name = getString(pkg, "name");
		package->script = getString(pkg, "script");
		package->path = getString(pkg, "path");
		package->language = getString(pkg, "language");
		package->kind = getString(pkg, "kind");
		package->objdir = getString(pkg, "objdir");
		if (package->objdir == NULL) package->objdir = "obj";
		package->url = getString(pkg, "url");
		package->data = NULL;

		/* Load the files */
		files = getArray(pkg, "files");
		package->numFiles = getDeepArraySize(files);
		package->files = (const char**)malloc(sizeof(char**) * (package->numFiles + 1));
		package->files[package->numFiles] = NULL;
		for (j = 0; j < package->numFiles; ++j)
			package->files[j] = getStringFromArray(files, j);

		package->fileConfigs = (FileConfig**)malloc(sizeof(FileConfig*) * (package->numFiles + 1));
		package->fileConfigs[package->numFiles] = NULL;
		for (j = 0; j < package->numFiles; ++j)
		{
			int cfg;
			FileConfig* fc = (FileConfig*)malloc(sizeof(FileConfig));
			fc->buildAction = NULL;
			package->fileConfigs[j] = fc;

			cfg = getObjectByName(pkgConfigs, package->files[j]);
			if (cfg != 0)
			{
				fc->buildAction = getString(cfg, "buildaction");
			}
		}

		/* Build the configurations */
		configs = getArray(getProject(), "configs");
		package->numConfigs = getArraySize(configs);
		package->config = (Config**)malloc(sizeof(Config*) * (package->numConfigs + 1));
		package->config[package->numConfigs] = NULL;
		for (j = 0; j < package->numConfigs; ++j)
		{
			int cfg;
			const char* configName;
			Config* config = (Config*)malloc(sizeof(Config));
			package->config[j] = config;
			config->package = package;

			config->projectConfig = project->config[j];

			configName = getStringFromArray(configs, j);
			cfg = getObjectByName(pkgConfigs, configName);

			config->name = getString(cfg, "name");
			config->target = getString(cfg, "target");
			if (config->target == NULL) config->target = getString(pkg, "target");
			if (config->target == NULL) config->target = package->name;
			config->extension = getString(cfg, "targetextension");
			if (config->extension == NULL) config->extension = getString(pkg, "targetextension");

			getConfigList(pkg, cfg, "buildflags", &config->buildFlags, &config->numBuildFlags);
			getConfigList(pkg, cfg, "buildoptions", &config->buildOptions, &config->numBuildOptions);
			getConfigList(pkg, cfg, "defines", &config->defines, &config->numDefines);
			getConfigList(pkg, cfg, "includepaths", &config->includePaths, &config->numIncludePaths);
			getConfigList(pkg, cfg, "libpaths", &config->libPaths, &config->numLibPaths);
			getConfigList(pkg, cfg, "linkflags", &config->linkFlags, &config->numLinkFlags);
			getConfigList(pkg, cfg, "linkoptions", &config->linkOptions, &config->numLinkOptions);
			getConfigList(pkg, cfg, "links", &config->links, &config->numLinks);
		}
	}

	return 0;
}


void closeProject()
{
	int i, j;

	for (i = 0; i < project->numPackages; ++i)
	{
		Package* package = project->package[i];

		for (j = 0; j < package->numFiles; ++j)
		{
			free(package->fileConfigs[j]);
		}

		for (j = 0; j < package->numConfigs; ++j)
		{
			Config* config = package->config[j];
			free((char**)config->buildFlags);
			free((char**)config->buildOptions);
			free((char**)config->defines);
			free((char**)config->includePaths);
			free((char**)config->libPaths);
			free((char**)config->linkFlags);
			free((char**)config->linkOptions);
			free((char**)config->links);
			free(package->config[j]);
		}
		free(package->config);
		if (package->data) free(package->data);
		free(package);
	}

	free(project->package);
	free(project);
	project = NULL;
}


static int getArray(int ref, const char* name)
{
	if (ref == -1)
		ref = LUA_REFREGISTRY;

	lua_getref(L, ref);
	lua_pushstring(L, name);
	lua_gettable(L, -2);

	if (!lua_istable(L, -1))
	{
		char msg[256];
		sprintf(msg, "'%s' should be a table.\nPlace value between brackets like: { values }", name);
		lua_error(L, msg);
	}

	ref = lua_ref(L, -1);
	lua_pop(L, 1);
	return ref;
}


static int getArraySize(int ref)
{
	if (ref > 0)
	{
		int size;
		lua_getref(L, ref);
		size = lua_getn(L, -1);
		lua_pop(L, 1);
		return size;
	}

	return 0;
}


static int getDeepArraySize(int ref)
{
	int i, size = 0;
	if (ref == 0) return 0;

	lua_getref(L, ref);
	for (i = 1; i <= lua_getn(L,-1); ++i)
	{
		lua_rawgeti(L, -1, i);
		if (lua_istable(L, -1))
		{
			size += getDeepArraySize(lua_ref(L,0));
		}
		else
		{
			lua_pop(L, 1);
			++size;
		}
	}

	lua_pop(L, 1);
	return size;
}


static int getObjectFromArray(int ref, int i)
{
	lua_getref(L, ref);
	lua_rawgeti(L, -1, i + 1);
	ref = lua_ref(L, -1);
	lua_pop(L, 1);
	return ref;
}


static void dumpTable(int indent)
{
	lua_pushnil(L);
	while (lua_next(L, -2) != 0)
	{
		const char* key;
		int i;

		lua_pushvalue(L, -2);
		key = lua_tostring(L, -1);
		for (i = 0; i < indent; ++i)
			printf(" ");
		puts(key);
		lua_pop(L, 1);		

		lua_pushvalue(L, -1);
		if (lua_istable(L, -1))
			dumpTable(indent + 1);
		lua_pop(L, 1);

		lua_pop(L, 1);
	}
}


static int scanKey(const char* name)
{
	if (lua_isstring(L, -1))
	{
		const char* key = lua_tostring(L, -1);
		if (strcmp(key, name) == 0)
			return 1;
	}
	else if (lua_istable(L, -1))
	{
		int i, n;
		int ref = lua_ref(L, -1);
		lua_getref(L, ref);
		n = getDeepArraySize(ref);
		for (i = 0; i < n; ++i)
		{
			const char* key = getStringFromArray(ref, i);
			if (strcmp(key, name) == 0)
				return 1;
		}
	}
	return 0;
}

static int getObjectByName(int ref, const char* name)
{
	lua_getref(L, ref);
	lua_pushnil(L);
	while (lua_next(L, -2) != 0)
	{
		lua_pushvalue(L, -2);
		if (scanKey(name))
		{
			lua_pop(L, 1);
			ref = lua_ref(L, -1);
			lua_pop(L, 2);
			return ref;
		}
		lua_pop(L, 2);
	}
	lua_pop(L, 1);
	return 0;
}


static const char* getString(int ref, char* name)
{
	const char* str;
	lua_getref(L, ref);
	lua_pushstring(L, name);
	lua_gettable(L, -2);
	str = lua_tostring(L, -1);
	lua_pop(L, 2);

	return str;
}


static const char* goDeep(int ref, int* index)
{
	int i;
	const char* str = NULL;

	lua_getref(L, ref);
	for (i = 1; i <= lua_getn(L,-1) && *index >= 0; ++i)
	{
		lua_rawgeti(L, -1, i);
		if (lua_istable(L, -1))
		{
			int item = lua_ref(L, 0);
			str = goDeep(item, index);
			if (str != NULL)
				break;
		}
		else if (*index == 0)
		{
			str = lua_tostring(L, -1);
			lua_pop(L, 1);
			break;
		}
		else
		{
			lua_pop(L, 1);
			--(*index);
		}
	}

	lua_pop(L, 1);
	return str;
}


static const char* getStringFromArray(int ref, int index)
{
	return goDeep(ref, &index);
}


static void createEmptyConfig(lua_State* L, const char* name)
{
	lua_newtable(L);

	if (name != NULL)
	{
		lua_pushstring(L, "name");
		lua_pushstring(L, name);
		lua_settable(L, -3);
	}

	lua_pushstring(L, "buildflags");
	lua_newtable(L);
	if (name != NULL && strcmp(name, "Release") == 0) {
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

	lua_pushstring(L, "includepaths");
	lua_newtable(L);
	lua_settable(L, -3);

	lua_pushstring(L, "libpaths");
	lua_newtable(L);
	lua_settable(L, -3);

	lua_pushstring(L, "linkflags");
	lua_newtable(L);
	lua_settable(L, -3);

	lua_pushstring(L, "linkoptions");
	lua_newtable(L);
	lua_settable(L, -3);

	lua_pushstring(L, "links");
	lua_newtable(L);
	lua_settable(L, -3);
}


static void registerTable(lua_State* L, char* listname)
{
	/* The registry stores the list of projects and packages. This function
	 * will add the table at the top of the stack to one of these lists. */
	int n;

	lua_getregistry(L);

	lua_pushstring(L, listname);
	lua_gettable(L, -2);
	lua_pushvalue(L, -3);

	n = lua_getn(L, -2);
	lua_rawseti(L, -2, n + 1);

	lua_pop(L, 2);  /* remove list and registry */
}


static int addoption(lua_State* L)
{
	lua_getregistry(L);
	lua_pushstring(L, "options");
	lua_gettable(L, -2);

	lua_newtable(L);
	lua_pushvalue(L, 1);
	lua_rawseti(L, -2, lua_getn(L,-2) + 1);

	lua_pushvalue(L, 2);
	lua_rawseti(L, -2, lua_getn(L,-2) + 1);

	lua_rawseti(L, -2, lua_getn(L,-2) + 1);

	lua_pop(L, 2);
	return 0;
}


static int copyfile(lua_State* L)
{
	const char* src  = luaL_check_string(L, 1);
	const char* dest = luaL_check_string(L, 2);
	if (copyFile(src, dest))
		lua_pushnumber(L, 1);
	else
		lua_pushnil(L);
	return 1;
}


static int docommand(lua_State* L)
{
	int argc, i;
	const char** argv;

	/* If this function is overriden in Lua, the second parameter will be a
	 * table. Otherwise there will be zero or more string parameters. */
	if (lua_gettop(L) > 1 && !lua_isnil(L,2))
	{
		if (lua_istable(L, 2))
		{
			argc = lua_getn(L, 2) + 1;
			argv = (const char**)malloc(sizeof(char*) * argc);
			argv[0] = 	luaL_check_string(L, 1);
			for (i = 1; i < argc; ++i)
			{
				lua_rawgeti(L, 2, i);
				argv[i] = luaL_check_string(L, -1);
				lua_pop(L, 1);
			}
		}
		else
		{
			argc = lua_gettop(L);
			argv = (const char**)malloc(sizeof(char*) * argc);
			for (i = 0; i < lua_gettop(L); ++i)
				argv[i] = luaL_check_string(L, i + 1);
		}
	}
	else
	{
		argc = 1;
		argv = (const char**)malloc(sizeof(char*));
		argv[0] = luaL_check_string(L, 1);
	}

	defaultCommand(argc, argv);

	free((char*)argv);
	return 0;
}


static int dopackage(lua_State* L)
{
	int result;
	currentScript = luaL_check_string(L, 1);

	lua_pushnil(L);
	lua_setglobal(L, "package");

	lua_pushvalue(L, 1);
	lua_pushstring(L, "/premake.lua");
	lua_concat(L, 2);
	currentScript = lua_tostring(L, -1);
	result = lua_dofile(L, currentScript);
	if (result == LUA_ERRFILE)
	{
		lua_pushvalue(L, 1);
		lua_pushstring(L, ".lua");
		lua_concat(L, 2);
		currentScript = lua_tostring(L, -1);
		result = lua_dofile(L, currentScript);
	}
	if (result == LUA_ERRFILE)
	{
		currentScript = lua_tostring(L, 1);
		result = lua_dofile(L, currentScript);
	}

	currentScript = NULL;
	return 0;
}


static int ERRORMESSAGE(lua_State* L)
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


static int chdir_(lua_State* L)
{
	const char* path = luaL_check_string(L, 1);
	int result = setCwd(translatePath(path, NATIVE));
	if (result)
		lua_pushnumber(L, 1);
	else
		lua_pushnil(L);
	return 1;
}


static int findlib(lua_State* L)
{
	const char* libname = luaL_check_string(L, 1);
	const char* result = findSharedLib(libname);
	if (result)
		lua_pushstring(L, result);
	else
		lua_pushnil(L);
	return 1;
}


static int getglobal(lua_State* L)
{
	const char* name = lua_tostring(L, 1);
	if (strcmp(name, "package") == 0)
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
	const char* path;
	int i, numArgs;
	
	/* How many masks were passed in? */
	numArgs = lua_gettop(L);

	/* Get the current package path */
	lua_getglobal(L, "package");
	lua_pushstring(L, "path");
	lua_gettable(L, -2);
	if (lua_isstring(L, -1))
	{
		path = lua_tostring(L, -1);
	}
	else
	{
		if (currentScript != NULL)
			path = currentScript;
		else
			path = ".";
	}
	lua_pop(L, 2);

	lua_newtable(L);
	for (i = 1; i <= numArgs; ++i)
	{
		const char* mask = luaL_check_string(L, i);
		int usePrefix = strcmp(getDirectory(mask, 0), ".") != 0;

		dirOpen(path, mask);
		while (dirGetNext())
		{
			if (dirIsFile())
			{
				if (usePrefix)
				{
					const char* result;
					lua_pushstring(L, getDirectory(mask,1));
					lua_pushstring(L, dirGetName());
					lua_concat(L, 2);
					result = lua_tostring(L, -1);
				}
				else
				{
					lua_pushstring(L, dirGetName());
				}

				lua_rawseti(L, -2, lua_getn(L,-2) + 1);
			}
		}
		dirClose();
	}

	return 1;
}


static int newprojconfig(lua_State* L)
{
	/* Add a project config to the project config table */
	lua_pushvalue(L, 2);
	lua_newtable(L);
	lua_settable(L, 1);

	/* Read it back and return it */
	lua_gettable(L, 1);
	return 1;
}


static int newfileconfig(lua_State* L)
{
	/* Add a file config to the package config table */
	lua_pushvalue(L, 2);
	lua_newtable(L);
	lua_settable(L, 1);

	/* Read it back and return it */
	lua_gettable(L, 1);
	return 1;
}


static int newpackage(lua_State* L)
{
	int configs;
	int n;

	createEmptyConfig(L, NULL);

	/* Add the package to the master list in the registry */

	lua_getregistry(L);
	lua_pushstring(L, "packages");
	lua_gettable(L, -2);
	n = lua_getn(L, -1);

	lua_pushvalue(L, -3);
	lua_rawseti(L, -2, n + 1);
	lua_pop(L, 2);

	/* Set default values */

	lua_pushstring(L, "name");
	if (n == 0)
		lua_pushstring(L, getString(getProject(), "name"));
	else
	{
		lua_pushstring(L, "Package");
		lua_pushnumber(L, n + 1);
		lua_concat(L, 2);
	}
	lua_settable(L, -3);

	lua_pushstring(L, "script");
	lua_pushstring(L, currentScript);
	lua_settable(L, -3);

	lua_pushstring(L, "path");
	lua_pushstring(L, getDirectory(currentScript, 0));
	lua_settable(L, -3);

	lua_pushstring(L, "language");
	lua_pushstring(L, "c++");
	lua_settable(L, -3);

	lua_pushstring(L, "kind");
	lua_pushstring(L, "winexe");
	lua_settable(L, -3);

	lua_pushstring(L, "files");
	lua_newtable(L);
	lua_settable(L, -3);

	/* Build list of configurations that can be indexed by name or number */

	lua_pushstring(L, "config");
	lua_newtable(L);

	configs = getArray(getProject(), "configs");
	for (n = 0; n < getArraySize(configs); ++n)
	{
		const char* name = getStringFromArray(configs, n);
		lua_pushstring(L, name);
		createEmptyConfig(L, name);
		lua_settable(L, -3);

		lua_pushstring(L, name);
		lua_gettable(L, -2);
		lua_rawseti(L, -2, n + 1);
	}

	/* Mark the config table so I can dynamically add file configs later */
	lua_settag(L, fileConfigTag);

	lua_settable(L, -3);
	return 1;
}


static int rmdir(lua_State* L)
{
	const char* dir  = luaL_check_string(L, 1);
	deleteDirectory(".", dir);
	return 0;
}
