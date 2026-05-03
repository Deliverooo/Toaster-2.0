using System;
using System.Runtime.CompilerServices;
using System.Text;

namespace Toaster
{
	public class Log
	{
		public static void Trace(string p_str)
		{
			Console.ForegroundColor = ConsoleColor.Cyan;
			Console.WriteLine("{0}", p_str);
			Console.ResetColor();
		}

		public static void Info(string p_str)
		{
			Console.ForegroundColor = ConsoleColor.Green;
			Console.WriteLine("{0}", p_str);
			Console.ResetColor();
		}

		public static void Warn(string p_str)
		{
			Console.ForegroundColor = ConsoleColor.Yellow;
			Console.WriteLine("{0}", p_str);
			Console.ResetColor();
		}

		public static void Error(string p_str)
		{
			Console.ForegroundColor = ConsoleColor.Red;
			Console.WriteLine("{0}", p_str);
			Console.ResetColor();
		}
	}

	public static class InternalCalls
	{
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern bool HasComponent(uint p_entity_id, Type p_component_type);

		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern void AddComponent(uint p_entity_id, Type p_component_type);
	}

	public class Orbo
	{
		public static void StaticTest()
		{
			Console.WriteLine("Static Test!!");
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern void NativeTest();

		public Orbo(int p_num)
		{
			NativeOrbo();
		}

		public void PrintTest(string p_message)
		{
			Console.WriteLine("Message: {0}", p_message);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		public extern void NativeOrbo();
	}
}