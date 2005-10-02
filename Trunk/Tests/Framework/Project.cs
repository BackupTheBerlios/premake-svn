using System;
using System.Collections;

namespace Premake.Tests.Framework
{
	public class Project
	{
		public string      ID;
		public string      Name;
		public string      Path;
		public ArrayList   Configuration;
		public PackageCollection Package;

		public Project()
		{
			Configuration = new ArrayList();
			Package = new PackageCollection();
		}

		public void CompareTo(Project actual)
		{
			this.Package.CompareTo(actual.Package);
		}



#if OBSOLETE
		public string Name;
		public string Path;
		public ArrayList Configuration;
		public char   PathSeparator;

		public Project()
		{
			Configuration = new ArrayList();
			Package = new PackageCollection();
		}

		public void CompareTo(Project expects)
		{
			Test(expects.Name, this.Name, "Project name");
			TestPath(expects.Path, this.Path, "Project path");

			/* Test project configurations */
			if (expects.Configuration.Count > 0 && expects.Configuration.Count != this.Configuration.Count)
				throw new FormatException("Expected " + expects.Configuration.Count + " configurations but got " + this.Configuration.Count);

			for (int i = 0; i < expects.Configuration.Count; ++i)
			{
				if ((string)expects.Configuration[i] != (string)this.Configuration[i])
					throw new FormatException("Expected configuration '" + expects.Configuration[i] + "' but got '" + this.Configuration[i] + "'");
			}

			/* Test packages */
			if (expects.Package.Count > 0 && expects.Package.Count != this.Package.Count)
				throw new FormatException("Expected " + expects.Package.Count + " packages but got " + this.Package.Count);

			for (int i = 0; i < expects.Package.Count; ++i)
			{
				Package ex = expects.Package[i];
				Package ac = this.Package[i];

				Test(ex.Name,       ac.Name,       "Package name");
				Test(ex.Kind,       ac.Kind,       "Package kind");
				Test(ex.Language,   ac.Language,   "Package language");
				Test(ex.ScriptName, ac.ScriptName, "Package script name");
				TestPath(ex.Path,   ac.Path,       "Package path");

				if (ex.Config.Count > 0)
					TestConfigList(ex.Config, ac.Config);

				if (ex.File.Count > 0)
					TestFileList(ex.File, ac.File);
			}
		}

		private void TestConfigList(ConfigCollection expects, ConfigCollection actual)
		{
			if (expects.Count != actual.Count)
				throw new FormatException("Expected " + expects.Count + " configurations but got " + actual.Count);

			for (int i = 0; i < expects.Count; ++i)
			{
				Configuration ex = expects[i];
				Configuration ac = actual[i];

				TestPath(ex.BinDir, ac.BinDir, "Bin directory");
				TestPath(ex.LibDir, ac.LibDir, "Lib directory");
				TestPath(ex.ObjDir, ac.ObjDir, "Obj directory");
			}
		}

		private void TestFileList(ArrayList expects, ArrayList actual)
		{
			if (expects.Count != actual.Count)
				throw new FormatException("Expected " + expects.Count + " files but got " + actual.Count);

			foreach (string ename in expects)
			{
				bool found = false;
				foreach (string aname in actual)
					if (ename.CompareTo(aname) == 0)
						found = true;

				if (!found)
					throw new FormatException("Expected '" + ename + "' in file list, but not found");
			}
		}

		private void Test(string expected, string actual, string description)
		{
			if (expected != null && expected.CompareTo(actual) != 0)
				throw new FormatException(description + " should be '" + expected + "' but is '" + actual + "'");
		}

		private void TestPath(string expected, string actual, string description)
		{
			if (expected != null)
				expected = expected.Replace('/', PathSeparator);
			Test(expected, actual, description);
		}
#endif
	}
}
