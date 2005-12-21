package.name = "Premake.Tests"
package.kind = "dll"
package.language = "c#"

-- Adding this for GNU generator on Windows; how does VS.NET find it?

if (windows) then
	package.libpaths = { "C:/Program Files/NUnit 2.2/bin" }
end

package.links =
{
	"System",
	"nunit.framework"
}

package.files =
{
	matchfiles("*.cs", "Framework/*.cs"),
	matchfiles("Gnu/*.cs", "Gnu/Cpp/*.cs", "Gnu/Cs/*.cs"),
	matchfiles("MonoDev/*.cs", "MonoDev/Cs/*.cs"),
	matchfiles("SharpDev/*.cs", "SharpDev/Cs/*.cs"),
	matchfiles("Vs6/*.cs", "Vs6/Cpp/*.cs"),
	matchfiles("Vs2002/*.cs", "Vs2002/Cpp/*.cs", "Vs2002/Cs/*.cs"),
	matchfiles("Vs2003/*.cs", "Vs2003/Cpp/*.cs", "Vs2003/Cs/*.cs")
}
