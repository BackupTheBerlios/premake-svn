//-----------------------------------------------------------------------------
// Premake - premake.c
//
// Program entry point and project/package file parsing.
//
// Copyright (C) 2002-2005 by Jason Perkins
// Source code licensed under the GPL, see LICENSE.txt for details.
//
// $Id: premake.c,v 1.73 2005/09/22 21:03:13 jason379 Exp $
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include "project.h"
#include "util.h"
#include "Lua/lua.h"

#define VERSION   "3.0-beta1"
#define COPYRIGHT "Copyright (C) 2002-2005 Jason Perkins"

extern int  makeGnuScripts();
extern int  makeSharpDevScripts(int version);
extern int  makeVs6Scripts();
extern int  makeVsXmlScripts(int version);
extern int  makeVs2005Scripts();
extern int  makeClean();

static void showCompilers();
static void showOptions();
static void showTargets();
static void showUsage();
static void showCustomUsage();

const char** commandLineArgs;
const char* rootProjectFile = NULL;
const char* cc = NULL;
const char* dotnet = NULL;
int verbose = 0;

//-----------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	int i;

	char* filename = "premake.lua";
	args = argv;

	if (argc == 1)
	{
		puts("Type 'premake --help' for help");
		return 1;
	}

	/* Look for some "high priority" command line options */
	for (i = 1; argv[i] != NULL; ++i)
	{
		if (strcmp(argv[i], "--file") == 0)
		{
			if (argv[i + 1] == NULL || strncmp(argv[i + 1], "--", 2) == 0)
			{
				fprintf(stderr, "Usage: --file filename");
				return 0;
			}
			filename = argv[i + 1];
		}
		else if (strcmp(argv[i], "--os") == 0)
		{
			os = argv[i + 1];
		}
		else if (strcmp(argv[i], "--help") == 0)
		{
			showUsage();
		}
		else if (strcmp(argv[i], "--version") == 0)
		{
			printf("premake (Premake Build Script Generator) %s\n", VERSION);
		}
	}

	// chdir() to the directory containing the project, so that relative
	// paths to the packages will work
	setCwd(getDirectory(filename, 0));

	// Remember the root project filename. This is used by the gmake generator
	// to rebuild the makefiles when the premake files change
	rootProjectFile = filename;

	createProject(argv);
	if (!loadProject(filename))
		return 1;

	// Project must have at least one package
	if (project->numPackages == 0)
	{
		puts("** Error: project must have at least one package.");
		closeProject();
		return 0;
	}

	/* Process the command-line options */
	for (i = 0; argv[i] != NULL; ++i)
	{
		if (strncmp(argv[i], "--", 2) == 0)
		{
			if (strcmp(argv[i], "--file") == 0 ||
			    strcmp(argv[i], "--version") == 0)
			{
				/* Already handled above */ ;
			}
			else if (strcmp(argv[i], "--help") == 0)
			{
				showCustomUsage();
			}
			else if (strcmp(argv[i], "--show-options") == 0 ||
				     strcmp(argv[i], "--show-compilers") == 0 ||
					 strcmp(argv[i], "--show-targets") == 0)
			{
				/* deprecated commands */
				printf("The option '%s' has been deprecated.\n", argv[i]);
				printf("Type 'premake --help' for help.\n");
			}
			else
			{
				handleCommand(argv, i);
			}
		}
	}

	closeProject();
	return 0;
}

//-----------------------------------------------------------------------------

void defaultCommand(int argc, const char* argv[])
{
	int result, i;

	if (strcmp(argv[0], "target") == 0)
	{
		for (i = 1; i < argc; ++i)
		{
			if (strcmp(argv[i], "gnu") == 0)
				result = makeGnuScripts();
			else if (strcmp(argv[i], "md") == 0 || strcmp(argv[i], "monodev") == 0)
				result = makeSharpDevScripts(1);
			else if (strcmp(argv[i], "sd") == 0 || strcmp(argv[i], "sharpdev") == 0)
				result = makeSharpDevScripts(0);
			else if (strcmp(argv[i], "vs6") == 0)
				result = makeVs6Scripts();
			else if (strcmp(argv[i], "vs7") == 0 || strcmp(argv[i], "vs2002") == 0)
				result = makeVsXmlScripts(7);
			else if (strcmp(argv[i], "vs2003") == 0)
				result = makeVsXmlScripts(2003);
			else if (strcmp(argv[i], "vs2005") == 0)
				result = makeVs2005Scripts();
			else
			{
				printf("** Unrecognized target '%s'\n", argv[i]);
				return;
			}

			if (!result) break;
		}
	}
	else if (strcmp(argv[0], "clean") == 0)
	{
		makeClean();
	}
	else if (strcmp(argv[0], "cc") == 0)
	{
		cc = argv[1];
	}
	else if (strcmp(argv[0], "dotnet") == 0)
	{
		dotnet = argv[1];
	}
	else if (strcmp(argv[0], "verbose") == 0)
	{
		verbose = 1;
	}
	else if (strcmp(argv[0], "very-verbose") == 0)
	{
		verbose = 2;
	}
}

//-----------------------------------------------------------------------------

static void showUsage()
{
	printf("Premake %s, a build script generator\n", VERSION);
	puts(COPYRIGHT);
	printf("%s %s\n", LUA_VERSION, LUA_COPYRIGHT);
	puts("");
	puts(" --file name       Process the specified premake script file");
	puts("");
	puts(" --clean           Remove all binaries and build scripts");
	puts(" --verbose         Generate verbose makefiles (where applicable)");
	puts(" --very-verbose    Generate very verbose makefiles (where applicable)");
	puts("");
	puts(" --cc name         Choose a C/C++ compiler, if supported by target; one of:");
	puts("      gcc       GNU gcc compiler");
	puts("      dmc       Digital Mars C/C+ compiler (experimental)");
	puts("");
	puts(" --dotnet name     Choose a .NET compiler set, if supported by target; one of:");
	puts("      ms        Microsoft (csc)");
	puts("      mono      Mono (msc)");
	puts("      pnet      Portable.NET (cscc)");
	puts("");
	puts(" --target name     Generate input files for the specified toolset; one of:");
	puts("      gnu       GNU Makefile for POSIX, MinGW, and Cygwin");
	puts("      monodev   MonoDevelop (experimental)");
	puts("      sharpdev  ICSharpCode SharpDevelop");
	puts("      vs6       Microsoft Visual Studio 6");
	puts("      vs2002    Microsoft Visual Studio 2002");
	puts("      vs2003    Microsoft Visual Studio 2003");
	puts("");
	puts(" --help            Display this information");
	puts(" --version         Display version information");
	puts("");
}

static void showCustomUsage()
{
	int i;
	if (project != NULL && project->numOptions > 0)
	{
		puts("This premake configuration also supports the following custom options:");
		puts("");

		for (i = 0; i < project->numOptions; ++i)
			printf(" --%-15s %s\n", project->option[i]->flag, project->option[i]->description);
	}
	puts("");
}