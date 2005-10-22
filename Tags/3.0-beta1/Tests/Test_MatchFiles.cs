using System;
using NUnit.Framework;
using Premake.Tests.Framework;

namespace Premake.Tests
{
	[TestFixture]
	public class Test_MatchFiles
	{
		#region Setup and Teardown
		TestEnvironment _test;
		Script  _script;
		Project _expects;
		Parser  _parser;

		[SetUp]
		public void Test_Setup()
		{
			_test = new TestEnvironment();

			_script = Script.MakeBasic("exe", "c++");
			_test.AddScript(_script);

			_expects = new Project();
			_expects.Package.Add(1);
			_expects.Package[0].Config.Add(2);

			_parser = new GnuParser();
		}

		public void Run()
		{
			_test.Run(_parser, _expects);
		}
		#endregion

		[Test]
		public void Test_SimplePattern()
		{
			_script.Replace("'somefile.txt'", "matchfiles('*.cpp')");
			_test.AddFile("file0.cpp");
			_test.AddFile("file1.cpp");
			_expects.Package[0].File.Add("file0.cpp");
			_expects.Package[0].File.Add("file1.cpp");
			Run();
		}

		[Test]
		public void Test_SubdirPattern()
		{
			_script.Replace("'somefile.txt'", "matchfiles('Code/*.cpp')");
			_test.AddFile("Code/file0.cpp");
			_test.AddFile("Code/file1.cpp");
			_expects.Package[0].File.Add("Code/file0.cpp");
			_expects.Package[0].File.Add("Code/file1.cpp");
			Run();
		}
	}
}
