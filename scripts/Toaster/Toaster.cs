using System;
using System.Runtime.CompilerServices;
using System.Text;
using System.Text.Unicode;

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
		public static extern void GetTranslation(uint p_entity_id, out Vec3 p_out_translation);

		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern void SetTranslation(uint p_entity_id, ref Vec3 p_translation);
	}

	public class Entity
	{
		protected Entity()
		{
			ID = 0;
		}

		internal Entity(uint p_id)
		{
			ID = p_id;
		}

		protected readonly uint ID;

		public Vec3 Translation
		{
			get
			{
				InternalCalls.GetTranslation(ID, out Vec3 outTranslation);
				return outTranslation;
			}
			set => InternalCalls.SetTranslation(ID, ref value);
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