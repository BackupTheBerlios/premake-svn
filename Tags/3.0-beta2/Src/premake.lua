package.name     = "Premake"
package.language = "c"
package.kind     = "exe"
package.target   = "premake"

-- Build Flags

	package.buildflags = 
	{ 
		"no-64bit-checks",
		"static-runtime" 
	}

	package.config["Release"].buildflags = 
	{ 
		"no-symbols", 
		"optimize-size",
		"no-frame-pointers"
	}


-- Defined Symbols

	if (OS == "windows") then
		package.defines = { "_WIN32" }
	end
	
		
-- Files

	package.files =
	{
		matchfiles("*.h", "*.c"),
		matchfiles("Lua/*.h", "Lua/*.c")
	}
