#include "opts.h"
#include <iostream>
#include <fstream>
#include "dataf/dataf.h"
#include "dataf/calibration.h"
#include "mrcimg/mrc2img.h"
#include <string>
#include<unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <mrcimg/img_util.h>

using namespace std;

int main(int argc, char **argv)
{
	struct options opts;
	if(GetOpts(argc, argv, &opts) < 0){
		return 0;
	}
	
	std::cout<<"MRC to PGM."<<std::endl;
	
	util::MrcStack mrcs;
	string mrcfile = opts.input;
	
	if(!mrcs.Open(mrcfile.c_str())){
		return 0;
	}
	
	int dos = mrcfile.find('.');
	
	mrcfile.erase(dos, mrcfile.size());
	
	if(access(mrcfile.c_str(),0) == -1){
        if (mkdir(mrcfile.c_str(),0777)){
            printf("creat file bag failed!!!");
			return 0;
        }
    }
	
	std::stringstream ss(opts.idxs);
	int idx_s, idx_e; char ch;
	
	ss>>idx_s>>ch>>idx_e;
	for(int i = idx_s; i <= idx_e; i++){
		IplImage* img = mrcs.GetIplImage(i);
		
		util::ConvertTo1(img, true);
		std::stringstream dest(mrcfile);
		dest<<mrcfile<<"/"<<i<<".pgm";
		util::SaveImage(img, dest.str().c_str());
		cvReleaseImage(&img);
	}
	
    mrcs.Close();	
}

