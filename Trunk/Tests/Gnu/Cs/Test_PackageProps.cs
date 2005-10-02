using System;
using NUnit.Framework;
using Premake.Tests.Framework;

namespace Premake.Tests.Gnu.Cs
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

		#region Basic Property Tests
		[Test]
		public void Test_PackageName()
		{
			_expects.Package[0].Name = "MyPackage";
			Run();
		}

		[Test]
		public void Test_LanguageIsCs()
		{
			_expects.Package[0].Language = "c#";
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
		public void Test_KindIsAspNet()
		{
			_script.Replace("exe", "aspnet");
			_expects.Package[0].Kind = "aspnet";
			Run();
		}
		#endregion
	}
}
