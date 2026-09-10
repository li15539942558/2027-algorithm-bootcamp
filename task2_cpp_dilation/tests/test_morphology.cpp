// tests/test_morphology.cpp —— 自写测试样例（任务要求：自行编写测试样例测试）
// 采用无依赖的轻量断言框架：统计通过/失败，失败时返回非 0
#include "morphology.hpp"

#include <iostream>
#include <vector>
#include <string>

using namespace morph;

static int g_pass = 0;
static int g_fail = 0;

static void check(bool cond, const std::string& what) {
    if (cond) { ++g_pass; std::cout << "  [PASS] " << what << "\n"; }
    else      { ++g_fail; std::cout << "  [FAIL] " << what << "\n"; }
}

// ---------- 测试 1：与任务书示例逐格比对（3x3 方形结构元素） ----------
static void test_doc_example() {
    std::cout << "[测试1] 任务书示例：12x12 + 3x3 方形 → 与期望矩阵逐格一致\n";
    const auto in = std::vector<std::vector<int>>{
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
    const auto expect = std::vector<std::vector<int>>{
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
    BinaryImage src = BinaryImage::fromRows(in);
    RectElement se(3, 3);
    BinaryImage out = DilateOp().apply(src, se);
    check(out == BinaryImage::fromRows(expect), "膨胀结果与任务书期望矩阵完全一致");
}

// ---------- 测试 2：单点 + 3x3 → 变成 3x3 全 1 ----------
static void test_single_point() {
    std::cout << "[测试2] 单点前景 + 3x3 方形 → 中心 3x3 全 1\n";
    BinaryImage src(5, 5);
    src.set(2, 2, 1);
    BinaryImage out = DilateOp().apply(src, RectElement(3, 3));
    check(out.count() == 9, "前景像素数 = 9");
    bool ok = true;
    for (int r = 1; r <= 3; ++r) for (int c = 1; c <= 3; ++c) if (!out.at(r, c)) ok = false;
    check(ok, "3x3 邻域全部变为前景");
}

// ---------- 测试 3：5x5 结构元素（任务要求） ----------
static void test_5x5() {
    std::cout << "[测试3] 5x5 方形结构元素膨胀：单点 → 5x5 全 1\n";
    BinaryImage src(9, 9);
    src.set(4, 4, 1);
    BinaryImage out = DilateOp().apply(src, RectElement(5, 5));
    check(out.count() == 25, "前景像素数 = 25");
    check(out.at(2, 2) && out.at(2, 6) && out.at(6, 2) && out.at(6, 6), "四个角落在 5x5 范围内被填充");
}

// ---------- 测试 4：结构元素可替换（同图不同元素 → 结果不同） ----------
static void test_replaceable_se() {
    std::cout << "[测试4] 结构元素可替换：方形 vs 十字 vs 椭圆\n";
    BinaryImage src(11, 11);
    for (int r = 3; r <= 7; ++r) for (int c = 3; c <= 7; ++c) src.set(r, c, 1);

    BinaryImage a = DilateOp().apply(src, RectElement(5, 5));      // 5x5 方块外扩 2 → 9x9
    BinaryImage b = DilateOp().apply(src, CrossElement(2));        // 十字 → 9x9 去掉四角
    BinaryImage c = DilateOp().apply(src, EllipseElement(2, 2));   // 圆盘 → 9x9 圆角

    check(a.count() == 81, "5x5 方形膨胀实心方块 → 9x9 = 81 像素(尖角外扩)");
    check(b.count() == 65, "十字(r=2)膨胀实心方块 → 65 像素(四角被切掉, 菱形外扩)");
    check(c.count() == 69, "椭圆(2,2)膨胀实心方块 → 69 像素(比十字更圆, 圆角外扩)");
    check(a.count() > c.count() && c.count() > b.count(),
          "像素数有序: 方形 81 > 椭圆 69 > 十字 65 (越接近圆形, 结果越圆润)");
    check(a != c, "换结构元素后结果不同 → 证明结构元素可替换");
}

// ---------- 测试 5：腐蚀（膨胀的对偶） ----------
static void test_erosion() {
    std::cout << "[测试5] 腐蚀：方块缩小、边界前景消失\n";
    BinaryImage src(11, 11);
    for (int r = 3; r <= 7; ++r) for (int c = 3; c <= 7; ++c) src.set(r, c, 1);
    BinaryImage out = ErodeOp().apply(src, RectElement(5, 5));
    check(out.count() == 1, "5x5 方块被 5x5 结构元素腐蚀 → 只剩 1 个中心像素");
    check(out.at(5, 5) == 1, "中心像素保留");

    BinaryImage src2(9, 9);
    for (int r = 2; r <= 6; ++r) for (int c = 2; c <= 6; ++c) src2.set(r, c, 1);
    BinaryImage out2 = ErodeOp().apply(src2, RectElement(3, 3));
    check(out2.count() == 9, "5x5 方块被 3x3 结构元素腐蚀 → 3x3=9 像素");
}

// ---------- 测试 6：边界处理（越界当背景，图像不被撑大） ----------
static void test_border() {
    std::cout << "[测试6] 边界处理：越界视为背景，尺寸保持不变\n";
    BinaryImage src(4, 4);
    src.set(0, 0, 1);
    BinaryImage out = DilateOp().apply(src, RectElement(3, 3));
    check(out.rows() == 4 && out.cols() == 4, "输出尺寸与输入相同");
    check(out.count() == 4, "左上角点膨胀后填充 (0,0)(0,1)(1,0)(1,1) 共 4 个像素");
}

// ---------- 测试 7：方向性膨胀（锚点平移） ----------
static void test_directional() {
    std::cout << "[测试7] 方向性膨胀：平移锚点后只朝一个方向扩展\n";
    BinaryImage src(7, 7);
    src.set(3, 3, 1);
    BinaryImage out = DirectionalDilateOp(2, 2).apply(src, RectElement(5, 5));
    check(out.at(3, 3) && out.at(5, 5), "向右下扩展: (5,5) 为前景");
    check(!out.at(1, 1), "左上方向未被扩展: (1,1) 仍为背景");
}

int main() {
    std::cout << "================ 形态学算法单元测试 ================\n";
    test_doc_example();
    test_single_point();
    test_5x5();
    test_replaceable_se();
    test_erosion();
    test_border();
    test_directional();
    std::cout << "====================================================\n";
    std::cout << "通过: " << g_pass << "   失败: " << g_fail << "\n";
    return g_fail == 0 ? 0 : 1;
}
