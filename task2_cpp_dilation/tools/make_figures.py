"""把 demo_output.txt 里程序真实输出的矩阵渲染成 PNG 图片（纯标准库，无第三方依赖）。

用法: python make_figures.py <demo_output.txt> <输出目录>
"""
import os
import re
import struct
import sys
import zlib

CELL = 22          # 每个像素格子的边长
GAP = 26           # 面板之间的间隔
MARGIN = 18        # 画布外边距
BG = (255, 255, 255)
PANEL_BG = (238, 240, 243)
GRID = (205, 210, 216)
FG = (44, 111, 187)      # 前景：蓝
FG_ALT = (219, 122, 45)  # 对照用的第二色（原图）


def write_png(path, width, height, rows):
    raw = b''.join(b'\x00' + bytes(v for px in row for v in px) for row in rows)

    def chunk(tag, data):
        return (struct.pack('>I', len(data)) + tag + data +
                struct.pack('>I', zlib.crc32(tag + data) & 0xffffffff))

    png = (b'\x89PNG\r\n\x1a\n' +
           chunk(b'IHDR', struct.pack('>IIBBBBB', width, height, 8, 2, 0, 0, 0)) +
           chunk(b'IDAT', zlib.compress(raw, 9)) +
           chunk(b'IEND', b''))
    with open(path, 'wb') as f:
        f.write(png)


def new_canvas(width, height):
    return [[BG for _ in range(width)] for _ in range(height)]


def draw_panel(canvas, matrix, x0, y0, color, grid=True):
    """把 0/1 矩阵画到画布上，返回面板宽高"""
    h = len(matrix)
    w = len(matrix[0])
    pw, ph = w * CELL, h * CELL

    # 面板底色
    for y in range(y0 - 2, y0 + ph + 2):
        for x in range(x0 - 2, x0 + pw + 2):
            if 0 <= y < len(canvas) and 0 <= x < len(canvas[0]):
                canvas[y][x] = PANEL_BG

    # 先把网格画满面板，再用实心前景覆盖上去 —— 前景格保持纯色，便于逐格核对
    if grid:
        for c in range(w + 1):
            x = x0 + c * CELL
            for y in range(y0, y0 + ph):
                if x < len(canvas[0]):
                    canvas[y][x] = GRID
        for r in range(h + 1):
            y = y0 + r * CELL
            if y < len(canvas):
                for x in range(x0, x0 + pw):
                    canvas[y][x] = GRID

    for r in range(h):
        for c in range(w):
            if not matrix[r][c]:
                continue
            for dy in range(CELL):
                row = canvas[y0 + r * CELL + dy]
                for dx in range(CELL):
                    row[x0 + c * CELL + dx] = color

    return pw, ph


def draw_gray_panel(canvas, values, x0, y0):
    """把 0~1 的灰度矩阵画成面板（白→蓝渐变），用于展示卷积模糊后的灰度"""
    h, w = len(values), len(values[0])
    pw, ph = w * CELL, h * CELL

    for y in range(y0 - 2, y0 + ph + 2):
        for x in range(x0 - 2, x0 + pw + 2):
            if 0 <= y < len(canvas) and 0 <= x < len(canvas[0]):
                canvas[y][x] = PANEL_BG

    for r in range(h):
        for c in range(w):
            v = max(0.0, min(1.0, values[r][c]))
            color = (int(255 - 211 * v), int(255 - 144 * v), int(255 - 68 * v))
            for dy in range(CELL):
                row = canvas[y0 + r * CELL + dy]
                for dx in range(CELL):
                    row[x0 + c * CELL + dx] = color

    for c in range(w + 1):
        x = x0 + c * CELL
        for y in range(y0, y0 + ph):
            if x < len(canvas[0]):
                canvas[y][x] = GRID
    for r in range(h + 1):
        y = y0 + r * CELL
        if y < len(canvas):
            for x in range(x0, x0 + pw):
                canvas[y][x] = GRID
    return pw, ph


def conv_sum(matrix, k=3):
    """k×k 全 1 核的卷积（零填充），返回每格的邻域和；除以 k*k 就是均值模糊"""
    h, w = len(matrix), len(matrix[0])
    half = k // 2
    out = [[0] * w for _ in range(h)]
    for r in range(h):
        for c in range(w):
            s = 0
            for dr in range(-half, half + 1):
                for dc in range(-half, half + 1):
                    rr, cc = r + dr, c + dc
                    if 0 <= rr < h and 0 <= cc < w:
                        s += matrix[rr][cc]
            out[r][c] = s
    return out


def compose_mixed(panels, path):
    """panels: [('bin', matrix[, color]) | ('gray', values), ...] 横向排布"""
    widths = [(len(p[1][0]) * CELL) for p in panels]
    heights = [(len(p[1]) * CELL) for p in panels]
    total_w = MARGIN * 2 + sum(widths) + GAP * (len(panels) - 1)
    total_h = MARGIN * 2 + max(heights)
    canvas = new_canvas(total_w, total_h)
    x = MARGIN
    for p, w in zip(panels, widths):
        if p[0] == 'gray':
            draw_gray_panel(canvas, p[1], x, MARGIN)
        else:
            color = p[2] if len(p) > 2 else FG
            draw_panel(canvas, p[1], x, MARGIN, color)
        x += w + GAP
    write_png(path, total_w, total_h, canvas)
    return total_w, total_h


