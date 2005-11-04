/**********************************************************************
 * Premake - premake.c
 * The program entry point.
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

#include <stdio.h>
#include "premake.h"
#include "arg.h"
#include "os.h"
#include "script.h"
#include "Lua/lua.h"

#include "gnu.h"

const char* DEFAULT   = "premake.lua";
const char* VERSION   = "3.0-cvs";
const char* COPYRIGHT = "Copyright (C) 2002-2005 Jason Perkins and the Premake Project";
const char* HELP_MSG  = "Type 'premake --help' for help";

const char* g_filename;
const char* g_cc;
const char* g_dotnet;
int         g_verbose;

static int  preprocess();
static int  postprocess();
static void showUsage();

int clean();


int main(int argc, char** argv)
{
	/* If no args are specified... */
	if (argc == 1)
	{
		puts(HELP_MSG);
		return 1;
	}

	/* Set defaults */
	os_detect();
	g_filename = DEFAULT;
	g_cc       = NULL;
	g_dotnet   = NULL;
	g_verbose  = 0;

	/* Process any options that will effect script processing */
	arg_set(argc, argv);
	if (!preprocess())
		return 1;

	/* chdir() to the directory containing the project script, so that
	 * relative paths may be used in the script */
	io_setcwd(path_getdir(g_filename));

	/* Now run the script */
	if (!script_run(g_filename))
	{
		puts("** Script failed to run, ending.");
		return 1;
	}

	/* Process any options that depend on the script output */
	arg_reset();
	if (!postprocess())
		return 1;

	/* All done */
	script_close();
	prj_close();
	return 0;
}


static int preprocess()
{
	const char* flag = arg_getflag();
	while (flag != NULL)
	{
		if (matches(flag, "--file"))
		{
			g_filename = arg_getflagarg();
			if (g_filename == NULL)
			{
				puts("** Usage: --file filename");
				puts(HELP_MSG);
				return 1;
			}
		}
		else if (matches(flag, "--os"))
		{
			const char* os = arg_getflagarg();
			if (os == NULL || !os_set(os))
			{
				puts("** Usage: --os osname");
				puts(HELP_MSG);
				return 1;
			}
		}
		else if (matches(flag, "--version"))
		{
			printf("premake (Premake Build Script Generator) %s\n", VERSION);
		}

		flag = arg_getflag();
	}

	return 1;
}


static int postprocess()
{
	const char* flag = arg_getflag();
	while (flag != NULL)
	{
		if (matches(flag, "--help"))
		{
			showUsage();
		}
		else
		{
			script_docommand(flag);
		}

		flag = arg_getflag();
	}

	return 1;
}


int onCommand(const char* cmd, const char* arg)
{
	if (matches(cmd, "target"))
	{
		if (!script_export())
			return 0;

		if (matches(arg, "gnu"))
		{
			return gnu_generate();
		}
		else
		{
			printf("** Unrecognized target '%s'\n", arg);
			return 0;
		}
	}

	if (matches(cmd, "clean"))
	{
		return clean();
	}

	if (matches(cmd, "cc"))
	{
		g_cc = arg;
	}
	else if (matches(cmd, "dotnet"))
	{
		g_dotnet = arg;
	}
	else if (matches(cmd, "verbose"))
	{
		g_verbose = 1;
	}

	return 1;
}


void showUsage()
{
	int i;

	printf("Premake %s, a build script generator\n", VERSION);
	puts(COPYRIGHT);
	printf("%s %s\n", LUA_VERSION, LUA_COPYRIGHT);
	puts("");
	puts(" --file name       Process the specified premake script file");
	puts("");
	puts(" --clean           Remove all binaries and build scripts");
	puts(" --verbose       Generate verbose makefiles (where applicable)");
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
	puts(" --os name         Generate files for different operating system; one of:");
	puts("      bsd       OpenBSD, NetBSD, or FreeBSD");
	puts("      linux     Linux");
	puts("      macosx    MacOS X");
	puts("      windows   Microsoft Windows");
	puts("");
	puts(" --target name     Generate input files for the specified toolset; one of:");
	puts("      gnu       GNU Makefile for POSIX, MinGW, and Cygwin");
	puts("      monodev   MonoDevelop");
	puts("      sharpdev  ICSharpCode SharpDevelop");
	puts("      vs6       Microsoft Visual Studio 6");
	puts("      vs2002    Microsoft Visual Studio 2002");
	puts("      vs2003    Microsoft Visual Studio 2003");
	puts("");
	puts(" --help            Display this information");
	puts(" --version         Display version information");
	puts("");

	if (project != NULL && prj_get_numoptions() > 0)
	{
		puts("This premake configuration also supports the following custom options:");
		puts("");

		for (i = 0; i < prj_get_numoptions(); ++i)
		{
			prj_select_option(i);
			printf(" --%-15s %s\n", prj_get_optname(), prj_get_optdesc());
		}
	}

	puts("");
}
