import iv

def validate(section, key, value, type):
    """
    检查值是否符合以下格式：
    1. 第一个值是 0 到 INT_MAX 的数字。
    2. 第二个值是 ARMOR、FIREPOWR、<none> 中的一种。
    3. 第三个值的第一个字符必须是 bool。
    4. 第四个值可选，数字。

    Args:
        section (dict): 当前 Section 的键值对。
        key (str): 当前被检查的键名。
        value (str): 当前被检查的键值。
        type (str): 类型信息。

    Returns:
        tuple: (状态码, 错误说明)
            -1: 没有错误
             0: DEFAULT (程序自身导致的错误)
             1: INFO (不影响游戏运行)
             2: WARNING (可能产生非预期结果)
             3: ERROR (会导致游戏崩溃)
    """
    # 分割 value
    parts = value.split(',')

    # 检查值的个数是否在 3 到 4 个之间
    if len(parts) < 3 or len(parts) > 4:
        return 2, f"值的个数不符合要求，应为3到4个，实际为{len(parts)}: {value}"

    # 检查第一个值是否为 0 到 INT_MAX 的数字
    try:
        first_value = int(parts[0].strip())
        if first_value < 0:
            return 3, f"第一个值必须是非负整数，但实际值为: {parts[0]}"
    except ValueError:
        return 3, f"第一个值不是有效的整数: {parts[0]}"

    # 检查第二个值是否为动画
    second_value = parts[1].strip()
    section_name = "Animations"

    # 调用主程序模块iv以获取其他section信息
    global_section = iv.get_section(section_name)
    if not global_section:
        return 3, f"注册表{section_name}是空的"

    if second_value not in global_section.values():
        return 3, f"键{second_value}无法在注册表{section_name}中找到"

    # 检查第三个值的第一个字符
    valid_third_first_chars = {'1', '0', 't', 'T', 'y', 'Y', 'n', 'N', 'f', 'F'}
    third_value = parts[2].strip()
    if not third_value or third_value[0] not in valid_third_first_chars:
        return 3, f"第三个值{third_value}不是bool"

    # 检查第四个值（如果存在），是否是一个数字
    if len(parts) == 4:
        fourth_value = parts[3].strip()
        try:
            if '%' in fourth_value:
                fourth_value = fourth_value.split('%', 1)[0].strip()
            _ = float(fourth_value)
        except ValueError:
            return 3, f"第四个值必须是一个数字，但实际值为: {fourth_value}"

    return -1, "验证通过"
