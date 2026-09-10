#!/bin/bash
# 2027 赛季算法组竞培营 —— 第一周环境配置（Ubuntu 22.04 on WSL2）
# 用法(在 Windows 侧)：wsl -d Ubuntu-22.04 -u root -- bash /mnt/d/User/_env_setup/setup_ubuntu.sh
LOG=/mnt/d/User/_env_setup/setup_ubuntu.log
exec > >(tee "$LOG") 2>&1
set -x
export DEBIAN_FRONTEND=noninteractive

step() { echo; echo "################ $* ################"; }

step "1. 系统信息"
cat /etc/os-release
uname -a
id
nproc
free -h
df -h / | tail -1

step "2. 网络连通性"
curl -sS -m 20 -o /dev/null -w "TUNA       HTTP=%{http_code} speed=%{speed_download}B/s\n" https://mirrors.tuna.tsinghua.edu.cn/ || echo "TUNA 失败"
curl -sS -m 20 -o /dev/null -w "DockerHub  HTTP=%{http_code}\n" https://registry-1.docker.io/v2/ || echo "DockerHub 失败"
curl -sS -m 20 -o /dev/null -w "GitHub     HTTP=%{http_code}\n" https://github.com || echo "GitHub 失败"

step "3. apt 换清华源"
if [ ! -f /etc/apt/sources.list.dsh.bak ]; then cp /etc/apt/sources.list /etc/apt/sources.list.dsh.bak; fi
sed -i 's|http://archive.ubuntu.com/ubuntu|https://mirrors.tuna.tsinghua.edu.cn/ubuntu|g; s|http://security.ubuntu.com/ubuntu|https://mirrors.tuna.tsinghua.edu.cn/ubuntu|g' /etc/apt/sources.list
grep -v '^#' /etc/apt/sources.list | grep -v '^$'

step "4. 安装基础工具链 (git/gcc/cmake/python)"
apt-get update -y
apt-get install -y --no-install-recommends \
    build-essential cmake gdb git curl wget vim nano unzip zip \
    python3 python3-pip python3-venv ca-certificates gnupg lsb-release \
    net-tools iputils-ping pkg-config
echo "--- 版本 ---"
gcc --version | head -1
g++ --version | head -1
cmake --version | head -1
git --version
python3 --version

step "5. pip 换清华源"
mkdir -p /etc/pip
cat > /etc/pip.conf <<'EOF'
[global]
index-url = https://pypi.tuna.tsinghua.edu.cn/simple
trusted-host = pypi.tuna.tsinghua.edu.cn
EOF
cat /etc/pip.conf

step "6. 创建普通用户 ubuntu"
if ! id -u ubuntu >/dev/null 2>&1; then
    useradd -m -s /bin/bash -G sudo ubuntu
    echo 'ubuntu:ubuntu' | chpasswd
    echo 'ubuntu ALL=(ALL) NOPASSWD:ALL' > /etc/sudoers.d/ubuntu
    chmod 440 /etc/sudoers.d/ubuntu
else
    usermod -aG sudo ubuntu
    echo 'ubuntu ALL=(ALL) NOPASSWD:ALL' > /etc/sudoers.d/ubuntu
    chmod 440 /etc/sudoers.d/ubuntu
fi
id ubuntu

step "7. Git 全局配置 + SSH 密钥"
runuser -u ubuntu -- git config --global user.name "miles"
runuser -u ubuntu -- git config --global user.email "3281458786@qq.com"
runuser -u ubuntu -- git config --global init.defaultBranch main
runuser -u ubuntu -- git config --global core.autocrlf input
runuser -u ubuntu -- git config --global pull.rebase false
runuser -u ubuntu -- git config --global credential.helper store
runuser -u ubuntu -- git config --global --list
if [ ! -f /home/ubuntu/.ssh/id_ed25519 ]; then
    runuser -u ubuntu -- ssh-keygen -t ed25519 -N "" -C "3281458786@qq.com" -f /home/ubuntu/.ssh/id_ed25519
fi
chmod 700 /home/ubuntu/.ssh; chmod 600 /home/ubuntu/.ssh/id_ed25519
echo "----- 公钥（添加到 GitHub -> Settings -> SSH keys）-----"
cat /home/ubuntu/.ssh/id_ed25519.pub

step "8. 安装 Docker (docker.io)"
apt-get install -y docker.io
usermod -aG docker ubuntu
docker --version
systemctl --version 2>/dev/null | head -1 || echo "(systemd 尚未启用，重启 WSL 后生效)"

step "9. wsl.conf (systemd + 默认用户)"
cat > /etc/wsl.conf <<'EOF'
[boot]
systemd=true

[user]
default=ubuntu

[interop]
enabled=true
appendWindowsPath=true
EOF
cat /etc/wsl.conf

step "10. 完成"
echo "OS:        $(. /etc/os-release; echo $PRETTY_NAME)"
echo "Kernel:    $(uname -r)"
echo "gcc:       $(gcc -dumpversion)"
echo "cmake:     $(cmake --version | head -1)"
echo "docker:    $(docker --version)"
echo "下一步: Windows 侧执行 wsl --shutdown 让 systemd 生效"
