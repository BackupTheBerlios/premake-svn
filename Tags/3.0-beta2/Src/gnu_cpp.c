/**********************************************************************
 * Premake - gnu_cpp.c
 * The GNU C/C++ makefile target
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
#include "gnu.h"
#include "os.h"

static char buffer[8192];

static const char* filterLinks(const char* name);
static const char* listCppSources(const char* name);
static const char* listCppTargets(const char* name);
static const char* listLinkerDeps(const char* name);


int gnu_cpp()
{
	int i;

	const char* prefix = (g_verbose) ? "" : "@";

	/* Open package makefile and write the header */
	if (gnu_pkgOwnsPath())
		strcpy(buffer, path_join(prj_get_pkgpath(), "Makefile", ""));
	else
		strcpy(buffer, path_join(prj_get_pkgpath(), prj_get_pkgname(), DOT_MAKE));
	io_openfile(buffer);

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

		/* Write C flags */
		io_print("  CFLAGS += $(CPPFLAGS)");
		if (prj_is_kind("dll") && !os_is("windows"))
			io_print(" -fPIC");
		if (!prj_has_flag("no-symbols"))
			io_print(" -g");
		if (prj_has_flag("optimize-size"))
			io_print(" -Os");
		if (prj_has_flag("optimize-speed"))
			io_print(" -O3");
		if (prj_has_flag("optimize") && !prj_has_flag("optimize-size") && !prj_has_flag("optimize-speed"))
			io_print(" -O2");
		if (prj_has_flag("extra-warnings"))
			io_print(" -Wall");
		if (prj_has_flag("fatal-warnings"))
			io_print(" -Werror");
		if (prj_has_flag("no-frame-pointer"))
			io_print(" -fomit-frame-pointer");
		print_list(prj_get_buildoptions(), " ", "", "", NULL);
		io_print("\n");

		/* Write C++ flags */
		io_print("  CXXFLAGS = $(CFLAGS)");
		if (prj_has_flag("no-exceptions"))
			io_print(" --no-exceptions");
		if (prj_has_flag("no-rtti"))
			io_print(" --no-rtti");
		io_print("\n");

		/* Write linker flags */
		io_print("  LDFLAGS += -L$(BINDIR) -L$(LIBDIR)");
		if (prj_is_kind("dll") && (g_cc == NULL || matches(g_cc, "gcc")))
			io_print(" -shared");
		if (prj_has_flag("no-symbols"))
			io_print(" -s");
		if (os_is("macosx") && prj_has_flag("dylib"))
			io_print(" -dynamiclib -flat_namespace");
		print_list(prj_get_linkoptions(), " ", "", "", NULL);
		print_list(prj_get_libpaths(), " -L \"", "\"", "", NULL);
		print_list(prj_get_links(), " ", "", "", filterLinks);
		io_print("\n");

		/* Build a list of libraries this target depends on */
		io_print("  LDDEPS =");
		print_list(prj_get_links(), " ", "", "", listLinkerDeps);
		io_print("\n");

		/* Build the target name */
		io_print("  TARGET = %s\n", path_getname(prj_get_target()));
		if (os_is("macosx") && prj_is_kind("winexe"))
			io_print("  MACAPP = %s.app/Contents\n", path_getname(prj_get_target()));

		io_print("endif\n\n");
	}

	/* Write out the list of object file targets for all C/C++ sources */
	io_print("OBJECTS = \\\n");
	print_list(prj_get_files(), "\t$(OBJDIR)/", " \\\n", "", listCppSources);
	io_print("\n");

	io_print(".PHONY: clean\n");
	io_print("\n");

	/* Write the main build target */
	if (os_is("macosx") && prj_is_kind("winexe"))
	{
		io_print("all: $(OUTDIR)/$(MACAPP)/PkgInfo $(OUTDIR)/$(MACAPP)/Info.plist $(OUTDIR)/$(MACAPP)/MacOS/$(TARGET)\n\n");
		io_print("$(OUTDIR)/$(MACAPP)/MacOS/$(TARGET)");
	}
	else
	{
		io_print("$(OUTDIR)/$(TARGET)");
	}

	io_print(": $(OBJECTS) $(LDDEPS)\n");
	if (!g_verbose)
		io_print("\t@echo Linking %s\n", prj_get_pkgname());
	io_print("\t-%sif [ ! -d $(BINDIR) ]; then mkdir -p $(BINDIR); fi\n", prefix);
	io_print("\t-%sif [ ! -d $(LIBDIR) ]; then mkdir -p $(LIBDIR); fi\n", prefix);
	io_print("\t-%sif [ ! -d $(OUTDIR) ]; then mkdir -p $(OUTDIR); fi\n", prefix);
	if (os_is("macosx") && prj_is_kind("winexe"))
		io_print("\t-%sif [ ! -d $(OUTDIR)/$(MACAPP)/MacOS ]; then mkdir -p $(OUTDIR)/$(MACAPP)/MacOS; fi\n", prefix);

	if (prj_is_kind("lib"))
	{
		io_print("\t%sar -cr $@ $^\n", prefix);
		io_print("\t%sranlib $@\n", prefix);
	}
	else
	{
		io_print("\t%s$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)\n", prefix);
	}
	io_print("\n");

	if (os_is("macosx") && prj_is_kind("winexe"))
	{
		io_print("$(OUTDIR)/$(MACAPP)/PkgInfo:\n\n");
		io_print("$(OUTDIR)/$(MACAPP)/Info.plist:\n\n");
	}

	/* Write the "clean" target */
	io_print("clean:\n");
	io_print("\t@echo Cleaning %s\n", prj_get_pkgname());
	if (os_is("macosx") && prj_is_kind("winexe"))
	{
		io_print("\t-%srm -rf $(OUTDIR)/$(TARGET).app $(OBJDIR)\n", prefix);
	}
	else
	{
		io_print("\t-%srm -rf $(OUTDIR)/$(TARGET) $(OBJDIR)\n", prefix);
	}
	io_print("\n");

	/* Write static patterns for each source file. Note that in earlier
	 * versions I used pattern rules instead of listing each file. It worked
	 * fine but made it more difficult to test and also required the use of
	 * VPATH which I didn't like. This new approach of listing each file
	 * helps testing and opens the way for per-file configurations */
	print_list(prj_get_files(), "", "\n", "", listCppTargets);

	/* Include the automatically generated dependency lists */
	io_print("-include $(OBJECTS:%%.o=%%.d)\n\n");

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
		if (matches(lang, "c++") || matches(lang, "c"))
		{
			return prj_get_target_for(i);
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		strcpy(buffer, "-l");
		strcat(buffer, name);
		return buffer;
	}
}


