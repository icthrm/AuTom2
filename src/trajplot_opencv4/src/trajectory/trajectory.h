#ifndef TRAJECTORY_H
#define TRAJECTORY_H
#include "dataf/point2d.h"
#include "dataf/frame.h"
#include "dataf/dataf.h"
#include <vector>
using namespace std;

class trajectory
{
public:
    trajectory();
//     trajectory(trajectory* parent);
    trajectory(const trajectory &t);//famatnote: we need copy constructor for vectors and pointers
    ~trajectory();

    vector<Point2D*> p;
};

//famatnote: why are these functions outside the class?
void buildTrajectory(vector<trajectory>* T,util::TrackSpace& trackspace);
void updateTbyxf(vector<trajectory>* T , const char* xf_file , int width, int height);
void writeIMODfidModel(vector<trajectory> T, int width, int height, int num_frames,  const char* filename);
void writeIMODtiltalign(string basename,ostream &out, int imgnumber , char* tltfile , char* newtlt);
void writeIMODtiltScript(int W,int H,string pathTilt,string pathAli,string basename,int recoThickness,vector<frame> *frames,string endingRec);
void writeMATLABcontour(string, int);
void freeTrajectoryVector(vector<trajectory> T);
vector<trajectory> fiducialModel2Trajectory(string fidTxtFilename);
int * getMarkerTypeFromTrajectory(vector<trajectory> T);

void ReadTragectorsByfile(vector<trajectory>* T , const char* filename);
#endif