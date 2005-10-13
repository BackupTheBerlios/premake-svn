//-----------------------------------------------------------------------------
// Premake - sharpdev.c
//
// ICSharpCode SharpDevelop tool target.
// Source code licensed under the GPL, see LICENSE.txt for details.
//
// Originally written by Chris McGuirk (leedgitar@latenitegames.com)
// Maintained by Jason Perkins
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "project.h"
#include "project_api.h"
#include "util.h"

#define SHARPDEV   0
#define MONODEV    1

static char buffer[4096];
static int  warnContentBuildAction;
static int  version;
static int  pathType;

static int writeCombine();
static int writeCsProject(Package* package);

extern const char* dotnet;

//-----------------------------------------------------------------------------

int makeSharpDevScripts(int v)
{
	int i;

	version = v;
	pathType = (version == SHARPDEV) ? WINDOWS : UNIX;

	printf("Generating %sDevelop combine and project files:", version == SHARPDEV ? "Sharp" : "Mono");

	warnContentBuildAction = 0;
	for (i = 0; i < project->numPackages; ++i)
	{
		Package* package = project->package[i];
		printf("...%s\n", package->name);
		if (strcmp(package->language, "c#") == 0)
		{
			if (!writeCsProject(package))
				return 0;
		}
		else if (strcmp(package->language, "c++") == 0 || strcmp(package->language, "c") == 0)
		{
			printf("** Error: SharpDevelop does not support C/C++ development.\n");
			return 0;
		}
		else
		{
			printf("** Error: unrecognized language '%s'\n", package->language);
			return 0;
		}
	}

	if (!writeCombine())
		return 0;

	if (warnContentBuildAction)
	{
		puts("\n** Warning: this project uses the 'Content' build action. This action is not");
		puts("            supported by #develop; some manual configuration may be needed.");
	}

	return 1;
}

//-----------------------------------------------------------------------------

static int writeCombine()
{
	int i, j, k;

	FILE* file;
	file = openFile(project->path, project->name, ".cmbx");
	if (file == NULL)
		return 0;

	fprintf(file, "<Combine fileversion=\"1.0\" name=\"%s\" description=\"\">\n", project->name);

	// TODO: select the first executable project
	if(project->numPackages != 0)
	{
		fprintf(file, "  <StartMode startupentry=\"\" single=\"True\">\n");
	}

	// first write out the startup entries
	for (i = 0; i < project->numPackages; ++i)
	{
		Package* package = project->package[i];
		const char* name = package->name;
		fprintf(file, "    <Execute entry=\"%s\" type=\"None\" />\n", name);
	}

	fprintf(file, "  </StartMode>\n");
	fprintf(file, "  <Entries>\n");

	// now write out the project entries
	for (i = 0; i < prj_get_numpackages(); ++i)
	{
		prj_select_package(i);
		fprintf(file, "    <Entry filename=\"%s.prjx\" />\n", prj_get_pkgpathfromprj(pathType, 1));
	}

	fprintf(file, "  </Entries>\n");
	fprintf(file, "  <Configurations active=\"Debug\">\n");

	// write out the entries for each build configuration
	for (i = 0; i < project->package[0]->numConfigs; ++i)
	{
		const char* configName = project->package[0]->config[i]->name;

		fprintf(file, "    <Configuration name=\"%s\">\n", configName);

		// loop through each package.  if has a configuration matching the curent one, write an entry
		for(j = 0; j < project->numPackages; j++)
		{
			Package* package = project->package[j];
			const char* name = package->name;

			int isIncluded = 0;

			// look at each of this projects configs
			for(k = 0; k < package->numConfigs; k++)
			{
				if(strcmp(configName, package->config[k]->name) != 0)
				{
					isIncluded = 1;
					break;
				}
			}

			// write the entry for the current project in this build configuration
			fprintf(file, "      <Entry name=\"%s\" configurationname=\"%s\" build=\"%s\" />\n", package->name, configName, isIncluded ? "True" : "False");
		}

        fprintf(file, "    </Configuration>\n");
	}

	// Finish
	fprintf(file, "  </Configurations>\n");
	fprintf(file, "</Combine>");
	fclose(file);

	/* MonoDevelop adds another file */
	if (version == MONODEV)
	{
		file = openFile(project->path, project->name, ".mdsx");
		if (file == NULL)
			return 0;

		fprintf(file, "<MonoDevelopSolution fileversion=\"1.0\">\n");
		fprintf(file, "  <RelativeOutputPath>%s</RelativeOutputPath>\n", project->config[0]->bindir);
		fprintf(file, "</MonoDevelopSolution>\n");

		fclose(file);
	}

	return 1;
}

