### modeladvisor
modeladvisor用于检查simulink模型中存在的问题。  
### 交互界面
包含by product和by task两组检查项集合，两个集合不是互相独立的。
通过勾选需要的检查项决定要检查的项目。  
	<center> 
	![modeladvisor_pane](/modeladvisor/modeladvisor_pane.png)
	</center> 
可以点击run this check单独检查某一项。  

**导出检查项配置**  

- Model Advisor Configration Editor导出json配置文件  
setting->open configration editor->勾选检查项->保存乘json文件  
	<center> 
	![modeladvisor_configration_editor](/modeladvisor/modeladvisor_configration_editor.png)
	</center>
- 导出checkID到workspace  
勾选检查项->右击->send check ID to workspace  
	<center> 
	![send_checkid_workspace](/modeladvisor/send_checkid_workspace.png)
	</center>


### 代码调用
```
clc;
clear;
open_system('E:\matlab\04 RLS\RLS.slx');
SysList={'RLS'};
%% Method 1: use checkIDlist specify check items
% CheckIDList ={'mathworks.design.UnconnectedLinesPorts','mathworks.design.OptimizationSettings'};
% SysResultObjArray = ModelAdvisor.run(SysList,CheckIDList);
%% Method 2: use config file specify check items
config = 'E:\matlab\04 RLS\demo.json';
SysResultObjArray = ModelAdvisor.run(SysList,'Configuration',config);
% Show report
ModelAdvisor.summaryReport(SysResultObjArray);
```