#include "inpaint_core.h"
#include "mrcimg/img_util.h"
#include <stack>

InpaintApp::InpaintApp() {}

InpaintApp::~InpaintApp() {}

bool InpaintApp::RegionGrow(cv::Mat &img, const util::point2d &seed, float diameter, std::vector<cv::Point> &region, double &bg_avg)
{
	int idia;
	cv::Rect roi;
	cv::Scalar pixel_avg1, pixel_std1, pixel_avg2, pixel_std2;

	region.clear();

	idia = int(diameter * 3 + .5); // 1.2533 = sqrt(pi/2)

	roi.x = int(seed.x - idia * .5 + .5);
	roi.y = int(seed.y - idia * .5 + .5);
	roi.width = idia;
	roi.height = idia;
	cv::Mat sub = util::GetStackImage(img, roi);

	if (sub.empty())
	{
		return false;
	}

	cv::meanStdDev(sub, pixel_avg1, pixel_std1);

	idia = int(diameter * 1.2533 + .5); // 1.2533 = sqrt(pi/2)
	roi.x = int(seed.x - idia * .5 + .5);
	roi.y = int(seed.y - idia * .5 + .5);
	roi.width = idia;
	roi.height = idia;
	sub = util::GetStackImage(img, roi);

	if (sub.empty())
	{
		return false;
	}

	cv::meanStdDev(sub, pixel_avg2, pixel_std2);

	bg_avg = (pixel_avg1.val[0] * 9 - pixel_avg2.val[0]) * 0.125;

	int seed_y = seed.y;
	int seed_x = seed.x;

	// double d = roi_avg+.3*pixel_std1.val[0];                            //reversal
	double d = (bg_avg + pixel_avg2.val[0]) * 0.5 - 0.5 * pixel_std1.val[0];

	std::stack<cv::Point> seedd;
	seedd.push(cv::Point(int(seed.x), int(seed.y)));

	int width = img.cols;
	int height = img.rows;

	float d2 = diameter * diameter * 1.5;

	while (!seedd.empty())
	{
		cv::Point point = seedd.top();
		seedd.pop();

		if (!(point.x >= 0 && point.x < width - 1 && point.y >= 0 && point.y < height - 1))
		{
			continue;
		}

		if ((point.x - seed.x) * (point.x - seed.x) + (point.y - seed.y) * (point.y - seed.y) > d2)
		{
			continue;
		}

		img.at<float>(point.y, point.x) = bg_avg;
		region.push_back(point);

		float value = img.at<float>(point.y, point.x - 1); //(x-1, y)
		if (value < d)
		{
			seedd.push(cv::Point(point.x - 1, point.y));
		}

		value = img.at<float>(point.y, point.x + 1); //(x+1, y)
		if (value < d)
		{
			seedd.push(cv::Point(point.x + 1, point.y));
		}

		value = img.at<float>(point.y - 1, point.x); //(x, y-1)
		if (value < d)
		{
			seedd.push(cv::Point(point.x, point.y - 1));
		}

		value = img.at<float>(point.y + 1, point.x); //(x, y+1)
		if (value < d)
		{
			seedd.push(cv::Point(point.x, point.y + 1));
		}
	}

	int bound = int(diameter * 0.125 + 0.5);

	std::vector<cv::Point> boundv;

	for (int i = 0; i < region.size(); i++)
	{
		for (int j = -bound; j <= bound; j++)
		{
			for (int k = -bound; k <= bound; k++)
			{
				int x = region[i].x + j;
				int y = region[i].y + k;

				if (!(x >= 0 && x < width - 1 && y >= 0 && y < height - 1))
				{
					continue;
				}
				if (img.at<float>(y, x) == bg_avg)
				{
					continue;
				}
				img.at<float>(y, x) = bg_avg;

				boundv.push_back(cv::Point(x, y));
			}
		}
	}

	for (int i = 0; i < boundv.size(); i++)
	{
		region.push_back(boundv[i]);
	}

	return true;
}

void InpaintApp::ConstInpaint(cv::Mat &img, const std::vector<util::point2d> &fids, float dia, cv::Mat &cpy)
{
	img.copyTo(cpy);

	int r = dia * 0.6 + 2;
	for (int i = 0; i < fids.size(); i++)
	{
		cv::circle(cpy, cv::Point((int)fids[i].x, (int)fids[i].y), r, cv::Scalar(0.5), -1);
	}
}

void InpaintApp::ConstInpaintByRegionGrow(cv::Mat &img, const std::vector<util::point2d> &fids, float dia, cv::Mat &cpy)
{
	img.copyTo(cpy);

	int r = dia * 0.6 + 2;
	for (int i = 0; i < fids.size(); i++)
	{
		std::vector<cv::Point> region;
		double bg_avg;
		if (!RegionGrow(cpy, fids[i], dia, region, bg_avg))
		{
			cv::circle(cpy, cv::Point((int)fids[i].x, (int)fids[i].y), r, cv::Scalar(0.5), -1);
		}
	}
}

void InpaintApp::InpaintingMain(util::MrcStack &mrcs, util::FiducialStack &fidstk, float &diameter, const char *output)
{
	util::MrcStack mrcscpy;

	mrcs.CopyToNewStack(mrcscpy);
	mrcscpy.SetName(output);
	mrcscpy.SetHeader(util::MrcStack::MODE_FLOAT, 0, 0.5, 1);
	mrcscpy.WriteHeaderToFile();

	InpaintApp ipapp;

	for (int i = 0; i < mrcs.Size(); i++)
	{
		EX_TIME_BEGIN("Erasing fiducial markers in micrograph %d", i)
		cv::Mat img = mrcs.GetStackImage(i); // 需要检查这个函数是否返回 cv::Mat
		util::ConvertTo1(img);

		cv::Mat cpy;

		const std::vector<util::point2d> &fids = fidstk.V(i);
		//         ipapp.ConstInpaint(img, fids, diameter, cpy);
		ipapp.ConstInpaintByRegionGrow(img, fids, diameter, cpy);

		mrcscpy.AppendStackImageToFile(&cpy);
		EX_TIME_END("Erasing finished for micrograph %d", i)
	}

	mrcscpy.Close();
}