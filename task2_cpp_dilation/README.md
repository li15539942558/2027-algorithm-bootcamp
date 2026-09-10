# 任务二：二值图像形态学处理（膨胀 / 腐蚀）

C++17 实现，**面向对象 + 模块化**设计：结构元素（Structuring Element）与形态学算子（Morphology Operator）
都是抽象基类，可自由替换/扩展（例如把 5×5 方形换成圆形、十字，或新增"方向性膨胀"）。

## 目录结构

```
task2_cpp_dilation/
├── CMakeLists.txt          # cmake 构建脚本（库 + 演示程序 + 单元测试）
├── include/
│   └── morphology.hpp      # 全部算法实现（头文件库，便于复用）
├── src/
│   └── main.cpp            # 演示程序：任务书示例复现 / 5x5 膨胀 / 元素替换 / 腐蚀 / 方向性膨胀 / 可视化
└── tests/
    └── test_morphology.cpp # 自写测试样例（17 条断言）
```

## 设计（面向对象）

| 类 | 职责 |
|---|---|
| `StructuringElement`（抽象） | 结构元素基类：`offsets()` 返回覆盖的所有偏移；`setAnchor()` 支持锚点平移 |
| `RectElement / CrossElement / EllipseElement` | 具体结构元素：矩形(W×H)、十字(半径)、椭圆(半轴)——**可替换** |
| `BinaryImage` | 二值图像容器：读写像素、打印（数字矩阵/字符画）、导出 PGM 图片 |
| `MorphologyOperator`（抽象） | 形态学算子基类：`apply(src, se) → dst` |
| `DilateOp / ErodeOp / DirectionalDilateOp` | 膨胀 / 腐蚀 / 方向性膨胀（锚点平移） |

膨胀与腐蚀的定义（集合观点，`SE` 为结构元素）：

```
膨胀 (Minkowski 和)： dst(x) = 1  ⟺  ∃ s ∈ SE : src(x - s) = 1
腐蚀 (Minkowski 差)： dst(x) = 1  ⟺  ∀ s ∈ SE : src(x + s) = 1
```

越界像素一律按背景（0）处理，因此**图像尺寸不变、边界不会被凭空撑大**。

## 编译与运行

### 方式一：cmake（推荐）

```bash
cmake -S . -B build
cmake --build build -j
./build/morph_demo        # 演示
./build/morph_tests       # 单元测试
ctest --test-dir build    # 用 ctest 跑测试
```

### 方式二：直接 g++

```bash
g++ -std=c++17 -Wall -Wextra -Iinclude src/main.cpp -o morph_demo
g++ -std=c++17 -Wall -Wextra -Iinclude tests/test_morphology.cpp -o morph_tests
./morph_demo && ./morph_tests
```

## 运行结果（实测）

- 任务书示例（12×12 + 3×3 方形）→ 与任务书给出的期望矩阵**逐格完全一致**
- 5×5 结构元素膨胀：前景 16 → 86 像素
- 结构元素替换（同一 11×11 实心方块）：
  | 结构元素 | 结果像素 | 形状特征 |
  |---|---|---|
  | `Rect(5x5)` | 81 | 9×9 正方形，尖角 |
  | `Ellipse(2,2)` | 69 | 圆角外扩（更接近圆形） |
  | `Cross(r=2)` | 65 | 四角被切掉，菱形/星形 |
- 腐蚀：5×5 方块被 5×5 结构元素腐蚀后只剩 1 个中心像素
- 单元测试：**17 / 17 通过**

## 可视化

- 终端字符画（`#` = 前景，`.` = 背景）
- 导出 PGM 灰度图（`out_input.pgm` / `out_dilate5x5.pgm` / `out_erode.pgm`），
  可用看图软件打开或转 PNG 放进文档
