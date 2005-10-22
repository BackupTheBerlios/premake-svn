//-----------------------------------------------------------------------------
// Premake - platform.h
//
// A simple platform abstraction API.
//
// Written by Jason Perkins (jason@379.com)
// Copyright (C) 2002 by 379, Inc.
// Source code licensed under the GPL, see LICENSE.txt for details.
//
// $Id: platform.h,v 1.3 2005/09/22 21:03:13 jason379 Exp $
//-----------------------------------------------------------------------------

/* Identify the platform. I'm not sure how to definitively detect the Windows
 * platform, but since that is the most common I use it as a default */

#if defined(__linux__)
	#define PLATFORM_POSIX
	#define OS_IDENT   "linux"
#elif defined(__FreeBSD__)
	#define PLATFORM_POSIX
	#define OS_IDENT   "freebsd"
#elif defined(__NetBSD__)
	#define PLATFORM_POSIX
	#define OS_IDENT   "netbsd"
#elif defined(__OpenBSD__)
	#define PLATFORM_POSIX
	#define OS_IDENT   "openbsd"
#elif defined(__APPLE__) && defined(__MACH__)
	#define PLATFORM_POSIX
	#define OS_IDENT   "macosx"
#else
	#define PLATFORM_WINDOWS
	#define OS_IDENT   "windows"
#endif

extern char nativePathSep;

extern int         plat_chdir(const char* path);
extern int         plat_copyFile(const char* src, const char* dest);
extern void        plat_deleteDir(const char* path);
extern void        plat_deleteFile(const char* path);
extern int         plat_findLib(const char* lib, char* buffer, int size);
extern void        plat_generateUUID(char* uuid);
extern void        plat_getcwd(char* buffer, int size);
extern int         plat_isAbsPath(const char* path);
extern int         plat_mkdir(const char* path);

extern int         plat_dirOpen(const char* mask);
extern void        plat_dirClose();
extern int         plat_dirGetNext();
extern const char* plat_dirGetName();
extern int         plat_dirIsFile();


