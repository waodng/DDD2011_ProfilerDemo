using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

/* ==============================================================================
 * 创建日期：2020/9/3 09:22:01
 * 创 建 者：wgd
 * 功能描述：InjectTest  
 * ==============================================================================*/
namespace ProfilerTarget
{

    /// <summary>
    /// 测试该类可以所有方法注入日志记录
    /// </summary>
    public class InjectTest
    {
        public InjectTest()
        {
            Console.WriteLine("我是构造方法");
        }

        public static void SayHello(object sender,EventArgs e)
        {
            Console.WriteLine("我是测试方法");
        }

        public static void OnMethodToInstrument(object sender, EventArgs e)
        {
            Console.WriteLine("after finally clause");
        }
    }
}
