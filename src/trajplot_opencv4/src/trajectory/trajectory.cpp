#include "trajectory.h"
#include <limits>
#include <fstream>
#include <sstream>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

trajectory::trajectory()
{
    //famatnote: we need to know when there is no parent
//     parent=NULL;
}


// trajectory::trajectory(trajectory* parent)
// {
//     this->parent = parent;
// }

trajectory::trajectory(const trajectory &t)
{
//     parent=t.parent;
    p.clear();
    p=vector<Point2D*>(t.p);
//     child.clear();
//     child=vector<trajectory*>(t.child);
}

trajectory::~trajectory()
{
    //p.clear();
    //child.clear();

    //we don't that: otherwise we might destroy points we don't want to destroy
}

void buildTrajectory(vector<trajectory>* T,util::TrackSpace& trackspace)
{
    int num_frames = trackspace.Size();
    std::cout<<num_frames<<std::endl;
    int zerotilt = trackspace.Size()/2 + trackspace.Size()%2 +1;
    const unsigned int minTrajectoryLength =(10 > (int)(num_frames * 0.8)) ? 10 : (int)(num_frames * 0.8);
    for(int i = 0; i < trackspace.Size(); i++) {
        util::TrackSpace::Iterator itr = trackspace.Z_Iterator(i);
        while(!itr.IsNULL()) {
            util::TrackSpace::TrackNode node_itr = util::TrackSpace::TrackNode(itr);

            bool isStart = node_itr.IsBegin();
            if(isStart) {
                trajectory temp;
                while(!node_itr.IsNULL()) {
                    Point2D* point = new Point2D(node_itr.X() , node_itr.Y(), node_itr.Z() , 0);
                    temp.p.push_back(point);
                    node_itr++;
                }
                if(temp.p.size() > minTrajectoryLength) {
                    T->push_back(temp);
                }
            }

            itr++;
        }
    }


}

void updateTbyxf(vector<trajectory>* T , const char* xf_file , int width, int height)
{
    //read xf
    ifstream xfin(xf_file);
    vector<XfParam> xf_params;
    string s;
    while(getline(xfin ,s)) {
        stringstream ss;
        ss<<s;
        XfParam term;
        ss>>term.A11>>term.A12>>term.A21>>term.A22>>term.x_shift>>term.y_shift;
        xf_params.push_back(term);
    }

    float w_2 = width*.5, h_2 = height*.5;
    float x,y,xx,yy;
    int z;

    for (unsigned int i = 0; i < T->size(); i++) {
        cout << "contour " << i << " 0 " << T->at(i).p.size() << endl;
        for (unsigned int j = 0; j < T->at(i).p.size(); j++) {
            x=T->at(i).p.at(j)->x - w_2;
            y= T->at(i).p.at(j)->y -h_2;
            z = T->at(i).p.at(j)->frameID;
            XfParam term;
            term = xf_params[z];

// 	    X' = A11 * X + A12 * Y + A11 * SHIFT_X + A12 * SHIFT_Y
//         Y' = A21 * X + A22 * Y + A21 * SHIFT_X + A22 * SHIFT_Y
// 	    xx = term.A11*x + term.A12*y + term.A11*term.x_shift + term.A12*term.y_shift;
// 	    yy = term.A21*x + term.A22*y + term.A21*term.x_shift +term.A22*term.y_shift;

            xx = term.A11*x + term.A12*y + term.x_shift;
            yy = term.A21*x + term.A22*y +term.y_shift;

            T->at(i).p.at(j)->x = xx + w_2;
            T->at(i).p.at(j)->y = yy + h_2;
        }
    }
}

