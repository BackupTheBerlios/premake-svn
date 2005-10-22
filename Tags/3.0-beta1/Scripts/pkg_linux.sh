#!/bin/sh

# Make sure a build number is supplied
if [ $# -ne 1 ]; then
  echo 1>&2 "Usage: $0 version"
  exit 1
fi

echo "LINUX BUILD $1"
echo ""
echo "Have you updated the version number in premake.c?"
echo ""
echo "Ready to build Linux DLL for version $1."
echo "Press [Enter] to begin."
read line

echo "Build release version..."
cd ..
premake --clean --target gnu
make CONFIG=Release

if [ $? -ne 0 ]; then
  echo ""
  echo "** BUILD FAILED! **"
  exit 1
fi

echo "Packaging..."
cd bin
tar czvf ../Scripts/premake-linux-$1.tar.gz premake

echo "Cleaning up..."
cd ..
make clean
cd Scripts

echo ""
echo "Upload package to SourceForge?"
read line
if [ $line = "y" ]; then
  echo "Uploading to SourceForge..."
  ftp -n upload.sourceforge.net < pkg_linux_ftp.txt
fi

echo ""
echo "Done."

