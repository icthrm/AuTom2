# autom2 构建指南

## 方式一：Docker 编译 + 提取 dist（推荐）

### 1. 准备镜像

**导入现成镜像：**
```bash
docker load -i autom2-dist.tar
```

**或自己构建（需联网）：**
```bash
docker build --network host -t autom2-dist .
```

### 2. 一键编译 + 收集库 + 打包

```bash
./build.sh
```

输出到 `./dist/`，含二进制 + 全部动态库，可直接分发，无需 Docker。

### 3. 仅提取镜像中的 dist（不改源码）

```bash
mkdir output
docker run --rm -v $(pwd)/output:/host autom2-dist \
    bash -c "cp -r /workspace/autom2/dist/* /host/"
```

---

## 方式二：本地编译

**依赖**：CMake 3.27+, GCC 11+, CUDA 12.6, Qt6, OpenCV 4.5, Eigen3, FFTW3, MPI, Ceres, NLopt

```bash
cmake -B build -S .
cmake --build build -j$(nproc)
```

---

## 环境要求

| 项目 | 要求 |
|------|------|
| Docker | Engine 20.10+（方式一） |
| 磁盘 | 预留 20GB+（方式一）或 10GB+（方式二） |
| GPU | 编译不需要，运行 CUDA 程序需要 |
| 网络 | 方式一 build 时需要；提取 dist 后不需要 |