void writeIMODfidModel(vector<trajectory> T, int width, int height, int num_frames, const char* filename)
{

    ofstream out(filename);

    out << "imod 1" << endl;
    out << "max " << width << " " << height << " " << num_frames<< endl;
    out << "offsets 0 0 0" << endl;
    out << "angles 0 0 0" << endl;
    out << "scale 1 1 1" << endl;
    out << "mousemode 1" << endl;
    out << "drawmode 1" << endl;
    out << "b&w_level 0,200" << endl;
    out << "resolution 3" << endl;
    out << "threshold 128" << endl;
    out << "pixsize 1" << endl;
    out << "units pixels" << endl;
    out << "symbol circle"<< endl;//so imod displays all the markers at once
    out << "size 7"<<endl;

    out << "object 0 " << T.size() << " 0" << endl;
    out << "name" << endl;
    out << "color 0 1 0 0" << endl;
    out << "open" << endl;
    out << "linewidth 1" << endl;
    out << "surfsize  0" << endl;
    out << "pointsize 0" << endl;
    out << "axis      0" << endl;
    out << "drawmode  1" << endl;

    for (unsigned int i = 0; i < T.size(); i++)
    {
        out << "contour " << i << " 0 " << T.at(i).p.size() << endl;
        for (unsigned int j = 0; j < T.at(i).p.size(); j++)
        {
            out << T.at(i).p.at(j)->x << " " << T.at(i).p.at(j)->y<< " " << T.at(i).p.at(j)->frameID << endl;
        }
    }
}

void ReadTragectorsByfile(vector<trajectory>* T , const char* filename)
{
    ifstream fin(filename);
    int size;
    int a;
    string s;

    while(getline(fin , s)) {
        stringstream ss(s);
        string first_str;
        ss>>first_str;
        if(first_str == "object") {
            ss>>a>>size>>a;
            break;
        }
    }
    float x , y ;
    int n;
    int t_size;
    while(getline(fin ,s)) {
        stringstream ss(s);
        string first_str;
        ss>>first_str;
        if(first_str == "contour") {
            ss>>a>>a>>t_size;
            trajectory temp;
            for(int i=0; i < t_size ; i++) {
                getline(fin ,s);
                stringstream ss(s);
                ss>>x>>y>>n;
                Point2D* point = new Point2D(x , y, n , 0);
                temp.p.push_back(point);
            }
            T->push_back(temp);
        }
    }
}

void writeMATLABcontour(string dataset, int numFrames)
{
    ifstream in;
    ofstream out;
    string output_filename = dataset + "_.m";
    out.open(output_filename.c_str());
    out << (dataset + " = [") << endl;
    string input_filename = dataset + ".fid.txt";
    in.open(input_filename.c_str());
    string buffer;
    while (in.eof() == false)
    {
        getline(in, buffer);
        char* token = NULL;
        char temp[buffer.length()];
        buffer.copy(temp, buffer.length());
        //cout << buffer << endl;
        token = strtok(temp, " ");
        if (token == NULL)
            break;
        if (strcmp(token, "contour") == 0)
        {
            token = strtok(NULL, " ");
            token = strtok(NULL, " ");
            token = strtok(NULL, " ");
            istringstream num_points_(token);
            int num_points;
            num_points_ >> num_points;
            float temp[numFrames];
            for (int i = 0; i < numFrames; i++)
            {
                temp[i] = 0;
            }
            for (int i = 0; i < num_points; i++)
            {
                getline(in, buffer);
                //cout << buffer << endl;
                char temp2[buffer.length()];
                buffer.copy(temp2, buffer.length());
                int x, y, index;
                char* tok = NULL;
                tok = strtok(temp2, " ");
                istringstream xx(tok);
                xx >> x;
                tok = strtok(NULL, " ");
                istringstream yy(tok);
                yy >> y;
                tok = strtok(NULL, " ");
                istringstream ii(tok);
                ii >> index;
                index--;
                temp[index] = x;
            }
            for (int i = 0; i < numFrames; i++)
                out << temp[i] << ",";
            out << ";" << endl;
        }
        else
            continue;
    }
    in.close();
    out << "];" << endl;
    out.close();
}

