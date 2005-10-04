//-----------------------------------------------------------------------------
// Premake - vs6.c
//
// MS Visual Studio 6 tool target.
//
// Copyright (C) 2002-2005 by Jason Perkins
// Source code licensed under the GPL, see LICENSE.txt for details.
//
// $Id: vs6.c,v 1.32 2005/09/12 21:17:46 jason379 Exp $
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "project.h"
#include "project_api.h"
#include "util.h"

static char buffer[4096];

static int   writeWorkspace();
static int   writeVcProject();
static int   writeProjectHeader(FILE* file);

//-----------------------------------------------------------------------------

int makeVs6Scripts()
{
	int i, csharp = 0;

	puts("Generating Visual Studio 6 workspace and project files:");

	for (i = 0; i < prj_get_numpackages(); ++i)
	{
		prj_select_package(i);

		printf("...%s\n", prj_get_pkgname());

		if (matches(prj_get_language(), "c++") || matches(prj_get_language(), "c"))
		{
			writeVcProject();
		}
		else if (matches(prj_get_language(), "c#"))
		{
			printf("** Error: C# projects are not supported by Visual Studio 6\n");
			return 0;
		}
		else
		{
			printf("** Error: unrecognized language '%s'\n", prj_get_language());
			return 0;
		}
	}

	return writeWorkspace();
}

//-----------------------------------------------------------------------------

static const char* writePkgDeps(const char* file, void* data)
{
	Package* pkg = getPackage(file);
	if (pkg != NULL)
	{
		strcpy(buffer, "    Begin Project Dependency\n");
		strcat(buffer, "    Project_Dep_Name ");
		strcat(buffer, pkg->name);
		strcat(buffer, "\n");
		strcat(buffer, "    End Project Dependency\n");
		return buffer;
	}

	return NULL;
}

static int writeWorkspace()
{
	int i;

	FILE* file = openFile(project->path, project->name, ".dsw");
	if (file == NULL)
		return 0;

	fprintf(file, "Microsoft Developer Studio Workspace File, Format Version 6.00\n");
	fprintf(file, "# WARNING: DO NOT EDIT OR DELETE THIS WORKSPACE FILE!\n");
	fprintf(file, "\n");
	fprintf(file, "###############################################################################\n");
	fprintf(file, "\n");

	for (i = 0; i < prj_get_numpackages(); ++i)
	{
		prj_select_package(i);

		fprintf(file, "Project: \"%s\"=%s.dsp - Package Owner=<4>\n", prj_get_pkgname(), prj_get_pkgpath(WINDOWS,1));
		fprintf(file, "\n");
		fprintf(file, "Package=<5>\n");
		fprintf(file, "{{{\n");
		fprintf(file, "}}}\n");
		fprintf(file, "\n");
		fprintf(file, "Package=<4>\n");
		fprintf(file, "{{{\n");

		/* Write package dependencies */
		prj_select_config(0);
		writeList(file, prj_get_links(), "", "", "", writePkgDeps, NULL);

		fprintf(file, "}}}\n");
		fprintf(file, "\n");
		fprintf(file, "###############################################################################\n");
		fprintf(file, "\n");
	}

	fprintf(file, "Global:\n");
	fprintf(file, "\n");
	fprintf(file, "Package=<5>\n");
	fprintf(file, "{{{\n");
	fprintf(file, "}}}\n");
	fprintf(file, "\n");
	fprintf(file, "Package=<3>\n");
	fprintf(file, "{{{\n");
	fprintf(file, "}}}\n");
	fprintf(file, "\n");
	fprintf(file, "###############################################################################\n");
	fprintf(file, "\n");

	fclose(file);
	return 1;
}

//-----------------------------------------------------------------------------

