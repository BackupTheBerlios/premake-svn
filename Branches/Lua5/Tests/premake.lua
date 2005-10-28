package.name = "Premake.Tests"
package.kind = "dll"
package.language = "c#"

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
