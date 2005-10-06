//-----------------------------------------------------------------------------
// Premake - vs7.c
//
// MS Visual Studio XML projects files (v7.0-2003)
//
// Copyright (C) 2002-2005 by Jason Perkins
// Source code licensed under the GPL, see LICENSE.txt for details.
//
// $Id: vs7.c,v 1.58 2005/09/30 21:17:29 jason379 Exp $
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "project.h"
#include "project_api.h"
#include "util.h"
#include "vs_helpers.h"

static char buffer[4096];
static int  version;

static int writeSolution();
static int writeVcProject();
static int writeCsProject();

//-----------------------------------------------------------------------------

int makeVsXmlScripts(int v)
{
	int i;

	version = v;
	printf("Generating Visual Studio %d solution and project files:\n", version);

	/* Assign GUIDs to packages */
	vs_SetTargetVersion(version);
	vs_AssignPackageData(version);

	/* Generate the project files */
	for (i = 0; i < prj_get_numpackages(); ++i)
	{
		prj_select_package(i);

		printf("...%s\n", prj_get_pkgname());

		if (prj_is_language("c++") || prj_is_language("c"))
		{
			if (!writeVcProject(version, prj_get_package()))
				return 0;
		}
		else if (prj_is_language("c#"))
		{
			if (!writeCsProject(version, prj_get_package()))
				return 0;
		}
		else
		{
			printf("** Warning: %s packages are not supported by this generator\n", prj_get_language());
		}
	}

	if (!writeSolution(version))
		return 0;

	return 1;
}

//-----------------------------------------------------------------------------

static int writeSolution()
{
	int i, j;
	FILE* file;

	file = openFile(prj_get_prjpath(WINDOWS), prj_get_prjname(), ".sln");
	if (file == NULL)
		return 0;

	/* Format identification string */
	fprintf(file, "Microsoft Visual Studio Solution File, Format Version ");
	fprintf(file, "%d.00\n", version == 2003 ? 8 : 7);

	/* List the projects that make up the solution */
	vs_WriteProjectList(file);

	fprintf(file, "Global\n");
	fprintf(file, "\tGlobalSection(SolutionConfiguration) = preSolution\n");
	prj_select_package(0);
	for (i = 0; i < prj_get_numconfigs(); ++i)
	{
		prj_select_config(i);
		if (version == 2003)
		{
			fprintf(file, "\t\t%s = %s\n", prj_get_cfgname(), prj_get_cfgname());
		}
		else
		{
			fprintf(file, "\t\tConfigName.%d = %s\n", i, prj_get_cfgname());
		}
	}
	fprintf(file, "\tEndGlobalSection\n");

	/* Find package dependencies for VS2002 */
	if (version < 2003)
	{
		fprintf(file, "\tGlobalSection(ProjectDependencies) = postSolution\n");
		for (i = 0; i < prj_get_numpackages(); ++i)
		{
			prj_select_package(i);
			prj_select_config(0);
			writeList(file, prj_get_links(), "\t\t", "\n", "", vs_FindProjectDependencies, NULL);
		}
		fprintf(file, "\tEndGlobalSection\n");
	}

	/* Write configuration for each project */
	fprintf(file, "\tGlobalSection(ProjectConfiguration) = postSolution\n");
	for (i = 0; i < prj_get_numpackages(); ++i)
	{
		prj_select_package(i);
		for (j = 0; j < prj_get_numconfigs(); ++j)
		{
			PkgData* data;
			const char* name;

			prj_select_config(j);
			data = (PkgData*)prj_get_packagedata();
			name = prj_get_cfgname();

			if (version < 2005)
			{
				fprintf(file, "\t\t{%s}.%s.ActiveCfg = %s|%s\n", data->projGuid, name, name, data->projType);
				fprintf(file, "\t\t{%s}.%s.Build.0 = %s|%s\n", data->projGuid, name, name, data->projType);
			}
			else
			{
				fprintf(file, "\t\t{%s}.%s|%s.ActiveCfg = %s|%s\n", data->projGuid, name, data->projType, name, data->projType);
				fprintf(file, "\t\t{%s}.%s|%s.Build.0 = %s|%s\n", data->projGuid, name, data->projType, name, data->projType);
			}
		}
	}
	fprintf(file, "\tEndGlobalSection\n");

	/* Finish */
	fprintf(file, "\tGlobalSection(ExtensibilityGlobals) = postSolution\n");
	fprintf(file, "\tEndGlobalSection\n");
	fprintf(file, "\tGlobalSection(ExtensibilityAddIns) = postSolution\n");
	fprintf(file, "\tEndGlobalSection\n");
	fprintf(file, "EndGlobal\n");
	fclose(file);
	return 1;
}

//-----------------------------------------------------------------------------

static const char* checkDir(const char* path, void* data)
{
	return translatePath(path, WINDOWS);
}

