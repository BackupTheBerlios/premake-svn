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

	if (OS == "linux") then
		package.links = { "m" }
	end


-- Files

	package.files =
	{
		matchrecursive("*.h", "*.c")
	}