//-----------------------------------------------------------------------------

static const char* checkDir(const char* path, void* data)
{
	return translatePath(path, pathType);
}

static const char* checkLibs(const char* file, void* data)
{
	Package* package = getPackage(file);
	if (package == NULL) return file;
	if (strcmp(package->language, "c#") != 0) return NULL;
	return package->config[*((int*)data)]->target;
}

//-----------------------------------------------------------------------------

static const char* checkRefs(const char* ref, void* data)
{
	int i;
	int isSibling = 0;
	const char* fileName = getFilename(ref, 0);

	strcpy(buffer," type=\"");

	/* A bit of craziness here...#dev wants to know if an assembly is local
	 * (type == "Assembly") or in the GAC (type == "GAC"). I would prefer
	 * to not have to specify this in the premake script if I can get away
	 * with it. So for each reference I check to see if it is a sibling
	 * project, and if so I consider it local. If not, I check all of the
	 * reference paths to see if I can find the DLL and if so I consider
	 * it local. If not, I consider it in the GAC. Seems to work so far */

	for (i = 0; i < project->numPackages; ++i)
	{
		if (strcmp(project->package[i]->name, ref) == 0)
		{
			isSibling = 1;
			break;
		}
	}
		
	if(isSibling)
	{
		strcat(buffer, "Project\"");
		strcat(buffer, " refto=\"");
		strcat(buffer, fileName);
		strcat(buffer, "\" localcopy=\"True\"");
	}
	else
	{
		const char* ext = strchr(ref,',') ? "" : ".dll";

		/* See if this assembly exists on one of the link paths */
		Package* package = (Package*)data;
		if (fileExists(project->config[0]->bindir, ref, ext))
		{
			strcat(buffer, "Assembly\" refto=\"");
			strcat(buffer, reversePath(package->path, project->config[0]->bindir, pathType, 1));
			strcat(buffer, "/");
			strcat(buffer, ref);
			strcat(buffer, ".dll\" localcopy=\"False\"");
		}
		else
		{
			strcat(buffer, "Gac\" refto=\"");
			strcat(buffer, ref);
			strcat(buffer, ext);
			strcat(buffer, "\" localcopy=\"False\"");
		}
	}

	return buffer;
}

static const char* checkRefPaths(const char* ref, void* data)
{
	const char* projPath = (const char*)data;
	return makeAbsolute(projPath, ref);
}

static const char* writeFileList(const char* file, void* data)
{
	const char* ext;
	const char* prefix  = "";
	const char* subtype = "";
	const char* action  = "";
	const char* depends = "";

	Package* package = (Package*)data;

	if (file[0] != '.')
		prefix = (pathType == WINDOWS) ? ".\\" : "./";

	ext = getExtension(file);
	if (strcmp(ext, ".cs") == 0)
	{
		subtype = "Code";
		action = "Compile";
	}
	else if (strcmp(ext, ".resx") == 0)
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
			depends = (ptr != NULL) ? ptr+1 : depname;
		}
		strcat(buffer, "\t\t\t\t\tBuildAction = \"EmbeddedResource\"\n");

		action = "EmbedAsResource";
	}
	else
	{
		FileConfig* config = getFileConfig(package, file);
		action = config->buildAction;
		if (action == NULL || strcmp(action, "Content") == 0)
		{
			warnContentBuildAction = 1;
			action = "Nothing";
		}
	}

	sprintf(buffer, "    <File name=\"%s%s\" subtype=\"%s\" buildaction=\"%s\" dependson=\"%s\" data=\"\" />\n", prefix, translatePath(file, pathType), subtype, action, depends);
	return buffer;
}

