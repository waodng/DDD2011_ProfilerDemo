### 说明

该项目是通过CLR公共语言运行时的API来在JIT（即时编译）时期对代码进行IL注入来达到修改程序的目的。

CLR 的Profile api接口在微软官网可找到C++的说明文档，该API调用只能通过C++来调用，无法使用C#来调用



* 本项目的运行时先把DDDProfile.dll类库通过注册COM组件的方式，然后设置环境变量

~~~
startInfo.EnvironmentVariables.Add("Cor_Profiler", "{BDD57A0C-D4F7-486D-A8CA-86070DC12FA0}");
startInfo.EnvironmentVariables.Add("Cor_Enable_Profiling", "1");
            
~~~

的方式来运行，每次程序运行时，CLR会自动启动com组件来运行注入代码程序。。。

...

### Purpose

Demo code for the DDD Melbourne 2011 Profiler Talk

### Licence
All Original Software is licensed under the [MIT Licence] (License.md) and does not apply to any other 3rd party tools, utilities or code which may be used to develop this application.

If anyone is aware of any licence violations that this code may be making please inform the developers so that the issue can be investigated and rectified.

### Building
You will need Visual Studio VS2010 with C# and C++, all other software should be included with this repository. 

### Issues
Please raise issues on GitHub, if you can repeat the issue then please provide a sample to make it easier for us to also repeat it and then implement a fix.
Dropbox is very useful for sharing files [Dropbox] (http://db.tt/VanqFDn)

