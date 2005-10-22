using System;
using NUnit.Framework;
using Premake.Tests.Framework;

namespace Premake.Tests.Vs2002.Cpp
{
	[TestFixture]
	public class Test_LinkFlags
	{
		Script  _script;
		Project _expects;
		Parser  _parser;

		#region Setup and Teardown
		[SetUp]
		public void Test_Setup()
		{
			_script = Script.MakeBasic("exe", "c++");

			_expects = new Project();
			_expects.Package.Add(1);
			_expects.Package[0].Config.Add(2);

			_parser = new Vs2002Parser();
		}

		public void Run()
		{
			TestEnvironment.Run(_script, _parser, _expects, null);
		}
		#endregion

		[Test]
		public void Test_SetFlagOnPackage()
		{
			_script.Append("package.linkflags = { 'static-runtime' }");
			_expects.Package[0].Config[0].LinkFlags = new string[] { "static-runtime" };
			_expects.Package[0].Config[1].LinkFlags = new string[] { "static-runtime" };
			Run();
		}

		[Test]
		public void Test_SetFlagOnConfig()
		{
			_script.Append("package.config['Debug'].linkflags = { 'static-runtime' }");
			_expects.Package[0].Config[0].LinkFlags = new string[] { "static-runtime" };
			_expects.Package[0].Config[1].LinkFlags = new string[] {};
			Run();
		}

		public void Test_SetFlagOnPackageAndConfig()
		{
			/* Since I only have one link flag I can't do this yet */
		}
	}
}
