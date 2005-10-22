package.language = "c"
package.kind     = "exe"
package.target   = "premake"

package.buildflags = { "no-64bit-checks", "static-runtime" }
package.config["Release"].buildflags = { "no-symbols", "optimize-size" }

package.files =
{
	matchfiles("*.h", "*.c"),
	matchfiles("Lua/*.c", "Lua/*.h")
}
