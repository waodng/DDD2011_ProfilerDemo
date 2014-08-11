using Injected;
using System;

namespace ProfilerTarget
{
    class Program
    {
        static void Main(string[] args)
        {
            //System.Diagnostics.Debugger.Launch();
            //Console.WriteLine();
            //OnMethodToInstrument("hello", new EventArgs());
            //Console.WriteLine();

            // Without mocking enabled (the default)
            Console.WriteLine(new string('#', 90));
            Console.WriteLine("Calling ClassToMock.StaticMethodToMock() (a static method in a sealed class)");
            var result = ClassToMock.StaticMethodToMock();
            Console.WriteLine("Result: " + result);
            Console.WriteLine(new string('#', 90) + "\n");

            // With mocking enabled, doesn't call into the static method, calls the mocked version instead
            Console.WriteLine(new string('#', 90));
            Mocked.SetReturnValue = 1;
            Console.WriteLine("Turning ON mocking of \"Profilier.ClassToMock.StaticMethodToMock\"");
            //Mocked.Configure(typeof(ClassToMock).FullName + ".StaticMethodToMock", mockMethod: true);
            Mocked.Configure("ProfilerTarget.ClassToMock.StaticMethodToMock", mockMethod: true);

            Console.WriteLine("Calling ClassToMock.StaticMethodToMock() (a static method in a sealed class)");
            result = ClassToMock.StaticMethodToMock();
            Console.WriteLine("Result: " + result);
            Console.WriteLine(new string('#', 90) + "\n");
        }

        static void OnMethodToInstrument(object sender, EventArgs e)
        {
            //Logger.Log(new Object[2] { sender, e }); // Instrumented code, will be added dynamicially

            try
            {
                Console.WriteLine("OnDoThingInstrument");
                Console.WriteLine("OnDoThingInstrument {1}", 1);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Uh oh " + ex.Message);
            }
            finally
            {
                Console.WriteLine("finally clause");
            }
            Console.WriteLine("after finally clause");

            //Logger.AfterLog(); // Instrumented code, will be added dynamicially
        }
    }

    public sealed class ClassToMock
    {
        public static int StaticMethodToMock()
        {
            Log("StaticMethodToMock called, returning 42");
            return 42;
        }

        /// <summary>
        /// An example of how the instrumentation would look - use this under ILSpy, Reflector 
        /// or ILDASM to see the actual IL. We are going to use this as a template
        /// </summary>
        public static int StaticMethodToMockWhatWeWantToDo()
        {
            // Inject the IL to do this instead!!
            if (Mocked.ShouldMock("Profilier.ClassToMock.StaticMethodToMockWhatWeWantToDo"))
                return Mocked.MockedMethod();

            Log("StaticMethodToMockWhatWeWantToDo called, returning 42");
            return 42;
        }

        private static void Log(string format)
        {
            var origColour = Console.ForegroundColor;
            Console.ForegroundColor = ConsoleColor.Green;
            Console.WriteLine(format);
            Console.ForegroundColor = origColour;
        }

        private static void Log(string format, params object[] args)
        {
            var origColour = Console.ForegroundColor;
            Console.ForegroundColor = ConsoleColor.Green;
            Console.WriteLine(format, args);
            Console.ForegroundColor = origColour;
        }
    }
}