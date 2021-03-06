#!/bin/sh

script_dir=`pwd`

# Make sure a build number is supplied
if [ $# -ne 1 ]; then
  echo 1>&2 "Usage: $0 version"
  exit 1
fi

echo "POSIX BUILD $1"
echo ""

# Make sure all prerequisites are met
echo "Did you test against EVERY project?"
read line
echo ""
echo "Did you create a release branch?"
read line
echo ""
echo "Have you updated the version number in premake.c?"
read line
echo ""
echo "Did you update README.txt?"
read line
echo ""
echo "Did you update CHANGES.txt?"
read line
echo ""
echo "Ready to build POSIX executable for version $1."
echo "Press [Enter] to begin."
read line


#####################################################################
# Stage 1: Preparation
#
# Pull the source code from Subversion.
#####################################################################

echo ""
echo "RETRIEVING SOURCE CODE FROM REPOSITORY..."
echo ""
cd ../..
svn co https://svn.berlios.de/svnroot/repos/premake/Branches/$1 Premake-$1


#####################################################################
# Stage 2: Binary Package
#####################################################################

echo ""
echo "BUILDING RELEASE BINARY..."
echo ""

cd Premake-$1
premake --with-tests --target gnu
make CONFIG=Release


#####################################################################
# Stage 3: Unit Test
#
# I haven't gotten the unit tests to run on Linux yet
#####################################################################

# echo ""
# echo "RUNNING UNIT TESTS..."
# echo ""

# nunit-console.exe Premake.Tests.nunit

# echo "Did the unit tests run successfully?"
# read line
# echo ""


#####################################################################
# Stage 4: Pack Release
#####################################################################

cd bin
tar czvf $script_dir/premake-linux-$1.tar.gz premake


#####################################################################
# Stage 5: Publish Files
#
# Send the files to SourceForge
#####################################################################

cd $script_dir
echo ""
echo "Upload packages to SourceForge?"
read line
if [ $line = "y" ]; then
	echo "Uploading to SourceForge..."
	ftp -n upload.sourceforge.net < ftp_x11.txt
fi


#####################################################################
# All done
#####################################################################

echo ""
echo "CLEANING UP..."
echo ""
cd ../..
rm -rf Premake-$1

cd $script_dir
echo ""
echo "Done - NOW CREATE A TAG"