/************************************************************************
 * Checks each source code file and filters out everything that is
 * not a C or C++ file
 ***********************************************************************/

static const char* listCppSources(const char* name)
{
	if (is_cpp(name))
	{
		strcpy(buffer, path_getbasename(name));
		strcat(buffer, ".o");
		return buffer;
	}

	return NULL;
}


/************************************************************************
 * Creates the makefile build rules for all source code files
 ***********************************************************************/

static const char* listCppTargets(const char* name)
{
	if (is_cpp(name))
	{
		const char* ext = path_getextension(name);

		sprintf(buffer, "$(OBJDIR)/%s.o: %s\n", path_getbasename(name), name);
		strcat(buffer, "\t-");
		if (!g_verbose)
			strcat(buffer, "@");
		strcat(buffer, "if [ ! -d $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi\n");

		if (!g_verbose)
			strcat(buffer, "\t@echo $(notdir $<)\n");
      
		strcat(buffer, "\t");
		if (!g_verbose)
			strcat(buffer, "@");
		if (matches(g_cc, "dmc"))
		{
			/* Digital Mars compiler build step */
			/* FIXME: How to handle assembly files with DMC? */
			if (matches(ext, ".c"))
				strcat(buffer, "dmc $(CFLAGS) -o $@ -c $<\n");
			else if (!matches(ext, ".s"))
				strcat(buffer, "dmc -cpp -Ae -Ar -mn $(CXXFLAGS) -o $@ -c $<\n");
		}
		else
		{
			/* GNU GCC compiler build step */
			if (matches(ext, ".s"))
				strcat(buffer, "$(CC) -x assembler-with-cpp $(CPPFLAGS) -o $@ -c $<\n");
			else if (matches(ext, ".c"))
				strcat(buffer, "$(CC) $(CFLAGS) -o $@ -c $<\n");
			else
				strcat(buffer, "$(CXX) $(CXXFLAGS) -o $@ -c $<\n");
		}

		return buffer;
	}
	else
	{
		return NULL;
	}
}

/************************************************************************
 * This is called by the code that builds the list of dependencies for 
 * the link step. It looks for sibling projects, and then returns the 
 * full path to that target's output. So if an executable package 
 * depends on a library package, the library filename will be listed 
 * as a dependency
 ***********************************************************************/

static const char* listLinkerDeps(const char* name)
{
	int i = prj_find_package(name);
	if (i >= 0)
	{
		return prj_get_target_for(i);
	}

	return NULL;
}
