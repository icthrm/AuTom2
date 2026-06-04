#include "opts.h"
#include <iostream>
#include <fstream>
#include "dataf/dataf.h"
#include "modelmatch/match_core.h"
#include "detector/detector.h"
#include "nbundle/bundle_core.h"

#ifdef HAVE_MPI
#include <mpi.h>
#define RANK0 if(mpi_rank==0)
#else
#define RANK0 if(1)
#endif

// #define TESTDEVELOP

using namespace std;
// using namespace ann_1_1_char;


int main(int argc, char **argv)
{
#ifdef HAVE_MPI
    MPI_Init(&argc, &argv);
    int mpi_rank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
#endif

    int tmp_dia=24;

    EX_TRACE("\nMARKERAUTO --version 2.0\n")

    struct options opts;
    opts.diameter = -1;
    opts.verbose = 0;
    opts.rotation_angle = 0;
	opts.testmode = false;
	opts.split_angle = 45;
    
    if(GetOpts(argc, argv, &opts) <= 0) {
        EX_TRACE("***WRONG INPUT.\n");
        return -1;
    }
    
    vector<float> angles;
    vector<float> angles_heigh;
    vector<float> angles_new;
    vector<float> angles_po;
    vector<float> angles_neg;
    vector<int> m_index2;
    vector<int> m_index_high;
    vector<int> m_index_low;
    if(!util::ReadAnglesByName(opts.inputangle, &angles)) {
        std::cout<<"Can't open tilt angle file."<<endl;
        return -1;
    }
    
    for(int i=0;i<angles.size();i++)
    {
        m_index2.push_back(i);
        if(opts.split_angle <= 0 || abs(angles[i]) < opts.split_angle)
        {
            angles_new.push_back(angles[i]);
            m_index_low.push_back(i);
        }
        else{
            angles_heigh.push_back(angles[i]);
            m_index_high.push_back(i);
        }
        if(opts.split_angle > 0 && angles[i] > opts.split_angle) angles_po.push_back(angles[i]);
        if(opts.split_angle > 0 && angles[i] < -opts.split_angle) angles_neg.push_back(angles[i]);
    }
//     std::cout<<m_index_high.size()<<std::endl;
//     std::cout<<m_index_low.size()<<std::endl;
//     for(int i=0;i<angles_new.size();i++)
//     {
//         std::cout<<"new:"<<angles_new[i]<<std::endl;
//         std::cout<<"index:"<<m_index[i]<<std::endl;
//     }
    
    util::MrcStack mrcs;
    mrcs.Open(opts.input);

    util::FiducialStack fidstack;
//     fidstack.ReSize(angles_new.size());
//     fidstack.ReadFidsByFile("fids.txt");

#ifndef TESTDEVELOP
    EX_TIME_BEGIN("\n%sDo DetectorMain", _WAVE)
//     Detector::DetectorMain2(mrcs, &fidstack, opts.diameter, m_index_low, 1);
    Detector::DetectorMain(mrcs, &fidstack, opts.diameter, m_index_low, 1);
    RANK0 {
        fidstack.WriteFidsByFile("fids.txt");
    }
//  if(1) {
//         Detector::Test(mrcs, fidstack, opts.diameter);
//     }
    if(opts.verbose >= 1) {
        RANK0 {
            Detector::Test(mrcs, fidstack, opts.diameter);
        }
    }

    EX_TIME_END("Do DetectorMain")
#else
    fidstack.ReadFidsByFile("fids.txt");
#endif

//     std::cout<<m_index_low.size()<<std::endl;
    util::FiducialStack fidstack_low, fidstack_high;
    fidstack_low.SetWxH(fidstack.Width(), fidstack.Height());;
    fidstack_high.SetWxH(fidstack.Width(), fidstack.Height());;
    util::FiducialStack::div(m_index_high, m_index_low, &fidstack_low, &fidstack_high, &fidstack);
//     std::cout<<fidstack.Size()<<std::endl;
//     fidstack_low.WriteFidsByFile("fids_low.txt");
//     fidstack_high.WriteFidsByFile("fids_high.txt");
    int med=mrcs.Size()/2;
    int tmp=0;
//     int sum_high=0;
    int sum_low=0;

    for(int i=med-1;i<med+2;i++)
    {
        std::vector<util::point2d>& fids = fidstack.V(i);
        sum_low=sum_low+fids.size();
        tmp=tmp+1;
    }
//     std::cout<<sum_low/tmp<<std::endl;
    int number_low=sum_low/tmp;
//     std::cout<<tmp<<std::endl;


    bool method=true;
    if(fidstack.V(0).size()<95  || number_low<48) method=false;
    
    util::ImgMatchVector imvector;
    HSetVector hset;
// #undef TESTDEVELOP
#ifndef TESTDEVELOP
    cv::Mat tmplt;
    util::SeriesReadFromFile(&tmplt, "avgtmplt");
    EX_TIME_BEGIN("\n%sDo MatchMain", _WAVE)
//     std::cout<<tmplt.size().width<<std::endl;
//     ModelMatch::MatchMain(fidstack_low, angles_new, &imvector, &hset, 0.85*tmp_dia, true, opts.testmode);	//0.85
    ModelMatch::MatchMain(fidstack_low, angles_new, &imvector, &hset, 0.85*tmplt.size().width, method, true, opts.testmode);
    // ModelMatch::MatchMain(fidstack_low, angles_new, &imvector, &hset, 0.85*tmplt.size().width, true, opts.testmode);	//0.85   11.34 23  19
    RANK0 {
        imvector.WriteVectorByFolder("matches_iter1");
        hset.WriteVectorByFolder("transmx_iter1");
    }
    //     ModelMatch::MatchMain(fidstack, angles, &imvector_all, &hset_all, 0.5*tmplt.size().width, true, opts.testmode);	//0.85
    //     imvector_all.WriteVectorByFolder("matches_all");
    //     hset_all.WriteVectorByFolder("transmx_all");

    if(opts.verbose >= 1) {
        RANK0 {
            ModelMatch::Test(mrcs, imvector, 0.5f, "matches_ill");
        }
    }

    EX_TIME_END("Do MatchMain")
#else
    imvector.ReadVectorByFolder("matches");
    hset.ReadVectorByFolder("transmx");
#endif
    
    util::TrackSpace trackspace;
    trackspace.Create(imvector, angles_new);

    util::FiducialStack addedfsk;
    util::ImgMatchVector addedimv;
    
#ifndef TESTDEVELOP
#ifdef HAVE_MPI
    if (mpi_rank == 0) {
#endif
        Detector::LocalDetectorMain(mrcs, trackspace, hset, -1, &addedfsk, &addedimv, m_index_low);
        addedfsk.WriteFidsByFile("addfids_iter1.txt");
        addedimv.WriteVectorByFolder("addimvec_iter1");
#ifdef HAVE_MPI
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if (mpi_rank != 0) {
        addedfsk.ReadFidsByFile("addfids_iter1.txt");
        addedimv.ReadVectorByFolder("addimvec_iter1");
    }
#endif
#else
    addedfsk.ReadFidsByFile("addfids.txt");
    addedimv.ReadVectorByFolder("addimvec");
#endif
// 	Detector::Test(mrcs, addedfsk, opts.diameter, "addedfsk");
// 	ModelMatch::Test(mrcs, addedimv, 0.5f, "addedimv_ill");
    

    trackspace.InsertMatchVector(addedimv);


    trackspace.CoordinateTransform(fidstack.Width(), fidstack.Height());

    if(opts.rotation_angle < -0.01 || opts.rotation_angle > 0.01) {
        EX_TRACE("Do pre-rotation of series...\n")
        trackspace.PreRotate(-DEG2RAD(opts.rotation_angle));
    }
    
    std::vector<mx::pproj_params> cameras;
//     std::vector<mx::pproj_params> cameras_tmp;
    std::vector<mx::pproj_params> cameras_tmp;
    std::vector<v3_t> points;
    EX_TIME_BEGIN("\n%sDo BundleMain", _WAVE)
//     fidstack.Width()
    PPBundleApp::BundleMain(trackspace, fidstack.Width()/*1024*//*featsk.Width()*/, fidstack.Height() /*1024*//*featsk.Height()*/, &cameras, &cameras_tmp, &points, true);
    RANK0 {
        PPBundleApp::PrintCamerasAsIMOD(cameras, -DEG2RAD(opts.rotation_angle), 1, opts.outputxf, "xtiltangle_iter1.txt", opts.outputangle, "invalid_iter1.txt");
    }
    EX_TIME_END("Do BundleMain")

    if(opts.split_angle <= 0) {
        // -s 0: 第一遍已处理所有角度，直接输出最终结果，跳过后续第二遍流程
        RANK0 {
            PPBundleApp::PrintCamerasAsIMOD(cameras, -DEG2RAD(opts.rotation_angle), 1, opts.outputxf, "xtiltangle.txt", opts.outputangle, "invalid.txt");
        }
    } else {
        std::vector<mx::pproj_params> cameras_new;
//     std::vector<util::point2d> fid;
    util::FiducialStack addfids;
    util::FiducialStack proaddfids;
    addfids.SetWxH(mrcs.Width(), mrcs.Height());
    PPBundleApp::CreateMain(angles_heigh, &cameras_new, points, &addfids);
    

    util::ImgMatchVector imvector2;
    HSetVector hset2;
    ModelMatch::MatchSameAngleMain(fidstack_high, addfids, angles_heigh, &imvector2, &hset2, 0.85*tmplt.size().width, true, opts.testmode);	//0.85
//     ModelMatch::MatchSameAngleMain(fidstack_high, addfids, angles_heigh, &imvector2, &hset2, 0.85*tmp_dia, true, opts.testmode);	//0.85
//     imvector2.WriteVectorByFolder("matches2");
//     hset2.WriteVectorByFolder("transmx2");
    util::FiducialStack pro_fids;
    util::ImgMatchVector imvector3;
    HSetVector hset3;

    util::FiducialStack allfids;
    util::FiducialStack fidstack_new;
    Detector::TransMain(mrcs, hset2, &fidstack_high, &addfids, &proaddfids, &pro_fids, &allfids, -1, m_index_high);
//     util::FiducialStack::comb(m_index_high, m_index_low, &fidstack_low, &fidstack_high, &fidstack_new);      //test
    util::FiducialStack::comb(m_index_high, m_index_low, &fidstack_low, &allfids, &fidstack_new);
    RANK0 {
        proaddfids.WriteFidsByFile("proadd.txt");
    }
//     Detector::TransMain2(imvector3, hset2, &addfids, &pro_fids);
//     fidstack_new.WriteFidsByFile("fids_new");
    fidstack_new.SetWxH(mrcs.Width(), mrcs.Height());
    vector<float> angles_final;
    util::ImgMatchVector imvector_new;
    HSetVector hset_new;
    EX_TIME_BEGIN("\n%sDo MatchMain", _WAVE)
    // ModelMatch::MatchMain(fidstack_new, angles, &imvector_new, &hset_new, 0.85*tmplt.size().width, true, opts.testmode);	//0.85
    ModelMatch::MatchMain(fidstack_new, angles, &imvector_new, &hset_new, 0.85*tmplt.size().width, method, true, opts.testmode);
//     ModelMatch::MatchMain(fidstack_new, angles, &imvector_new, &hset_new, 0.85*tmp_dia, true, opts.testmode);	//0.85  23
    RANK0 {
        imvector_new.WriteVectorByFolder("matches");
        hset_new.WriteVectorByFolder("transmx");
    }
    EX_TIME_END("Do MatchMain")
//  if(1) {
//         ModelMatch::Test(mrcs, imvector_new, 0.5f, "matches_ill");
//     }

    util::TrackSpace trackspace2;
    trackspace2.Create(imvector_new, angles);

    util::FiducialStack addedfsk2;
    util::ImgMatchVector addedimv2;
#ifndef TESTDEVELOP
#ifdef HAVE_MPI
    if (mpi_rank == 0) {
#endif
        Detector::LocalDetectorMain(mrcs, trackspace2, hset_new, -1, &addedfsk2, &addedimv2, m_index2);
        addedfsk2.WriteFidsByFile("addfids.txt");
        addedimv2.WriteVectorByFolder("addimvec");
#ifdef HAVE_MPI
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if (mpi_rank != 0) {
        addedfsk2.ReadFidsByFile("addfids.txt");
        addedimv2.ReadVectorByFolder("addimvec");
    }
#endif
#else
    addedfsk.ReadFidsByFile("addfids.txt");
    addedimv.ReadVectorByFolder("addimvec");
#endif

    trackspace2.InsertMatchVector(addedimv2);



    trackspace2.CoordinateTransform(fidstack_new.Width(), fidstack_new.Height());

    if(opts.rotation_angle < -0.01 || opts.rotation_angle > 0.01) {
        EX_TRACE("Do pre-rotation of series...\n")
        trackspace2.PreRotate(-DEG2RAD(opts.rotation_angle));
    }

    std::vector<mx::pproj_params> cameras2;
    std::vector<v3_t> points2;
    ModelMatch::InitCamera(angles, hset_new, cameras, &cameras_tmp, m_index_low);

    EX_TIME_BEGIN("\n%sDo BundleMain", _WAVE)
    PPBundleApp::BundleMain(trackspace2, 0/*1024*//*featsk.Width()*/, 0/*1024*//*featsk.Height()*/, &cameras2, &cameras_tmp, &points2, false);
    RANK0 {
        PPBundleApp::PrintCamerasAsIMOD(cameras2, -DEG2RAD(opts.rotation_angle), 1, opts.outputxf, "xtiltangle.txt", opts.outputangle, "invalid.txt");
    }
    EX_TIME_END("Do BundleMain")
    }
    mrcs.Close();
#ifdef HAVE_MPI
    MPI_Finalize();
#endif
    return 0;
}
