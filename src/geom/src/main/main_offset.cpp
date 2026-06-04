#include <mpi.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>   // std::reverse
#include <string>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <libgen.h>
#include <cuda_runtime.h>
#include <unistd.h>

#include "../src/method/Util.h"
#include "../src/method/FindOffset.h"
#include "../src/method/Rotate.h"
#include "../src/mrc/mrcstack.h"
#include "../src/mrc/mrcheader.h"

/* ------------------------------------------------------------------ */
/* 命令行参数                                                           */
/* ------------------------------------------------------------------ */
struct FOOptions
{
    char input[512];   // -i  输入 MRC 文件
    char angle[512];   // -a  rawtlt 倾斜角文件
    int  GPU;          // -g  指定 GPU 编号 (默认 0)
};

static void PrintUsage(const char *prog)
{
    printf("Usage: %s -i <input.mrc> -a <angles.rawtlt> [-g gpu_id]\n", prog);
    printf("  -i   输入图像叠栈 (.mrc)\n");
    printf("  -a   倾斜角文件 (.rawtlt)\n");
    printf("  -g   GPU 编号 (默认 0)\n");
}

static void GetOpts(int argc, char *argv[], FOOptions *opts)
{
    opts->GPU       = 0;
    opts->input[0]  = '\0';
    opts->angle[0]  = '\0';

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-i") == 0 && i + 1 < argc)
            strncpy(opts->input, argv[++i], 511);
        else if (strcmp(argv[i], "-a") == 0 && i + 1 < argc)
            strncpy(opts->angle, argv[++i], 511);
        else if (strcmp(argv[i], "-g") == 0 && i + 1 < argc)
            opts->GPU = atoi(argv[++i]);
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            PrintUsage(argv[0]);
            exit(0);
        }
    }
}

/* ------------------------------------------------------------------ */
/* 读取倾斜角文件                                                        */
/* ------------------------------------------------------------------ */
static bool ReadAngles(std::vector<float> &angles, const char *name)
{
    std::ifstream in(name);
    if (!in.good()) return false;
    while (in.good())
    {
        float val;
        in >> val;
        if (in.fail()) break;
        angles.push_back(val);
    }
    in.close();
    return true;
}

/* ------------------------------------------------------------------ */
/* main                                                                 */
/* ------------------------------------------------------------------ */
int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    /* ---------- 解析参数 ---------- */
    FOOptions opts;
    GetOpts(argc, argv, &opts);

    if (opts.input[0] == '\0' || opts.angle[0] == '\0')
    {
        if (rank == 0) PrintUsage(argv[0]);
        MPI_Finalize();
        return 1;
    }

    if (rank == 0)
    {
        printf("Input  : %s\n", opts.input);
        printf("Angles : %s\n", opts.angle);
        printf("GPU    : %d\n\n", opts.GPU);
    }

    cudaSetDevice(opts.GPU);

    /* ---------- 读取图像叠栈 ---------- */
    MrcStackM projs, preprojs;
    int status = 0;

    if (rank == 0)
    {
        if (!projs.ReadFile(opts.input))
        {
            printf("[Error] Cannot open input file: %s\n", opts.input);
            status = -1;
        }
    }
    MPI_Bcast(&status, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (status != 0)
    {
        MPI_Finalize();
        return 1;
    }

    if (rank == 0) projs.ReadHeader();
    MPI_Bcast(&(projs.header), sizeof(MRCheader), MPI_CHAR, 0, MPI_COMM_WORLD);

    /* ---------- 建立预处理缓存文件 ---------- */
    char absInput[PATH_MAX];
    realpath(opts.input, absInput);
    std::string bufPath = std::string(dirname(absInput)) + "/fo_buf.mrc";

    preprojs.InitializeHeader(projs.X(), projs.Y(), projs.Z());
    preprojs.SetSize(projs.X(), projs.Y(), projs.Z());
    preprojs.WriteToFile(bufPath.c_str());
    if (rank == 0) preprojs.WriteHeader();

    if (rank == 0)
        printf("Stack size: %d x %d x %d\n\n", projs.X(), projs.Y(), projs.Z());

    /* ---------- 读取倾斜角 ---------- */
    std::vector<float> p_angles;
    if (!ReadAngles(p_angles, opts.angle))
    {
        if (rank == 0)
            printf("[Error] Cannot open angle file: %s\n", opts.angle);
        MPI_Finalize();
        return 1;
    }
    if ((int)p_angles.size() != preprojs.Z())
    {
        if (rank == 0)
            printf("[Warning] Angle count (%d) != slice count (%d)\n",
                   (int)p_angles.size(), preprojs.Z());
    }

    /* ---------- 图像预处理 ---------- */
    PreProcess preprocess;
    preprocess.rawstack = &projs;
    preprocess.stack    = &preprojs;
    preprocess.angles   = p_angles;
    preprocess.SetPositive();
    preprocess.MassNormalization();
    projs.Close();

    // 角度从小到大排列（与原 Process::mPreprocess 一致）
    if (p_angles[0] > p_angles[1])
        std::reverse(p_angles.begin(), p_angles.end());

    /* ---------- 初始化对齐参数 ---------- */
    AlignParam param;
    int nz = preprojs.Z();
    param.shiftX      = new float[nz]();
    param.shiftY      = new float[nz]();
    param.rotate      = new float[nz]();
    param.angleOffset = 0.0f;

    /* ---------- 计算倾斜轴角度 ---------- */
    // FindOffset 内部用 CorrTomoStack 校正图像，需要 param.rotate 有效值
    if (rank == 0) printf("=== Step 1: Estimating tilt axis ===\n");
    CalcTIltAxis rotate;
    for (int i = 1; i <= 3; i++)
        rotate.DoIt(preprojs, &param, p_angles, 180.0f / i, 100);

    /* ---------- 计算 angleOffset ---------- */
    if (rank == 0) printf("\n=== Step 2: Finding tilt angle offset ===\n");
    FindOffset findoffset;
    findoffset.DoIt(preprojs, &param, p_angles);

    /* ---------- 输出结果 ---------- */
    if (rank == 0)
    {
        printf("\n===========================\n");
        printf("angleOffset = %.4f degrees\n", param.angleOffset);
        printf("===========================\n");
    }

    /* ---------- 清理 ---------- */
    preprojs.Close();
    std::remove(bufPath.c_str());

    delete[] param.shiftX;
    delete[] param.shiftY;
    delete[] param.rotate;

    MPI_Finalize();
    return 0;
}
