# **INI Validator**

##  **版本信息**
- 当前版本：`v1.0`
- 发布日期：`2024-12-21`

##  **简介**
是星轨实验室研发的一款INI检查器，其预计可以检查《命令与征服：红色警戒2之尤里的复仇》1.001版本的配置文件内容，主为模组作者（Modders）服务，旨在构建CI流水线的静态代码分析与门禁校验器。
 
## **主要功能**

- **不符合键要求的值**
  - 输出不符合键要求的值。
  - 自动发现未使用的节、无效键值、重复定义等问题。

- **不规范的格式**
  - 输出不规范的格式，例如等号前后的空格、错误的继承等。  

## **使用说明**

### 1. 基本操作

#### 1.1 运行程序

- **方法 1**：将ini文件拖放到程序图标。

- **方法 2**：打开程序，输入ini文件路径，按下回车。

- **方法 3**：打开程序，将ini文件拖放到程序控制台上，控制台会自动解析文件路径，然后按下回车。

#### 1.2 查看检查结果
- 检查完成后，程序会在控制台输出结果，以下是示例内容：
    ```bash
    [建议] 第25453行        | 节[VEINHOLE]未被收录，忽略检查
    [建议] rulesmd1.001.ini | [BLUELAMP] LightGreenTint=0,01
    [详情] 第17399行        | 0,01不是浮点数，非整数部分会被忽略
    [警告] rulesmd1.001.ini | [CRNeutronRifle]
    [详情] 第24431行        | 键"Report"重复设定，第24429行的被覆盖为ChronoLegionAttack
    [错误] rulesmd1.001.ini | [AGGattlingE] Projectile=Invisiblelow ;GEF Anti ground ;SA
    [详情] 第24605行        | BulletType中声明的Invisiblelow没有实现
    ```

- 生成日志文件（默认名为 Checker.log）。

### 2. 配置文件结构

#### 2.1 INIConfigCheck.ini文件结构
由五大检查器: 注册表检查器、类型检查器、限制检查器、列表检查器、数字检查器组成。每个检查器都有其相应的注册表以及所注册内容的实现，程序会从注册表检查器为入口进行递归搜索，根据注册表对应的类型，逐个检查目标ini的每一个节的每一个键值对，根据键的类型调用相应的检查器进行检查。

> INIConfigCheck.ini中的注册表无需像原版ini那样填写序号，可以直接填写内容。

#### 2.2 Setting.ini文件结构

主要用于修改程序的运行模式，输出日志对应的字符串内容。

> 可以在[LogSetting]中修改日志内容，将其注释即可不输出此类错误，其格式为c++的format字符串: 使用{}作为可变参数，在花括号中填写数字可以控制参数的先后顺序

### 3. 检查器配置

#### 3.1 注册表检查器

> 注册表: [Globals]、[Registries]

- 对于[Globals]下注册的键，程序会将其视作全局唯一实例进行检查，所注册的值就对应INI中的同名节。

- 对于[Registries]下注册的键，程序会将其视作一个注册表，在目标INI中的同名节下的所有值将会根据其在[Registries]中对应的值(类型)进行检查。

例:
```ini
[Globals]
AudioVisual

[AudioVisual]
DetailMinFrameRateNormal=int
```

```ini
[Registries]
BuildingTypes=BuildingType

[BuildingType]
Capturable=bool
```

#### 3.2 类型检查器

用于标识一个具体类型

> 注册表: [Sections]

默认提供三种特殊数据类型: int(整数)、float(小数)、string(字符串)

例:
```ini
[Sections]
AbstractType

[AbstractType]
UIName=string
Name=string
```

#### 3.3 限制检查器

> 注册表: [Limits]  
> 可用标签:  
> StartWith = 前缀的限定内容, 不填则不检查  
> EndWith = 后缀的限定内容, 不填则不检查  
> LimitIn = 整体的限定内容, 不填则不检查  
> MaxLength = 字符串的长度限制  
> IgnoreCase = 是否忽略大小写检查, 作用于前面三条  

例:
```ini
[Limits]
bool
BuildCat

[bool]
StartWith=1,1,0,t,f,y,n
IgnoreCase=yes

[BuildCat]
LimitIn=Combat,Infrastructure,Resource,Power,Tech,DontCare
```

#### 3.4 列表检查器

**用于列表的的值**

> 注册表: [Lists]   
> 可用标签:  
> Type = 列表的数据类型, 可填Sections、NumberLimits、Limits中的值  
> Range = 列表长度最小值, 列表长度最大值  

例:
```ini
[Lists]
Point2D

[Point2D]
Type=int
Range=2,2
```

#### 3.5 数字检查器

**需要限制上下限的数值类型**

> 注册表: [NumberLimits]  
> Range = 数值下限, 数值上限  

例:
```ini
[Sections]
AbstractType

[AbstractType]
UIName=string
Name=string
```

## 未来展望

- **记录历史检查文件**
  - 打开程序，直接按下回车，程序会自动检查上次检查的路径(即Settings.ini中的[Files]LastFilePath=)。
  - 按↑键，可以选择历史检查文件。

- **重新检查**
  - 完成一轮检查后，程序会询问是否继续检查其他文件。用户可以选择退出或输入新的文件路径。

- **自定义检查器**
  - 允许用户写python脚本以实现一些拓展平台独有的ini格式。

- **支持Ares与Phobos标签**
  - 完善INIConfigCheck.ini以实现检查拓展平台的标签。

- **检查除Rules以外的ini及相关耦合项**
  - 完善INIConfigCheck.ini以实现检查拓展平台的标签。

## 更新日志
- 2024.12.21 
  - 发布1.0版本

- 2024.11.23 
  - 项目立项

## 鸣谢与反馈

### 开发者：

小星星

Uranusian

### 如何贡献
如果希望为项目贡献代码，请通过 GitHub 提交 Pull Request。

### 问题反馈：
如果在使用过程中遇到问题，请通过 GitHub 提交 Issue。

## 许可证
本项目基于 GPL3.0 许可证 开源。
