/**********************************************************************
 * Premake - gnu_cpp.c
 * The GNU C/C++ makefile target
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
#include "gnu.h"


int gnu_cpp()
{
	int i;

	/* Open package makefile and write the header */
	if (gnu_pkgOwnsPath())
		io_openfile(path_join(prj_get_pkgpath(), "Makefile", ""));
	else
		io_openfile(path_join(prj_get_pkgpath(), prj_get_pkgname(), DOT_MAKE));

	io_print("# %s ", prj_is_lang("c++") ? "C++" : "C");

	if (prj_is_kind("exe"))
		io_print("Console Executable");
	else if (prj_is_kind("winexe"))
		io_print("Windowed Executable");
	else if (prj_is_kind("dll"))
		io_print("Shared Library");
	else if (prj_is_kind("lib"))
		io_print("Static Library");

	io_print(" Makefile autogenerated by premake\n");
	io_print("# Don't edit this file! Instead edit `premake.lua` then rerun `make`\n\n");

	/* Set a default configuration */
	prj_select_config(0);
	io_print("ifndef CONFIG\n");
	io_print("  CONFIG=%s\n", prj_get_cfgname());
	io_print("endif\n\n");

	/* Process the build configurations */
	for (i = 0; i < prj_get_numconfigs(); ++i)
	{
		prj_select_config(i);

		io_print("ifeq ($(CONFIG),%s)\n", prj_get_cfgname());

		io_print("  BINDIR = %s\n", prj_get_bindir());
		io_print("  LIBDIR = %s\n", prj_get_libdir());
		io_print("  OBJDIR = %s\n", prj_get_objdir());
		io_print("  OUTDIR = %s\n", prj_get_outdir());

		/* Write preprocessor flags - how to generate dependencies for DMC? */
		io_print("  CPPFLAGS =");
		if (!matches(g_cc, "dmc"))
			io_print(" -MD");
		print_list(prj_get_defines(), " -D \"", "\"", "", NULL);
		print_list(prj_get_incpaths(), " -I \"", "\"", "", NULL);
		io_print("\n");
	}

	io_closefile();
	return 1;
}
