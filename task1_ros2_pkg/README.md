# 任务一：ROS2 简单的消息发布 / 订阅与日志输出

一个 ROS2（ament_cmake）功能包，包含**两个可分别执行的节点**：

| 可执行文件 | 源码 | 作用 |
|---|---|---|
| `talker_node` | `src/publisher_node.cpp` | 每 500 ms 向话题 `bootcamp_topic` 发布一条 `std_msgs::msg::String` |
| `listener_node` | `src/subscriber_node.cpp` | 订阅同一话题，收到后用 **Logger**（`RCLCPP_INFO`）打印接收到的数据 |

## 目录结构

```
task1_ros2_pkg/
├── package.xml
├── CMakeLists.txt
└── src/
    ├── publisher_node.cpp   # 发布者
    └── subscriber_node.cpp  # 订阅者
```

## 放到 workspace 里编译

```bash
# 1) 进入你的 ROS2 workspace（本文档示例用 ~/ros2_ws）
mkdir -p ~/ros2_ws/src && cd ~/ros2_ws/src
cp -r /path/to/task1_ros2_pkg .          # 把本包拷进来

# 2) 编译
cd ~/ros2_ws
colcon build --packages-select rm_bootcamp_pubsub
source install/setup.bash

# 3) 运行（开两个终端）
# 终端 A：发布者
ros2 run rm_bootcamp_pubsub talker_node

# 终端 B：订阅者
ros2 run rm_bootcamp_pubsub listener_node
```

## 预期输出

终端 A（发布者）：
```
[INFO] [talker_node]: 发布者节点已启动, 话题: bootcamp_topic
[INFO] [talker_node]: 已发布: "2027赛季算法组竞培营 第 1 条消息 | 时间戳: 123.456"
[INFO] [talker_node]: 已发布: "2027赛季算法组竞培营 第 2 条消息 | 时间戳: 123.956"
...
```

终端 B（订阅者，Logger 打印收到数据）：
```
[INFO] [listener_node]: 订阅者节点已启动, 正在监听话题: bootcamp_topic
[INFO] [listener_node]: 收到数据: "2027赛季算法组竞培营 第 1 条消息 | 时间戳: 123.456"
[INFO] [listener_node]: 收到数据: "2027赛季算法组竞培营 第 2 条消息 | 时间戳: 123.956"
...
```

## 常用排查命令

```bash
ros2 node list                 # 看两个节点是否都在
ros2 topic list                # 看话题 /bootcamp_topic
ros2 topic echo /bootcamp_topic  # 直接看话题内容
ros2 topic hz /bootcamp_topic    # 看发布频率（约 2 Hz）
```

> 说明：本包在 Windows 主机上无法编译运行（ROS2 环境在 Docker 镜像 `qzhhhi/rmcs-develop:latest` 里），
> 请在 Linux/Docker 环境中按上面步骤操作并截图。