void writeIMODtiltalign(string basename,ostream &out,int imgnumber , char* tltfile , char* newtlt)
{
    out<<"#If you know how to change the options for tiltalign in IMOD just edit this txt file"<<endl;
    out<<"#Execute from command line tiltalign -param "<<basename<<"_tiltalignScript.txt to align images with IMOD"<<endl;
    out<<"#Then execute from command line newstack -input ../prealign/"<<basename<<".preali -output "<<basename<<".ali -offset 0,0 -xform "<<basename<<".xf to align images with IMOD"<<endl<<endl<<endl;
    out<<"ModelFile	"<<basename<<"_cos.fid"<<endl;
    out<<"#ImageFile	"<<basename<<".preali"<<endl;
//out<<"OutputModelFile	"<<basename<<".3dmod"<<endl;
//out<<"OutputResidualFile	"<<basename<<".resid"<<endl;
//out<<"OutputFidXYZFile	"<<basename<<".xyz"<<endl;
    out<<"OutputTiltFile	"<<newtlt<<endl;
    out<<"OutputTransformFile	"<<basename<<"_fin.xf"<<endl;


    out<<"#THOSE ARE THE TWO MAIN PARAMETERS THAT NEED TO BE CHANGED!!!!!!!!!"<<endl;
    out<<"#estimated from our correspondence"<<endl;
    out<<"RotationAngle	 0"<<endl;
//out<<"#estimated from our correspondence using TxBR bundle adjustment"<<endl;

    //list of views to be included in alignment
    out<<"IncludeList ";
    bool flagFirst=true;
    for (unsigned int kk=0; kk<imgnumber; kk++)
    {
        out<<","<<kk+1;
    }
    out<<endl;
    //----------------------------------------

    out<<"TiltFile	"<<tltfile<<endl<<endl;

    out<<"#"<<endl;
    out<<"# ADD a recommended tilt angle change to the existing AngleOffset value: our estimation"<<endl;
    out<<"# should be pretty accurate"<<endl;
    out<<"#"<<endl;
    out<<"AngleOffset	0.0"<<endl;
    out<<"RotOption	3"<<endl;
    out<<"RotationFixedView 1"<<endl;
    out<<"RotDefaultGrouping	5"<<endl<<endl;

    out<<"#"<<endl;
    out<<"# TiltOption 0 fixes tilts, 2 solves for all tilt angles; change to 5 to solve"<<endl;
    out<<"# for fewer tilts by grouping views by the amount in TiltDefaultGrouping"<<endl;
    out<<"#"<<endl;
    out<<"TiltOption	5"<<endl;
    out<<"TiltFixedView 1"<<endl;
    out<<"TiltDefaultGrouping	5"<<endl;
    out<<"MagReferenceView	1"<<endl;
    out<<"MagOption	3"<<endl;
    out<<"MagDefaultGrouping	4"<<endl<<endl;

    out<<"CompOption 0"<<endl;
    out<<"#"<<endl;
    out<<"# To solve for distortion, change both XStretchOption and SkewOption to 3;"<<endl;
    out<<"# to solve for skew only leave XStretchOption at 0"<<endl;
    out<<"#"<<endl;
    out<<"XStretchOption	0"<<endl;
    out<<"XStretchDefaultGrouping	7"<<endl;
    out<<"SkewOption	0"<<endl;
    out<<"SkewDefaultGrouping	11"<<endl;
    out<<"# "<<endl;
    out<<"# Criterion # of S.Dfprintf above mean residual to report (- for local mean)"<<endl;
    out<<"#"<<endl;
    out<<"ResidualReportCriterion	2.0"<<endl;
    out<<"SurfacesToAnalyze	0"<<endl;
    out<<"MetroFactor	0.25"<<endl;
    out<<"MaximumCycles	5000"<<endl;
    out<<"#"<<endl;
    out<<"# ADD a recommended amount to shift up to the existing AxisZShift value"<<endl;
    out<<"#"<<endl;
    out<<"AxisZShift	0"<<endl;
    out<<"#"<<endl;
    out<<"# Set to 1 to do local alignments"<<endl;
    out<<"#"<<endl;
    out<<"LocalAlignments	0"<<endl;
    out<<"OutputLocalFile	DeinoG9blocal.xf"<<endl;
    out<<"#"<<endl;
    out<<"# Number of local patches to solve for in X and Y"<<endl;
    out<<"#"<<endl;
    out<<"NumberOfLocalPatchesXandY	5,5"<<endl;
    out<<"MinSizeOrOverlapXandY	0.5,0.5"<<endl;
    out<<"#"<<endl;
    out<<"# Minimum fiducials total and on one surface if two surfaces"<<endl;
    out<<"#"<<endl;
    out<<"MinFidsTotalAndEachSurface	8,3"<<endl;
    out<<"FixXYZCoordinates	0"<<endl;
    out<<"LocalOutputOptions	1,0,1"<<endl;
    out<<"LocalRotOption	3"<<endl;
    out<<"LocalRotDefaultGrouping	6"<<endl;
    out<<"LocalTiltOption	5"<<endl;
    out<<"LocalTiltDefaultGrouping	6"<<endl;
    out<<"LocalMagReferenceView	1"<<endl;
    out<<"LocalMagOption	3"<<endl;
    out<<"LocalMagDefaultGrouping	7"<<endl;
    out<<"LocalXStretchOption	0"<<endl;
    out<<"LocalXStretchDefaultGrouping	7"<<endl;
    out<<"LocalSkewOption	0"<<endl;
    out<<"LocalSkewDefaultGrouping	11"<<endl;
}

