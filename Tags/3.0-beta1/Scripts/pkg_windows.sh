#!/bin/sh

# Make sure a build number is supplied
if [ $# -ne 1 ]; then
  echo 1>&2 "Usage: $0 version"
  exit 1
fi

echo "WINDOWS BUILD $1"
echo ""
echo "Have you updated the version number in premake.c?"
echo ""
echo "Ready to build Windows DLL for version $1."
echo "Press [Enter] to begin."
read line

echo "Build release version..."
cd ..
premake --clean --target vs2003
"c:/Program Files/Microsoft Visual Studio .NET 2003/Common7/IDE/devenv.exe" /rebuild Release Premake.sln
if [ $? -ne 0 ]; then
  echo ""
  echo "** BUILD FAILED! **"
  exit 1
fi

echo "Packaging..."
cd bin
zip -j9 ../Scripts/premake-win32-$1.zip premake.exe

echo "Cleaning up..."
cd ..
"c:/Program Files/Microsoft Visual Studio .NET 2003/Common7/IDE/devenv.exe" /clean Premake.sln
cd Scripts

echo ""
echo "Upload package to SourceForge?"
read line
if [ $line = "y" ]; then
  echo "Uploading to SourceForge..."
  ftp -n upload.sourceforge.net < pkg_windows_ftp.txt
fi

echo ""
echo "Done."

