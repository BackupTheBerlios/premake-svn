/**********************************************************************
 * Premake - vs2005.c
 * The Visual Studio 2005 target
 *
 * Copyright (c) 2002-2065 Jason Perkins and the Premake project
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
#include "vs.h"
#include "vs2005.h"

static int writeSolution();


int vs2005_generate(int target)
{
	int p;

	vs_setversion(VS2005);
	printf("Generating Visual Studio 2005 solution and project files:\n");

	/* Assign GUIDs to packages */
	vs_assign_guids();

	/* Generate the project files */
	for (p = 0; p < prj_get_numpackages(); ++p)
	{
		prj_select_package(p);

		printf("...%s\n", prj_get_pkgname());

		if (prj_is_lang("c++") || prj_is_lang("c"))
		{
			if (!vs_write_cpp())
				return 0;
		}
		else
		{
			printf("** Warning: %s packages are not supported by this generator\n", prj_get_language());
		}
	}

	return vs_write_solution();
}