//famatnote: this is just a patch. I need a better solution to track duplicate points in the tajectories
void freeTrajectoryVector(vector<trajectory> T)
{
    //first pass is to identify repeated points
    for (vector<trajectory>::iterator iter=T.begin(); iter!=T.end(); ++iter)
    {
        //famatnote:if two trajectories are pointing to the same Point2D* this can cause segmentation fault!!!
        for (unsigned int kk=0; kk<iter->p.size(); kk++)
        {
            if (fabs(iter->p[kk]->x+10)<1e-3)//it means this pointer is repeated
            {
                iter->p[kk]=NULL;
            }
            else
            {
                iter->p[kk]->x=-10.0;
            }
        }
    }
    //free all the memory
    for (vector<trajectory>::iterator iter=T.begin(); iter!=T.end(); ++iter)
    {
        //famatnote:if two trajectories are pointing to the same Point2D* this can cause segmentation fault!!!
        for (unsigned int kk=0; kk<iter->p.size(); kk++)
        {
            if (iter->p[kk]!=NULL)
                delete iter->p[kk];//each iter->p[kk] is a Point2D*
        }
        iter->p.clear();
    }
}

vector<trajectory> fiducialModel2Trajectory(string fidTxtFilename)
{
    ifstream in(fidTxtFilename.c_str());
    if (!in.is_open())
    {
        cout<<"ERROR: opening fiducial model "<<fidTxtFilename<<endl;
        exit(-1);
    }

    vector<trajectory> T;
    int numTrajectories;
    string buffer;

    do
    {
        in >> buffer;
    }
    while (buffer != "object");

    in >> buffer;
    in >> numTrajectories;

    cout << "Number of trajectories: " << numTrajectories << endl;

    do
    {
        in >> buffer;
    }
    while (buffer != "contour");


    for (int i = 0; i < numTrajectories; i++)
    {
        trajectory* newTraj = new trajectory;
        int t1;
        int t2;
        int t3;
        in >> t1;
        in >> t2;
        in >> t3;

        for (int j = 0; j < t3; j++)
        {
            float p1;
            float p2;
            int p3;
            in >> p1;
            in >> p2;
            in >> p3;
            Point2D* newPoint = new Point2D;
            newPoint->x = p1;
            newPoint->y = p2;
            newPoint->frameID = p3;
            //cout << p1 << ", " << p2 << ", " << p3 << endl;
            (newTraj->p).push_back(newPoint);
        }
        T.push_back(*newTraj);
        delete newTraj;
        in >> buffer;
    }

    in.close();

    return T;
}


