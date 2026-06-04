#include "mrcview.h"

void callback(int, void *) {}
cv::Mat getWider(cv::Mat x, int target) {

  cv::Mat mask = cv::Mat::zeros(x.rows, target, x.type());
  for (int i = 0; i < target; i++) {
    x.col(0).copyTo(mask.col(i));
  }
  return mask;
}

double ViewSlicer::calculateAngle(){ // TODO : 改
    auto ans = getAllTAndZ(30, 60);
    int index = 0;
    int xysum = 0, xsum = 0, ysum = 0, x2sum = 0;
    double wxysum = 0, wxsum = 0, wysum = 0, wx2sum = 0, w2x2sum = 0;

    double lmt = (double)(ans.pos.size());
    double wsum = 0;
    for (auto &pos : ans.pos) {
      xsum += pos.a;
      ysum += pos.b;
      xysum += pos.a * pos.b;
      x2sum += pos.a * pos.a;
      double w = ans.ncc[index] / ans.totalncc;
      w = w*w;
      wsum += w;

      wxsum += w * pos.a;
      wysum += w * pos.b;
      wxysum += w * pos.a * pos.b;
      wx2sum += w * pos.a * pos.a;

      index++;
    }

    double ans_a = (wsum * wxysum - wxsum*wysum ) / (wsum * wx2sum - wxsum*wxsum );
    double ans_a_2 = (double)(index * xysum - xsum * ysum) /
                  (double)(index * x2sum - xsum * xsum);
    auto atan_a = atan(ans_a);
    auto ang_a = atan_a * 180 / M_PI;

    return ang_a;
}

std::pair<double, double> ViewSlicer::calculateAvgThickAndZ() {
    auto ans = getAllTAndZ(30, 60); // same params as calculateAngle
    double total_weight = 0;
    double weighted_thick = 0;
    double weighted_z = 0;

    int idx = 0;
    for (auto &pos : ans.pos) {
         double w = ans.ncc[idx] / ans.totalncc; 
         w = w * w; // Use squared weight like in calculateAngle
         
         weighted_thick += w * ans.thick[idx];
         weighted_z += w * pos.b; // pos.b is Z (row index)
         total_weight += w;
         idx++;
    }
    
    if(total_weight == 0) return {0.0, 0.0};
    return {weighted_thick / total_weight, weighted_z / total_weight};
}

double ViewSlicer::getThicknessAndZ(const cv::Mat onedim, int minnThickness, int maxxThickness,  int &z,
                      int &thickness) {
  if (onedim.cols != 1)
    return 0;
  int len = onedim.rows;
  float maxx = 0, lcnt = 0, rcnt = 0;
  for (int l = 0; l + minnThickness < onedim.rows; l++) {
    for (int r = l + minnThickness; r < onedim.rows; r++) {
      cv::Mat rectFunc = cv::Mat::ones(onedim.rows, 1, onedim.type());
      for (int cur = l; cur <= r; cur++)
        rectFunc.at<float>(cur, 0) = 0;
      double result = calculateNCC(onedim, rectFunc);
      if (std::abs(result) > std::abs(maxx)) {
        maxx = result;
        lcnt = l;
        rcnt = r;
      }
    }
  }


  if (maxx == 0.0) {
    z = -1;
    thickness = -1;
    return 0;
  }
  z = (lcnt + rcnt) / 2;
  thickness = rcnt - lcnt;
  return maxx;
}

void MrcData::getXView() {
  if (!YView.empty())
    return;
  cout << "generate X View ..." << endl;
  int xlen = st.X(), zlen = st.Z();
  YView.resize(xlen);
  std::fill(YView.begin(), YView.end(),
            cv::Mat::zeros(st.Y(), zlen, st.GetStackImage(0).type()));
  for (int i = 0; i < zlen; i++) {
    auto img = st.GetStackImage(i);
    for (int j = 0; j < xlen; j++) {
      img.col(j).copyTo(YView[j].col(i));
    }
  }
}

void MrcData::getYView() {
  if (!YView.empty())
    return;
  cout << "generate Y View ..." << endl;
  int xlen = st.X(), zlen = st.Z();
  YView.resize(zlen);
  std::fill(YView.begin(), YView.end(),
            cv::Mat::zeros(st.Y(), xlen, st.GetStackImage(0).type()));
  for (int i = 0; i < zlen; i++) {
    auto img = st.GetStackImage(i);
    YView[i] = img.clone();
  }
}
cv::Mat MrcData::convertTo8U(cv::Mat mt) {
  cv::Mat norm;
#ifdef USE_SHRESHOLD
#define MAX_LIGHTING 10000
#define MIN_LIGHTING -10000
  cv::threshold(mt, mt, MAX_LIGHTING, MAX_LIGHTING, cv::THRESH_TRUNC);
  cv::threshold(mt, mt, MIN_LIGHTING, MIN_LIGHTING, cv::THRESH_TOZERO);
#endif
  cv::normalize(mt, norm, 0.0, 1.0, cv::NORM_MINMAX, CV_32F);
  norm.convertTo(norm, CV_8U, 255.0);
  return norm;
}

void MrcData::update(int index) {
  displayimg = convertTo8U(XView[index]);
  imshow("Image", displayimg);
}

void MrcData::runCycle() {
  cv::namedWindow("Image", cv::WINDOW_FREERATIO);
  cv::createTrackbar("Index", "Image", NULL, 1023, callBack_, 0);
  while (1) {
    int value = cv::getTrackbarPos("Index", "Image");
    update(value);
    imshow("Image", displayimg);
    if (cv::waitKey(30) > 0)
      break;
  }
}

