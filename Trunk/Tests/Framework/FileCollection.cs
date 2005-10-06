using System;
using System.Collections;
using System.Text;

namespace Premake.Tests.Framework
{
	public class FileCollection : CollectionBase
	{
		public void Add(SourceFile file)
		{
			List.Add(file);
		}

		public void Add(string filename)
		{
			Add(filename, null, null, null);
		}

		public void Add(string filename, string buildaction)
		{
			Add(filename, null, buildaction, null);
		}

		public void Add(string filename, string subtype, string buildaction)
		{
			Add(filename, subtype, buildaction, null);
		}
			
		public void Add(string filename, string subtype, string buildaction, string depends)
		{
			SourceFile file = new SourceFile();
			file.Name = filename;
			file.Subtype = subtype;
			file.BuildAction = buildaction;
			file.DependsOn = depends;
			List.Add(file);
		}

		public void Add(int count)
		{
			for (int i = 0; i < count; ++i)
				List.Add(new SourceFile());
		}

		public SourceFile this[int index]
		{
			get { return (SourceFile)List[index]; }
			set { List[index] = value; }
		}

		public void CompareTo(FileCollection actual)
		{
			if (this.Count == 0)
				return;

			if (this.Count != actual.Count)
				throw new FormatException("Expected " + this.Count + " files but got " + actual.Count);

			Hashtable index = new Hashtable();
			foreach (SourceFile file in actual)
				index[file.Name] = file;

			foreach (SourceFile efile in this.List)
			{
				SourceFile afile = (SourceFile)index[efile.Name];
				if (afile == null)
				{
					StringBuilder msg = new StringBuilder();
					msg.Append("Expected file '" + efile.Name + "' not found in file list. Actual list follows:\n");
					foreach (SourceFile file in actual)
						msg.Append(file.Name + "\n");
					throw new FormatException(msg.ToString());
				}

				efile.CompareTo(afile);
			}
		}
	}
}
