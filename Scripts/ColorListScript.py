import re

def validate(section, key, value, type):
    """
    检查值是否为逗号分隔的颜色列表，并确保每个颜色在 0-255 范围内。

    Args:
        section (dict): 当前 Section 的键值对。
        key (str): 当前被检查的键名。
        value (str): 当前被检查的键值。

    Returns:
        tuple: (检查结果说明, 状态码)
            -1: 没有错误
            0: DEFAULT (程序自身导致的错误)
            1: INFO (不影响游戏运行)
            2: WARNING (可能产生非预期结果)
            3: ERROR (会导致游戏崩溃)
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
