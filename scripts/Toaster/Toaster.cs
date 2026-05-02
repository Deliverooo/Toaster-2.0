using System;
using System.Runtime.CompilerServices;
using System.Text;
using System.Text.Unicode;

namespace Toaster
{
    public class Orbo
    {
        public static void staticTest()
        {
            Console.WriteLine("Static Test!!");
        }
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void nativeTest();
        
        public Orbo(int p_num)
        {
            nativeOrbo();
        }
        public void printTest(string p_message)
        {
            Console.WriteLine("Message: {0}", p_message);
        }
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern void nativeOrbo();
    }
}