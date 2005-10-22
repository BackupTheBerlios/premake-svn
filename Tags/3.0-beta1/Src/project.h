//-----------------------------------------------------------------------------
// Premake - project.h
//
// An interface to the project settings.
//
// Copyright (C) 2002-2004 by Jason Perkins
// Source code licensed under the GPL, see LICENSE.txt for details.
//
// $Id: project.h,v 1.19 2005/09/22 21:03:13 jason379 Exp $
//-----------------------------------------------------------------------------

struct _Package;

typedef struct _ProjectConfig
{
	const char* name;
	const char* bindir;
	const char* libdir;
} ProjectConfig;

typedef struct _FileConfig
{
	const char* buildAction;
} FileConfig;

typedef struct _Config
{
	ProjectConfig* projectConfig;
	struct _Package* package;
	const char* name;
	const char* target;
	const char* extension;
	const char** buildFlags;
	const char** buildOptions;
	const char** defines;
	const char** includePaths;
	const char** libPaths;
	const char** linkFlags;
	const char** linkOptions;
	const char** links;
	int numBuildFlags;
	int numBuildOptions;
	int numDefines;
	int numIncludePaths;
	int numLibPaths;
	int numLinkFlags;
	int numLinkOptions;
	int numLinks;
} Config;

typedef struct _Package
{
	const char* name;
	const char* script;
	const char* path;
	const char* language;
	const char* kind;
	const char* objdir;
	const char* url;
	const char** files;
	FileConfig** fileConfigs;
	Config** config;
	int numFiles;
	int numConfigs;
	void* data;
} Package;

typedef struct _Option
{
	char* flag;
	char* description;
	struct _Option* next;
} Option;

typedef struct _Project
{
	const char* name;
	const char* path;
	ProjectConfig** config;
	Package** package;
	Option** option;
	int numConfigs;
	int numPackages;
	int numOptions;
} Project;


extern Project* project;
extern char*    os;

extern void createProject(char* argv[]);
extern int  loadProject(char* project);
extern void closeProject();
extern void handleCommand(char* args[], int i);
extern Package* getPackage(const char* name);
extern FileConfig* getFileConfig(Package* package, const char* name);