static void vcFiles(FILE* file, const char* path, int stage)
{
	const char* ptr = strrchr(path, '/');
	ptr = (ptr == NULL) ? path : ptr + 1;

	switch (stage)
	{
	case WST_OPENGROUP:
		if (strlen(path) > 0 && strcmp(ptr, "..") != 0) {
			fprintf(file, "# Begin Group \"%s\"\n\n", ptr);
			fprintf(file, "# PROP Default_Filter \"\"\n");
		}
		break;
	case WST_CLOSEGROUP:
		if (strlen(path) > 0 && strcmp(ptr, "..") != 0) 
			fprintf(file, "# End Group\n");
		break;
	case WST_SOURCEFILE:
		fprintf(file, "# Begin Source File\n\n");
		fprintf(file, "SOURCE=%s\n", translatePath(path,WINDOWS));
		fprintf(file, "# End Source File\n");
		break;
	}
}

static const char* checkLink(const char* path, void* data)
{
	Package* package = getPackage(path);
	if (package == NULL) return path;

	if (strcmp(package->language, "c++") == 0)
		return prj_get_target_for(package);
	
	return NULL;
}

static const char* convertPath(const char* path, void* data)
{
	return translatePath(path, WINDOWS);
}

static void writeCppFlags(FILE* file)
{
	int optimizeSize, optimizeSpeed, useDebugLibs;

	optimizeSize  =  prj_has_buildflag("optimize-size");
	optimizeSpeed =  prj_has_buildflag("optimize-speed") || prj_has_buildflag("optimize");
	useDebugLibs  =  (!optimizeSize && !optimizeSpeed);

	if (useDebugLibs)
		fprintf(file, prj_has_buildflag("static-runtime") ? " /MTd" : " /MDd");
	else
		fprintf(file, prj_has_buildflag("static-runtime") ? " /MT" : " /MD");
	
	fprintf(file, " /W%d", prj_has_buildflag("extra-warnings") ? 4 : 3);
	
	if (prj_has_buildflag("fatal-warnings"))
		fprintf(file, " /WX");
	
	if (useDebugLibs)
		fprintf(file, " /Gm");  /* minimal rebuild */
	
	if (!prj_has_buildflag("no-rtti"))
		fprintf(file, " /GR");
	
	if (!prj_has_buildflag("no-exceptions"))
		fprintf(file, " /GX");
	
	if (!prj_has_buildflag("no-symbols"))
		fprintf(file, " /ZI");  /* debug symbols for edit-and-continue */
	
	if (optimizeSize)
		fprintf(file, " /O1");
	else if (optimizeSpeed)
		fprintf(file, " /O2");
	else
		fprintf(file, " /Od");
	
	if (prj_has_buildflag("no-frame-pointer"))
		fprintf(file, " /Oy");
	
	writeList(file, prj_get_includepaths(), " /I \"", "\"", "", convertPath, NULL);

	writeList(file, prj_get_defines(), " /D \"", "\"", "", NULL, NULL);
	
	fprintf(file, " /YX /FD");
	
	if (!optimizeSize && !optimizeSpeed)
		fprintf(file, " /GZ");
	
	fprintf(file, " /c");
	
	writeList(file, prj_get_buildoptions(), " ", "", "", NULL, NULL);

	fprintf(file, "\n");
}


static void writeLinkFlags(FILE* file)
{
	writeList(file, prj_get_links(), " ", ".lib", "", checkLink, NULL);
	fprintf(file, " /nologo");

	if ((matches(prj_get_kind(), "winexe") || matches(prj_get_kind(), "exe")) && !prj_has_buildflag("no-main"))
		fprintf(file, " /entry:\"mainCRTStartup\"");

	if (matches(prj_get_kind(), "winexe"))
		fprintf(file, " /subsystem:windows");
	else if (matches(prj_get_kind(), "exe"))
		fprintf(file, " /subsystem:console");
	else
		fprintf(file, " /dll");

	if (!prj_has_buildflag("no-symbols"))
		fprintf(file, " /debug");

	fprintf(file, " /machine:I386");

	fprintf(file, " /out:\"%s", prj_get_outdir(WINDOWS,1));
	fprintf(file, "%s\"", prj_get_target());

	if (!prj_has_buildflag("no-symbols"))
		fprintf(file, " /pdbtype:sept");

	fprintf(file, " /libpath:\"%s\"", prj_get_libdir(WINDOWS,0));
	writeList(file, prj_get_libpaths(), " /libpath:\"", "\"", "", convertPath, NULL);

	writeList(file, prj_get_linkoptions(), " ", "", "", NULL, NULL);

	fprintf(file, "\n");
}


