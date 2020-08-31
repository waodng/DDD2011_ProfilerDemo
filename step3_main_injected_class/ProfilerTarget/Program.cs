using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ProfilerTarget
{
    class Program
    {
        static void Main(string[] args)
        {
            try {
                for (int i = 0; i < 5; i++)
                    TargetMethod(i);
            }
            catch (Exception e) {
                Console.WriteLine("Caught exception of type {0}",
                    e.GetType().FullName);
            }
            Console.Read();
        }

        static void TargetMethod(int i)
        {
            if (i == 4) throw new InvalidOperationException();
            Console.WriteLine("{0}", i);
        }
    }
}