static const char* checkLibs(const char* file, void* data)
{
	Package* package = getPackage(file);
	if (package == NULL) return file;
	if (strcmp(package->language, "c++") != 0) return NULL;
	return package->config[*((int*)data)]->target;
}

static void vcFiles(FILE* file, const char* path, int stage)
{
	char indent[128];
	const char* ptr;

	strcpy(indent, "\t");
	if (strlen(path) > 0) strcat(indent, "\t");

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
		if (strlen(path) > 0 && strcmp(ptr, "..") != 0)
		{
			fprintf(file, "%s<Filter\n", indent);
			fprintf(file, "%s\tName=\"%s\"\n", indent, ptr);
			fprintf(file, "%s\tFilter=\"\">\n", indent);
		}
		break;

	case WST_CLOSEGROUP:
		if (strlen(path) > 0 && strcmp(ptr, "..") != 0) 
			fprintf(file, "%s</Filter>\n", indent);
		break;

	case WST_SOURCEFILE:
		fprintf(file, "%s<File\n", indent);
		fprintf(file, "%s\tRelativePath=\"", indent);
		if (path[0] != '.')
			fprintf(file, ".\\");
		fprintf(file, "%s\">\n", translatePath(path, WINDOWS));
		fprintf(file, "%s</File>\n", indent);
		break;
	}
}

