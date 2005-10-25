//-----------------------------------------------------------------------------
// Premake - util.h
//
// Premake utility functions
//
// Copyright (C) 2002-2003 by Jason Perkins
// Source code licensed under the GPL, see LICENSE.txt for details.
//
// $Id: util.h,v 1.20 2005/09/27 20:10:32 jason379 Exp $
//-----------------------------------------------------------------------------

#include "platform.h"

static char* CPP_EXT[] = { ".cc", ".cpp", ".cxx", ".c", ".s", NULL };

enum { UNIX, WINDOWS, NATIVE };

enum { WST_OPENGROUP, WST_CLOSEGROUP, WST_SOURCEFILE };

extern char** args;

extern int   copyFile(const char* src, const char* dest);
extern void  deleteDirectory(const char* path, const char* name);
extern void  deleteFile(const char* path, const char* name, const char* extension);
extern int   endsWith(const char* haystack, const char* needle);
extern int   fileExists(const char* path, const char* name, const char* extension);
extern const char* findSharedLib(const char* lib);
extern void  generateUUID(char* uuid);
extern const char* getDirectory(const char* path, int trailingSlash);
extern const char* getExtension(const char* path);
extern const char* getFilename(const char* path, int trim);
extern int   hasOption(char* option);
extern int   inArray(const char** list, const char* value);
extern void  insertPath(FILE* file, const char* path, int type, int leader);
extern int   isCppFile(const char* file);
extern const char* makeAbsolute(const char* base, const char* path);
extern int   matches(const char* str0, const char* str1);
extern FILE* openFile(const char* path, const char* name, const char* extension);
extern const char* replaceChars(const char* str, const char* replace);
extern const char* replaceExtension(const char* path, const char* extension);
extern const char* reversePath(const char* path, const char* subdir, int type, int trailer);
extern const char* translatePath(const char* buffer, int type);
extern void  walkSourceList(FILE* file, Package* package, const char* path, void (*cb)(FILE*, const char*, int));
extern void  writeList(FILE* file, const char** list, const char* prefix, const char* postfix, const char* infix, const char* (*check)(const char*,void*), void* data);
extern const char* getCwd();
extern int   setCwd(const char* path);

extern int   dirOpen(const char* base, const char* mask);
extern void  dirClose();
extern int   dirGetNext();
extern const char* dirGetName();
extern int   dirIsFile();