static int writeVcProject()
{
	FILE* file;
	int i;

	/* Start the file */
	file = openFile(prj_get_pkgpath(NATIVE,0), prj_get_pkgname(), ".dsp");
	if (file == NULL)
		return 0;
	writeProjectHeader(file);

	fprintf(file, "CPP=cl.exe\n");
	if (!matches(prj_get_kind(), "lib"))
		fprintf(file, "MTL=midl.exe\n");
	fprintf(file, "RSC=rc.exe\n");
	fprintf(file, "\n");

	for (i = 0; i < prj_get_numconfigs(); ++i)
	{
		int optimizeSize, optimizeSpeed, useDebugLibs;
		const char* debugSymbol;

		prj_select_config(i);

		optimizeSize  =  prj_has_buildflag("optimize-size");
		optimizeSpeed =  prj_has_buildflag("optimize-speed") || prj_has_buildflag("optimize");
		useDebugLibs  =  (!optimizeSize && !optimizeSpeed);

		fprintf(file, "!%s  \"$(CFG)\" == \"%s - %s\"\n", 
			(i == 0 ? "IF" : "ELSEIF"), prj_get_pkgname(), prj_get_cfgname());
		fprintf(file, "\n");
		fprintf(file, "# PROP BASE Use_MFC 0\n");
		fprintf(file, "# PROP BASE Use_Debug_Libraries %d\n", useDebugLibs ? 1 : 0);
		fprintf(file, "# PROP BASE Output_Dir \"%s\"\n", prj_get_outdir(WINDOWS,0));
		fprintf(file, "# PROP BASE Intermediate_Dir \"%s\"\n", prj_get_objdir(WINDOWS,0));
		fprintf(file, "# PROP BASE Target_Dir \"\"\n");
		fprintf(file, "# PROP Use_MFC 0\n");
		fprintf(file, "# PROP Use_Debug_Libraries %d\n", useDebugLibs ? 1 : 0);
		fprintf(file, "# PROP Output_Dir \"%s\"\n", prj_get_outdir(WINDOWS,0));
		fprintf(file, "# PROP Intermediate_Dir \"%s\"\n", prj_get_objdir(WINDOWS,0));
		if (matches(prj_get_kind(), "dll") && prj_has_buildflag("no-import-lib"))
			fprintf(file, "# PROP Ignore_Export_Lib 1\n");
		fprintf(file, "# PROP Target_Dir \"\"\n");

		fprintf(file, "# ADD BASE CPP /nologo");
		writeCppFlags(file);
		fprintf(file, "# ADD CPP /nologo");
		writeCppFlags(file);

		debugSymbol = prj_has_buildflag("no-symbols") ? "NDEBUG" : "_DEBUG";
		if (matches(prj_get_kind(), "winexe") || matches(prj_get_kind(), "dll"))
		{
			fprintf(file, "# ADD BASE MTL /nologo /D \"%s\" /mktyplib203 /win32\n", debugSymbol);
			fprintf(file, "# ADD MTL /nologo /D \"%s\" /mktyplib203 /win32\n", debugSymbol);
		}

		fprintf(file, "# ADD BASE RSC /l 0x409 /d \"%s\"\n", debugSymbol);
		fprintf(file, "# ADD RSC /l 0x409 /d \"%s\"\n", debugSymbol);
		fprintf(file, "BSC32=bscmake.exe\n");
		fprintf(file, "# ADD BASE BSC32 /nologo\n");
		fprintf(file, "# ADD BSC32 /nologo\n");
		
		if (matches(prj_get_kind(), "lib"))
		{
			fprintf(file, "LINK32=link.exe -lib\n");
			fprintf(file, "# ADD BASE LIB32 /nologo\n");
			fprintf(file, "# ADD LIB32 /nologo\n");
		}
		else
		{
			fprintf(file, "LINK32=link.exe\n");
			fprintf(file, "# ADD BASE LINK32");
			writeLinkFlags(file);
			fprintf(file, "# ADD LINK32");
			writeLinkFlags(file);
		}
	}

	fprintf(file, "!ENDIF\n");
	fprintf(file, "\n");
	fprintf(file, "# Begin Target\n");
	fprintf(file, "\n");

	for (i = 0; i < prj_get_numconfigs(); ++i)
	{
		prj_select_config(i);
		fprintf(file, "# Name \"%s - %s\"\n", prj_get_pkgname(), prj_get_cfgname());
	}

	walkSourceList(file, prj_get_package(), "", vcFiles);

	fprintf(file, "# End Target\n");
	fprintf(file, "# End Project\n");

#if WORK_IN_PROGRESS


				if (strcmp(package->kind, "dll") == 0)
				{
					fprintf(file, " /implib:\"");
					if (importlib)
						fprintf(file, reversePath(package->path, prjCfg->libdir, WINDOWS, 1));
					else
						fprintf(file, "$(IntDir)\\");
					fprintf(file, "%s.lib\"", translatePath(config->target, WINDOWS));
				}

				fprintf(file, "\n");
	}
