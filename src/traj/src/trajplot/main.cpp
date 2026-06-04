#include <iostream>
#include "cstring"
#include "function.h"
#include "opts.h"

using namespace std;

int main(int argc, char **argv)
{
    cout<<"trajplot!"<<endl;
	options opts;

	if(GetOpts(argc, argv, &opts) < 0){
		return 1;
	}
	
	if(strcmp(opts.model, "ds") == 0){
		PlotTDistribute(opts.input);
	}
	else if(strcmp(opts.model, "xy") == 0 || strcmp(opts.model, "yx") == 0){
		PlotPointListXY(opts.input);
	}
	else if(strcmp(opts.model, "xz") == 0 || strcmp(opts.model, "zx") == 0){
		PlotPointListXZ(opts.input);
	}
	else if(strcmp(opts.model, "yz") == 0 || strcmp(opts.model, "zy") == 0){
		PlotPointListYZ(opts.input);
	}
	
	return 0;
//     PlotPointListXY("BBa.fid.txt");
//     Plot_PointList_xy1("BBa.fid.txt");
//      Plot_PointList_xy1("test081_fin.fid.txt");
//     PlotPointListYZ("test081_fin.fid.txt");
//     PlotPointListYZ("test08.fid.txt");
//     Plot_Tragitorys("test08.fid.txt");
// 	
//     PlotTDistribute("init.fid.txt");
// 	PlotPointListXY("init.fid.txt");
// 	PlotPointListYZ("fin.fid.txt");
//     wait_for_key();
	
}

