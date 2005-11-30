#!/bin/sh

script_dir=`pwd`

# Make sure a build number is supplied
if [ $# -ne 1 ]; then
  echo 1>&2 "Usage: $0 version"
  exit 1
fi

echo "WINDOWS BUILD $1"
echo ""

# Make sure all prerequisites are met
echo "Have you updated the version number in premake.c?"
read line
echo ""
echo "Did you create a tag for version $1?"
read line
echo ""
echo "Ready to build Linux executable for version $1."
echo "Press [Enter] to begin."
read line


#####################################################################
# Stage 1: Preparation
#
# Pull the source code from Subversion and update the embedded
# version numbers.
#####################################################################

echo ""
echo "RETRIEVING SOURCE CODE FROM REPOSITORY..."
echo ""
cd ../..
svn co https://svn.berlios.de/svnroot/repos/premake/Tags/$1 Premake-$1


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
	ftp -n upload.sourceforge.net < pkg_linux_ftp.txt
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
echo "Done."
