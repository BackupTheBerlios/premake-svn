/**********************************************************************
 * Premake - vs2002_cpp.c
 * The Visual Studio 2002 and 2003 C++ target
 *
 * Copyright (c) 2002-2005 Jason Perkins and the Premake project
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
#include <stdlib.h>
#include <string.h>
#include "premake.h"
#include "vs2002.h"

static const char* filterLinks(const char* name);
static void listFiles(const char* path, int stage);


int vs2002_cpp()
{
	int configTypeId;
	int i;

	VsPkgData* data = (VsPkgData*)prj_get_data();

	if (prj_is_kind("winexe") || prj_is_kind("exe"))
	{
		configTypeId = 1;
	}
	else if (prj_is_kind("dll"))
	{
		configTypeId = 2;
	}
	else if (prj_is_kind("lib"))
	{
		configTypeId = 4;
	}
	else if (prj_is_kind("aspnet"))
	{
		puts("** Error: C++ ASP.NET projects are not supported");
		return 0;
	}
	else
	{
		printf("** Error: unknown package kind '%s'\n", prj_get_kind());
		return 0;
	}

	/* Open the file and write the header */
	if (!io_openfile(path_join(prj_get_pkgpath(), prj_get_pkgname(), "vcproj")))
		return 0;

	prj_select_config(0);
	io_print("<?xml version=\"1.0\" encoding=\"Windows-1252\"?>\n");
	io_print("<VisualStudioProject\n");
	io_print("\tProjectType=\"Visual C++\"\n");
	io_print("\tVersion=\"7.%d0\"\n", (vs_target == 2003) ? 1 : 0);
	io_print("\tName=\"%s\"\n", prj_get_pkgname());
	io_print("\tProjectGUID=\"{%s}\"\n", data->projGuid);
	io_print("\tKeyword=\"%s\">\n", prj_has_flag("managed") ? "ManagedCProj" : "Win32Proj");  

	io_print("\t<Platforms>\n");
	io_print("\t\t<Platform\n");
	io_print("\t\t\tName=\"Win32\"/>\n");
	io_print("\t</Platforms>\n");

	/* Write configurations */
	io_print("\t<Configurations>\n");
	for (i = 0; i < prj_get_numconfigs(); ++i)
	{
		int optimization, debug, runtime, symbols;

		prj_select_config(i);

		if (prj_has_flag("optimize-speed"))
			optimization = 2;
		else if (prj_has_flag("optimize-size"))
			optimization = 1;
		else if (prj_has_flag("optimize"))
			optimization = 3;
		else
			optimization = 0;

		debug = (optimization ==0);

		if (prj_has_flag("static-runtime"))
			runtime = (debug) ? 1 : 0;
		else
			runtime = (debug) ? 3 : 2;

		if (prj_has_flag("no-symbols"))
			symbols = 0;
		else
			symbols = prj_has_flag("managed") ? 3 : 4;

		io_print("\t\t<Configuration\n");
		io_print("\t\t\tName=\"%s|Win32\"\n", prj_get_cfgname());
		io_print("\t\t\tOutputDirectory=\"%s\"\n", prj_get_outdir());
		io_print("\t\t\tIntermediateDirectory=\"%s\"\n", prj_get_objdir());
		io_print("\t\t\tConfigurationType=\"%d\"\n", configTypeId);
		io_print("\t\t\tCharacterSet=\"%d\"", prj_has_flag("unicode") ? 1 : 2);
		if (prj_has_flag("managed")) 
			io_print("\n\t\t\tManagedExtensions=\"TRUE\"");
		io_print(">\n");

		io_print("\t\t\t<Tool\n");
		io_print("\t\t\t\tName=\"VCCLCompilerTool\"\n");

		if (prj_get_numbuildoptions() > 0)
		{
			io_print("\t\t\t\tAdditionalOptions=\"");
			print_list(prj_get_buildoptions(), "", "", " ", NULL);
			io_print("\"\n");
		}

		io_print("\t\t\t\tOptimization=\"%d\"\n", optimization);

		if (prj_has_flag("no-frame-pointer")) 
			io_print("\t\t\t\tOmitFramePointers=\"TRUE\"\n");

		if (prj_get_numincpaths() > 0)
		{
			io_print("\t\t\t\tAdditionalIncludeDirectories=\"");
			print_list(prj_get_incpaths(), "", "", ";", NULL);
			io_print("\"\n");
		}

		if (prj_has_flag("managed"))
			io_print("\t\t\t\tAdditionalUsingDirectories=\"%s\"\n", prj_get_bindir());

		if (prj_get_numdefines() > 0)
		{
			io_print("\t\t\t\tPreprocessorDefinitions=\"");
			print_list(prj_get_defines(), "", "", ";", NULL);
			io_print("\"\n");
		}

		if (debug && !prj_has_flag("managed"))
			io_print("\t\t\t\tMinimalRebuild=\"TRUE\"\n");

		if (prj_has_flag("no-exceptions")) 
			io_print("\t\t\t\tExceptionHandling=\"FALSE\"\n");

		if (debug && !prj_has_flag("managed"))
			io_print("\t\t\t\tBasicRuntimeChecks=\"3\"\n");
		
		if (!debug) 
			io_print("\t\t\t\tStringPooling=\"TRUE\"\n");
		
		io_print("\t\t\t\tRuntimeLibrary=\"%d\"\n", runtime);
		io_print("\t\t\t\tEnableFunctionLevelLinking=\"TRUE\"\n");

		if (prj_has_flag("no-rtti"))
			io_print("\t\t\t\tRuntimeTypeInfo=\"FALSE\"\n");

		io_print("\t\t\t\tUsePrecompiledHeader=\"0\"\n");
		io_print("\t\t\t\tWarningLevel=\"%d\"\n", prj_has_flag("extra-warnings") ? 4 : 3);
		if (prj_has_flag("fatal-warnings"))
			io_print("\t\t\t\tWarnAsError=\"TRUE\"\n");
		if (!prj_has_flag("managed")) 
			io_print("\t\t\t\tDetect64BitPortabilityProblems=\"%s\"\n", prj_has_flag("no-64bit-checks") ? "FALSE" : "TRUE");

		io_print("\t\t\t\tDebugInformationFormat=\"%d\"/>\n", symbols);

		io_print("\t\t\t<Tool\n");
		io_print("\t\t\t\tName=\"VCCustomBuildTool\"/>\n");

		if (!prj_is_kind("lib"))
		{
			io_print("\t\t\t<Tool\n");
			io_print("\t\t\t\tName=\"VCLinkerTool\"\n");
			if (prj_has_flag("no-import-lib"))
				io_print("\t\t\t\tIgnoreImportLibrary=\"TRUE\"\n");

			if (prj_get_numlinkoptions() > 0)
			{
				io_print("\t\t\t\tAdditionalOptions=\"");
				print_list(prj_get_linkoptions(), " ", "", "", NULL);
				io_print("\"\n");
			}

			if (prj_get_numlinks() > 0)
			{
				io_print("\t\t\t\tAdditionalDependencies=\"");
				print_list(prj_get_links(), "", ".lib", " ", filterLinks);
				io_print("\"\n");
			}

			io_print("\t\t\t\tOutputFile=\"$(OutDir)/%s\"\n", path_getname(prj_get_target()));
			io_print("\t\t\t\tLinkIncremental=\"%d\"\n", debug ? 2 : 1);

			io_print("\t\t\t\tAdditionalLibraryDirectories=\"%s", prj_get_libdir());
			print_list(prj_get_libpaths(), ";", "", "", NULL);
			io_print("\"\n");

			/* Look for a .def file for DLLs */
			if (prj_find_filetype("def") != NULL)
				io_print("\t\t\t\tModuleDefinitionFile=\"%s\"\n", prj_find_filetype(".def"));

			io_print("\t\t\t\tGenerateDebugInformation=\"%s\"\n", symbols ? "TRUE" : "FALSE");
			if (symbols) 
				io_print("\t\t\t\tProgramDatabaseFile=\"$(OutDir)/%s.pdb\"\n", path_getbasename(prj_get_target()));

			io_print("\t\t\t\tSubSystem=\"%d\"\n", prj_is_kind("exe") ? 1 : 2);
			if (!debug) io_print("\t\t\t\tOptimizeReferences=\"2\"\n");
			if (!debug) io_print("\t\t\t\tEnableCOMDATFolding=\"2\"\n");

			if ((prj_is_kind("exe") || prj_is_kind("winexe")) && !prj_has_flag("no-main"))
			{
				io_print("\t\t\t\tEntryPointSymbol=\"mainCRTStartup\"\n");
			}
			else if (prj_is_kind("dll")) 
			{
				io_print("\t\t\t\tImportLibrary=\"");
				if (prj_has_flag("no-import-lib"))
					io_print(prj_get_objdir());
				else
					io_print(prj_get_libdir());
				io_print("/%s.lib\"\n", path_getbasename(prj_get_target()));
			}
			io_print("\t\t\t\tTargetMachine=\"1\"/>\n");
		}
		else
		{
			io_print("\t\t\t<Tool\n");
			io_print("\t\t\t\tName=\"VCLibrarianTool\"\n");
			io_print("\t\t\t\tOutputFile=\"$(OutDir)/%s.lib\"/>\n", path_getbasename(prj_get_target()));
		}

		io_print("\t\t\t<Tool\n");
		io_print("\t\t\t\tName=\"VCMIDLTool\"/>\n");

		io_print("\t\t\t<Tool\n");
		io_print("\t\t\t\tName=\"VCPostBuildEventTool\"/>\n");

		io_print("\t\t\t<Tool\n");
		io_print("\t\t\t\tName=\"VCPreBuildEventTool\"/>\n");

		io_print("\t\t\t<Tool\n");
		io_print("\t\t\t\tName=\"VCPreLinkEventTool\"/>\n");

		io_print("\t\t\t<Tool\n");
		io_print("\t\t\t\tName=\"VCResourceCompilerTool\"");
		if (prj_get_numincpaths() > 0)
		{
			io_print("\n\t\t\t\tAdditionalIncludeDirectories=\"");
			print_list(prj_get_incpaths(), "", "", ";", NULL);
			io_print("\"");
		}
		io_print("/>\n");

		io_print("\t\t\t<Tool\n");
		io_print("\t\t\t\tName=\"VCWebServiceProxyGeneratorTool\"/>\n");

		if (vs_target == 2003)
		{
			io_print("\t\t\t<Tool\n");
			io_print("\t\t\t\tName=\"VCXMLDataGeneratorTool\"/>\n");
		}

		io_print("\t\t\t<Tool\n");
		io_print("\t\t\t\tName=\"VCWebDeploymentTool\"/>\n");

		if (vs_target == 2003)
		{
			io_print("\t\t\t<Tool\n");
			io_print("\t\t\t\tName=\"VCManagedWrapperGeneratorTool\"/>\n");
			io_print("\t\t\t<Tool\n");
			io_print("\t\t\t\tName=\"VCAuxiliaryManagedWrapperGeneratorTool\"/>\n");
		}

		io_print("\t\t</Configuration>\n");
	}

	io_print("\t</Configurations>\n");

	if (vs_target == 2003)
	{
		io_print("\t<References>\n");
		io_print("\t</References>\n");
	}

	io_print("\t<Files>\n");
	print_source_tree("", listFiles);
	io_print("\t</Files>\n");

	io_print("\t<Globals>\n");
	io_print("\t</Globals>\n");
	io_print("</VisualStudioProject>\n");

	io_closefile();
	return 1;
}


