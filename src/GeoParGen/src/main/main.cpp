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
#include <mpi.h>
#include <numeric>
#include "mrcview.h"

int package_mat(const cv::Mat& mat, uchar* &buffer){

  if(buffer != nullptr){ delete[] buffer; buffer = nullptr;}
  int rows = mat.rows;
  int cols = mat.cols;
  int type = mat.type();
  int channel = mat.channels();
  int byteperele = 1;
  if(type == CV_32F) byteperele = 8;
  int bytes = rows*cols*byteperele*channel;
  buffer = new uchar[bytes+4*sizeof(int)];
  memcpy(&buffer[0], &rows, sizeof(int));
  memcpy(&buffer[1 * sizeof(int)], &cols, sizeof(int));
  memcpy(&buffer[2 * sizeof(int)], &type, sizeof(int));
  memcpy(&buffer[3 * sizeof(int)], &channel, sizeof(int));

  if(!mat.isContinuous())
      memcpy(&buffer[4 * sizeof(int)], mat.clone().data, bytes);
  else 
      memcpy(&buffer[4 * sizeof(int)], mat.data, bytes);
  return bytes+4*sizeof(int);
}

cv::Mat unpack_mat(uchar* &buffer, int bytes){
  int rows, cols, type, channel;
  memcpy(&rows, &buffer[0], sizeof(int));
  memcpy(&cols, &buffer[1 * sizeof(int)], sizeof(int));
  memcpy(&type, &buffer[2 * sizeof(int)], sizeof(int));
  memcpy(&channel, &buffer[3 * sizeof(int)], sizeof(int));

  cv::Mat received_mat(rows, cols, type, &buffer[4*sizeof(int)]);
  return received_mat;
}
void send_mat(const cv::Mat mat, MPI_Comm comm, int des){
  uchar* buffer = nullptr;
  int bytes = package_mat(mat, buffer);
  MPI_Send(buffer, bytes+sizeof(int), MPI_UNSIGNED_CHAR, des, 0, comm);
  if(buffer != nullptr) delete[] buffer;
}

cv::Mat recv_mat(MPI_Comm comm, int src){
  MPI_Status status;
  int count;
  MPI_Probe(src, 0, comm, &status);
  MPI_Get_count(&status, MPI_UNSIGNED_CHAR, &count);
  uchar* buffer = new uchar[count];
  MPI_Recv(buffer, count, MPI_UNSIGNED_CHAR, src, 0, comm, MPI_STATUS_IGNORE);

  cv::Mat ret(unpack_mat(buffer, count));
  return ret;
}

#define MPI_PG

