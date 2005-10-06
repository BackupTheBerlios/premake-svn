//-----------------------------------------------------------------------------
// Premake - vs_helpers.h
//
// Support functions for the different versions of Visual Studio.
//
// Copyright (C) 2002-2005 by Jason Perkins
// Source code licensed under the GPL, see LICENSE.txt for details.
//
// $Id: vs_helpers.h,v 1.1 2005/08/22 01:43:41 jason379 Exp $
//-----------------------------------------------------------------------------

typedef struct _PkgData
{
	char projGuid[38];
	char toolGuid[38];
	char projExt[8];
	char projType[8];
	int  numDependencies;
} PkgData;

void        vs_AssignPackageData();
const char* vs_FindProjectDependencies(const char* name, void* data);
void        vs_SetTargetVersion(int version);
void        vs_WriteProjectList(FILE* file);