static int writeCsProject(Package* package)
{
	FILE* file;
	const char* runtime;
	const char* csc;
	int i;

	const char* name = package->name;
	const char* path = package->path;
	const char* kind = package->kind;

	if (strcmp(kind, "winexe") == 0)
		kind = "WinExe";
	else if (strcmp(kind, "exe") == 0)
		kind = "Exe";
	else if (strcmp(kind, "dll") == 0 || strcmp(kind, "aspnet") == 0)
		kind = "Library";
	else
	{
		printf("** Error: unknown package kind '%s'\n", kind);
		return 0;
	}

	/* Figure out what .NET environment I'm using */
	if (dotnet == NULL)
		dotnet = (strcmp(os, "windows") == 0) ? "ms" : "mono";
	
	if (strcmp(dotnet, "ms") == 0)
	{
		runtime = "MsNet";
		csc = "Csc";
	}
	else if (strcmp(dotnet, "mono") == 0)
	{
		runtime = "Mono";
		csc = "Mcs";
	}
	else if (strcmp(dotnet, "pnet") == 0)
	{
		printf("** Error: SharpDevelop does not yet support Portable.NET\n");
		return 0;
	}
	else
	{
		printf("** Error: unknown .NET runtime '%s'\n", dotnet);
		return 0;
	}


	/* Open the project file and write the header */
	file = openFile(path, name, ".prjx");
	if (file == NULL)
		return 0;

	/* Project Header */
	if (version == SHARPDEV)
		fprintf(file, "<Project name=\"%s\" standardNamespace=\"%s\" description=\"\" newfilesearch=\"None\" enableviewstate=\"True\" version=\"1.1\" projecttype=\"C#\">\n", name, name);
	else
		fprintf(file, "<Project name=\"%s\" description=\"\" newfilesearch=\"None\" enableviewstate=\"True\" version=\"1.1\" projecttype=\"C#\">\n", name, name);

	/* File List */
	fprintf(file, "  <Contents>\n");
	writeList(file, package->files, "", "", "", writeFileList, package);
	fprintf(file, "  </Contents>\n");

	/* References - all configuration will use the same set */
	fprintf(file, "  <References>\n");
	writeList(file, package->config[0]->links, "    <Reference", " />\n", "", checkRefs, package);
	fprintf(file, "  </References>\n");

	fprintf(file, "  <DeploymentInformation target=\"\" script=\"\" strategy=\"File\" />\n");
	  
	 /* Configurations */
	fprintf(file, "  <Configurations active=\"%s\">\n", package->config[0]->name);

	for (i = 0; i < package->numConfigs; ++i)
	{
		ProjectConfig* prjCfg = project->config[i];
		Config* config = package->config[i];

		int symbols   = !inArray(config->buildFlags, "no-symbols");
		int optimize  =  inArray(config->buildFlags, "optimize") || inArray(config->buildFlags, "optimize-size") || inArray(config->buildFlags, "optimize-speed");
		int unsafe    =  inArray(config->buildFlags, "unsafe");
		int fatalWarn =  inArray(config->buildFlags, "fatal-warnings");

		fprintf(file, "    <Configuration runwithwarnings=\"%s\" name=\"%s\">\n", fatalWarn ? "False" : "True", config->name);
		fprintf(file, "      <CodeGeneration runtime=\"%s\" compiler=\"%s\" compilerversion=\"\" ", runtime, csc);
		fprintf(file, "warninglevel=\"4\" nowarn=\"\" ");  /* C# defaults to highest warning level */
		fprintf(file, "includedebuginformation=\"%s\" ", symbols ? "True" : "False"); 
		fprintf(file, "optimize=\"%s\" ", optimize ? "True" : "False");
		fprintf(file, "unsafecodeallowed=\"%s\" ", unsafe ? "True" : "False");
		fprintf(file, "generateoverflowchecks=\"True\" ");
		fprintf(file, "mainclass=\"\" ");
		fprintf(file, "target=\"%s\" ", kind); 
		fprintf(file, "definesymbols=\"");
			writeList(file, config->defines, "", "", ";", NULL, NULL);
			fprintf(file, "\" ");
		fprintf(file, "generatexmldocumentation=\"False\" ");
		fprintf(file, "win32Icon=\"\" noconfig=\"\" nostdlib=\"False\" ");
		fprintf(file, "/>\n");

		fprintf(file, "      <Execution commandlineparameters=\"\" consolepause=\"True\" />\n");

		fprintf(file, "      <Output ");
		fprintf(file, "directory=\"");
			fprintf(file, reversePath(path, prjCfg->bindir, pathType, 1));
			insertPath(file, getDirectory(config->target, 0), pathType, 0);
			fprintf(file, "\" ");
	
		fprintf(file, "assembly=\"%s\" ", getFilename(package->config[0]->target,0));
		fprintf(file, "executeScript=\"\" ");
		fprintf(file, "executeBeforeBuild=\"\" ");
		fprintf(file, "executeAfterBuild=\"\" ");
		fprintf(file, "executeBeforeBuildArguments=\"\" ");
		fprintf(file, "executeAfterBuildArguments=\"\" ");
		fprintf(file, "/>\n");		

		fprintf(file, "    </Configuration>\n", config->name);
	}

	fprintf(file, "  </Configurations>\n");
	fprintf(file, "</Project>\n");

	fclose(file);
	return 1;
}
