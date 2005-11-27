using System;
using NUnit.Framework;
using Premake.Tests.Framework;

namespace Premake.Tests.Gnu.Cs
{
	[TestFixture]
	public class Test_BuildActions
	{
		Script  _script;
		Project _expects;
		Parser  _parser;

		#region Setup and Teardown
		[SetUp]
		public void Test_Setup()
		{
			_script = Script.MakeBasic("exe", "c#");

			_expects = new Project();
			_expects.Package.Add(1);
			_expects.Package[0].Config.Add(2);

			_parser = new GnuParser();
		}

		public void Run()
		{
			TestEnvironment.Run(_script, _parser, _expects, null);
		}
		#endregion

		[Test]
		public void Test_DefaultCodeAction()
		{
			_script.Replace("'somefile.txt'", "'file0.cs'");
			_expects.Package[0].File.Add("file0.cs", "Compile");
			Run();
		}

		[Test]
		public void Test_AspxCodeAction()
		{
			_script.Replace("'somefile.txt'", "'file0.aspx.cs'");
			_expects.Package[0].File.Add("file0.aspx.cs", "Compile");
			Run();
		}

		[Test]
		public void Test_AspxAction()
		{
			_script.Replace("'somefile.txt'", "'file0.aspx'");
			_expects.Package[0].File.Add("file0.aspx", "Content");
			Run();
		}

		[Test]
		public void Test_AsaxAction()
		{
			_script.Replace("'somefile.txt'", "'file0.asax'");
			_expects.Package[0].File.Add("file0.asax", "Content");
			Run();
		}

		[Test]
		public void Test_ResxAction()
		{
			_script.Replace("'somefile.txt'", "'file0.resx'");
			_expects.Package[0].File.Add("obj/Debug/MyPackage.file0.resources", null, "EmbeddedResource");
			Run();
		}


		[Test]
		public void Test_ResxWithDependencyAction()
		{
			_script.Replace("'somefile.txt'", "'file0.resx','file0.cs'");
			_expects.Package[0].File.Add("obj/Debug/MyPackage.file0.resources", "EmbeddedResource");
			_expects.Package[0].File.Add("file0.cs");
			Run();
		}

		[Test]
		public void Test_DefaultBuildAction()
		{
			_script.Replace("'somefile.txt'", "'file0.cs','somefile.txt'");
			_expects.Package[0].File.Add("file0.cs", "Compile");
			Run();
		}

		[Test]
		public void Test_CustomBuildAction()
		{
			_script.Append("package.config['somefile.txt'].buildaction = 'EmbeddedResource'");
			_expects.Package[0].File.Add("somefile.txt", "EmbeddedResource");
			Run();
		}

	}
}