/************************************************************************
 * Checks each entry in the list of package links. If the entry refers
 * to a sibling package, returns the path to that package's output
 ***********************************************************************/

static const char* filterLinks(const char* name)
{
	int i = prj_find_package(name);
	if (i >= 0)
	{
		const char* lang = prj_get_language_for(i);
		if (matches(lang, "c") || matches(lang, "c++"))
			return prj_get_target_for(i);
		else
			return NULL;
	}
	else
	{
		return name;
	}
}


/************************************************************************
 * Callback for print_source_tree()
 ***********************************************************************/

static void listFiles(const char* path, int stage)
{
	char indent[128];
	const char* ptr;

	strcpy(indent, "\t");
	if (strlen(path) > 0) 
		strcat(indent, "\t");

	ptr = path;
	while (strncmp(ptr, "../", 3) == 0)
		ptr += 3;

	ptr = strchr(ptr, '/');
	while (ptr != NULL) 
	{
		strcat(indent, "\t");
		ptr = strchr(ptr + 1, '/');
	}

	ptr = strrchr(path, '/');
	ptr = (ptr == NULL) ? (char*)path : ptr + 1;

	switch (stage)
	{
	case WST_OPENGROUP:
		if (strlen(path) > 0 && !matches(ptr, ".."))
		{
			io_print("%s<Filter\n", indent);
			io_print("%s\tName=\"%s\"\n", indent, ptr);
			io_print("%s\tFilter=\"\">\n", indent);
		}
		break;

	case WST_CLOSEGROUP:
		if (strlen(path) > 0 && !matches(ptr, "..")) 
			io_print("%s</Filter>\n", indent);
		break;

	case WST_SOURCEFILE:
		io_print("%s<File\n", indent);
		io_print("%s\tRelativePath=\"", indent);
		if (path[0] != '.')
			io_print(".\\");
		io_print("%s\">\n", path_translate(path, "windows"));
		io_print("%s</File>\n", indent);
		break;
	}
}
