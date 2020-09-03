﻿using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;

namespace Injected
{
    public class Logger
    {
        [ThreadStatic]
        private static Stopwatch timer;

        /// <summary>
        /// Log - this is the method we are going to call
        /// </summary>
        /// <param name="data">the data from the method being instrumented</param>
        public static void Log(object data)
        {
            var array = data as object[];
            if (array == null) return;
            if (array.Count() != 2) return;

            var source = array[0];
            var e = array[1];
            if (e == null) return;
            if (source == null) return;

            Console.WriteLine(string.Format("[注入开始] 方法: {0}", source.ToString()));
            Console.WriteLine(string.Format("[注入开始] e: {0}", e.ToString()));
            if (timer == null)
                timer = new Stopwatch();
            timer.Restart();
        }

        /// <summary>
        /// Log - this is the method we are going to call AFTER the method has finished
        /// </summary>
        public static void LogAfter()
        {
            timer.Stop();
            Console.WriteLine(string.Format("[注入结束] took {0:0.00} ms ({1})", timer.Elapsed.TotalMilliseconds, timer.Elapsed));
        }

        /// <summary>
        /// An example of how the instrumentation would look - use this under ILSpy, Reflector 
        /// or ILDASM to see the actual IL. We are going to use this as a template
        /// </summary>
        /// <param name="source"></param>
        /// <param name="e"></param>
        static public void OnDoThing(object source, EventArgs e)
        {
            var data = new[] { source, e };
            Log(data);

            Console.WriteLine("OnDoThing");
        }
    }
}
