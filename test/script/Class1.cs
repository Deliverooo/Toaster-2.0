using System.Runtime.InteropServices;

namespace Test;

public static class TestClass
{
	[UnmanagedCallersOnly]
	public static int Init(IntPtr p_args, int p_count)
	{
		Console.WriteLine("Orbo!");

		return 67;
	}
}