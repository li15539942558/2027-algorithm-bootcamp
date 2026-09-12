# 2027 赛季算法组竞培营 · 第一周任务

| 内容 | 位置 |
|---|---|
| 提交文档（Markdown，导出 PDF 用） | `2027赛季算法组竞培营第一周任务_提交文档.md` |
| 任务一：ROS2 发布 / 订阅 + Logger | `task1_ros2_pkg/` |
| 任务二：二值图像区域膨胀算法（C++17） | `task2_cpp_dilation/` |

## 任务一：编译运行

```bash
cp -r task1_ros2_pkg ~/ros2_ws/src/                     # 拷进 ROS2 workspace
cd ~/ros2_ws && colcon build --packages-select rm_bootcamp_pubsub
source install/setup.bash

ros2 run rm_bootcamp_pubsub talker_node                 # 终端 A：发布
ros2 run rm_bootcamp_pubsub listener_node               # 终端 B：订阅 + Logger 打印
```

## 任务二：编译运行

```bash
cmake -S task2_cpp_dilation -B build && cmake --build build -j
./build/morph_demo && ./build/morph_tests      # 测试应输出: 通过: 17  失败: 0
```

详细说明见各目录下的 `README.md`。
