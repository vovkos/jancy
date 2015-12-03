// This file is part of Jancy (R) Project
// Tibbo Technology Inc (C) 2004-2015. All rights reserved
// Author: Vladimir Gladkov

//.............................................................................

public class JncExtensionLib
{
	public native int sourceFileCount ();
	public native String sourceFileName (int index);
	public native String sourceFileContents (int index);
	public native String findSourceFileContents (String fileName);

	public void load (String fileName)
	{
		System.load (fileName);
	}
	
	public static void main (String args []) 	
	{	
		if (args.length == 0)
		{
			System.err.println ("<file.jncx> argument missing");
			return;
		}
	
		JncExtensionLib lib = new JncExtensionLib ();
		lib.load (args [0]);

		int count = lib.sourceFileCount ();
		for (int i = 0; i < count; i++)
		{
			System.out.println (lib.sourceFileName (i));
			System.out.println ("<<<");
			System.out.println (lib.sourceFileContents (i));
			System.out.println (">>>\n");
		}
	}
}

//.............................................................................
