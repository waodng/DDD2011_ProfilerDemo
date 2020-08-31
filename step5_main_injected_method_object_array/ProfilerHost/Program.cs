using System;
using System.Diagnostics;
using System.IO;
using System.Reflection;

namespace ProfilerHost
{
    class Program
    {
        static void Main(string[] args)
        {
#if DEBUG
            Console.WriteLine("DEBUG mode\n");
#else
            Console.WriteLine("RELEASE mode\n");
#endif

            var currentDirectory = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
            Console.WriteLine("Executing Assembly:\n{0}\n", Assembly.GetExecutingAssembly().Location);
            Console.WriteLine("Current directory:\n{0}\n", currentDirectory);
            //var argument = "/s " + Path.Combine(currentDirectory, "DDDProfiler.dll");
            //var registerProfilerDll = Process.Start("regsvr32", argument);
            //Console.WriteLine("Running:\nregsvr32 {0}\n", argument);
            //registerProfilerDll.WaitForExit();

            var startInfo = new ProcessStartInfo(Path.Combine(currentDirectory, "ProfilerTarget.exe"));
            //if (startInfo.EnvironmentVariables.ContainsKey("Cor_Profiler"))
            //    startInfo.EnvironmentVariables.Remove("Cor_Profiler");
           
            //if (startInfo.EnvironmentVariables.ContainsKey("Cor_Enable_Profiling"))
            //    startInfo.EnvironmentVariables.Remove("Cor_Enable_Profiling");

            //startInfo.EnvironmentVariables.Add("Cor_Profiler", "{BDD57A0C-D4F7-486D-A8CA-86070DC12FA0}");
            //startInfo.EnvironmentVariables.Add("Cor_Enable_Profiling", "1");
            startInfo.UseShellExecute = false;

            Console.WriteLine("Launching:\n{0}\n", startInfo.FileName);
            var process = Process.Start(startInfo);
            process.WaitForExit();
            Console.WriteLine();
        }
    }
}
