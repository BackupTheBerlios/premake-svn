using System;
using NUnit.Framework;
using Premake.Tests.Framework;

namespace Premake.Tests.Gnu.Cpp
{
	[TestFixture]
	public class Test_PackageProps
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

			_parser = new GnuParser();
		}

		public void Run()
		{
			TestEnvironment.Run(_script, _parser, _expects, null);
		}
		#endregion

		#region Basic Property Tests
		[Test]
		public void Test_PackageName()
		{
			_expects.Package[0].Name = "MyPackage";
			Run();
		}

		[Test]
		public void Test_LanguageIsCpp()
		{
			_expects.Package[0].Language = "c++";
			Run();
		}
		#endregion

		#region Kind Tests
		[Test]
		public void Test_KindIsExe()
		{
			_expects.Package[0].Kind = "exe";
			Run();
		}

		[Test]
		public void Test_KindIsWinExe()
		{
			_script.Replace("exe", "winexe");
			_expects.Package[0].Kind = "winexe";
			Run();
		}

		[Test]
		public void Test_KindIsDll()
		{
			_script.Replace("exe", "dll");
			_expects.Package[0].Kind = "dll";
			Run();
		}

		[Test]
		public void Test_KindIsLib()
		{
			_script.Replace("exe", "lib");
			_expects.Package[0].Kind = "lib";
			Run();
		}
		#endregion
	}
}
