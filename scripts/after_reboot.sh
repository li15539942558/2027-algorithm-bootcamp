#!/bin/bash
# 重启 WSL 后的验证与 Docker/ROS2 准备
LOG=/mnt/d/User/_env_setup/after_reboot.log
exec > >(tee "$LOG") 2>&1
set -x

echo "################ 1. systemd 状态 ################"
systemctl is-system-running || true
systemctl --version | head -1

echo "################ 2. 启动 Docker 服务 ################"
systemctl enable --now docker || true
sleep 3
systemctl is-active docker || true
systemctl is-enabled docker || true
docker info --format 'ServerVersion={{.ServerVersion}} Driver={{.Driver}} Cgroup={{.CgroupVersion}}' || true

echo "################ 3. Docker 冒烟测试 (hello-world) ################"
time docker pull hello-world
docker run --rm hello-world | head -8

echo "################ 4. Docker Hub 上 rmcs-develop 镜像大小 ################"
TOKEN=$(curl -s -m 30 "https://auth.docker.io/token?service=registry.docker.io&scope=repository:qzhhhi/rmcs-develop:pull" | python3 -c "import sys,json;print(json.load(sys.stdin).get('token',''))" 2>/dev/null)
if [ -n "$TOKEN" ]; then
  curl -s -m 30 -H "Authorization: Bearer $TOKEN" \
       -H "Accept: application/vnd.docker.distribution.manifest.list.v2+json,application/vnd.oci.image.index.v1+json,application/vnd.docker.distribution.manifest.v2+json" \
       https://registry-1.docker.io/v2/qzhhhi/rmcs-develop/manifests/latest > /tmp/manifest.json
  head -c 600 /tmp/manifest.json; echo
  python3 - <<'PY'
import json
try:
    d = json.load(open("/tmp/manifest.json"))
    if "manifests" in d:
        print("多架构清单:", [(m["platform"]["os"], m["platform"]["architecture"]) for m in d["manifests"]])
    else:
        tot = sum(l["size"] for l in d.get("layers", []))
        print("层数=%d 压缩总大小=%.1f MB" % (len(d.get("layers", [])), tot/1048576))
except Exception as e:
    print("解析失败:", e)
PY
else
  echo "无法获取 Docker Hub token（网络受限）"
fi

echo "################ 5. 用户环境检查 (ubuntu) ################"
runuser -u ubuntu -- bash -lc 'whoami; id; docker --version; groups'
echo "--- git ---"
runuser -u ubuntu -- git config --global --list
echo "--- 公钥 ---"
cat /home/ubuntu/.ssh/id_ed25519.pub

echo "################ 6. 完成 ################"
