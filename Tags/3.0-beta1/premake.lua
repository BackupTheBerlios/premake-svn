project.name     = "Premake"
project.bindir   = "bin"

addoption("with-tests", "Include the unit tests (requires NUnit)")

dopackage("Src")
if (options["with-tests"]) then
	dopackage("Tests")
end

function docommand(cmd, ...)
  %docommand(cmd, arg)
  if (cmd == "clean") then
		rmdir("bin")
	end
end
