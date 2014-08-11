using System;
using System.Collections.Concurrent;

namespace Injected
{
    public class Mocked
    {
        private static ConcurrentDictionary<string, bool> setup = new ConcurrentDictionary<string, bool>();

        public static int SetReturnValue { get; set; }

        public static int MockedMethod()
        {
            Log("Mocked::MockedMetod called INSTEAD, returning {0}", SetReturnValue);
            return SetReturnValue;
        }

        public static bool ShouldMock(string methodNameAndPath)
        {
            bool shouldMock;
            if (setup.TryGetValue(methodNameAndPath, out shouldMock))
            {
                Log("Mocked::ShouldMock for \"{0}\", returning {1}", methodNameAndPath, shouldMock);
                return shouldMock;
            }

            Log("Mocked::ShouldMock for \"{0}\", returning {1}", methodNameAndPath, false);
            return false;
        }

        public static void Configure(string methodNameAndPath, bool mockMethod)
        {
            Log("Mocked::Configure \"{0}\" will {1}be mocked", methodNameAndPath, mockMethod ? "" : "NOT ");
            setup.AddOrUpdate(methodNameAndPath, mockMethod, (k, update) => mockMethod);
        }

        private static void Log(string format, params object[] args)
        {
            var origColour = Console.ForegroundColor;
            Console.ForegroundColor = ConsoleColor.DarkYellow;
            Console.WriteLine(format, args);
            Console.ForegroundColor = origColour;
        }
    }
}