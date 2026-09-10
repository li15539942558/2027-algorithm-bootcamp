#!/bin/bash
# 收尾：为 ubuntu 用户配置 ROS2 自动 source + 常用别名 + 欢迎信息
UB=/home/ubuntu

# .bashrc 追加 ROS2 与环境配置
if ! grep -q "ROS2 Humble 自动加载" $UB/.bashrc 2>/dev/null; then
cat >> $UB/.bashrc <<'EOF'

# ===== ROS2 Humble 自动加载（竞培营环境）=====
if [ -f /opt/ros/humble/setup.bash ]; then
    source /opt/ros/humble/setup.bash
fi
if [ -f ~/ros2_ws/install/setup.bash ]; then
    source ~/ros2_ws/install/setup.bash
fi
export ROS_DOMAIN_ID=42
export RCUTILS_COLORIZED_OUTPUT=1

# ===== 常用别名 =====
alias cb='colcon build --symlink-install'
alias cbs='colcon build --symlink-install && source install/setup.bash'
alias st='source install/setup.bash'
alias nodes='ros2 node list'
alias topics='ros2 topic list'
alias dk='docker'
alias ll='ls -alF'
EOF
fi
chown ubuntu:ubuntu $UB/.bashrc

# 每次登录的提示
if ! grep -q "竞培营环境已就绪" $UB/.bashrc; then
cat >> $UB/.bashrc <<'EOF'
echo ""
echo "===== 2027 赛季算法组竞培营 · 第一周环境 ====="
echo "  OS     : $(. /etc/os-release; echo $PRETTY_NAME)"
echo "  ROS2   : ${ROS_DISTRO:-(未加载)}"
echo "  工具链 : gcc $(gcc -dumpversion) / cmake $(cmake --version | head -1 | awk '{print $3}') / $(git --version | awk '{print $1" "$3}')"
echo "  Docker : $(docker --version 2>/dev/null | awk '{print $3}' | tr -d ,)"
echo "  示例   : cd ~/ros2_ws && cb   (colcon build)"
echo "=============================================="
echo ""
EOF
fi
chown ubuntu:ubuntu $UB/.bashrc

echo "--- .bashrc 末尾 ---"
tail -20 $UB/.bashrc

echo "--- 验证：新 shell 是否自动加载 ROS2 ---"
runuser -u ubuntu -- bash -lc 'echo ROS_DISTRO=$ROS_DISTRO; which ros2; ros2 pkg list 2>/dev/null | wc -l'
