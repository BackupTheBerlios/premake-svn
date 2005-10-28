using System;
using System.Collections;
using System.IO;
using Premake.Tests.Framework;

namespace Premake.Tests.Gnu
{
	public class GnuParser : Parser
	{
		#region Parser Methods
		public override string TargetName
		{
			get { return "gnu"; }
		}
		#endregion

		#region Root Makefile Parsing
		public override void Parse(Project project, string filename)
		{
			string root = Path.GetDirectoryName(filename);
			if (root != String.Empty)
				root += Path.DirectorySeparatorChar;
			Begin(root + "Makefile");

			/* File header */
			Match("# Makefile autogenerated by premake");
			Match("# Don't edit this file! Instead edit `premake.lua` then rerun `make`");
			Match("# Options:");

			/* Project configurations */
			string[] matches = Regex("#   CONFIG=\\[(.+)\\]");
			matches = matches[0].Split('|');
			foreach (string config in matches)
				project.Configuration.Add(config);

			Match("");
			Match("ifndef CONFIG");
			Match("  CONFIG=" + project.Configuration[0]);
			Match("endif");
			Match("");
			Match("export CONFIG");
			Match("");

			/* Package list */
			matches = Regex(".PHONY: all clean (.+)");
			matches = matches[0].Split(' ');
			foreach (string name in matches)
			{
				Package package = new Package(project);
				project.Package.Add(package);
				package.Name = name;
			}

			Match("");
			Match("all: " + String.Join(" ", matches));
			Match("");

			/* TO DO: need to figure out way to test this part */
			Regex("Makefile: (.+)");
			Match("\t@echo ==== Regenerating Makefiles ====");
			Regex("\t@premake (.+)");
			Match("");

			foreach (Package package in project.Package)
			{
				matches = Regex(package.Name + ":(.*)");
				foreach (Configuration config in package.Config)
				{
					config.Dependencies = matches[0].Trim().Split(' ');
				}

				Match("\t@echo ==== Building " + package.Name + " ====");
				
				/* Pull out the package path */
				matches = Regex("\t@\\$\\(MAKE\\) --no-print-directory -C (.+)");
				int split = matches[0].IndexOf(" -f ");
				if (split > 0)
				{
					package.Path = matches[0].Substring(0, split);
					package.ScriptName = matches[0].Substring(split + 4);
				}
				else
				{
					package.Path = matches[0];
					package.ScriptName = "Makefile";
				}

				Match("");
			}

			Match("clean:");
			foreach (Package package in project.Package)
			{
				Regex("\t@\\$\\(MAKE\\) --no-print-directory -C " + package.Path + "(.*)clean$");
			}

			/* Parse the individual packages */
			foreach (Package package in project.Package)
			{
				filename = Path.Combine(Path.Combine(project.Path, package.Path), package.ScriptName);
				ParsePackageHeader(project, package, filename);
			}
		}
		#endregion

		#region Parse Generic Package Header
		private void ParsePackageHeader(Project project, Package package, string filename)
		{
			Begin(filename);
			
			string[] matches = Regex("# (.+?) (.+?) Makefile autogenerated by premake");
			package.Language = matches[0].ToLower();
			switch (matches[1])
			{
			case "Console Executable":  package.Kind = "exe";    break;
			case "Windowed Executable": package.Kind = "winexe"; break;
			case "Shared Library":      package.Kind = "dll";    break;
			case "Static Library":      package.Kind = "lib";    break;
			case "ASP.NET":             package.Kind = "aspnet"; break;
			}

			Match("# Don't edit this file! Instead edit `premake.lua` then rerun `make`");
			Match("");
			Match("ifndef CONFIG");
			Match("  CONFIG=" + project.Configuration[0]);
			Match("endif");
			Match("");

			if (package.Language == "c++" || package.Language == "c")
				ParseCpp(project, package);
			else
				ParseManaged(project, package);
		}
		#endregion