def parse_matrices(text):
    """从演示输出里收集所有矩阵，返回 [(标签, matrix), ...]"""
    out = []
    label = ''
    buf = []
    for line in text.splitlines():
        s = line.strip()
        is_digit_row = bool(re.fullmatch(r'[01]( [01])*', s))
        is_char_row = bool(re.fullmatch(r'[.#]( [.#])*', s))
        if is_digit_row or is_char_row:
            buf.append([1 if t in '1#' else 0 for t in s.split()])
            continue
        if buf:
            out.append((label, buf))
            buf = []
        if s:
            label = s          # 任何非空行都作为紧随其后矩阵的标签
    if buf:
        out.append((label, buf))
    return out


def find(mats, *keywords):
    for label, m in mats:
        if all(k in label for k in keywords):
            return m
    return None


def compose(panels, path):
    """panels: [(matrix, color), ...] 横向排布"""
    heights = [len(m) * CELL for m, _ in panels]
    widths = [len(m[0]) * CELL for m, _ in panels]
    total_w = MARGIN * 2 + sum(widths) + GAP * (len(panels) - 1)
    total_h = MARGIN * 2 + max(heights)
    canvas = new_canvas(total_w, total_h)
    x = MARGIN
    for (m, color), w in zip(panels, widths):
        draw_panel(canvas, m, x, MARGIN, color)
        x += w + GAP
    write_png(path, total_w, total_h, canvas)
    return total_w, total_h


def main():
    demo = sys.argv[1] if len(sys.argv) > 1 else 'demo_output.txt'
    outdir = sys.argv[2] if len(sys.argv) > 2 else 'docs/img'
    os.makedirs(outdir, exist_ok=True)

    text = open(demo, encoding='utf-8').read()
    mats = parse_matrices(text)
    print(f'解析到 {len(mats)} 个矩阵：')
    for label, m in mats:
        print(f'  {m[0].__len__()}x{len(m)}  {label[:60]}')

    made = []

    # 图1：任务书 12x12 示例 —— 输入 vs 3x3 膨胀
    inp = find(mats, '[输入图像]')
    out3 = find(mats, '[输出图像]')
    if inp and out3:
        p = os.path.join(outdir, 'fig1_example_12x12.png')
        compose([(inp, FG_ALT), (out3, FG)], p)
        made.append(p)

    # 图2：5x5 结构元素膨胀
    out5 = find(mats, '5x5 结构元素')
    if out5 is None and len(mats) > 3:
        for label, m in mats:
            if '5x5' in label and 'Rect' in label:
                out5 = m
                break
    if out5:
        p = os.path.join(outdir, 'fig2_dilate5x5.png')
        compose([(inp or out5, FG_ALT), (out5, FG)], p)
        made.append(p)

    # 图3：结构元素可替换（原图 + 方形/椭圆/十字，均为 11x11，用第 3 节的精确标签）
    src = find(mats, '[原图')
    se_rect = find(mats, '--- 结构元素: Rect(5x5)')
    se_ell = find(mats, '--- 结构元素: Ellipse')
    se_cross = find(mats, '--- 结构元素: Cross')
    panels = [(m, FG) for m in (se_rect, se_ell, se_cross) if m]
    if src and panels:
        p = os.path.join(outdir, 'fig3_se_compare.png')
        compose([(src, FG_ALT)] + panels, p)
        made.append(p)

    # 图4：腐蚀（原图 + 腐蚀结果）
    er_src = find(mats, '11x11 实心方块做 Rect(5x5) 腐蚀')
    if er_src and src:
        p = os.path.join(outdir, 'fig4_erode.png')
        compose([(src, FG_ALT), (er_src, FG)], p)
        made.append(p)

    # 图5：同一个 3x3 窗口 —— 卷积模糊 vs 形态学膨胀
    if inp and out3:
        sums = conv_sum(inp, 3)
        mean = [[v / 9.0 for v in row] for row in sums]
        thresh = [[1 if v > 0 else 0 for v in row] for row in sums]
        p = os.path.join(outdir, 'fig5_conv_vs_dilate.png')
        compose_mixed([('bin', inp, FG_ALT), ('gray', mean), ('bin', thresh, FG)], p)
        made.append(p)

        same = all(thresh[r][c] == out3[r][c]
                   for r in range(len(out3)) for c in range(len(out3[0])))
        erode3 = sum(1 for row in sums for v in row if v == 9)
        from collections import Counter
        dist = dict(sorted(Counter(v for row in sums for v in row).items()))
        print('\n--- 卷积与形态学的对账 ---')
        print(f'3x3 全 1 核卷积邻域和分布: {dist}')
        print(f'卷积结果 >0 阈值化 == 3x3 膨胀: {same}')
        print(f'卷积邻域和 == 9 的格数: {erode3}（3x3 腐蚀在示例图上的前景数）')

    print('\n生成图片：')
    for p in made:
        print(f'  {p}  ({os.path.getsize(p)} 字节)')


if __name__ == '__main__':
    main()
