"""校验生成的 PNG 图：解码后数前景格子，与程序输出对账（纯标准库）。"""
import os
import struct
import sys
import zlib

FG = (44, 111, 187)       # 膨胀/腐蚀结果
FG_ALT = (219, 122, 45)   # 原图
CELL = 22
CELL_AREA = CELL * CELL

EXPECT = {
    'fig1_example_12x12.png': (16, 52, '任务书示例：输入 16 → 3x3 膨胀 52'),
    'fig2_dilate5x5.png': (16, 86, '5x5 结构元素：输入 16 → 膨胀 86'),
    'fig3_se_compare.png': (25, 81 + 69 + 65, '结构元素替换：原图 25 → 方形81+椭圆69+十字65'),
    'fig4_erode.png': (25, 1, '腐蚀：5x5 方块 25 → 1'),
}


def read_png(path):
    data = open(path, 'rb').read()
    assert data[:8] == b'\x89PNG\r\n\x1a\n', '不是 PNG'
    pos = 8
    idat = b''
    width = height = None
    while pos < len(data):
        ln = struct.unpack('>I', data[pos:pos + 4])[0]
        tag = data[pos + 4:pos + 8]
        body = data[pos + 8:pos + 8 + ln]
        if tag == b'IHDR':
            width, height = struct.unpack('>II', body[:8])
        elif tag == b'IDAT':
            idat += body
        pos += 12 + ln
    raw = zlib.decompress(idat)
    stride = width * 3 + 1
    rows = []
    for y in range(height):
        line = raw[y * stride:(y + 1) * stride]
        assert line[0] == 0, '仅支持无滤波行'
        rows.append([tuple(line[1 + x * 3:4 + x * 3]) for x in range(width)])
    return width, height, rows


def main():
    outdir = sys.argv[1] if len(sys.argv) > 1 else 'docs/img'
    ok = True
    for name, (exp_alt, exp_fg, desc) in EXPECT.items():
        path = os.path.join(outdir, name)
        if not os.path.exists(path):
            print(f'缺少 {name}')
            ok = False
            continue
        w, h, rows = read_png(path)
        alt = sum(row.count(FG_ALT) for row in rows) // CELL_AREA
        fg = sum(row.count(FG) for row in rows) // CELL_AREA
        good = (alt == exp_alt and fg == exp_fg)
        ok = ok and good
        print(f'{"OK  " if good else "FAIL"} {name:26s} {w}x{h}  原图格数={alt}(应 {exp_alt})  结果格数={fg}(应 {exp_fg})   {desc}')
    print('\n全部对账通过' if ok else '\n存在不一致')
    return 0 if ok else 1


if __name__ == '__main__':
    sys.exit(main())
