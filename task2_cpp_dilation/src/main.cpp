// main.cpp —— 二值图像形态学处理演示程序（任务二）
// 内容：
//   1) 复现任务书示例：12x12 二值图 + 3x3 结构元素 → 与任务书给出的期望结果逐格比对
//   2) 任务要求：5x5 结构元素膨胀
//   3) 结构元素可替换演示：方形 vs 十字 vs 椭圆（得到"圆角/圆形"膨胀结果）
//   4) 反向算子：腐蚀
//   5) 方向性膨胀（平移锚点 → 只朝一个方向扩展）
//   6) 可视化：终端字符画 + 导出 PGM 图片
#include "morphology.hpp"

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

using namespace morph;

static const std::vector<std::vector<int>> kDemoInput = {
    {0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,1,1,0,0,0,0,0},
    {0,0,0,0,1,0,1,0,0,0,0,0},
    {0,0,0,0,0,1,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,1,0,0,0,0},
    {0,0,0,0,0,1,1,1,0,0,0,0},
    {0,0,0,0,0,0,0,1,1,0,0,0},
    {0,0,0,0,0,0,0,1,1,0,0,0},
    {0,0,0,0,0,0,0,0,1,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0},
};

static const std::vector<std::vector<int>> kDemoExpected3x3 = {
    {0,0,0,0,1,1,1,1,0,0,0,0},
    {0,0,0,1,1,1,1,1,0,0,0,0},
    {0,0,0,1,1,1,1,1,0,0,0,0},
    {0,0,0,1,1,1,1,1,0,0,0,0},
    {0,0,0,0,1,1,1,1,1,0,0,0},
    {0,0,0,0,1,1,1,1,1,0,0,0},
    {0,0,0,0,1,1,1,1,1,1,0,0},
    {0,0,0,0,1,1,1,1,1,1,0,0},
    {0,0,0,0,0,0,1,1,1,1,0,0},
    {0,0,0,0,0,0,1,1,1,1,0,0},
    {0,0,0,0,0,0,0,1,1,1,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0},
};

static void title(const std::string& s) {
    std::cout << "\n================ " << s << " ================\n";
}

int main() {
    std::cout << "########## 任务二：二值图像形态学处理（膨胀/腐蚀）演示 ##########\n";

    const BinaryImage src = BinaryImage::fromRows(kDemoInput);
    const BinaryImage expect3x3 = BinaryImage::fromRows(kDemoExpected3x3);

    // ---------- 1) 复现任务书示例（3x3 方形结构元素） ----------
    title("1) 任务书示例：12x12 输入 + 3x3 方形结构元素 → 膨胀");
    std::cout << "[输入图像]  前景像素数 = " << src.count() << "\n";
    src.printNumbers(std::cout);

    RectElement se3(3, 3);
    DilateOp dilate;
    BinaryImage out3 = dilate.apply(src, se3);

    std::cout << "\n[输出图像] 结构元素 = " << se3.name()
              << "  前景像素数 = " << out3.count() << "\n";
    out3.printNumbers(std::cout);

    std::cout << "\n与任务书给出的期望结果逐格比对: "
              << ((out3 == expect3x3) ? "完全一致 ✔" : "不一致 ✘") << "\n";

    // ---------- 2) 任务要求：5x5 结构元素 ----------
    title("2) 任务要求：使用 5x5 结构元素进行膨胀");
    RectElement se5(5, 5);
    BinaryImage out5 = dilate.apply(src, se5);
    std::cout << "结构元素 = " << se5.name()
              << "   前景: " << src.count() << " → " << out5.count() << "\n";
    out5.print(std::cout);

    // ---------- 3) 结构元素可替换：方形 / 十字 / 椭圆 ----------
    title("3) 结构元素可替换：同一图像、不同结构元素");
    const int w = 11, h = 11;
    BinaryImage square(h, w);
    for (int r = 3; r <= 7; ++r) for (int c = 3; c <= 7; ++c) square.set(r, c, 1);

    struct Demo { std::string label; const StructuringElement* se; };
    RectElement    seRect(5, 5);
    CrossElement   seCross(2);
    EllipseElement seDisk(2, 2);
    std::vector<Demo> demos = {
        {"5x5 方形 (尖角, 正方形外扩)", &seRect},
        {"十字 r=2 (菱形/星形外扩)",    &seCross},
        {"椭圆 rx=ry=2 (圆角外扩)",     &seDisk},
    };
    std::cout << "[原图 11x11 实心方块]\n";
    square.print(std::cout);
    for (const auto& d : demos) {
        BinaryImage o = dilate.apply(square, *d.se);
        std::cout << "\n--- 结构元素: " << d.se->name() << "  （" << d.label << "）\n";
        o.print(std::cout);
        std::cout << "前景像素数 = " << o.count() << "\n";
    }
    std::cout << "\n结论: 膨胀结果的形状由结构元素决定 —— 想要圆角/圆形外扩，"
                 "把结构元素换成圆形(椭圆)即可; 想只往某方向扩展, 平移锚点即可(见第 5 节)。\n";

    // ---------- 4) 腐蚀 ----------
    title("4) 反向操作：腐蚀（与膨胀对偶）");
    ErodeOp erode;
    BinaryImage e3 = erode.apply(src, se3);
    std::cout << "对任务书示例图像做 " << se3.name() << " 腐蚀"
              << "   前景: " << src.count() << " → " << e3.count() << "\n";
    e3.print(std::cout);
    BinaryImage eSquare = erode.apply(square, seRect);
    std::cout << "\n对 11x11 实心方块做 " << seRect.name() << " 腐蚀: 前景 "
              << square.count() << " → " << eSquare.count() << "  (由 5x5 方块缩成 1x1)\n";
    eSquare.print(std::cout);

    // ---------- 5) 方向性膨胀（锚点平移） ----------
    title("5) 方向性膨胀：平移锚点，使膨胀只朝指定方向");
    DirectionalDilateOp dilateRightDown(2, 2);   // 锚点移到右下 → 只向右下扩展
    BinaryImage od = dilateRightDown.apply(square, seRect);
    std::cout << "结构元素 = " << seRect.name() << " , 锚点 = (2,2) → 结果向右下方向扩展\n";
    od.print(std::cout);

    // ---------- 6) 可视化：导出 PGM ----------
    title("6) 可视化（选做）：导出 PGM 图片");
    try {
        src.savePGM("out_input.pgm");
        out5.savePGM("out_dilate5x5.pgm");
        eSquare.savePGM("out_erode.pgm");
        std::cout << "已导出: out_input.pgm / out_dilate5x5.pgm / out_erode.pgm\n"
                     "（PGM 为无损灰度图，可直接用看图软件或转成 PNG 放进文档）\n";
    } catch (const std::exception& e) {
        std::cout << "导出失败: " << e.what() << "\n";
    }

    std::cout << "\n全部演示结束。\n";
    return 0;
}