static int writeVcProject()
{
	int configType, subsystem, managed, i;
	const char* extension;
	const char* exports;
	FILE* file;

	Package* package = prj_get_package();

	const char* name = package->name;
	const char* path = package->path;
	const char* kind = package->kind;
	PkgData* data = (PkgData*)package->data;

	/* Set up target type information */
	if (strcmp(kind, "winexe") == 0 || strcmp(kind, "exe") == 0)
	{
		configType = 1;
		extension = "exe";
	}
	else if (strcmp(kind, "dll") == 0)
	{
		configType = 2;
		extension = "dll";
	}
	else if (strcmp(kind, "lib") == 0)
	{
		configType = 4;
		extension = "lib";
	}
	else if (strcmp(kind, "aspnet") == 0)
	{
		puts("** Error: C++ ASP.NET projects are not supported");
		return 0;
	}
	else
	{
		printf("** Error: unknown package kind '%s'\n", kind);
		return 0;
	}

	/* Check the build subsystem...must be the same for all configs */
	subsystem = (strcmp(kind, "exe") == 0) ? 1 : 2;
	managed = inArray(package->config[0]->buildFlags, "managed");

	/* Open the file and write the header */
	file = openFile(path, name, ".vcproj");
	if (file == NULL)
		return 0;

	fprintf(file, "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>\n");
	fprintf(file, "<VisualStudioProject\n");
	fprintf(file, "\tProjectType=\"Visual C++\"\n");
	fprintf(file, "\tVersion=\"7.%d0\"\n", version == 2003 ? 1 : 0);
	fprintf(file, "\tName=\"%s\"\n", name);
	fprintf(file, "\tProjectGUID=\"{%s}\"\n", data->projGuid);
	if (version == 2005)
	{
		fprintf(file, "\tRootNamespace=\"%s\"\n", package->name);
	}
	fprintf(file, "\tKeyword=\"%s\">\n", managed ? "ManagedCProj" : "Win32Proj");  

	fprintf(file, "\t<Platforms>\n");
	fprintf(file, "\t\t<Platform\n");
	fprintf(file, "\t\t\tName=\"Win32\"/>\n");
	fprintf(file, "\t</Platforms>\n");

	if (version == 2005)
	{
		fprintf(file, "\t<ToolFiles>\n");
		fprintf(file, "\t</ToolFiles>\n");
	}

	/* Look for a .def file */
	exports = NULL;
	for (i = 0; i < package->numFiles; ++i)
	{
		if (strcmp(getExtension(package->files[i]), ".def") == 0)
			exports = package->files[i];
	}

	/* Write configurations */
	fprintf(file, "\t<Configurations>\n");
	for (i = 0; i < package->numConfigs; ++i)
	{
		int optLevel, debug, symbols, runtime, noMain;
		ProjectConfig* prjCfg = project->config[i];
		Config* config = package->config[i];

		const char* targetext = (config->extension != NULL) ? config->extension : extension;

		int size  = inArray(config->buildFlags, "optimize-size");
		int speed = inArray(config->buildFlags, "optimize-speed");
		int optimize = inArray(config->buildFlags, "optimize") || size || speed;

		int check64bit = !inArray(config->buildFlags, "no-64bit-checks");
		int importlib  = !inArray(config->buildFlags, "no-import-lib");
		int exceptions = !inArray(config->buildFlags, "no-exceptions");
		int rtti       = !inArray(config->buildFlags, "no-rtti");
		int omitFrame  = inArray(config->buildFlags, "no-frame-pointer");
		int warnings   = inArray(config->buildFlags, "extra-warnings") ? 4 : 3;
		int fatalWarn  = inArray(config->buildFlags, "fatal-warnings");
		int unicode    = inArray(config->buildFlags, "unicode");

		prj_select_config(i);

		if (speed)
			optLevel = 2;
		else if (size)
			optLevel = 1;
		else if (optimize)
			optLevel = 3;
		else
			optLevel = 0;
		debug = (optLevel == 0);

		if (!inArray(config->buildFlags, "no-symbols"))
			symbols = (managed) ? 3 : 4;
		else
			symbols = 0;

		if (inArray(config->linkFlags, "static-runtime") || inArray(config->buildFlags, "static-runtime"))
			runtime = (debug) ? 1 : 0;
		else
			runtime = (debug) ? 3 : 2;

		noMain = inArray(config->buildFlags, "no-main");

		fprintf(file, "\t\t<Configuration\n");
		fprintf(file, "\t\t\tName=\"%s|Win32\"\n", config->name);
		fprintf(file, "\t\t\tOutputDirectory=\"%s\"\n", prj_get_outdir(WINDOWS, 0));
		fprintf(file, "\t\t\tIntermediateDirectory=\"%s\"\n", prj_get_objdir(WINDOWS, 0));
		fprintf(file, "\t\t\tConfigurationType=\"%d\"\n", configType);
		fprintf(file, "\t\t\tCharacterSet=\"%d\"", (unicode) ? 1 : 2);
		if (managed) fprintf(file, "\n\t\t\tManagedExtensions=\"TRUE\"");
		fprintf(file, ">\n");

		fprintf(file, "\t\t\t<Tool\n");
		fprintf(file, "\t\t\t\tName=\"VCCLCompilerTool\"\n");

		if (config->numBuildOptions > 0)
		{
			fprintf(file, "\t\t\t\tAdditionalOptions=\"");
				writeList(file, config->buildOptions, "", "", " ", NULL, NULL);
				fprintf(file, "\"\n");
		}

		fprintf(file, "\t\t\t\tOptimization=\"%d\"\n", optLevel);

		/*		if (!debug) fprintf(file, "\t\t\t\tInlineFunctionExpansion=\"2\"\n"); */
		if (omitFrame) 
			fprintf(file, "\t\t\t\tOmitFramePointers=\"TRUE\"\n");

		if (config->numIncludePaths > 0)
		{
			fprintf(file, "\t\t\t\tAdditionalIncludeDirectories=\"");
				writeList(file, config->includePaths, "", "", ";", checkDir, NULL);
				fprintf(file, "\"\n");
		}

		if (managed)
		{
			fprintf(file, "\t\t\t\tAdditionalUsingDirectories=\"");
			fprintf(file, reversePath(path, prjCfg->bindir, WINDOWS, 1));
			fprintf(file, "\"\n");
		}

		if (config->numDefines > 0)
		{
			fprintf(file, "\t\t\t\tPreprocessorDefinitions=\"");
				writeList(file, config->defines, "", "", ";", NULL, NULL);
				fprintf(file, "\"\n");
		}

		if (debug && !managed)
			fprintf(file, "\t\t\t\tMinimalRebuild=\"TRUE\"\n");

		if (!exceptions) 
			fprintf(file, "\t\t\t\tExceptionHandling=\"FALSE\"\n");

		if (debug && !managed)
			fprintf(file, "\t\t\t\tBasicRuntimeChecks=\"3\"\n");
		
		if (!debug) fprintf(file, "\t\t\t\tStringPooling=\"TRUE\"\n");
		
		fprintf(file, "\t\t\t\tRuntimeLibrary=\"%d\"\n", runtime);
		fprintf(file, "\t\t\t\tEnableFunctionLevelLinking=\"TRUE\"\n");

		if (!rtti)
			fprintf(file, "\t\t\t\tRuntimeTypeInfo=\"FALSE\"\n");

		fprintf(file, "\t\t\t\tUsePrecompiledHeader=\"0\"\n");
		fprintf(file, "\t\t\t\tWarningLevel=\"%d\"\n", warnings);
		if (fatalWarn)
			fprintf(file, "\t\t\t\tWarnAsError=\"TRUE\"\n");
		if (!managed) fprintf(file, "\t\t\t\tDetect64BitPortabilityProblems=\"%s\"\n", check64bit ? "TRUE" : "FALSE");

		fprintf(file, "\t\t\t\tDebugInformationFormat=\"%d\"/>\n", symbols);

		if (version < 2005)
		{
			fprintf(file, "\t\t\t<Tool\n");
			fprintf(file, "\t\t\t\tName=\"VCCustomBuildTool\"/>\n");
		}
		else
		{
			fprintf(file, "\t\t\t<Tool\n");
			fprintf(file, "\t\t\t\tName=\"VCManagedResourceCompilerTool\"\n");
			fprintf(file, "\t\t\t/>\n");
			fprintf(file, "\t\t\t<Tool\n");
			fprintf(file, "\t\t\t\tName=\"VCResourceCompilerTool\"\n");
			fprintf(file, "\t\t\t/>\n");
			fprintf(file, "\t\t\t<Tool\n");
			fprintf(file, "\t\t\t\tName=\"VCPreLinkEventTool\"\n");
			fprintf(file, "\t\t\t/>\n");
		}

		if (configType != 4)
		{
			fprintf(file, "\t\t\t<Tool\n");
			fprintf(file, "\t\t\t\tName=\"VCLinkerTool\"\n");
			if (!importlib)
				fprintf(file, "\t\t\t\tIgnoreImportLibrary=\"TRUE\"\n");
			if (config->numLinkOptions > 0)
			{
				fprintf(file, "\t\t\t\tAdditionalOptions=\"");
					writeList(file, config->linkOptions, " ", "", "", NULL, NULL);
					fprintf(file, "\"\n");
			}
			if (config->numLinks > 0)
			{
				fprintf(file, "\t\t\t\tAdditionalDependencies=\"");
					writeList(file, config->links, "", ".lib", " ", checkLibs, &i);
					fprintf(file, "\"\n");
			}
			fprintf(file, "\t\t\t\tOutputFile=\"$(OutDir)/%s.%s\"\n", getFilename(config->target, 0), targetext);
			fprintf(file, "\t\t\t\tLinkIncremental=\"%d\"\n", debug ? 2 : 1);
			fprintf(file, "\t\t\t\tAdditionalLibraryDirectories=\"");
				fprintf(file, prj_get_libdir(WINDOWS, 0));
/*				fprintf(file, reversePath(path, prjCfg->libdir, WINDOWS, 0)); */
				writeList(file, config->libPaths, ";", "", "", checkDir, NULL);

				fprintf(file, "\"\n");
			if (exports)
				fprintf(file, "\t\t\t\tModuleDefinitionFile=\"%s\"\n", translatePath(exports, WINDOWS));
			fprintf(file, "\t\t\t\tGenerateDebugInformation=\"%s\"\n", symbols ? "TRUE" : "FALSE");
			if (symbols && version < 2005) 
			{
				fprintf(file, "\t\t\t\tProgramDatabaseFile=\"$(OutDir)/%s.pdb\"\n", getFilename(config->target, 0));
			}
			fprintf(file, "\t\t\t\tSubSystem=\"%d\"\n", subsystem);
			if (!debug) fprintf(file, "\t\t\t\tOptimizeReferences=\"2\"\n");
			if (!debug) fprintf(file, "\t\t\t\tEnableCOMDATFolding=\"2\"\n");
			if ((strcmp(kind, "winexe") == 0 || strcmp(kind, "exe") == 0) && !noMain)
				fprintf(file, "\t\t\t\tEntryPointSymbol=\"mainCRTStartup\"\n");
			else if (strcmp(kind, "dll") == 0) {
				fprintf(file, "\t\t\t\tImportLibrary=\"");
				if (prj_has_buildflag("no-import-lib"))
					fprintf(file, prj_get_objdir(WINDOWS,0));
				else
					fprintf(file, prj_get_libdir(WINDOWS,0));
				fprintf(file, "\\%s.lib\"\n", prj_get_targetname());
			}
			fprintf(file, "\t\t\t\tTargetMachine=\"1\"/>\n");
		}
		else
		{
			fprintf(file, "\t\t\t<Tool\n");
			fprintf(file, "\t\t\t\tName=\"VCLibrarianTool\"\n");
			fprintf(file, "\t\t\t\tOutputFile=\"$(OutDir)/%s.lib\"/>\n", config->target);
		}

		if (version < 2005)
		{
			fprintf(file, "\t\t\t<Tool\n");
			fprintf(file, "\t\t\t\tName=\"VCMIDLTool\"/>\n");

			fprintf(file, "\t\t\t<Tool\n");
			fprintf(file, "\t\t\t\tName=\"VCPostBuildEventTool\"/>\n");

			fprintf(file, "\t\t\t<Tool\n");
			fprintf(file, "\t\t\t\tName=\"VCPreBuildEventTool\"/>\n");

			fprintf(file, "\t\t\t<Tool\n");
			fprintf(file, "\t\t\t\tName=\"VCPreLinkEventTool\"/>\n");

			fprintf(file, "\t\t\t<Tool\n");
			fprintf(file, "\t\t\t\tName=\"VCResourceCompilerTool\"");
			if (config->numIncludePaths > 0)
			{
				fprintf(file, "\n\t\t\t\tAdditionalIncludeDirectories=\"");
					writeList(file, config->includePaths, "", "", ";", checkDir, NULL);
				fprintf(file, "\"");
			}
			fprintf(file, "/>\n");

			fprintf(file, "\t\t\t<Tool\n");
			fprintf(file, "\t\t\t\tName=\"VCWebServiceProxyGeneratorTool\"/>\n");

			if (version == 2003)
			{
				fprintf(file, "\t\t\t<Tool\n");
				fprintf(file, "\t\t\t\tName=\"VCXMLDataGeneratorTool\"/>\n");
			}

			fprintf(file, "\t\t\t<Tool\n");
			fprintf(file, "\t\t\t\tName=\"VCWebDeploymentTool\"/>\n");

			if (version == 2003)
			{
				fprintf(file, "\t\t\t<Tool\n");
				fprintf(file, "\t\t\t\tName=\"VCManagedWrapperGeneratorTool\"/>\n");
				fprintf(file, "\t\t\t<Tool\n");
				fprintf(file, "\t\t\t\tName=\"VCAuxiliaryManagedWrapperGeneratorTool\"/>\n");
			}
		}
		else
		{
			fprintf(file, "\t\t\t<Tool\n");
			fprintf(file, "\t\t\t\tName=\"VCALinkTool\"\n");
			fprintf(file, "\t\t\t/>\n");
			fprintf(file, "\t\t\t<Tool\n");
			fprintf(file, "\t\t\t\tName=\"VCManifestTool\"\n");
			fprintf(file, "\t\t\t/>\n");
			fprintf(file, "\t\t\t<Tool\n");
			fprintf(file, "\t\t\t\tName=\"VCXDCMakeTool\"\n");
			fprintf(file, "\t\t\t/>\n");
			fprintf(file, "\t\t\t<Tool\n");
			fprintf(file, "\t\t\t\tName=\"VCBscMakeTool\"\n");
			fprintf(file, "\t\t\t/>\n");
			fprintf(file, "\t\t\t<Tool\n");
			fprintf(file, "\t\t\t\tName=\"VCFxCopTool\"\n");
			fprintf(file, "\t\t\t/>\n");
			fprintf(file, "\t\t\t<Tool\n");
			fprintf(file, "\t\t\t\tName=\"VCAppVerifierTool\"\n");
			fprintf(file, "\t\t\t/>\n");
			fprintf(file, "\t\t\t<Tool\n");
			fprintf(file, "\t\t\t\tName=\"VCWebDeploymentTool\"\n");
			fprintf(file, "\t\t\t/>\n");
			fprintf(file, "\t\t\t<Tool\n");
			fprintf(file, "\t\t\t\tName=\"VCPostBuildEventTool\"\n");
			fprintf(file, "\t\t\t/>\n");
		}

		fprintf(file, "\t\t</Configuration>\n");
	}

	fprintf(file, "\t</Configurations>\n");

	if (version == 2003)
	{
		fprintf(file, "\t<References>\n");
		fprintf(file, "\t</References>\n");
	}

	fprintf(file, "\t<Files>\n");
	walkSourceList(file, package, "", vcFiles);
	fprintf(file, "\t</Files>\n");

	fprintf(file, "\t<Globals>\n");
	fprintf(file, "\t</Globals>\n");
	fprintf(file, "</VisualStudioProject>\n");

	fclose(file);
	return 1;
}