void writeIMODtiltScript(int W,int H,string pathTilt,string pathAli,string basename,int recoThickness,vector<frame> *frames,string endingRec)
{

//create tilt.com script to run reconstruction
//change some of teh parameters (like RADIAL or thickness if needed)

    ofstream fid((pathAli + "tilt.com").c_str());

    fid<<"# Command file to run Tilt "<<endl;
    fid<<"#"<<endl;
    fid<<"####CreatedVersion#### 3.6.9"<<endl;
    fid<<"# "<<endl;
    fid<<"# RADIAL specifies the frequency at which the Gaussian low pass filter begins"<<endl;
    fid<<"#   followed by the standard deviation of the Gaussian roll-off"<<endl;
    fid<<"#"<<endl;
    fid<<"# LOG takes the logarithm of tilt data after adding the given value"<<endl;
    fid<<"#"<<endl;

    fid<<"$tilt"<<endl;
    fid<<pathAli+basename<<".ali"<<endl;
    fid<<pathAli+basename<<endingRec<<endl;
    fid<<"IMAGEBINNED 1"<<endl;
    fid<<"FULLIMAGE "<<W<<" "<<H<<endl;
    fid<<"LOG 0.0"<<endl;
    fid<<"MODE 2"<<endl;//always mode 2 and then we rescale if necessary using trimvol
    fid<<"OFFSET 0.0"<<endl;
    fid<<"PARALLEL"<<endl;
    fid<<"RADIAL 0.35 0.05"<<endl;
    fid<<"SCALE 1.39 500"<<endl;
    fid<<"SHIFT 0.0 0.0"<<endl;
    fid<<"SUBSETSTART 0 0"<<endl;
    fid<<"THICKNESS "<<recoThickness<<endl;
    //fid<<"TILTFILE "<<pathTilt + basename<<".tlt"<<endl;
    ifstream inTlt((pathTilt + basename+".tlt").c_str());
    if (!inTlt.is_open())
    {
        cout<<"ERROR: opening tilt file "<<pathTilt + basename+".tlt"<<endl;
        cout<<"RAPTOR can not proceed with the reconstruction"<<endl;
        exit(-1);
    }
    float tltAngle;
    fid<<"ANGLE ";
    bool flagFirst=true;
    for (unsigned int kk=0; kk<frames->size(); kk++)
    {
        inTlt>>tltAngle;
        if (frames->at(kk).discard==false)
        {
            if (flagFirst)
            {
                fid<<tltAngle;
                flagFirst=false;
            }
            else fid<<","<<tltAngle;
        }
    }
    fid<<endl;
    inTlt.close();

    fid<<"XAXISTILT 0.0"<<endl;
    fid<<"DONE"<<endl;
    fid<<"$if (-e ./savework) ./savework"<<endl;


    fid.close();
}
int * getMarkerTypeFromTrajectory(vector<trajectory> T)
{
//     cout<<"getMarkerTypeFromTrajectory"<<endl;
    int *mType=new int[T.size()];
    //we average the marker type for all the points in each trajectory to find out each type
    int count=0;
    for (vector<trajectory>::iterator iter=T.begin(); iter!=T.end(); ++iter)
    {
        mType[count]=0;
        for (vector<Point2D*>::iterator iterP=iter->p.begin(); iterP!=iter->p.end(); ++iterP)
        {
            mType[count]+=(*iterP)->markerType;
        }
        mType[count]=(int)floor(0.5+(float)(mType[count])/(float)(iter->p.size()));
        count++;
    }
    return mType;
}