import re

def validate(section, key, value, type):
    """
    检查 value 是否是逗号分隔的颜色列表，每个颜色为格式化的 (R,G,B)，并且 RGB 值在 0-255 范围内。
    """
    # 定义正则表达式匹配 (R,G,B) 格式
    pattern = r"\((\d+),(\d+),(\d+)\)"
    matches = re.findall(pattern, value)

    if not matches:
        return f"Invalid format: {value}", 1

    colors = []
    for match in matches:
        try:
            r, g, b = map(int, match)
            if 0 <= r <= 255 and 0 <= g <= 255 and 0 <= b <= 255:
                colors.append((r, g, b))
            else:
                return f"RGB values out of range in {match}", 1
        except ValueError:
            return f"Invalid RGB value in {match}", 1

    # 检查是否完全匹配 value 的长度，确保没有额外的非法字符
    reconstructed_value = ",".join(f"({r},{g},{b})" for r, g, b in colors)
    if reconstructed_value != value:
        return f"Invalid format or extra characters in: {value}", 1

    return f"Validation succeeded: {len(colors)} colors found", 0
