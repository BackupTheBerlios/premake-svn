/**********************************************************************
 * Premake - vs2002.c
 * The Visual Studio 2002 and 2003 target
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
#include "vs.h"
#include "vs2002.h"


int vs2002_generate(int target)
{
	int i;

	vs_setversion(target == 2002 ? VS2002 : VS2003);

	printf("Generating Visual Studio %d solution and project files:\n", vs_getversion());

	/* Assign GUIDs to packages */
	vs_assign_guids();

	/* Generate the project files */
	for (i = 0; i < prj_get_numpackages(); ++i)
	{
		prj_select_package(i);

		printf("...%s\n", prj_get_pkgname());

		if (prj_is_lang("c++") || prj_is_lang("c"))
		{
			if (!vs_write_cpp())
				return 0;
		}
		else if (prj_is_lang("c#"))
		{
			if (!vs2002_cs())
				return 0;
		}
		else
		{
			printf("** Warning: %s packages are not supported by this generator\n", prj_get_language());
		}
	}

	return vs_write_solution();
}

