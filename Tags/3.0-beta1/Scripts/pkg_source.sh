#!/bin/bash

# Make sure build number is supplied
if [ $# -ne 1 ]; then
	echo 1>&2 Usage: $0 build_number
	exit 1
fi

echo "PREPARING BUILD $1"
echo ""
echo "Did you update README.txt?"
read line
echo "Did you update CHANGES.txt?"
read line
echo "Did you update premake.c?"
read line
echo ""
echo "Ready to build source package for version $1"
echo "Press [Enter] to begin."
read line

echo "Versioning directory..."
cd ../..
cp -r Premake Premake-$1

echo "Removing CVS directories..."
cd Premake-$1
rm -rf `find . -name CVS`

premake --clean
premake --target gnu
premake --target vs2002

echo "Removing support files..."
rm -rf Scripts
rm -rf Tests

echo "Building package..."
cd ..
zip -ll -9 -r Premake/Scripts/premake-src-$1.zip Premake-$1/*

echo "Cleaning up..."
rm -rf Premake-$1
cd Premake/Scripts

echo ""
echo "Upload package to SourceForge?"
read line
if [ $line = "y" ]; then
	echo "Uploading to SourceForge..."
	ftp -n upload.sourceforge.net < pkg_source_ftp.txt
fi

echo "Done."
exit 0