//-----------------------------------------------------------------------------

static const char* checkRefs(const char* ref, void* data)
{
	int i;
	char* comma;
	char tmp[1024];

	/* Pull the name out of the full reference */
	strcpy(tmp, ref);
	comma = strchr(tmp, ',');
	if (comma != NULL)
		*comma = '\0';

	/* Write the reference name */
	strcpy(buffer, "\t\t\t\t\tName = \"");
	strcat(buffer, tmp);
	strcat(buffer, "\"\n");

	/* Is this a sibling project? */
	for (i = 0; i < project->numPackages; ++i)
	{
		if (strcmp(project->package[i]->name, ref) == 0)
		{
			PkgData* data = (PkgData*)project->package[i]->data;
			strcat(buffer, "\t\t\t\t\tProject = \"{");
			strcat(buffer, data->projGuid);
			strcat(buffer, "}\"\n");
			strcat(buffer, "\t\t\t\t\tPackage = \"{");
			strcat(buffer, data->toolGuid);
			strcat(buffer, "}\"\n");
			return buffer;
		}
	}

	strcat(buffer, "\t\t\t\t\tAssemblyName = \"");
	strcat(buffer, getFilename(tmp, 0));
	strcat(buffer, "\"\n");
	if (strlen(tmp) != strlen(getFilename(tmp, 0))) {
		strcat(buffer, "\t\t\t\t\tHintPath = \"");
		strcat(buffer, tmp);
		strcat(buffer, ".dll\"\n");
	}

	/* Tack on any extra information about the assembly */
	while (comma != NULL)
	{
		char* start;
		for (start = comma + 1; *start == ' '; ++start);
		comma = strchr(start, '=');
		*comma = '\0';
		strcat(buffer, "\t\t\t\t\t");
		strcat(buffer, start);
		strcat(buffer, " = \"");

		start = comma + 1;
		comma = strchr(start, ',');
		if (comma != NULL) *comma = '\0';
		strcat(buffer, start);
		strcat(buffer, "\"\n");
	}

	return buffer;
}

