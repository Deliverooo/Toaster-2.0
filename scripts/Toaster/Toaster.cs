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
            this.orbo(p_num);
        }
        public void orbo(int p_num)
        {
            Console.WriteLine("Number: {0}", p_num);
            
        }
    }
}