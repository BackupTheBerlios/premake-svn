/**********************************************************************
 * Premake - clean.c
 * The cleanup target.
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
#include <string.h>
#include "premake.h"

static char buffer[8192];


int clean()
{
	int i, j;

	puts("Removing all project and intermediate files...");

	/* VS.NET 200x */
	io_remove(path_join(prj_get_path(), prj_get_name(), "sln"));
	io_remove(path_join(prj_get_path(), prj_get_name(), "suo"));

	/* VS6 */
	io_remove(path_join(prj_get_path(), prj_get_name(), "ncb"));
	io_remove(path_join(prj_get_path(), prj_get_name(), "dsw"));
	io_remove(path_join(prj_get_path(), prj_get_name(), "opt"));

	/* GNU */
	io_remove(path_join(prj_get_path(), "Makefile", ""));

	/* SharpDevelop */
	io_remove(path_join(prj_get_path(), prj_get_name(), "cmbx"));

	/* MonoDevelop */
	io_remove(path_join(prj_get_path(), prj_get_name(), "mdsx"));
	io_remove(path_join(prj_get_path(), "make", "sh"));

	for (i = 0; i < prj_get_numpackages(); ++i)
	{
		prj_select_package(i);

		for (j = 0; j < prj_get_numconfigs(); ++j)
		{
			prj_select_config(j);

			/* POSIX shared lib */
			strcpy(buffer, prj_get_prefix() != NULL ? prj_get_prefix() : "lib");
			strcat(buffer, path_getbasename(prj_get_target()));
			strcat(buffer, ".");
			strcat(buffer, prj_get_extension() != NULL ? prj_get_extension() : "so");
			io_remove(path_join(prj_get_outdir(), buffer, ""));

			/* POSIX executable */
			strcpy(buffer, prj_get_prefix() != NULL ? prj_get_prefix() : "");
			strcat(buffer, path_getbasename(prj_get_target()));
			io_remove(path_join(prj_get_outdir(), buffer, ""));

			/* Windows executable */
			io_remove(path_join(prj_get_outdir(), buffer, "exe"));

			/* .NET assembly manifest */
			io_remove(path_join(prj_get_outdir(), buffer, "exe.manifest"));

			/* DLL or assembly */
			io_remove(path_join(prj_get_outdir(), buffer, "dll"));

			/* Windows static library */
			io_remove(path_join(prj_get_outdir(), buffer, "lib"));

			/* Visual Studio symbol file */
			io_remove(path_join(prj_get_outdir(), buffer, "pdb"));

			/* Visual Studio incremental link file */
			io_remove(path_join(prj_get_outdir(), buffer, "ilk"));

			/* Windows DLL exports library */
			io_remove(path_join(prj_get_libdir(), buffer, "lib"));
			io_remove(path_join(prj_get_libdir(), buffer, "exp"));

			/* All */
			io_rmdir(prj_get_pkgpath(), prj_get_objdir());
		}

		/* VS.NET 200x */
		io_remove(path_join(prj_get_pkgpath(), prj_get_pkgname(), "csproj"));
		io_remove(path_join(prj_get_pkgpath(), prj_get_pkgname(), "csproj.user"));
		io_remove(path_join(prj_get_pkgpath(), prj_get_pkgname(), "csproj.webinfo"));
		io_remove(path_join(prj_get_pkgpath(), prj_get_pkgname(), "vcproj"));

		/* VS6 */
		io_remove(path_join(prj_get_pkgpath(), prj_get_pkgname(), "dsp"));
		io_remove(path_join(prj_get_pkgpath(), prj_get_pkgname(), "plg"));

		/* GNU */
		io_remove(path_join(prj_get_pkgpath(), "Makefile", ""));
		io_remove(path_join(prj_get_pkgpath(), prj_get_pkgname(), "mak"));

		/* SharpDevelop */
		io_remove(path_join(prj_get_pkgpath(), prj_get_pkgname(), "prjx"));

		/* MonoDevelop */
		io_remove(path_join(prj_get_pkgpath(), prj_get_pkgname(), "cmbx"));
		io_remove(path_join(prj_get_pkgpath(), "Makefile", prj_get_pkgname()));
		io_remove(path_join(prj_get_pkgpath(), prj_get_pkgname(), "pidb"));
	}

	return 1;
}