static const char* checkRefPaths(const char* ref, void* data)
{
	const char* projPath = (const char*)data;
	return makeAbsolute(projPath, ref);
}

static const char* checkSrcFileType(const char* file, void* data)
{
	Package* package = (Package*)data;

	strcpy(buffer, translatePath(file, WINDOWS));
	strcat(buffer, "\"\n");

	if (endsWith(file, ".aspx.cs") || endsWith(file, ".asax.cs"))
	{
		/* The path to the parent .aspx file is relative to the .cs file. 
		 * I assume that they are in the same directory and strip off all
		 * path information, then cut off the '.cs' */
		char* ptr = strrchr(file, '/');
		if (ptr == NULL) 
			ptr = (char*)file;
		else
			ptr++;

		strcat(buffer, "\t\t\t\t\tDependentUpon = \"");
		strncat(buffer, ptr, strlen(ptr) - 3);
		strcat(buffer, "\"\n");
		strcat(buffer, "\t\t\t\t\tSubType = \"ASPXCodeBehind\"\n");
		strcat(buffer, "\t\t\t\t\tBuildAction = \"Compile\"\n");
	}
	else if (endsWith(file, ".cs"))
	{
		strcat(buffer, "\t\t\t\t\tSubType = \"Code\"\n");
		strcat(buffer, "\t\t\t\t\tBuildAction = \"Compile\"\n");
	}
	else if (endsWith(file, ".aspx"))
	{
		strcat(buffer, "\t\t\t\t\tSubType = \"Form\"\n");
		strcat(buffer, "\t\t\t\t\tBuildAction = \"Content\"\n");
	}
	else if (endsWith(file, ".asax"))
	{
		strcat(buffer, "\t\t\t\t\tSubType = \"Component\"\n");
		strcat(buffer, "\t\t\t\t\tBuildAction = \"Content\"\n");
	}
	else if (endsWith(file, ".resx"))
	{
		/* If a matching .cs file exists, link it */
		char depname[2048];
		strcpy(depname, file);
		strcpy(depname + strlen(file) - 5, ".cs");
		if (inArray(package->files, depname))
		{
			/* Path is relative to .resx file, I assume both are in same
			 * directory and cut off path information */
			char* ptr = strrchr(depname, '/');
			ptr = (ptr != NULL) ? ptr+1 : depname;
			strcat(buffer, "\t\t\t\t\tDependentUpon = \"");
			strcat(buffer, ptr);
			strcat(buffer, "\"\n");
		}
		strcat(buffer, "\t\t\t\t\tBuildAction = \"EmbeddedResource\"\n");
	}
	else
	{
		FileConfig* config = getFileConfig(package, file);
		strcat(buffer, "\t\t\t\t\tBuildAction = \"");
		if (config->buildAction)
			strcat(buffer, config->buildAction);
		else
			strcat(buffer, "Content");
		strcat(buffer, "\"\n");
	}

	return buffer;
}

