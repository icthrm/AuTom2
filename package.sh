#!/bin/bash
set -e

BIN_DIR="${1:-/workspace/autom2/build/bin}"
OUT_DIR="${2:-/workspace/autom2/dist}"

echo "=== 1. 编译项目 ==="
cd /workspace/autom2
rm -rf build dist
# 必须在容器里编译，否则找不到库路径
cmake -B build -S . >/dev/null 2>&1
cmake --build build -j$(nproc) >/dev/null 2>&1

echo "=== 2. 创建输出目录 ==="
mkdir -p "$OUT_DIR"
cp -r "$BIN_DIR"/* "$OUT_DIR/"

echo "=== 3. 收集所有依赖库 ==="
for f in "$OUT_DIR"/*; do
    [ -f "$f" ] && [ -x "$f" ] || continue
    ldd "$f" 2>/dev/null | grep '=>' | while read -r line; do
        libpath=$(echo "$line" | awk '{print $3}')
        [ -f "$libpath" ] || continue
        # 跳过系统核心库（glibc、ld-linux、linux-vdso）
        case "$(basename "$libpath")" in
            ld-linux*|libc.so*|libm.so*|libdl.so*|libpthread.so*|librt.so*|linux-vdso*) continue ;;
        esac
        cp -n "$libpath" "$OUT_DIR/" 2>/dev/null || true
    done
done

echo "=== 4. 用 patchelf 修改 RPATH 为 \$ORIGIN ==="
for f in "$OUT_DIR"/*; do
    [ -f "$f" ] && [ -x "$f" ] || continue
    # 先清空旧 rpath，再设为 \$ORIGIN
    patchelf --set-rpath '\$ORIGIN' "$f" 2>/dev/null || true
done

echo "=== 5. 验证 ==="
echo "输出目录: $OUT_DIR"
ls -lh "$OUT_DIR" | head -20
