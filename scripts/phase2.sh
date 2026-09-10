#!/bin/bash
# 阶段二：修复 Docker Hub 连通性 + 配置镜像加速 + 原生 ROS2 Humble(清华源)
LOG=/mnt/d/User/_env_setup/phase2.log
exec > >(tee "$LOG") 2>&1
set -x
export DEBIAN_FRONTEND=noninteractive

echo "################ 1. DNS 状况 ################"
cat /etc/resolv.conf
echo "--- getent registry-1.docker.io ---"
getent hosts registry-1.docker.io || echo "(解析失败)"
echo "--- 强制用 223.5.5.5 解析 ---"
command -v nslookup >/dev/null && nslookup registry-1.docker.io 223.5.5.5 | tail -6 || echo "(无 nslookup)"

echo "################ 2. 磁盘空间 ################"
df -h / /mnt/d | tail -3

echo "################ 3. 测试 Docker Hub 镜像加速站 ################"
MIRRORS="https://docker.m.daocloud.io https://docker.1ms.run https://docker.xuanyuan.me https://docker.1panel.live https://hub.rat.dev https://dockerpull.org https://registry.dockermirror.com https://mirror.ccs.tencentyun.com https://hub-mirror.c.163.com https://docker.1panel.top"
OK=""
for m in $MIRRORS; do
    code=$(curl -s -m 8 -o /dev/null -w "%{http_code}" "$m/v2/" || echo "000")
    echo "$m -> HTTP $code"
    if [ "$code" = "200" ] || [ "$code" = "401" ]; then OK="$OK $m"; fi
done
echo "可用镜像: $OK"

echo "################ 4. 写入 /etc/docker/daemon.json ################"
mkdir -p /etc/docker
{
  echo "{"
  echo '  "registry-mirrors": ['
  first=1
  for m in $OK; do
      if [ $first -eq 1 ]; then first=0; else echo ","; fi
      printf '    "%s"' "$m"
  done
  echo
  echo "  ],"
  echo '  "dns": ["223.5.5.5", "119.29.29.29"],'
  echo '  "log-driver": "json-file",'
  echo '  "log-opts": { "max-size": "50m", "max-file": "3" },'
  echo '  "features": { "buildkit": true }'
  echo "}"
} > /etc/docker/daemon.json
cat /etc/docker/daemon.json
python3 -c "import json;json.load(open('/etc/docker/daemon.json'));print('daemon.json JSON 合法')" || echo "JSON 非法!"

systemctl restart docker
sleep 4
systemctl is-active docker

echo "################ 5. Docker 拉取测试 ################"
docker pull hello-world && docker run --rm hello-world | head -6 || echo "hello-world 仍失败"

echo "################ 6. rmcs-develop 镜像清单 ################"
for m in $OK; do
    out=$(curl -s -m 20 -H "Accept: application/vnd.docker.distribution.manifest.list.v2+json,application/vnd.oci.image.index.v1+json,application/vnd.docker.distribution.manifest.v2+json" "$m/v2/qzhhhi/rmcs-develop/manifests/latest")
    if echo "$out" | grep -q '"layers"\|"manifests"'; then
        echo "=== 通过 $m 获取成功 ==="
        echo "$out" | head -c 800; echo
        echo "$out" > /tmp/rmcs_manifest.json
        break
    else
        echo "$m -> $(echo "$out" | head -c 120)"
    fi
done
python3 - <<'PY'
import json, os
p = "/tmp/rmcs_manifest.json"
if os.path.exists(p):
    d = json.load(open(p))
    if "manifests" in d:
        print("多架构:", [(m["platform"]["os"], m["platform"]["architecture"]) for m in d["manifests"]])
        for m in d["manifests"]:
            print("   ", m["digest"], m["size"]/1048576, "MB (索引项)")
    else:
        tot = sum(l["size"] for l in d.get("layers", []))
        print("层数=%d 压缩总大小=%.1f MB" % (len(d.get("layers", [])), tot/1048576))
else:
    print("未获取到 manifest")
PY

echo "################ 7. 原生 ROS2 Humble（清华镜像）################"
apt-get install -y curl gnupg lsb-release
GOT_KEY=0
curl -sSL -m 30 https://raw.githubusercontent.com/ros/rosdistro/master/ros.key -o /usr/share/keyrings/ros-archive-keyring.gpg && GOT_KEY=1
if [ $GOT_KEY -ne 1 ]; then
    echo "raw.githubusercontent 取 key 失败，改用 keyserver"
    gpg --keyserver keyserver.ubuntu.com --recv-keys C1CF6E31E6BADE8868B172B4F42ED6FBAB17C654 && \
      gpg --export C1CF6E31E6BADE8868B172B4F42ED6FBAB17C654 > /usr/share/keyrings/ros-archive-keyring.gpg && GOT_KEY=1
fi
if [ $GOT_KEY -eq 1 ]; then
    echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] https://mirrors.tuna.tsinghua.edu.cn/ros2/ubuntu $(. /etc/os-release && echo $UBUNTU_CODENAME) main" > /etc/apt/sources.list.d/ros2.list
    cat /etc/apt/sources.list.d/ros2.list
    apt-get update -y
    apt-get install -y ros-humble-ros-base ros-dev-tools python3-colcon-common-extensions python3-rosdep
    echo "--- ROS2 版本 ---"
    runuser -u ubuntu -- bash -lc 'source /opt/ros/humble/setup.bash && echo ROS_DISTRO=$ROS_DISTRO && ros2 --help | head -3'
else
    echo "!! ROS2 密钥获取失败，跳过原生安装"
fi

echo "################ 8. 完成 ################"
df -h / | tail -1