/* Determine if any other packages depend on this one */
static int packageIsReferenced(const char* name)
{
	int i, j, k;
	for (i = 0; i < project->numPackages; ++i)
	{
		Package* pkg = project->package[i];
		for (j = 0; j < pkg->numConfigs; ++j)
		{
			Config* cfg = pkg->config[j];
			for (k = 0; k < cfg->numLinks; ++k)
			{
				if (strcmp(cfg->links[k], name) == 0)
					return 1;
			}
		}
	}
	return 0;
}


static int writeCsProject()
{
	FILE* file;
	const char* target;
	const char* outputType;
	int i;

	Package* package = prj_get_package();

	const char* name = package->name;
	const char* path = package->path;
	const char* kind = package->kind;
	PkgData* data = (PkgData*)package->data;

	if (strcmp(kind, "winexe") == 0)
		outputType = "WinExe";
	else if (strcmp(kind, "exe") == 0)
		outputType = "Exe";
	else if (strcmp(kind, "dll") == 0 || strcmp(kind, "aspnet") == 0)
		outputType = "Library";
	else
	{
		printf("** Error: unknown package kind '%s'\n", kind);
		return 0;
	}

	/* Open the project file and write the header */
	file = openFile(path, name, ".csproj");
	if (file == NULL)
		return 0;

	fprintf(file, "<VisualStudioProject>\n");
	fprintf(file, "\t<CSHARP\n");
	fprintf(file, "\t\tProjectType = \"");
		fprintf(file, strcmp(kind, "aspnet") == 0 ? "Web" : "Local");
		fprintf(file, "\"\n");
	
	if (version == 2003)
	{
		fprintf(file, "\t\tProductVersion = \"7.10.3077\"\n");
		fprintf(file, "\t\tSchemaVersion = \"2.0\"\n");
	}
	else
	{
		fprintf(file, "\t\tProductVersion = \"7.0.9254\"\n");
		fprintf(file, "\t\tSchemaVersion = \"1.0\"\n");
	}

	fprintf(file, "\t\tProjectGuid = \"{%s}\"\n", data->projGuid);
	fprintf(file, "\t>\n");
	fprintf(file, "\t\t<Build>\n");
	fprintf(file, "\t\t\t<Settings\n");
	fprintf(file, "\t\t\t\tApplicationIcon = \"\"\n");
	fprintf(file, "\t\t\t\tAssemblyKeyContainerName = \"\"\n");
	fprintf(file, "\t\t\t\tAssemblyName = \"%s\"\n", getFilename(package->config[0]->target, 0));
	fprintf(file, "\t\t\t\tAssemblyOriginatorKeyFile = \"\"\n");
	fprintf(file, "\t\t\t\tDefaultClientScript = \"JScript\"\n");
	fprintf(file, "\t\t\t\tDefaultHTMLPageLayout = \"Grid\"\n");
	fprintf(file, "\t\t\t\tDefaultTargetSchema = \"IE50\"\n");
	fprintf(file, "\t\t\t\tDelaySign = \"false\"\n");
	if (version < 2003) 
	{
		fprintf(file, "\t\t\t\tNoStandardLibraries = \"false\"\n");
	}
	fprintf(file, "\t\t\t\tOutputType = \"%s\"\n", outputType);
	if (version == 2003)
	{
		fprintf(file, "\t\t\t\tPreBuildEvent = \"\"\n");
		fprintf(file, "\t\t\t\tPostBuildEvent = \"\"\n");
	}
	fprintf(file, "\t\t\t\tRootNamespace = \"%s\"\n", getFilename(package->config[0]->target, 0));
	if (version == 2003)
	{		
		fprintf(file, "\t\t\t\tRunPostBuildEvent = \"OnBuildSuccess\"\n");
	}

	fprintf(file, "\t\t\t\tStartupObject = \"\"\n");
	fprintf(file, "\t\t\t>\n");

	for (i = 0; i < package->numConfigs; ++i)
	{
		ProjectConfig* prjCfg = project->config[i];
		Config* config = package->config[i];

		int symbols   = !inArray(config->buildFlags, "no-symbols");
		int optimize  =  inArray(config->buildFlags, "optimize") || inArray(config->buildFlags, "optimize-size") || inArray(config->buildFlags, "optimize-speed");
		int unsafe    =  inArray(config->buildFlags, "unsafe");
		int fatalWarn =  inArray(config->buildFlags, "fatal-warnings");

		fprintf(file, "\t\t\t\t<Config\n");
		fprintf(file, "\t\t\t\t\tName = \"%s\"\n", config->name);
		fprintf(file, "\t\t\t\t\tAllowUnsafeBlocks = \"%s\"\n", unsafe ? "true" : "false");
		fprintf(file, "\t\t\t\t\tBaseAddress = \"285212672\"\n");
		fprintf(file, "\t\t\t\t\tCheckForOverflowUnderflow = \"false\"\n");
		fprintf(file, "\t\t\t\t\tConfigurationOverrideFile = \"\"\n");
		fprintf(file, "\t\t\t\t\tDefineConstants = \"");
			writeList(file, config->defines, "", "", ";", NULL, NULL);
			fprintf(file, "\"\n");
		fprintf(file, "\t\t\t\t\tDocumentationFile = \"\"\n");
		fprintf(file, "\t\t\t\t\tDebugSymbols = \"%s\"\n", symbols ? "true" : "false");
		fprintf(file, "\t\t\t\t\tFileAlignment = \"4096\"\n");
		fprintf(file, "\t\t\t\t\tIncrementalBuild = \"false\"\n");
		if (version == 2003)
		{
			fprintf(file, "\t\t\t\t\tNoStdLib = \"false\"\n");
			fprintf(file, "\t\t\t\t\tNoWarn = \"\"\n");
		}
		fprintf(file, "\t\t\t\t\tOptimize = \"%s\"\n", optimize ? "true" : "false");

		/* VS.NET has a bug that causes builds to break when the size of a
		 * compiled assembly grows > 64K. It happens when DLLs are built to
		 * the same directory as an executable that references them. Since
		 * MS apparently has no plans to fix this problem that has been 
		 * around since 2003, try to work around it here */
		fprintf(file, "\t\t\t\t\tOutputPath = \"");
		if (strcmp(package->kind, "dll") == 0 && packageIsReferenced(package->name))
		{
			fprintf(file, "%s\\%s", translatePath(package->objdir, WINDOWS), config->name);
		}
		else
		{
			fprintf(file, reversePath(path, prjCfg->bindir, WINDOWS, 1));
		}
		insertPath(file, getDirectory(config->target, 0), WINDOWS, 0);
		fprintf(file, "\"\n");

		fprintf(file, "\t\t\t\t\tRegisterForComInterop = \"false\"\n");
		fprintf(file, "\t\t\t\t\tRemoveIntegerChecks = \"false\"\n");
		fprintf(file, "\t\t\t\t\tTreatWarningsAsErrors = \"%s\"\n", fatalWarn ? "true" : "false");
		fprintf(file, "\t\t\t\t\tWarningLevel = \"4\"\n");  /* C# defaults to highest warning level */
		fprintf(file, "\t\t\t\t/>\n");
	}

	fprintf(file, "\t\t\t</Settings>\n");

	/* VS7 requires same references for all configurations */
	fprintf(file, "\t\t\t<References>\n");
	writeList(file, package->config[0]->links, "\t\t\t\t<Reference\n", "\t\t\t\t/>\n", "", checkRefs, package);
	fprintf(file, "\t\t\t</References>\n");
	fprintf(file, "\t\t</Build>\n");

	fprintf(file, "\t\t<Files>\n");
	fprintf(file, "\t\t\t<Include>\n");
	writeList(file, package->files, "\t\t\t\t<File\n\t\t\t\t\tRelPath = \"", "\t\t\t\t/>\n", "", checkSrcFileType, package);
	fprintf(file, "\t\t\t</Include>\n");
	fprintf(file, "\t\t</Files>\n");
	fprintf(file, "\t</CSHARP>\n");
	fprintf(file, "</VisualStudioProject>\n");

	fclose(file);

	/* Now write the .csproj.user file for non-web applications or
	 * .csproj.webinfo for web applications */
	
	if (strcmp(kind, "aspnet") != 0)
	{
		file = openFile(path, name, ".csproj.user");
		if (file == NULL)
			return 0;

		fprintf(file, "<VisualStudioProject>\n");
		fprintf(file, "\t<CSHARP>\n");
		fprintf(file, "\t\t<Build>\n");
		fprintf(file, "\t\t\t<Settings ReferencePath = \"");
			writeList(file, package->config[0]->libPaths, "", ";", "", checkRefPaths, (void*)path);
			target = makeAbsolute(".", project->config[0]->bindir);
			if (target != NULL) fprintf(file, target);
			fprintf(file, "\" >\n");

		for (i = 0; i < package->numConfigs; ++i)
		{
			fprintf(file, "\t\t\t\t<Config\n");
			fprintf(file, "\t\t\t\t\tName = \"%s\"\n", package->config[i]->name);
			fprintf(file, "\t\t\t\t\tEnableASPDebugging = \"false\"\n");
			fprintf(file, "\t\t\t\t\tEnableASPXDebugging = \"false\"\n");
			fprintf(file, "\t\t\t\t\tEnableUnmanagedDebugging = \"false\"\n");
			fprintf(file, "\t\t\t\t\tEnableSQLServerDebugging = \"false\"\n");
			fprintf(file, "\t\t\t\t\tRemoteDebugEnabled = \"false\"\n");
			fprintf(file, "\t\t\t\t\tRemoteDebugMachine = \"\"\n");
			fprintf(file, "\t\t\t\t\tStartAction = \"Project\"\n");
			fprintf(file, "\t\t\t\t\tStartArguments = \"\"\n");
			fprintf(file, "\t\t\t\t\tStartPage = \"\"\n");
			fprintf(file, "\t\t\t\t\tStartProgram = \"\"\n");
			fprintf(file, "\t\t\t\t\tStartURL = \"\"\n");
			fprintf(file, "\t\t\t\t\tStartWorkingDirectory = \"\"\n");
			fprintf(file, "\t\t\t\t\tStartWithIE = \"false\"\n");
			fprintf(file, "\t\t\t\t/>\n");
		}

		fprintf(file, "\t\t\t</Settings>\n");
		fprintf(file, "\t\t</Build>\n");
		fprintf(file, "\t\t<OtherProjectSettings\n");
		fprintf(file, "\t\t\tCopyProjectDestinationFolder = \"\"\n");
		fprintf(file, "\t\t\tCopyProjectUncPath = \"\"\n");
		fprintf(file, "\t\t\tCopyProjectOption = \"0\"\n");
		fprintf(file, "\t\t\tProjectView = \"ProjectFiles\"\n");
		fprintf(file, "\t\t\tProjectTrust = \"0\"\n");
		fprintf(file, "\t\t/>\n");
		fprintf(file, "\t</CSHARP>\n");
		fprintf(file, "</VisualStudioProject>\n");

		fclose(file);
	}
	else
	{
		if (package->url == NULL)
		{
			sprintf(buffer, "http://localhost/%s", package->name);
			package->url = buffer;
		}

		file = openFile(path, name, ".csproj.webinfo");
		if (file == NULL)
			return 0;

		fprintf(file, "<VisualStudioUNCWeb>\n");
		fprintf(file, "\t<Web URLPath = \"%s/%s.csproj\" />\n", package->url, package->name);
		fprintf(file, "</VisualStudioUNCWeb>\n");

		fclose(file);
	}

	return 1;
}