		#region C++ Parsing
		private void ParseCpp(Project project, Package package)
		{
			bool isMac = false;
			string[] matches;
			foreach (Configuration config in package.Config)
			{
				ArrayList buildFlags = new ArrayList();

				Match("ifeq ($(CONFIG)," + config.Name + ")");

				matches = Regex("  BINDIR = (.+)");
				config.BinDir = matches[0];

				matches = Regex("  LIBDIR = (.+)");
				config.LibDir = matches[0];

				matches = Regex("  OBJDIR = (.+)");
				config.ObjDir = matches[0];

				matches = Regex("  OUTDIR = (.+)");
				config.OutDir = matches[0];

				matches = Regex("  CPPFLAGS = (.*)");

				System.Text.RegularExpressions.MatchCollection mc;
				mc = System.Text.RegularExpressions.Regex.Matches(matches[0], "-I \"(.+?)\"");
				config.IncludePaths = new string[mc.Count];
				for (int i = 0; i < mc.Count; ++i)
					config.IncludePaths[i] = mc[i].Groups[1].ToString();

				mc = System.Text.RegularExpressions.Regex.Matches(matches[0], "-D \"(.+?)\"");
				config.Defines = new string[mc.Count];
				for (int i = 0; i < mc.Count; ++i)
					config.Defines[i] = mc[i].Groups[1].ToString();

				matches = Regex("  CFLAGS \\+= \\$\\(CPPFLAGS\\)(.*)");
				if (matches != null)
				{
					bool symbols = false;
					config.BuildOptions = "";

					string[] flags = matches[0].Split(' ');
					foreach (string flag in flags)
					{
						switch (flag)
						{
						case "-fomit-frame-pointer":
							buildFlags.Add("no-frame-pointer");
							break;
						case "-g":
							symbols = true;
							break;
						case "-O2":
							buildFlags.Add("optimize");
							break;
						case "-O3":
							buildFlags.Add("optimize-speed");
							break;
						case "-Os":
							buildFlags.Add("optimize-size");
							break;
						case "-Wall":
							buildFlags.Add("extra-warnings");
							break;
						case "-Werror":
							buildFlags.Add("fatal-warnings");
							break;
						default:
							config.BuildOptions += flag + " ";
							break;
						}
					}

					if (!symbols) buildFlags.Add("no-symbols");
					config.BuildOptions = config.BuildOptions.Trim(' ');
				}

				matches = Regex("  CXXFLAGS = \\$\\(CFLAGS\\)(.*)");
				if (matches != null)
				{
					string[] flags = matches[0].Split(' ');
					foreach (string flag in flags)
					{
						switch (flag)
						{
						case "--no-exceptions":
							buildFlags.Add("no-exceptions");
							break;
						case "--no-rtti":
							buildFlags.Add("no-rtti");
							break;
						}
					}
				}

				matches = Regex("  LDFLAGS \\+= -L\\$\\(BINDIR\\) -L\\$\\(LIBDIR\\)(.*)");

				mc = System.Text.RegularExpressions.Regex.Matches(matches[0], "-L \"(.+?)\"");
				config.LibPaths = new string[mc.Count];
				for (int i = 0; i < mc.Count; ++i)
					config.LibPaths[i] = mc[i].Groups[1].ToString();

				mc = System.Text.RegularExpressions.Regex.Matches(matches[0], "-l([^ $]+)");
				config.Links =  new string[mc.Count];
				for (int i = 0; i < mc.Count; ++i)
					config.Links[i] = mc[i].Groups[1].ToString();

				foreach (string flag in matches[0].Split(' '))
				{
					switch (flag)
					{
					case "-dynamiclib":
						buildFlags.Add("dylib");
						break;
					}
				}

				/* Pull out the link options...this gets a bit hairy */
				int split = matches[0].IndexOf("-L");
				if (split < 0)
					split = matches[0].IndexOf("-l");
				if (split < 0)
					split = matches[0].Length;

				config.LinkOptions = matches[0].Substring(0, split);
				config.LinkOptions = config.LinkOptions.Trim(' ');
				if (config.LinkOptions.StartsWith("-s"))
				{
					config.LinkOptions = config.LinkOptions.Substring(2);
					config.LinkOptions = config.LinkOptions.Trim(' ');
				}

				matches = Regex("  LDDEPS =(.*)");
				config.LinkDeps = matches[0].Trim().Split(' ');

				matches = Regex("  TARGET = (.+)");
				config.Target = matches[0];

				isMac = Match("  MACAPP = " + Path.GetFileName(config.Target), true);

				Match("endif");
				Match("");

				config.BuildFlags = (string[])buildFlags.ToArray(typeof(string));
			}

			Match("OBJECTS = \\");
			do
			{
				matches = Regex("\t\\$\\(OBJDIR\\)/(.+?) \\\\", true);
			} while (matches != null);
			Match("");

			Match(".PHONY: clean");
			Match("");

			if (isMac)
			{
				Match("all: $(BINDIR)/$(MACAPP).app/Contents/PkgInfo $(BINDIR)/$(MACAPP).app/Contents/Info.plist $(BINDIR)/$(TARGET)");
				Match("");
			}

			Match("$(OUTDIR)/$(TARGET): $(OBJECTS) $(LDDEPS)");
			Match("\t@echo Linking " + package.Name);
			Match("\t-@if [ ! -d $(BINDIR) ]; then mkdir -p $(BINDIR); fi");
			Match("\t-@if [ ! -d $(LIBDIR) ]; then mkdir -p $(LIBDIR); fi");
			if (isMac)
			{
				Match("\t-@if [ ! -d $(BINDIR)/$(MACAPP).app/Contents/MacOS ]; then mkdir -p $(BINDIR)/$(MACAPP).app/Contents/MacOS; fi");
			}

			if (package.Kind == "lib")
			{
				Match("\t@ar -cr $@ $^");
				Match("\t@ranlib $@");
			}
			else
			{
				Match("\t@$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)");
			}
			Match("");

			if (isMac)
			{
				Match("$(BINDIR)/$(MACAPP).app/Contents/PkgInfo:");
				Match("");
				Match("$(BINDIR)/$(MACAPP).app/Contents/Info.plist:");
				Match("");
			}
			
			Match("clean:");
			Match("\t@echo Cleaning " + package.Name);
			if (isMac)
			{
				Match("\t-@rm -rf $(OUTDIR)/$(MACAPP).app $(OBJDIR)/*");
			}
			else
			{
				Match("\t-@rm -rf $(OUTDIR)/$(TARGET) $(OBJDIR)/*");
			}
			Match("");

			do
			{
				matches = Regex("^\\$\\(OBJDIR\\)/(.+)[.]o: (.+)", true);
				if (matches != null)
				{
					package.File.Add(matches[1]);

					Match("\t-@if [ ! -d $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi");
					Match("\t@echo $(notdir $<)");

					switch (Path.GetExtension(matches[1]))
					{
					case ".c":
						Match("\t@$(CC) $(CFLAGS) -o $@ -c $<");
						break;
					case ".s":
						Match("\t@$(CC) -x assembler-with-cpp $(CPPFLAGS) -o $@ -c $<");
						break;
					default:
						Match("\t@$(CXX) $(CXXFLAGS) -o $@ -c $<");
						break;
					}

					Match("");
				}
			} while (matches != null);

			Match("-include $(OBJECTS:%.o=%.d)");
		}
		#endregion