int main(int argc, char* argv[]) {

// MPI Init

#ifdef MPI_PG

  if(MPI_Init(NULL, NULL) != MPI_SUCCESS){
      cout << "MPI init failed" << endl;
      return 0;
  }

  int world_size, world_rank;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
  MPI_Get_processor_name(processor_name, &name_len);


  MrcData mrc;

  int view_num;
  vector<cv::Mat> to_deal;
  
  if(world_rank == 0){
    printf("MPI Init success! from processor %s, rank %d out of %d processors\n", processor_name, world_rank, world_size);

    std::string mrc_file_path;
    std::string cal_what = "x";
    if(argc < 2){
      std::string executable_path = argv[0];
      std::string program_dir = executable_path.substr(0, executable_path.find_last_of("/\\"));
      std::string mrc_file_path = program_dir + "/../res/BBa_rec.mrc";
    }
    else if(argc == 2){
      mrc_file_path = argv[1]; 
    }
    else if(argc == 3){
      mrc_file_path = argv[1]; 
      cal_what = argv[2];
    }
    else {
      cout << "invalid parameter! " << endl; 
      MPI_Finalize();
      return 0;
    }

    if (!mrc.st.Open(mrc_file_path.c_str())) {
      cout << "file read failed! " << mrc_file_path << endl;
      MPI_Finalize();
      return 0;
    }
    cout << "file read success" << endl;
    cout << "begin generate " << cal_what << " view ... " << endl;
    if(cal_what == "x")
      mrc.getXView();
    else if(cal_what == "y")
      mrc.getYView();
    else {
      cout << "invalid parameter! " << endl; 
      MPI_Finalize();
      return 0;
    }
    mrc.st.Close();

  // 分发切片

    int ev_num = mrc.YView.size()/world_size;
    int mod = (mrc.YView.size())%world_size;

    view_num = ev_num; // 为processor 0 准备数据
    for(int i = 0; i < view_num; i++){
      to_deal.push_back(mrc.YView[i]);
    }

    for(int i = 1 ; i < world_size; i ++){ // 为其余processor准备数据
      int st = i*ev_num, ed = st+ev_num, len;
      if(i == world_size-1) ed += mod;
      len = ed-st;
      MPI_Send(&len, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
      cout << "send len = " << len << endl;
      for(int j = st; j < ed; j ++){
        send_mat(mrc.YView[j], MPI_COMM_WORLD, i);
      }
    }
  //结束分发
  }
  else{
    MPI_Recv(&view_num, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    for(int i = 0; i < view_num ; i++){
      to_deal.push_back(recv_mat(MPI_COMM_WORLD, 0));
    }
    cout << "processor " << world_rank << " get " << view_num << " slices " << endl;
  }

  ViewSlicer view;
  vector<float> result(to_deal.size());
  vector<float> result_thick(to_deal.size());
  vector<float> result_z(to_deal.size());

  //分发完毕，开始计算
  for(int i = 0; i < to_deal.size(); i++){
    view = ViewSlicer(to_deal[i], 50, 50);
    result[i] = (view.calculateAngle());
    auto tz = view.calculateAvgThickAndZ();
    result_thick[i] = (float)tz.first;
    result_z[i] = (float)tz.second;
  }

  if(world_rank == 0){
    vector<float> angles;
    angles.resize(mrc.YView.size());
    vector<float> thicks;
    thicks.resize(mrc.YView.size());
    vector<float> zs;
    zs.resize(mrc.YView.size());

    int cur = result.size();
    std::copy(result.begin(), result.end(), angles.begin());
    std::copy(result_thick.begin(), result_thick.end(), thicks.begin());
    std::copy(result_z.begin(), result_z.end(), zs.begin());

    for(int i = 1; i < world_size; i++){
      int sz;
      MPI_Recv(&sz, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Recv((void*)&angles.data()[cur], sz, MPI_FLOAT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Recv((void*)&thicks.data()[cur], sz, MPI_FLOAT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Recv((void*)&zs.data()[cur], sz, MPI_FLOAT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      for(int k = cur ; k < cur+sz ; k++){
        cout << "recv angle : " << angles[k] << ", thick: " << thicks[k] << ", z: " << zs[k] << endl;
      }
      cur += sz;
    }
    cout << "average angle : " << std::accumulate(angles.begin(), angles.end(), 0.0f) / angles.size() << endl;
    cout << "average thick : " << std::accumulate(thicks.begin(), thicks.end(), 0.0f) / thicks.size() << endl;
    cout << "average z : " << std::accumulate(zs.begin(), zs.end(), 0.0f) / zs.size() << endl;
  }
  else{
    int sz = result.size();
    MPI_Send(&sz, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    MPI_Send(result.data(), sz, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
    MPI_Send(result_thick.data(), sz, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
    MPI_Send(result_z.data(), sz, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
  }
  
  MPI_Finalize();
  // ViewSlicer view(mrc.XView[512], 50, 50);


  // cv::Mat norm;
  // cv::normalize(view.sub.onedim[1], norm, 0, 1, cv::NORM_MINMAX, CV_32F);
  // showNCC(norm);
  #else
    MrcData mrc;
    std::string mrc_file_path;

    if(argc < 2){
    std::string executable_path = argv[0];
     std::string program_dir = executable_path.substr(0, executable_path.find_last_of("/\\"));
     mrc_file_path = program_dir + "/../res/BBa_rec.mrc";
     }
     else{
     mrc_file_path = argv[1];
     }
     if (!mrc.st.Open(mrc_file_path.c_str())) {
      cout << "file read failed! " << mrc_file_path << endl;
       //MPI_Finalize();
       return 0;
     }
     cout << "file read success" << endl;

    cout << "X = " << mrc.st.X() << ", Y = " << mrc.st.Y() << ", Z = " << mrc.st.Z() << endl;

     cv::Mat img = mrc.st.GetStackImage(1000);
     //cv::normalize(img, img, 0.0, 1.0, cv::NORM_MINMAX, CV_32F);

       // showNCC(view.sub.onedim[0]);

    ViewSlicer view(img,50,50);
    showAngle(view);

    // int pos = 1;
     // cv::namedWindow("Image");
    // cv::createTrackbar("z position", "Image", NULL, 299, callback_main, 0);
     // while(1){
     //   pos = cv::getTrackbarPos("z position", "Image");
     //   showAngle(img,0.532719, pos, "Image");
     //   if (cv::waitKey(30) > 0)
     //     break;
     // }    MrcData mrc;
     

  #endif
  

}



