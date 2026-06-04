#ifndef MRCVIEW_H
#define MRCVIEW_H

#include "../mrcimg/mrc2img.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <opencv2/core/base.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <thread>
#include <vector>
// #include "mpi.h"

using std::cout;
using std::endl;
using std::vector;

struct MrcData;
struct SubImage;

struct MrcData {

  util::MrcStack st;
  vector<cv::Mat> XView;
  vector<cv::Mat> YView;
  cv::Mat displayimg;

  void update(int index);
  static void callBack_(int pos, void *) {};
  void runCycle();

  void getXView();
  void getYView();
  static cv::Mat convertTo8U(cv::Mat mt);
};



class ViewSlicer{
  struct SubImage {
    int step, offset;
    int mat_type;
    bool inited = false;
    vector<cv::Mat> img, onedim;
    SubImage(int step, int offset) : step(step), offset(offset) {}
    bool init(cv::Mat origin) {
      int len = origin.cols;
      if (len < offset)
        return false;
      mat_type = origin.type();
      cv::Mat tmp = cv::Mat::zeros(origin.rows, offset, origin.type());
      for (int i = 0; i < len - offset; i += step) {
        tmp = cv::Mat::zeros(origin.rows, offset, origin.type());
        for (int j = 0; j < offset; j++) {
          origin.col(i + j).copyTo(tmp.col(j));
        }
        img.emplace_back(tmp.clone());
      }
      inited = true;
      return true;
    }
    void project() {
      cv::Mat ones = cv::Mat::ones(offset, 1, mat_type);
      for (const auto &chip : img) {
        onedim.push_back(chip * ones);
      }
    }
  };
  struct PosAndThickness {
    template <class T1, class T2> struct mypair {
      T1 a;
      T2 b;
      mypair(const T1 &aa, const T2 &bb) : a(aa), b(bb) {}
    };
    template <class Ta, class Tb> mypair<Ta, Tb> mkpr(const Ta &aa, const Tb &bb) {
      return mypair<Ta, Tb>(aa, bb);
    }

    vector<mypair<int, int>> pos;
    vector<int> thick;
    vector<float> ncc;
    float totalncc = 0;
    void add(int x, int z, int t,float nc) {
      pos.push_back(mkpr(x, z));
      thick.push_back(t);
      if(nc < 0) nc = 0;
      ncc.push_back(nc);
      totalncc += nc;
    }
    void clear(){
      pos.clear();
      thick.clear();
      ncc.clear();
      totalncc = 0;
    }
  };

  public:
  cv::Mat target;
  SubImage sub;
  PosAndThickness pt;
  public:

  ViewSlicer():sub(10, 10){
  }

  ViewSlicer(cv::Mat tar, int step, int offset):target(tar), sub(step, offset){
    cv::normalize(target, target, 0, 1, cv::NORM_MINMAX, CV_32F);
    sub.init(target);
    sub.project();
  }
  static double calculateNCC(const cv::Mat a, const cv::Mat b) {
    if (a.rows != b.rows)
      return -1.0;
    double amean = cv::mean(a).val[0], bmean = cv::mean(b).val[0];
    double sum = 0, sum1 = 0, sum2 = 0;
    for (int i = 0; i < a.rows; i++) {
      auto p1 = (a.at<float>(i, 0) - amean), p2 = (b.at<float>(i, 0) - bmean);
      sum += p1 * p2;
      sum1 += p1 * p1;
      sum2 += p2 * p2;
    }
    double ans = sum / (sqrt(sum1 * sum2));
    return std::abs(ans);
  }
  int index;
  PosAndThickness getAllTAndZ( int minThickness, int maxThickness) {
    pt.clear();
    int z, t;
    index = 0;
    for (auto &dim1 : sub.onedim) {
      double nc = getThicknessAndZ(dim1, minThickness, maxThickness, z, t);
      pt.add(sub.step * index + sub.offset / 2, z, t, nc);
      index++;
    }
    return pt;
  }

  double getThicknessAndZ(const cv::Mat onedim, int minnThickness, int maxxThickness, int &z,
                      int &thickness); // from : onedim ; to : thickness and z pos
  std::pair<double, double> calculateAvgThickAndZ();
  double calculateAngle();
};

cv::Mat getWider(cv::Mat x, int target);

void showNCC(cv::Mat onedim);
double showAngle(ViewSlicer& view);



#endif