double showAngle(ViewSlicer& view){
    auto ans = view.getAllTAndZ(30, 60);
    cv::Mat displayimg = MrcData::convertTo8U(view.target.clone());
    int index = 0;
    double xysum = 0, xsum = 0, ysum = 0, x2sum = 0;
    double wxysum = 0, wxsum = 0, wysum = 0, wx2sum = 0, w2x2sum = 0;

    double lmt = (double)(ans.pos.size());
    double wsum = 0;

    for (auto &pos : ans.pos) {
      cv::circle(displayimg, cv::Point(pos.a, pos.b), 5, cv::Scalar(0), 10,
                cv::LINE_8);

  #define WIDTH 5
      cv::line(displayimg, cv::Point(pos.a - WIDTH, pos.b - ans.thick[index] / 2),
              cv::Point(pos.a + WIDTH, pos.b - ans.thick[index] / 2),
              cv::Scalar(0), 5, cv::LINE_8);
      cv::line(displayimg, cv::Point(pos.a - WIDTH, pos.b + ans.thick[index] / 2),
              cv::Point(pos.a + WIDTH, pos.b + ans.thick[index] / 2),
              cv::Scalar(0), 5, cv::LINE_8);

      xsum += pos.a;
      ysum += pos.b;
      xysum += pos.a * pos.b;
      x2sum += pos.a * pos.a;

      double w = ans.ncc[index] / ans.totalncc;
      // double w = 1;
      w = w*w;
      wsum += w;

      wxsum += w * pos.a;
      wysum += w * pos.b;
      wxysum += w * pos.a * pos.b;
      wx2sum += w * pos.a * pos.a;

      index++;
      
      cout << "index[" << index << "] : " << ans.ncc[index-1] << ", the weight = " << w << endl;
    }

    double ans_a = (wsum * wxysum - wxsum*wysum ) / (wsum * wx2sum - wxsum*wxsum );
    double ans_a_2 = (double)(index * xysum - xsum * ysum) /
                  (double)(index * x2sum - xsum * xsum);
    auto atan_a = atan(ans_a);
    auto ang_a = atan_a * 180 / M_PI;
    cout << "ans_a: "    << ans_a << ", arctan: " << atan(ans_a) << ", angle: " << atan(ans_a) * 180 / M_PI << endl;

    cout << "wsum  = " << wsum << "; wxysum = " << wxysum << "; wxsum = " << wxsum << "; wysum = " << wysum << endl;
    cout << "index = " << index<< "; xysum  = " << xysum <<  ";  xsum = " << xsum  << ";  ysum = " << ysum << endl;

    double ans_b = ysum / index - ans_a * xsum / index;

    cv::Point pointl(0, ans_b);
    cv::Point pointr(displayimg.cols - 1, ans_a * (displayimg.cols - 1) + ans_b);

    cv::line(displayimg, pointl, pointr, cv::Scalar(0), 3, cv::LINE_8);


    cv::namedWindow("Image", cv::WINDOW_FREERATIO);
    imshow("Image", displayimg);

    cv::waitKey(0);
    return ang_a;
}

void showAngle(ViewSlicer& view, double angle, int pos){
    auto ans = view.getAllTAndZ(30, 60);
    cv::Mat displayimg = MrcData::convertTo8U(view.target.clone());
    double ans_a = angle;
    cv::Point pointl(0, pos);
    cv::Point pointr(displayimg.cols - 1, ans_a * (displayimg.cols - 1) + pos);

    cv::line(displayimg, pointl, pointr, cv::Scalar(0), 3, cv::LINE_8);

    cv::namedWindow("Image", cv::WINDOW_FREERATIO);
    imshow("Image", displayimg);

    cv::waitKey(0);
}

void showNCC(cv::Mat onedim) {

  cv::namedWindow("Image", cv::WINDOW_FREERATIO);
  cv::createTrackbar("Index", "Image", NULL, 299, callback, 0);
  cv::createTrackbar("Index2", "Image", NULL, 299, callback, 0);

  cv::Mat displaysub = getWider(MrcData::convertTo8U(onedim), 50);

  cv::Mat rectFunc = cv::Mat::ones(onedim.rows, 1, onedim.type());

  while (1) {
    int wide = 30;

    int pos = cv::getTrackbarPos("Index", "Image");
    int pos2 = cv::getTrackbarPos("Index2", "Image");
    rectFunc = cv::Mat::ones(onedim.rows, 1, onedim.type());
    for (int i = pos; i < pos2 && i < rectFunc.rows; i++) {
      rectFunc.at<float>(i, 0) = 0;
    }

// display logic below ****************
    cv::Mat rect8u = getWider(MrcData::convertTo8U(rectFunc), 50);
    cv::Mat temp =
        cv::Mat::zeros(displaysub.rows, displaysub.cols + rect8u.cols, CV_8U);
    for (int i = 0; i < displaysub.cols; i++) {
      displaysub.col(i).copyTo(temp.col(i));
    }
    for (int i = displaysub.cols; i < temp.cols; i++) {
      rect8u.col(i - displaysub.cols).copyTo(temp.col(i));
    }

    cv::line(temp, cv::Point(0, pos), cv::Point(49, pos), cv::Scalar(0), 2,
             cv::LINE_8, 0);
    cv::line(temp, cv::Point(0, pos2 - 1), cv::Point(49, pos2 - 1),
             cv::Scalar(0), 2, cv::LINE_8, 0);

    cv::transpose(temp, temp);
    imshow("Image", temp);

  // display logic ends ***************

  cout << ViewSlicer::calculateNCC(onedim, rectFunc) << endl;

    if (cv::waitKey(30) > 0)
      break;
    }
}
