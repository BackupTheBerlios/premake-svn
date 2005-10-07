using System;
using NUnit.Framework;
using Premake.Tests.Framework;

namespace Premake.Tests.MonoDev.Cs
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

			_parser = new MonoDevParser();
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
			_expects.Package[0].File.Add(".\\file0.cs", "Code", "Compile");
			Run();
		}

		[Test]
		public void Test_ResxAction()
		{
			_script.Replace("'somefile.txt'", "'file0.resx'");
			_expects.Package[0].File.Add(".\\file0.resx", null, "EmbeddedResource");
			Run();
		}


		[Test]
		public void Test_ResxWithDependencyAction()
		{
			_script.Replace("'somefile.txt'", "'file0.resx','file0.cs'");
			_expects.Package[0].File.Add(".\\file0.resx", null, "EmbeddedResource", "file0.cs");
			_expects.Package[0].File.Add(".\\file0.cs");
			Run();
		}

		[Test]
		public void Test_DefaultBuildAction()
		{
			/* #develop does not support the 'Content' build action so I am forced to do this */
			_expects.Package[0].File.Add(".\\somefile.txt", "Nothing");
			Run();
		}

		[Test]
		public void Test_CustomBuildAction()
		{
			_script.Append("package.config['somefile.txt'].buildaction = 'EmbeddedResource'");
			_expects.Package[0].File.Add(".\\somefile.txt", "EmbeddedResource");
			Run();
		}

	}
}
