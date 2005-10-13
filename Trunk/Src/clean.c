//-----------------------------------------------------------------------------
// Premake - clean.c
//
// Remove all project and intermediate files.
//
// Copyright (C) 2002-2003 by Jason Perkins
// Source code licensed under the GPL, see LICENSE.txt for details.
//
// $Id: clean.c,v 1.9 2005/03/01 16:02:00 jason379 Exp $
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include "project.h"
#include "util.h"

//-----------------------------------------------------------------------------

int makeClean()
{
	int i, j;

	puts("Removing all project and intermediate files...");

	deleteFile(project->path, project->name, ".sln");    // VS200x
	deleteFile(project->path, project->name, ".suo");    // VS200x
	deleteFile(project->path, project->name, ".ncb");    // VS6/200x
	deleteFile(project->path, project->name, ".dsw");    // VS6
	deleteFile(project->path, project->name, ".opt");    // VS6
	deleteFile(project->path, "Makefile", "");           // GNU
	deleteFile(project->path, project->name, ".cmbx");   // SharpDevelop/MonoDevelop
	deleteFile(project->path, project->name, ".mdsx");   // MonoDevelop
	deleteFile(project->path, "make.sh", "");            // MonoDevelop
	
	for (i = 0; i < project->numPackages; ++i)
	{
		Package* package = project->package[i];
		const char* name = package->name;
		const char* path = package->path;

		for (j = 0; j < package->numConfigs; ++j)
		{
			char buffer[256];
			ProjectConfig* prjCfg = project->config[j];
			Config* config = package->config[j];
			const char* target = config->target;
			
			strcpy(buffer, "lib");                       // posix shared lib
			strcat(buffer, target);
			deleteFile(prjCfg->bindir, buffer, ".so");

			deleteFile(prjCfg->bindir, target, "");      // posix executable
			deleteFile(prjCfg->bindir, target, ".exe");  // windows executable
			deleteFile(prjCfg->bindir, target, ".dll");  // windows or .NET shared lib
			deleteFile(prjCfg->bindir, target, ".pdb");  // VS symbol file
			deleteFile(prjCfg->bindir, target, ".ilk");  // VS incremental link
			deleteFile(prjCfg->libdir, target, ".pdb");  // VS symbol file
			deleteFile(prjCfg->libdir, target, ".exp");  // VS export lib
			deleteFile(prjCfg->libdir, target, ".lib");  // windows static lib
		}
		
		deleteFile(path, name, ".csproj");          // VS200x
		deleteFile(path, name, ".csproj.user");     // VS200x
		deleteFile(path, name, ".csproj.webinfo");  // VS200x
		deleteFile(path, name, ".vcproj");          // VS200x
		deleteFile(path, name, ".dsp");             // VS6
		deleteFile(path, name, ".plg");             // VS6
		deleteFile(path, name, ".make");            // GNU
		deleteFile(path, "Makefile", "");           // GNU
		deleteFile(path, name, ".prjx");            // SharpDevelop/MonoDevelop
		deleteFile(path, name, ".cmbx");            // MonoDevelop
		deleteFile(path, "Makefile.", name);        // MonoDevelop
		deleteFile(path, name, ".pidb");            // MonoDevelop
		
		deleteDirectory("", makeAbsolute(package->path, package->objdir));
	}
	
	return 1;
}