#endif

	fclose(file);
	return 1;
}

//-----------------------------------------------------------------------------

static int writeProjectHeader(FILE* file)
{
	int i;
	const char* tag;
	const char* num;

	if (strcmp(prj_get_kind(), "winexe") == 0)
	{
		tag = "Win32 (x86) Application";
		num = "0x0101";
	}
	else if (strcmp(prj_get_kind(), "exe") == 0)
	{
		tag = "Win32 (x86) Console Application";
		num = "0x0103";
	}
	else if (strcmp(prj_get_kind(), "dll") == 0)
	{
		tag = "Win32 (x86) Dynamic-Link Library";
		num = "0x0102";
	}
	else if (strcmp(prj_get_kind(), "lib") == 0)
	{
		tag = "Win32 (x86) Static Library";
		num = "0x0104";
	}
	else
	{
		puts("** Error: unrecognized package type");
		return 0;
	}

	fprintf(file, "# Microsoft Developer Studio Project File - Name=\"%s\" - Package Owner=<4>\n", prj_get_pkgname());
	fprintf(file, "# Microsoft Developer Studio Generated Build File, Format Version 6.00\n");
	fprintf(file, "# ** DO NOT EDIT **\n");
	fprintf(file, "\n");
	fprintf(file, "# TARGTYPE \"%s\" %s\n", tag, num);
	fprintf(file, "\n");

	prj_select_config(0);
	fprintf(file, "CFG=%s - %s\n", prj_get_pkgname(), prj_get_cfgname());

	fprintf(file, "!MESSAGE This is not a valid makefile. To build this project using NMAKE,\n");
	fprintf(file, "!MESSAGE use the Export Makefile command and run\n");
	fprintf(file, "!MESSAGE \n");
	fprintf(file, "!MESSAGE NMAKE /f \"%s.mak\".\n", prj_get_pkgname());
	fprintf(file, "!MESSAGE \n");
	fprintf(file, "!MESSAGE You can specify a configuration when running NMAKE\n");
	fprintf(file, "!MESSAGE by defining the macro CFG on the command line. For example:\n");
	fprintf(file, "!MESSAGE \n");
	fprintf(file, "!MESSAGE NMAKE /f \"%s.mak\" CFG=\"%s - %s\"\n", prj_get_pkgname(), prj_get_pkgname(), prj_get_cfgname());
	fprintf(file, "!MESSAGE \n");
	fprintf(file, "!MESSAGE Possible choices for configuration are:\n");
	fprintf(file, "!MESSAGE \n");


	for (i = 0; i < prj_get_numconfigs(); ++i)
	{
		prj_select_config(i);
		fprintf(file, "!MESSAGE \"%s - %s\" (based on \"%s\")\n", prj_get_pkgname(), prj_get_cfgname(), tag);
	}

	fprintf(file, "!MESSAGE \n");
	fprintf(file, "\n");
	fprintf(file, "# Begin Project\n");
	fprintf(file, "# PROP AllowPerConfigDependencies 0\n");
	fprintf(file, "# PROP Scc_ProjName \"\"\n");
	fprintf(file, "# PROP Scc_LocalPath \"\"\n");

	return 1;
}
