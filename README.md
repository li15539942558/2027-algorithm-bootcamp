# 2027 赛季算法组竞培营 · 第一周任务

| 内容 | 位置 |
|---|---|
| **提交文档（Markdown，导出 PDF 用）** | `2027赛季算法组竞培营第一周任务_提交文档.md` |
| 任务一：ROS2 发布/订阅 + Logger | `task1_ros2_pkg/` |
| 任务二：二值图像区域膨胀算法（C++17） | `task2_cpp_dilation/` |

## 任务二：快速验证（本机已实测通过）

```bash
# cmake（推荐）
cmake -S task2_cpp_dilation -B build && cmake --build build -j
./build/morph_demo && ./build/morph_tests      # 测试应输出: 通过: 17  失败: 0

# 或直接 g++
cd task2_cpp_dilation
g++ -std=c++17 -Wall -Wextra -Iinclude src/main.cpp -o morph_demo
g++ -std=c++17 -Wall -Wextra -Iinclude tests/test_morphology.cpp -o morph_tests
./morph_demo && ./morph_tests
```

已完成内容：
- 膨胀（`DilateOp`）、腐蚀（`ErodeOp`）、方向性膨胀（`DirectionalDilateOp`）
- 结构元素可替换：`RectElement` / `CrossElement` / `EllipseElement`（抽象基类 `StructuringElement`）
- 任务书 12×12 示例**逐格复现一致**；5×5 结构元素膨胀；17 条单元测试全部通过
- 可视化：终端字符画 + PGM 图片导出（选做）

## 任务一：在 ROS2 环境（Docker 镜像）中运行

见 `task1_ros2_pkg/README.md`：拷进 `~/ros2_ws/src` → `colcon build` → 两个终端分别
`ros2 run rm_bootcamp_pubsub talker_node` / `listener_node`，订阅端用 Logger 打印收到的数据。

## 提交前

1. 把提交文档中所有 `📷 截图位置` 换成真实截图；
2. 把仓库链接换成你自己的 GitHub 仓库地址；
3. Markdown 导出为 PDF 提交（Typora / VS Code Markdown PDF 插件）。
