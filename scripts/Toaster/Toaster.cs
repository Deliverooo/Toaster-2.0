using System;
using System.Runtime.CompilerServices;
using System.Text;
using System.Text.Unicode;

namespace Toaster
{
	public struct Vec2(float p_x, float p_y)
	{
		public float X = p_x;
		public float Y = p_y;
	}

	public struct Vec3(float p_x, float p_y, float p_z)
	{
		public float X = p_x;
		public float Y = p_y;
		public float Z = p_z;
	}

	public struct Vec4(float p_x, float p_y, float p_z, float p_w)
	{
		public float X = p_x;
		public float Y = p_y;
		public float Z = p_z;
		public float W = p_w;
	}

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

	public class Entity
	{
		public Entity()
		{
			
		}
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