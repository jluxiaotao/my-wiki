### polyspace
polyspace是matlab中一个用于静态代码检测的工具箱，可以用于检查c、c++代码，
包含polyspace bug finder和polyspace code prover两个工具。
### bug finder
Bug finder用于识别嵌入式c或c++代码中的运行时错误、并发问题、安全漏洞等缺陷。
Bug finder能够检查代码是否符合编码规范，比如MISRA C、MISRA C++、JSF++、自定义规范。
Bug finder生成的结果报告中包含缺陷、编码规则问题、代码质量指标，比如圈复杂度。
对于由simulink自动生成的代码，这些检查结果可以追溯到模型的具体位置。
### code prover
Code prover用于验证c或c++代码中不存在溢出、除零、越界等运行时错误。
同样能够检查代码是否符合编码规范。
根据验证结果代码会被标示为不同颜色，绿色、红色、橙色代表验证无问题、有问题、不确定。
### 差异
**1.bug finder速度更快**  

bug finder的耗时随着应用的代码量的增加呈线性增长，code prover耗时随代码量的增加增长的更快。
推荐在集成时用code prover，模块测试时用bug finder。  

**2.code prover的检查更为详尽**  

code prover只有确认完代码不存在任何问题时，才能将代码标识为绿色，
其工作模式决定了code prover需要更为详尽的检查代码。  

**3.bug finder无假阳性，code prover无假阴性**  

两者工作模式的差异，决定了bug finder找到的问题都是确定存在的，code prover标识的安全代码都是安全的。  

**4.bug finder的缺陷类别更多**  
code prover有时候仅会关注那些在数学意义上有问题的缺陷，而bug finder还会提示其它可能存在的问题。
如, 

- if(a=b),这句代码是可以运行的而且不一定会导致问题，但不一定是符合意图的。  
- 存在未使用的函数参数，不符合最佳实现。  
- 资源未关闭等资源管理缺陷。 
 
**5.bug finder使用更简单**  

- Code Prover和bug finder代码检查的时候均需要对代码进行编译，Code Prover编译过程更为严格且可能比正常编译时严格。  
- Code Prover发现错误时会终止后续的检查。  
- bug finder会在检查过程中输出结果，Code Prover需要在检查结束后才能输出结果（不重要）。  

### 使用方法

**交互界面**  

1. 打开polyspace  
	- 方式1：直接在polyspace安装路径下（matlab安装路径）\polyspace\bin文件夹找到polyspace可执行文件双击
	- 方式2：在matlab界面app中找到polyspace bug finder或polyspace code prover工具

2. 新建项目  
	File > New Project
	<center> 
	![new project](/polyspace/new_project.png)
	</center>  
3. 添加源文件和头文件  
	创建好新项目界面如下：
	<center> 
	![add_source_include](/polyspace/add_source_include.png)
	</center>
	依次添加源文件和头文件，添加结果如下：
	<center> 
	![add_result](/polyspace/add_result.png)
	</center>
	注意是否勾选add recursively。  
4. 配置  
	添加完源文件和头文件后点击finish，导航栏中和新建项目时设置根目录下增加Moudule_1文件夹和.psprj后缀的配置文件。
	<center> 
	![configure](/polyspace/configure.png)
	</center>
	配置文件中可以配置语言类型、语言标准、目标编译环境。
	<center> 
	![target_compiler_config](/polyspace/target_compiler_config.png)
	</center>
	配置文件中可以配置编码规范、自定义规范。
	<center> 
	![coding_standard_config](/polyspace/coding_standard_config.png)
	</center>
	配置文件中可以配置bug finder的检查项。
	<center> 
	![bug_finder_checkers_config](/polyspace/bug_finder_checkers_config.png)
	</center>
5. 运行查看结果  
	左上角的运行图标可以选择run bug finder或者run code prover，两个工具的运行结果保存在Moudule_1文件夹中的.psbf及.pscp文件中。
	<center> 
	![result](/polyspace/result.png)
	</center>
	
**windows或linux命令行**  
1. 保证polyspace可执行文件的路径\polyspace\bin添加到环境变量中  
2. 调用polyspace-bug-finder分析指定.c或.cpp文件
```
polyspace-bug-finder -sources FILES -I PATH -results-dir PATH
```
- -sources指定要分析的源文件用逗号隔开
- -I指定头文件的路径，可以多次使用指定多个文件夹
- -results-dir指定结果路径

用以下命令查看结果，
```
polyspace ps_results.psbf
```
还可以通过txt文件传入命令选项,比如传入源文件
```
polyspace-bug-finder -sources-list-file FILE
```
传入命令选项，
```
polyspace-bug-finder -options-file FILE
```
```
-lang c
-prog MyCodeProverProject
-author jsmith
-sources "mymain.c,funAlgebra.c,funGeometry.c"
-target x86_64
-compiler generic
-dos
-misra2 required-rules
-do-not-generate-results-for all-headers
-main-generator
-results-dir C:\Polyspace\MyCodeProverProject
```
交互界面中的配置均可以通过命令选项进行配置，比如c++编码标准，
<center> 
![cpp_standard_options](/polyspace/cpp_standard_options.png)
</center>