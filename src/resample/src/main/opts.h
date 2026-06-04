#ifndef OPTS_H__
#define OPTS_H__

#include <iostream>
#include <sstream>
#include <cassert>
extern "C"
{
#include <getopt.h>
}
#include "util/exception.h"

struct options
{
    std::string input;  // 输入图像文件路径
    std::string output; // 输出图像文件路径
    int bin_factor;     // 降采样因子
};

inline int GetOpts(int argc, char **argv, options *opts_)
{
    static struct option longopts[] = {
        {"help", no_argument, NULL, 'h'},
        {"output", required_argument, NULL, 'o'},
        {"input", required_argument, NULL, 'i'},
        {"bin", required_argument, NULL, 'b'},
        {NULL, 0, NULL, 0}};

    // 检查参数数量是否正确 (必须包含 -i, -o 和 -b)
    if (argc != 7 || (argc == 2 && argv[1][0] != '-' && argv[1][1] != 'h') || argc == 1)
    {
        EX_TRACE("Usage: %s -i INPUT_FILE -o OUTPUT_FILE -b BIN_FACTOR\n", argv[0]);
        EX_TRACE("Options:\n");
        EX_TRACE("  -i, --input      Input image file\n");
        EX_TRACE("  -o, --output     Output image file\n");
        EX_TRACE("  -b, --bin        Bin factor (integer >= 1)\n");
        EX_TRACE("  -h, --help       Show this help message\n");
        return -1;
    }

    int ch;
    while ((ch = getopt_long(argc, argv, "hi:o:b:", longopts, NULL)) != -1)
    {
        switch (ch)
        {
        case '?':
            EX_TRACE("Invalid option '%s'.", argv[optind - 1]);
            return -1;

        case ':':
            EX_TRACE("Missing option argument for '%s'.", argv[optind - 1]);
            return -1;

        case 'h':
            EX_TRACE("Usage: %s -i INPUT_FILE -o OUTPUT_FILE -b BIN_FACTOR\n", argv[0]);
            EX_TRACE("Options:\n");
            EX_TRACE("  -i, --input      Input image file\n");
            EX_TRACE("  -o, --output     Output image file\n");
            EX_TRACE("  -b, --bin        Bin factor (integer >= 1)\n");
            return 0;

        case 'i':
        {
            opts_->input = optarg;
            if (opts_->input.empty())
            {
                EX_TRACE("Input file path cannot be empty\n");
                return -1;
            }
        }
        break;

        case 'o':
        {
            opts_->output = optarg;
            if (opts_->output.empty())
            {
                EX_TRACE("Output file path cannot be empty\n");
                return -1;
            }
        }
        break;

        case 'b':
        {
            std::istringstream iss(optarg);
            iss >> opts_->bin_factor;
            if (iss.fail() || opts_->bin_factor <= 0)
            {
                EX_TRACE("Invalid bin factor '%s'. Must be a positive integer.\n", optarg);
                return -1;
            }
        }
        break;

        case 0:
            break;

        default:
            assert(false);
        }
    }
    return 1;
}

#endif