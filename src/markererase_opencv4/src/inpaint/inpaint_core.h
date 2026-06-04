#ifndef INPAINT_CORE_H__
#define INPAINT_CORE_H__

#include <opencv2/opencv.hpp>
#include "dataf/dataf.h"
#include "mrcimg/mrc2img.h"

/** inpaint for the fiducial markers; cope with image with 0~1*/
class InpaintApp
{

protected:
	InpaintApp();
	~InpaintApp();

private:
	bool RegionGrow(cv::Mat &img, const util::point2d &seed, float diameter, std::vector<cv::Point> &region, double &bg_avg);

protected:
	void ConstInpaint(cv::Mat &img, const std::vector<util::point2d> &fids, float dia, cv::Mat &cpy);
	void ConstInpaintByRegionGrow(cv::Mat &img, const std::vector<util::point2d> &fids, float dia, cv::Mat &cpy);

public:
	static void InpaintingMain(util::MrcStack &mrcs, util::FiducialStack &fidstk, float &diameter, const char *output);
};

#endif