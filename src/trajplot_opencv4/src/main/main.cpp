#include "opts.h"
#include <iostream>
#include <fstream>
#include "dataf/dataf.h"
#include "dataf/calibration.h"
#include "mrcimg/mrc2img.h"
#include "trajectory/trajectory.h"
#include <string>

using namespace std;
// using namespace ann_1_1_char;

int main(int argc, char **argv){
	struct options opts;
	GetOpts(argc, argv, &opts);
	
	util::MrcStack mrcs;
	string mrcfile = opts.input;
	cout<<"mrcfile : "<<mrcfile<<endl;
	mrcs.Open(mrcfile.c_str());

	util::ImgMatchVector imvector;
	imvector.ReadVectorByFolder("matches");
	
	vector<float> angles;
	if(!util::ReadAnglesByName(opts.inputangle , &angles)) {
	    std::cout<<"can't open tilt angle file."<<endl;
	    return -1;
	}
// 	
	cout<<"--------------------------------------->>"<<endl;
	cout<<"Build trajectory by matchs:"<<endl;
	util::TrackSpace trackspace;
	trackspace.Create(imvector, angles);
	
	util::FiducialStack addedfsk;
    util::ImgMatchVector addedimv;
	addedfsk.ReadFidsByFile("addfids.txt");
    addedimv.ReadVectorByFolder("addimvec");
	
	trackspace.InsertMatchVector(addedimv);
	
	addedimv.Release();
	imvector.Release();
	trackspace.WriteIMODfidModel(trackspace, mrcs.Width(), mrcs.Height(), mrcs.Size(), "init.fid.txt");
	
	vector<trajectory> T;
	ReadTragectorsByfile(&T , "init.fid.txt");
	
	updateTbyxf(&T, opts.xfile, mrcs.Width(), mrcs.Height());
	
	writeIMODfidModel(T, mrcs.Width(), mrcs.Height(), mrcs.Size(), "fin.fid.txt");
	
    cout<<"------------------------------------"<<endl;
    cout<<"Releasing memory"<<endl;
    //free memory correctly
    freeTrajectoryVector(T);
    T.clear();
    mrcs.Close();
}
