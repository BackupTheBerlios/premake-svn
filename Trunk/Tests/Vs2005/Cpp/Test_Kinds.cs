using System;
using NUnit.Framework;
using Premake.Tests.Framework;

namespace Premake.Tests.Vs2005.Cpp
{
	[TestFixture]
	public class Test_Kinds
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

			_parser = new Vs2005Parser();
		}

		public void Run()
		{
			TestEnvironment.Run(_script, _parser, _expects, null);
		}
		#endregion

		[Test]
		public void Test_KindIsExe()
		{
			_expects.Package[0].Config[0].Kind = "exe";
			_expects.Package[0].Config[1].Kind = "exe";
			Run();
		}

		[Test]
		public void Test_KindIsWinExe()
		{
			_script.Replace("exe", "winexe");
			_expects.Package[0].Config[0].Kind = "winexe";
			_expects.Package[0].Config[1].Kind = "winexe";
			Run();
		}

		[Test]
		public void Test_KindIsDll()
		{
			_script.Replace("exe", "dll");
			_expects.Package[0].Config[0].Kind = "dll";
			_expects.Package[0].Config[1].Kind = "dll";
			Run();
		}

		[Test]
		public void Test_KindIsLib()
		{
			_script.Replace("exe", "lib");
			_expects.Package[0].Config[0].Kind = "lib";
			_expects.Package[0].Config[1].Kind = "lib";
			Run();
		}

		[Test]
		public void Test_MixedKinds()
		{
			_script.Append("package.config['Debug'].kind = 'lib'");
			_script.Append("package.config['Release'].kind = 'dll'");
			_expects.Package[0].Config[0].Kind = "lib";
			_expects.Package[0].Config[1].Kind = "dll";
			Run();
		}
	}
}
