#include <iostream>
#include <vector>
#include <cmath>
#include <opencv2/opencv.hpp>
#include "mrcimg/mrc2img.h"
#include "opts.h"

/**
 * @brief 对单张图像进行降采样（2D Binning）
 * @param src 输入图像（OpenCV Mat）
 * @param bin 降采样率（如2表示2×2降采样）
 * @return 降采样后的图像
 */
cv::Mat binImage(const cv::Mat &src, int bin)
{
    if (bin <= 1)
    {
        return src.clone(); // 无需降采样
    }

    int newWidth = src.cols / bin;
    int newHeight = src.rows / bin;
    cv::Mat dst(newHeight, newWidth, src.type());

    // 方法1：OpenCV的resize（简单平均）
    cv::resize(src, dst, cv::Size(newWidth, newHeight), 0, 0, cv::INTER_AREA);

    // 方法2：手动计算降采样（更接近MATLAB代码的逻辑）
    // for (int y = 0; y < newHeight; y++)
    // {
    //     for (int x = 0; x < newWidth; x++)
    //     {
    //         float sum = 0.0f;
    //         int count = 0;

    //         // 遍历bin×bin的像素块
    //         for (int dy = 0; dy < bin; dy++)
    //         {
    //             for (int dx = 0; dx < bin; dx++)
    //             {
    //                 int srcX = x * bin + dx;
    //                 int srcY = y * bin + dy;

    //                 if (srcX < src.cols && srcY < src.rows)
    //                 {
    //                     sum += src.at<float>(srcY, srcX);
    //                     count++;
    //                 }
    //             }
    //         }

    //         dst.at<float>(y, x) = sum / count; // 取平均值
    //     }
    // }

    return dst;
}

/**
 * @brief 对MRC图像进行降采样
 * @param input_path 输入MRC文件路径
 * @param output_path 输出MRC文件路径
 * @param bin 降采样率
 * @return 是否成功
 */

bool binMRC(const char *input_path, const char *output_path, int bin)
{
    // 1. 打开输入MRC文件
    util::MrcStack mrc_input;
    if (!mrc_input.Open(input_path))
    {
        std::cerr << "Failed to open input MRC file: " << input_path << std::endl;
        return false;
    }

    // 2. 创建输出MRC文件
    util::MrcStack mrc_output;
    mrc_input.CopyToNewStack(mrc_output); // 复制头信息


    // 3. 设置降采样后的尺寸
    int newWidth = mrc_input.Width() / bin;
    int newHeight = mrc_input.Height() / bin;
    int numSlices = mrc_input.Size(); // Z方向不变

    mrc_output.AllocHeader();
    mrc_output.SetVolumeSize(newWidth, newHeight, numSlices);

    // 调整像素间距 (物理尺寸不变，像素间距增大bin倍)
    // mrc_output.Header().xlen = mrc_input.Header().xlen; // 保持物理尺寸不变
    // mrc_output.Header().ylen = mrc_input.Header().ylen;
    // mrc_output.Header().zlen = mrc_input.Header().zlen;

    MrcHeader &out_header = const_cast<MrcHeader &>(mrc_output.Header());
    out_header.xlen = mrc_input.Header().xlen; // 保持物理尺寸不变
    out_header.ylen = mrc_input.Header().ylen;
    out_header.zlen = mrc_input.Header().zlen;
    out_header.mx = newWidth;
    out_header.my = newHeight;
    out_header.mz = numSlices;

    // 初始统计值设为0（将在处理后更新）
    // out_header.amin = 0;
    // out_header.amax = 0;
    // out_header.amean = 0;
    out_header.mode = 2;

    // mrc_output.SetHeader(util::MrcStack::MODE_FLOAT, 0, 0, 0); // 初始值设为0，后面更新
    mrc_output.SetName(output_path);
    mrc_output.WriteHeaderToFile();

    // 4. 逐帧降采样
    float amin = FLT_MAX, amax = -FLT_MAX, amean = 0.0f;
    long pixel_count = newWidth * newHeight * numSlices;
    for (int i = 0; i < numSlices; i++)
    {
        cv::Mat slice = mrc_input.GetStackImage(i);
        if (slice.empty())
        {
            std::cerr << "Failed to read slice " << i << std::endl;
            continue;
        }

        // 降采样
        cv::Mat binnedSlice = binImage(slice, bin);

        // 更新统计信息（amin, amax, amean）
        double minVal, maxVal;
        cv::minMaxLoc(binnedSlice, &minVal, &maxVal);
        amin = std::min(amin, (float)minVal);
        amax = std::max(amax, (float)maxVal);
        amean += cv::mean(binnedSlice)[0] / numSlices;

        // 写入降采样后的图像
        // mrc_output.WriteStackImage(i, &binnedSlice);
        mrc_output.AppendStackImageToFile(&binnedSlice);
    }

    // std::cout << "amin: " << amin << " amean: " << amean << " amax: " << amax <<std::endl;

    // out_header.amin = amin;
    // out_header.amax = amax;
    // out_header.amean = amean;

    // 5. 更新头信息并保存
    mrc_output.SetHeader(util::MrcStack::MODE_FLOAT, amin, amean, amax);
    mrc_output.WriteHeaderToFile();

    std::cout << "Binning completed! Output saved to: " << output_path << std::endl;
    return true;
}

int main(int argc, char **argv)
{
    struct options opts;
    if (GetOpts(argc, argv, &opts) <= 0)
    {
        return 0; // 返回0表示帮助信息或错误
    }

    const char *input_mrc = opts.input.c_str();
    const char *output_mrc = opts.output.c_str();
    int bin = opts.bin_factor;

    std::cout << "Input file: " << input_mrc << std::endl;
    std::cout << "Output file: " << output_mrc << std::endl;
    std::cout << "Bin factor: " << bin << std::endl;

    if (!binMRC(input_mrc, output_mrc, bin))
    {
        return 1;
    }

    return 0;
}