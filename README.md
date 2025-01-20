# **INI Validator**
是星轨实验室研发的一款INI检查器，其预计可以检查《命令与征服：红色警戒2之尤里的复仇》1.001版本的配置文件内容，主为模组作者（Modders）服务，旨在构建CI流水线的静态代码分析与门禁校验器。

## **版本信息**
- 当前版本：[`v1.3`](https://github.com/Super-StarX/INIValidator/releases)
- 发布日期：`2025-01-15`

## **主要功能**

- **不符合键要求的值**
  - 输出不符合键要求的值。
  - 自动发现未使用的节、无效键值、重复定义等问题。

- **不规范的格式**
  - 输出不规范的格式，例如等号前后的空格、错误的继承等。

## **使用说明**

### 1. 基本操作

#### 1.1 运行程序

- **方法 1**：将ini文件(或多个)拖放到程序图标。

- **方法 2**：打开程序，直接按下回车，程序会以`Settings.ini`中的`[INIValidator]->FolderPath`作为文件夹路径进行读取。

- **方法 3**：打开程序，输入ini文件(或文件夹)路径，按下回车。

- **方法 4**：打开程序，将ini文件拖放到程序控制台上，控制台会自动解析文件路径，然后按下回车。

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
由六大检查器: 注册表检查器、类型检查器、限制检查器、列表检查器、数字检查器、自定义检查器组成。每个检查器都有其相应的注册表以及所注册内容的实现，程序会从注册表检查器为入口进行递归搜索，根据注册表对应的类型，逐个检查目标ini的每一个节的每一个键值对，根据键的类型自动调用相应的检查器进行检查。

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
> StartWith = 第一个值控制检查的前缀的长度，其余值控制前缀的限制内容，不填则不检查  
> EndWith = 第一个值控制检查的后缀的长度，其余值控制后缀的限制内容，不填则不检查  
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

#### 3.5 自定义检查器

用于通过Python脚本检查键值对是否合法, 程序将在`Scripts`文件夹中寻找同名脚本, 调用Python解释器检查键值对  
除此之外, 我们提供了一系列预置函数并打包成名为iv模块, 通过`import iv`来实现Python脚本与c++程序之间的联动

##### 3.5.1 脚本规范

> 参数:  
> ```
> section (dict): 当前 Section 的键值对。  
> key (str): 当前被检查的键名。  
> value (str): 当前被检查的键值。  
> ```
> 返回值:  
> ```
> tuple: (检查结果说明, 状态码)  
> ```
> 状态码:  
> ```
> ​-1: 没有错误, 不会报错  
> ​ 0: DEFAULT (程序自身导致的错误)  
> ​ 1: INFO (不影响游戏运行)  
> ​ 2: WARNING (可能产生非预期结果)  
> ​ 3: ERROR (会导致游戏崩溃)
>```


例:
```python
import re

def validate(section, key, value, type):
    """
    检查值是否为逗号分隔的颜色列表，并确保每个颜色在 0-255 范围内。
    """

    # 定义正则表达式匹配 (R,G,B) 格式
    pattern = r"\((-?\d+),(-?\d+),(-?\d+)\)"
    rgbs = re.findall(pattern, value)

    if not rgbs:
        return 2, f"非法的颜色列表格式: {value}"

    correct_values = []
    for rgb in rgbs:
        try:
            r, g, b = map(int, rgb)
            if 0 <= r <= 255 and 0 <= g <= 255 and 0 <= b <= 255:
                correct_values.append((r, g, b))
            else:
                return 2, f"RGB颜色值必须在0~255之间: ({r},{g},{b})"
        except ValueError:
            return 2, f"非法的RGB颜色值: {rgb}"

    # 检查是否完全匹配 value 的长度，确保没有额外的非法字符
    valid_part  = ",".join(f"({r},{g},{b})" for r, g, b in correct_values)
    remaining = value.replace(valid_part, "").strip()
    if remaining:
        return 2, f"存在非法字符或格式错误: {remaining}"

    return -1, f""

```

##### 3.5.2 获取其他section的内容

> 函数格式: iv.get_section(section_name: str)

```python
import iv

section_name = "Animations"
global_section = iv.get_section(section_name)
if not global_section:
    return 3, f"注册表{section_name}是空的"

if second_value != "<none>" and second_value not in global_section.values():
    return 3, f"键{second_value}无法在注册表{section_name}中找到"
```

## 未来展望

- **支持Ares与Phobos标签**
  - 完善INIConfigCheck.ini以实现检查拓展平台的标签。

- **与VSC插件交互**
  - 向Visual Studio Code插件INI-Intellisense提供语法合规检查支持。

## 更新日志
- 2025.1.15
  - 修复各种小的疑难杂症

- 2025.1.3
  - 允许用户拖放/输入多个文件/文件夹、允许用户直接回车以读取`Settings.ini`中的路径
  - 检测完毕后可以直接进行下一次检测了
  - 支持输出为json格式的日志

- 2025.1.1
  - 实现区分文件类型(例如是rules还是art)

- 2024.12.23
  - 修复Windows10下中文路径无法正确识别的问题

- 2024.12.22
  - 修复拖文件打开的方式运作不正常的问题
  - 允许用户通过写Python脚本的方式自定义检查器

- 2024.12.21
  - 发布1.0版本

- 2024.11.23
  - 项目立项

## 鸣谢与反馈

### 开发者：

小星星  
Uranusian

### 贡献者：

Noble_Fish

### 如何贡献
如果希望为项目贡献代码，请通过 GitHub 提交 Pull Request。

### 问题反馈
如果在使用过程中遇到问题，请通过 GitHub 提交 Issue。

### 联系我们
QQ群：[966912243](http://qm.qq.com/cgi-bin/qm/qr?_wv=1027&k=3pF9dRngeMmduKU6JzTEXMZMqtTF0RXg&authKey=YWnaMP7Pe6oSD0%2FsYAOFdZ643iuDPxWdLLesaTurCCcqW1yDwvE3jKTI9Tcc%2BiAS&noverify=0&group_code=966912243)

## 许可证
本项目基于 GPL3.0 许可证 开源。
