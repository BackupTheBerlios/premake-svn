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
	matchfiles("Framework/*.cs"),
	matchfiles("Gnu/Cpp/*.cs"),
	matchfiles("Gnu/Cs/*.cs"),
	matchfiles("SharpDev/Cs/*.cs"),
	matchfiles("Vs2003/Cpp/*.cs"),
	matchfiles("Vs2003/Cs/*.cs")
}