		#region Managed Code Parsing
		private void ParseManaged(Project project, Package package)
		{
			string[] matches = Regex("CSC = ([a-z]+)");
			package.Compiler = matches[0];

			matches = Regex("RESGEN = ([a-z]+)");
			bool ok = false;
			switch (package.Compiler)
			{
			case "csc":
			case "cscc":
				ok = (matches[0] == "resgen");
				break;
			case "mcs":
				ok = (matches[0] == "monoresgen");
				break;
			default:
				throw new NotImplementedException("Unrecognized compiler: " + package.Compiler);
			}
			if (!ok)
				throw new FormatException("Compiler/Resgen mismatch");
			Match("");

			matches = Regex("OBJDIR = (.+)");
			string objdir = matches[0];
			Match("");

			foreach (Configuration config in package.Config)
			{
				ArrayList buildFlags = new ArrayList();
				config.ObjDir = objdir;

				Match("ifeq ($(CONFIG)," + config.Name + ")");

				matches = Regex("  BINDIR = (.+)");
				config.BinDir = matches[0];

				matches = Regex("  OUTDIR = (.+)");
				config.OutDir = matches[0];

				matches = Regex("  FLAGS \\+=(.*)");

				bool symbols = false;
				config.BuildOptions = "";

				string[] flags = matches[0].Split(' ');
				foreach (string flag in flags)
				{
					switch (flag)
					{
					case "/warnaserror":
						buildFlags.Add("fatal-warnings");
						break;
					case "/o":
						buildFlags.Add("optimize");
						break;
					case "/unsafe":
						buildFlags.Add("unsafe");
						break;
					case "/debug":
						symbols = true;
						break;
					default:
						config.BuildOptions += flag + " ";
						break;
					}
				}

				if (!symbols) buildFlags.Add("no-symbols");
				config.BuildOptions = config.BuildOptions.Trim(' ');

				System.Text.RegularExpressions.MatchCollection mc;
				mc = System.Text.RegularExpressions.Regex.Matches(matches[0], "/d:([A-Za-z0-9_]+)");
				config.Defines = new string[mc.Count];
				for (int i = 0; i < mc.Count; ++i)
					config.Defines[i] = mc[i].Groups[1].ToString();

				mc = System.Text.RegularExpressions.Regex.Matches(matches[0], "/lib:\"(.+?)\"");
				config.LibPaths = new string[mc.Count];
				for (int i = 0; i < mc.Count; ++i)
					config.LibPaths[i] = mc[i].Groups[1].ToString();

				mc = System.Text.RegularExpressions.Regex.Matches(matches[0], "/r:([^ $]+)");
				config.Links = new string[mc.Count];
				for (int i = 0; i < mc.Count; ++i)
					config.Links[i] = mc[i].Groups[1].ToString();

				matches = Regex("  DEPS =(.*)");
				config.LinkDeps = matches[0].Trim().Split(' ');

				matches = Regex("  TARGET = (.+)");
				config.Target = Path.Combine(Path.GetDirectoryName(matches[0]), Path.GetFileNameWithoutExtension(matches[0]));

				Match("endif");
				Match("");

				config.BuildFlags = (string[])buildFlags.ToArray(typeof(string));
			}

			Match("SOURCES = \\");
			do
			{
				matches = Regex("\t(.+?)[ ]\\\\", true);
				if (matches != null)
					package.File.Add(matches[0], "Compile");
			} while (matches != null);
			Match("");

			Match("EMBEDDEDFILES = \\");
			do
			{
				matches = Regex("\t(.+?)[ ]\\\\", true);
				if (matches != null)
					package.File.Add(matches[0], "EmbeddedResource");
			} while (matches != null);
			Match("");

			Match("EMBEDDEDCOMMAND = \\");
			foreach (SourceFile file in package.File)
			{
				if (file.BuildAction == "EmbeddedResource")
					Match("\t/resource:" + file.Name + " \\");
			} while (matches != null);
			Match("");

			Match("LINKEDFILES = \\");
			do
			{
				matches = Regex("\t(.+?)[ ]\\\\", true);
				if (matches != null)
					package.File.Add(matches[0], "Linked");
			} while (matches != null);
			Match("");

			Match("LINKEDCOMMAND = \\");
			foreach (SourceFile file in package.File)
			{
				if (file.BuildAction == "Linked")
					Match("\t/linkresource:" + file.Name + " \\");
			}
			Match("");

			Match("CONTENTFILES = \\");
			do
			{
				matches = Regex("\t(.+?)[ ]\\\\", true);
				if (matches != null)
					package.File.Add(matches[0], "Content");
			} while (matches != null);
			Match("");

			Match("COMPILECOMMAND = $(SOURCES) $(EMBEDDEDCOMMAND) $(LINKEDCOMMAND)");
			Match("");
			Match(".PHONY: clean");
			Match("");
			Match("all: \\");
			Match("\t$(OUTDIR)/$(TARGET) \\");
			foreach (SourceFile file in package.File)
			{
				if (file.BuildAction == "Content")
					Match("\t$(BINDIR)/" + Path.GetFileName(file.Name) + " \\");
			}
			Match("");

			Match("$(OUTDIR)/$(TARGET): $(SOURCES) $(EMBEDDEDFILES) $(LINKEDFILES) $(DEPS)");
			Match("\t-@if [ ! -d $(OUTDIR) ]; then mkdir -p $(OUTDIR); fi");

			matches = Regex("\t@\\$\\(CSC\\) /nologo /out:\\$@ /t:([a-z]+) /lib:\\$\\(BINDIR\\) \\$\\(FLAGS\\) \\$\\(COMPILECOMMAND\\)");
			if ((package.Kind == "exe"    && matches[0] != "exe")     ||
				 (package.Kind == "winexe" && matches[0] != "winexe")  ||
				 (package.Kind == "dll"    && matches[0] != "library") ||
				 (package.Kind == "aspnet" && matches[0] != "library"))
			{
				throw new FormatException("Unexpected value /t:" + matches[0] + " for " + package.Kind + " package");
			}

			Match("");

			foreach (SourceFile file in package.File)
			{
				if (file.BuildAction == "Linked")
				{
					string basename = Path.GetFileName(file.Name);
					Match("$(BINDIR)/" + basename + ": " + file.Name);
					Match("\t-@cp -fR $^ $@");
					Match("");
				}
			}

			foreach (SourceFile file in package.File)
			{
				if (file.BuildAction == "Content")
				{
					string basename = Path.GetFileName(file.Name);
					Match("$(BINDIR)/" + basename + ": " + file.Name);
					Match("\t-@cp -fR $^ $@");
					Match("");
				}
			}

			Match("clean:");
			Match("\t@echo Cleaning " + package.Name);
			Match("\t-@rm -f $(OUTDIR)/$(TARGET)");
			Match("\t-@rm -fR $(OBJDIR)");
		}
		#endregion
	}
}
