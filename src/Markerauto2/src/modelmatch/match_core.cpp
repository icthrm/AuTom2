#include "match_core.h"
#include <opencv2/opencv.hpp>
#include <iomanip>
#include <sys/stat.h>
#include <unistd.h>
#include <levmar.h>
#include "matrix/matrix.h"
#include <sstream>

#ifdef HAVE_MPI
#include <mpi.h>

static std::string SerializeImgMatches(const std::vector<util::img_match>& matches, const std::vector<int>& indices) {
    std::ostringstream oss;
    oss << matches.size() << "\n";
    for (size_t i = 0; i < matches.size(); i++) {
        oss << indices[i] << " " << matches[i].idx1 << " " << matches[i].idx2 << " " << matches[i].pairs.size() << "\n";
        for (size_t j = 0; j < matches[i].pairs.size(); j++) {
            oss << matches[i].pairs[j].first.x << " " << matches[i].pairs[j].first.y << " "
                << matches[i].pairs[j].second.x << " " << matches[i].pairs[j].second.y << "\n";
        }
    }
    return oss.str();
}

static void DeserializeImgMatches(const std::string& str, util::ImgMatchVector* imvector) {
    std::istringstream iss(str);
    std::vector<std::pair<int, util::img_match> > indexed_matches;
    while (true) {
        size_t nmatches;
        if (!(iss >> nmatches)) break;
        for (size_t i = 0; i < nmatches; i++) {
            int gidx;
            util::img_match im;
            size_t npairs;
            iss >> gidx >> im.idx1 >> im.idx2 >> npairs;
            im.pairs.resize(npairs);
            for (size_t j = 0; j < npairs; j++) {
                iss >> im.pairs[j].first.x >> im.pairs[j].first.y
                    >> im.pairs[j].second.x >> im.pairs[j].second.y;
            }
            indexed_matches.push_back(std::make_pair(gidx, im));
        }
    }
    std::sort(indexed_matches.begin(), indexed_matches.end(),
              [](const std::pair<int, util::img_match>& a, const std::pair<int, util::img_match>& b) {
                  return a.first < b.first;
              });
    for (size_t i = 0; i < indexed_matches.size(); i++) {
        util::img_match& dest = imvector->MallocNewMatch();
        dest = indexed_matches[i].second;
    }
}

static std::string SerializeHSets(const std::vector<h_set>& hsets, const std::vector<int>& indices) {
    std::ostringstream oss;
    oss << hsets.size() << "\n";
    for (size_t i = 0; i < hsets.size(); i++) {
        oss << indices[i] << " " << hsets[i].idx1 << " " << hsets[i].idx2 << " " << hsets[i].h.size() << "\n";
        for (size_t j = 0; j < hsets[i].h.size(); j++) {
            for (int r = 0; r < 3; r++) {
                for (int c = 0; c < 3; c++) {
                    oss << hsets[i].h[j].at<double>(r, c) << " ";
                }
            }
            oss << "\n";
        }
    }
    return oss.str();
}

static void DeserializeHSets(const std::string& str, HSetVector* hsetvec) {
    std::istringstream iss(str);
    std::vector<std::pair<int, h_set> > indexed_hsets;
    while (true) {
        size_t nhsets;
        if (!(iss >> nhsets)) break;
        for (size_t i = 0; i < nhsets; i++) {
            int gidx;
            h_set hs;
            size_t nmat;
            iss >> gidx >> hs.idx1 >> hs.idx2 >> nmat;
            hs.h.resize(nmat);
            for (size_t j = 0; j < nmat; j++) {
                hs.h[j] = cv::Mat::zeros(3, 3, CV_64FC1);
                for (int r = 0; r < 3; r++) {
                    for (int c = 0; c < 3; c++) {
                        double val;
                        iss >> val;
                        hs.h[j].at<double>(r, c) = val;
                    }
                }
            }
            indexed_hsets.push_back(std::make_pair(gidx, hs));
        }
    }
    std::sort(indexed_hsets.begin(), indexed_hsets.end(),
              [](const std::pair<int, h_set>& a, const std::pair<int, h_set>& b) {
                  return a.first < b.first;
              });
    for (size_t i = 0; i < indexed_hsets.size(); i++) {
        h_set& dest = hsetvec->MallocNewHSet();
        dest = indexed_hsets[i].second;
    }
}

static void GatherAndBroadcastStrings(const std::string& local_str, std::string& global_str) {
    int rank = 0, num_procs = 1;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    int local_size = local_str.size();
    std::vector<int> all_sizes(num_procs);
    MPI_Gather(&local_size, 1, MPI_INT, &all_sizes[0], 1, MPI_INT, 0, MPI_COMM_WORLD);

    std::vector<int> displs(num_procs, 0);
    if (rank == 0) {
        displs[0] = 0;
        int total = all_sizes[0];
        for (int r = 1; r < num_procs; r++) {
            displs[r] = displs[r - 1] + all_sizes[r - 1];
            total += all_sizes[r];
        }
        global_str.resize(total);
    }

    MPI_Gatherv(local_str.empty() ? NULL : &local_str[0], local_size, MPI_CHAR,
                global_str.empty() ? NULL : &global_str[0], &all_sizes[0], &displs[0], MPI_CHAR,
                0, MPI_COMM_WORLD);

    int total_size = global_str.size();
    MPI_Bcast(&total_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (rank != 0) global_str.resize(total_size);
    MPI_Bcast(global_str.empty() ? NULL : &global_str[0], total_size, MPI_CHAR, 0, MPI_COMM_WORLD);
}
#endif

#define PI_180				0.0174532925199433

void ModelMatch::PairMatch(std::vector< util::point2d >& __fids1, std::vector< util::point2d >& __fids2, double beta1, double beta2, double dist_err_tol,
						   std::vector<std::pair<util::point2d, util::point2d> >& matchvec, std::vector<cv::Mat>& hset, bool do_test, int idx1, int idx2)
{
	Ran4PEstimator ran4PEstimator(__fids1, __fids2, 0);
	srand((unsigned)(idx1 * 7919 + idx2 * 104729 + 1234567));
	ran4PEstimator.SetSupplementData(beta1, beta2);
	ran4PEstimator.AffineTransformEstimation(dist_err_tol, matchvec, hset, false, do_test);	//ran4p use half diameter
	
	if(!ran4PEstimator.UmFids1().size()|!ran4PEstimator.UmFids2().size()){		//there are no obvious warp
	}
	else{
		CPDEstimator cpdEstimator(ran4PEstimator.UmFids2(), ran4PEstimator.UmFids1(), 0);		//the first is fixed and the second is moving; therefore, it is inverse with 4p method
		cpdEstimator.PointDriftEstimation(dist_err_tol, matchvec, hset, false, false);		//CPD use full diameter
	}
}

void ModelMatch::MatchSameAngleMain(util::FiducialStack& O_fstack, util::FiducialStack& P_fstack, const std::vector<float>& angles, util::ImgMatchVector* imvector, HSetVector* hsetvec, float dist_err_tol, bool eigenlimit, bool do_test)
{
    int rank = 0, num_procs = 1;
#ifdef HAVE_MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
#endif

    EX_TRACE("Used distance threshold = %.2f\n", dist_err_tol)

    imvector->Clear();
	hsetvec->Clear();

    int turn = 0;
    std::cout<<O_fstack.Width()<<std::endl;
    int sampling = O_fstack.Width()*O_fstack.Height()/(dist_err_tol*dist_err_tol*4)*.5;

    std::vector<util::img_match> local_matches;
    std::vector<h_set> local_hsets;
    std::vector<int> local_indices;

    for(int i=0;i<O_fstack.Size();i++)
    {
        if (i % num_procs != rank) continue;

        EX_TIME_BEGIN("#\nMatching Point Set (MRC[%d] & MRC[%d])", i, i)
        util::img_match imatch;
        h_set hset;
        imatch.idx1 = i;
        imatch.idx2 = i;
        hset.idx1 = i;
        hset.idx2 = i;

        if(0){
            if(1){
                CPDEstimator cpdEstimator(P_fstack.V(i), O_fstack.V(i), 1);
                cpdEstimator.GlobalMatch(1.7*dist_err_tol, imatch.pairs, hset.h, false, eigenlimit);
                if(imatch.size() == 0){
                    Ran4PEstimator ran4PEstimator(P_fstack.V(i), O_fstack.V(i), 1);
                    ran4PEstimator.GlobalMatch(dist_err_tol, imatch.pairs, hset.h, false, eigenlimit);
                }
                if(sampling < 2500){
                    EX_TRACE("Low sampling --> Distribution correction is turned on.\n");
                    cpdEstimator.DistributionCorrection(4*dist_err_tol, imatch.pairs);
                }
            }
            else{
                Ran4PEstimator ran4PEstimator(P_fstack.V(i), O_fstack.V(i), 1);
                ran4PEstimator.GlobalMatch(dist_err_tol, imatch.pairs, hset.h, false, eigenlimit);
                if(sampling < 2500){
                    EX_TRACE("Low sampling --> Distribution correction is turned on.\n");
                    ran4PEstimator.DistributionCorrection(4*dist_err_tol, imatch.pairs);
                }
            }
        }
        else
        {
            PairMatch(P_fstack.V(i), O_fstack.V(i), angles[i], angles[i], dist_err_tol, imatch.pairs, hset.h, false, i, i);
        }

        if(imatch.size() < 5 || (imatch.size() < 9 && imatch.size() < P_fstack.V(i).size()*CPDEstimator::COVERAGE_REFRESH_RATIO*.25 &&
            imatch.size() < O_fstack.V(i).size()*CPDEstimator::COVERAGE_REFRESH_RATIO*.25) ){
            imatch.pairs.clear();
            hset.h.clear();
        }

        local_matches.push_back(imatch);
        local_hsets.push_back(hset);
        local_indices.push_back(i);

        EX_TIME_END("MRC[%d] & MRC[%d]: %ld/(%ld,%ld) Pairs found", i, i, imatch.size(), P_fstack.V(i).size(), O_fstack.V(i).size())
    }

#ifdef HAVE_MPI
    std::string match_str = SerializeImgMatches(local_matches, local_indices);
    std::string hset_str = SerializeHSets(local_hsets, local_indices);
    std::string global_match_str, global_hset_str;
    GatherAndBroadcastStrings(match_str, global_match_str);
    GatherAndBroadcastStrings(hset_str, global_hset_str);

    imvector->Clear();
    hsetvec->Clear();
    DeserializeImgMatches(global_match_str, imvector);
    DeserializeHSets(global_hset_str, hsetvec);
    MPI_Barrier(MPI_COMM_WORLD);
#else
    for (size_t i = 0; i < local_matches.size(); i++) {
        util::img_match& dest = imvector->MallocNewMatch();
        dest = local_matches[i];
        h_set& hdest = hsetvec->MallocNewHSet();
        hdest = local_hsets[i];
    }
#endif
}

void ModelMatch::MatchSplitMain(util::FiducialStack& fstack, const std::vector<float>& angles, util::ImgMatchVector* imvector, HSetVector* hsetvec, float dist_err_tol,int num_po, bool eigenlimit, bool do_test)
{
    EX_TRACE("Used distance threshold = %.2f\n", dist_err_tol)
	std::cout<<num_po<<std::endl;
	#define STEP_ARRAY_SIZE			2
    assert(STEP_ARRAY_SIZE < fstack.Size());

    imvector->Clear();
	hsetvec->Clear();

    int step_length[STEP_ARRAY_SIZE];
    for(int i = 0; i < STEP_ARRAY_SIZE; i++){
        step_length[i] = i+1;
    }

    int turn = 0;

	int sampling = fstack.Width()*fstack.Height()/(dist_err_tol*dist_err_tol*4)*.5;

    while(turn < STEP_ARRAY_SIZE){
        for(int i = 0; i+step_length[turn] < fstack.Size(); i++){
            int idx1 = i;//14;//6;//
            int idx2 = i+step_length[turn];//16;//7

            if(i==num_po-2) i=i+1;

            if(fstack.V(idx1).size() > fstack.V(idx2).size()){		//this strategy is for Ran4PEstimator
                    int tmp = idx1;
                    idx1 = idx2;
                    idx2 = tmp;
            }

            EX_TIME_BEGIN("#\nMatching Point Set (MRC[%d] & MRC[%d])", idx1, idx2)
            util::img_match& imatch = imvector->MallocNewMatch();
            imatch.idx1 = idx1;
            imatch.idx2 = idx2;

			h_set& hset = hsetvec->MallocNewHSet();
			hset.idx1 = idx1;
			hset.idx2 = idx2;
            PairMatch(fstack.V(idx1), fstack.V(idx2), angles[idx1], angles[idx2], dist_err_tol, imatch.pairs, hset.h, false, idx1, idx2);

			if(imatch.size() < 5 || (imatch.size() < 9 && imatch.size() < fstack.V(idx1).size()*CPDEstimator::COVERAGE_REFRESH_RATIO*.25 &&
				imatch.size() < fstack.V(idx2).size()*CPDEstimator::COVERAGE_REFRESH_RATIO*.25) ){
				imatch.pairs.clear();
				hset.h.clear();
			}

            EX_TIME_END("MRC[%d] & MRC[%d]: %ld/(%ld,%ld) Pairs found", idx1, idx2, imatch.size(), fstack.V(idx1).size(), fstack.V(idx2).size())
        }
        turn++;
    }
}

void ModelMatch::MatchMain(util::FiducialStack& fstack,const std::vector<float>& angles, util::ImgMatchVector* imvector, HSetVector* hsetvec, float dist_err_tol, bool method, bool eigenlimit, bool do_test)
{
    int rank = 0, num_procs = 1;
#ifdef HAVE_MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
#endif

	EX_TRACE("Used distance threshold = %.2f\n", dist_err_tol)

	#define STEP_ARRAY_SIZE			2
    assert(STEP_ARRAY_SIZE < fstack.Size());

    imvector->Clear();
	hsetvec->Clear();

    int step_length[STEP_ARRAY_SIZE];
    for(int i = 0; i < STEP_ARRAY_SIZE; i++){
        step_length[i] = i+1;
    }

    int turn = 0;

	int sampling = fstack.Width()*fstack.Height()/(dist_err_tol*dist_err_tol*4)*.5;

    bool new_method=true;

    std::vector<util::img_match> local_matches;
    std::vector<h_set> local_hsets;
    std::vector<int> local_indices;
    int global_idx = 0;

    while(turn < STEP_ARRAY_SIZE){
        for(int i = 0; i+step_length[turn] < fstack.Size(); i++){
            if (global_idx % num_procs != rank) {
                global_idx++;
                continue;
            }
            int cur_gidx = global_idx;
            global_idx++;

            int idx1 = i;
            int idx2 = i+step_length[turn];

            if(!method)
            {
                if(fstack.V(idx1).size() < fstack.V(idx2).size()){
					int tmp = idx1;
					idx1 = idx2;
					idx2 = tmp;
				}
            }
            else{
                if(fstack.V(idx1).size() > fstack.V(idx2).size()){
                    int tmp = idx1;
                    idx1 = idx2;
                    idx2 = tmp;
                }
            }

            EX_TIME_BEGIN("#\nMatching Point Set (MRC[%d] & MRC[%d])", idx1, idx2)

            util::img_match imatch;
            h_set hset;
            imatch.idx1 = idx1;
            imatch.idx2 = idx2;
            hset.idx1 = idx1;
            hset.idx2 = idx2;

            if(!method){
                if(0){
                    std::cout<<"old"<<std::endl;
                    CPDEstimator cpdEstimator(fstack.V(idx1), fstack.V(idx2), 1);
                    cpdEstimator.GlobalMatch(1.7*dist_err_tol, imatch.pairs, hset.h, false, eigenlimit);

                    if(imatch.size() == 0){
                        Ran4PEstimator ran4PEstimator(fstack.V(idx1), fstack.V(idx2), 1);
                        ran4PEstimator.GlobalMatch(dist_err_tol, imatch.pairs, hset.h, false, eigenlimit);
                    }
                    if(sampling < 2500){
                        EX_TRACE("Low sampling --> Distribution correction is turned on.\n");
                        cpdEstimator.DistributionCorrection(4*dist_err_tol, imatch.pairs);
                    }
                }
                else{
                    Ran4PEstimator ran4PEstimator(fstack.V(idx1), fstack.V(idx2), 1);
                    srand((unsigned)(idx1 * 7919 + idx2 * 104729 + 1234567));
                    ran4PEstimator.GlobalMatch(dist_err_tol, imatch.pairs, hset.h, false, eigenlimit);
                    if(sampling < 2500){
                        EX_TRACE("Low sampling --> Distribution correction is turned on.\n");
                        ran4PEstimator.DistributionCorrection(4*dist_err_tol, imatch.pairs);
                    }
                }
            }
            else
            {
                PairMatch(fstack.V(idx1), fstack.V(idx2), angles[idx1], angles[idx2], dist_err_tol, imatch.pairs, hset.h, true, idx1, idx2);
            }

            if(imatch.size() < 5 || (imatch.size() < 9 && imatch.size() < fstack.V(idx1).size()*CPDEstimator::COVERAGE_REFRESH_RATIO*.25 &&
                imatch.size() < fstack.V(idx2).size()*CPDEstimator::COVERAGE_REFRESH_RATIO*.25) ){
                imatch.pairs.clear();
                hset.h.clear();
            }

            local_matches.push_back(imatch);
            local_hsets.push_back(hset);
            local_indices.push_back(cur_gidx);

            EX_TIME_END("MRC[%d] & MRC[%d]: %ld/(%ld,%ld) Pairs found", idx1, idx2, imatch.size(), fstack.V(idx1).size(), fstack.V(idx2).size())
        }
        turn++;
    }

#ifdef HAVE_MPI
    std::string match_str = SerializeImgMatches(local_matches, local_indices);
    std::string hset_str = SerializeHSets(local_hsets, local_indices);
    std::string global_match_str, global_hset_str;
    GatherAndBroadcastStrings(match_str, global_match_str);
    GatherAndBroadcastStrings(hset_str, global_hset_str);

    imvector->Clear();
    hsetvec->Clear();
    DeserializeImgMatches(global_match_str, imvector);
    DeserializeHSets(global_hset_str, hsetvec);
    MPI_Barrier(MPI_COMM_WORLD);
#else
    for (size_t i = 0; i < local_matches.size(); i++) {
        util::img_match& dest = imvector->MallocNewMatch();
        dest = local_matches[i];
        h_set& hdest = hsetvec->MallocNewHSet();
        hdest = local_hsets[i];
    }
#endif
}


void ModelMatch::DrawMatch(cv::Mat& canvas, const cv::Mat& img1, const cv::Mat& img2, const std::vector<std::pair<util::point2d, util::point2d> >& vpair)
{
//     cv::Zero(canvas);
    canvas.setTo(0);
//     std::cout<<img1.size()<<std::endl;
//     std::cout<<img2.size()<<std::endl;
//     std::cout<<canvas.size()<<std::endl;
// //     std::cout<<canvas.size()<<std::endl;
//     cv::Mat tmp=img1.clone();
//     cv::Mat tmp2=cv::Mat::zeros(3,3,CV_32FC1);
//     cv::Mat tmp3=cv::Mat::zeros(3,3,CV_32FC1);
//     cv::Mat tmp4=cv::Mat::zeros(3,3,CV_32FC1);
    cv::Mat roilimage = canvas(cv::Rect(0, 0, img1.size().width, img1.size().height));
//     cvSetImageROI(canvas, cvRect(0, 0, img1->width, img1->height));
//     std::cout<<roilimage.type()<<"   "<<canvas.type()<<std::endl;
//     cv::add(img1, roilimage, roilimage);
    img1.copyTo(roilimage);
    cv::Mat roilimage2 = canvas(cv::Rect(img1.size().width, 0, img2.size().width, img2.size().height));
    img2.copyTo(roilimage2);
//     cv::Mat roilimage2 = canvas(cv::Rect(img1.size().width, 0, img2.size().width, img2.size().height));
//         std::cout<<roilimage2.size()<<std::endl;
//     cvSetImageROI(canvas, cvRect(img1->width, 0, img1->width+img2->width, img2->height));
//     cv::add(img2, roilimage2, roilimage2);
    for(int i = 0; i < vpair.size(); i++) {
        cv::Scalar color = CV_RGB(255, 255, 255);
//         util::DrawX(canvas, vpair[i].first.x, vpair[i].first.y);
//         util::DrawX(canvas, img1->width+vpair[i].second.x, vpair[i].second.y);
		util::DrawLine(canvas, vpair[i].first, util::_point(img1.size().width+vpair[i].second.x, vpair[i].second.y));
    }
}

void ModelMatch::Test(util::MrcStack& mrcr, const util::ImgMatchVector& imvector, float ratio, const char* folder)
{
    EX_TIME_BEGIN("%sMatch Testing", _DASH)
    cv::Mat p[2], canvas, tmp;
//     IplImage* p[2], * canvas, *tmp;
    canvas = cv::Mat::zeros(cv::Size(mrcr.Width()*2*ratio, mrcr.Height()*ratio), CV_32FC1);
//     std::cout<<mrcr.Width()*2*ratio<<std::endl;
//     canvas = cvCreateImage(cvSize(mrcr.Width()*2*ratio, mrcr.Height()*ratio), IPL_DEPTH_32F, 1);

    for(int i = 0; i < imvector.Size(); i++) {
        tmp = mrcr.GetStackImage(imvector[i].idx1);
        p[0] = cv::Mat::zeros(cv::Size(tmp.size().width*ratio, tmp.size().height*ratio), CV_32FC1);
        cv::resize(tmp, p[0], cv::Size(tmp.size().width*ratio, tmp.size().height*ratio), 0 , 0, cv::INTER_CUBIC);
        util::ConvertTo1(p[0], true);

        tmp = mrcr.GetStackImage(imvector[i].idx2);
        p[1] = cv::Mat::zeros(cv::Size(tmp.size().width*ratio, tmp.size().height*ratio), CV_32FC1);
        cv::resize(tmp, p[1], cv::Size(tmp.size().width*ratio, tmp.size().height*ratio), 0 , 0, cv::INTER_CUBIC);
        util::ConvertTo1(p[1], true);

		std::vector<std::pair<util::point2d, util::point2d> > tmppairs;
		for(int j = 0; j < imvector[i].pairs.size(); j++){
			util::point2d pair1, pair2;
			pair1.x = imvector[i].pairs[j].first.x*ratio;
			pair1.y = imvector[i].pairs[j].first.y*ratio;
			pair2.x = imvector[i].pairs[j].second.x*ratio;
			pair2.y = imvector[i].pairs[j].second.y*ratio;
			tmppairs.push_back(std::make_pair(pair1, pair2));
		}

        DrawMatch(canvas, p[0], p[1], tmppairs);
        if(access(folder,0) == -1) {		//create file folder
            mkdir(folder,0777);
        }
        std::ostringstream oss;
//         oss <<folder<<"/"<<"("<<imvector[i].idx1<<")&("<<imvector[i].idx2<<").pgm";
//         oss <<"/home/xzh/文档/markerauto/mk_all/tmp"<<"("<<i<<").pgm";
        oss <<"/home/xzh/文档/markerauto/"<<"("<<i<<").pgm";
        try {
            util::SaveImage(canvas, oss.str().c_str());
        } catch(ex::Exception& e) {
            EX_TRACE("%s\n", e.Msg())
        }
    }
    EX_TIME_END("Match Testing")
}

void HSetVector::WriteTransforms(const h_set& hset, std::ostream& out)
{
	out<<hset.idx1<<"\t"<<hset.idx2<<"\t"<<hset.size()<<std::endl;
	for(int i = 0; i < hset.size(); i++){
		for(int m = 0; m < 3; m++){
			for(int n = 0; n < 3; n++){
                out<<hset.h[i].at<double>(m, n)<<"\t";
// 				out<<cvGetReal2D(hset.h[i], m, n)<<"\t";
			}
			out<<std::endl;
		}
		out<<std::endl<<std::endl;
	}
}

void HSetVector::ReadTransforms(h_set* hset, std::istream& in)
{
	int hsize;
	in>>hset->idx1>>hset->idx2>>hsize;
	for(int i = 0; i < hsize; i++){
		cv::Mat h = cv::Mat::zeros(3, 3, CV_64FC1);
		for(int m = 0; m < 3; m++){
			for(int n = 0; n < 3; n++){
				double val;
				in>>val;
                h.at<double>(m, n) = val;
// 				cvmSet(h, m, n, val);
			}
		}
		hset->h.push_back(h);
	}
}

void HSetVector::WriteVectorByFolder(const char* folderpath) const
{
    if(access(folderpath,0) == -1) {		//create file folder
        mkdir(folderpath,0777);
    }
    std::ostringstream ooo;
    ooo <<folderpath<<"/attributes";
    std::ofstream out(ooo.str().c_str());
    out<<"Z:"<<hset_vector->size()<<"\n";
    out.close();
    for(int i = 0; i < hset_vector->size(); i++) {
        std::ostringstream oss;
        oss <<folderpath<<"/"<<i<<".txt";
        std::ofstream o(oss.str().c_str());
        try {
            WriteTransforms((*hset_vector)[i], o);
        } catch(ex::Exception& e) {
            EX_TRACE("%s\n", e.Msg())
        }
    }
}

void HSetVector::ReadVectorByFolder(const char* folderpath)
{
    Clear();
    std::cout <<std::setprecision(8)<<std::endl;
    std::ostringstream iii;
    iii <<folderpath<<"/attributes";
    std::ifstream in(iii.str().c_str());
    char ch;
    int _size;
    in>>ch>>ch>>_size;
    in.close();

    for(int i = 0; i < _size; i++) {
        std::ostringstream oss;
        oss <<folderpath<<"/"<<i<<".txt";
        std::ifstream in(oss.str().c_str());
        if(!in.good()) {
            ex::EX_THROW("Can't Open File");
        }
        h_set& hset = MallocNewHSet();
        ReadTransforms(&hset, in);
        in.close();
    }
}

void ModelMatch::InitCamera(const std::vector<float>& angles, HSetVector& hsets, std::vector<mx::pproj_params>& camera1, std::vector<mx::pproj_params>* camera2, const std::vector<int>& m_index_low)
{
    std::vector<mx::pproj_params> tmp_cam;
    for(int i=0;i<angles.size();i++)
    {
        mx::pproj_params cam;
        cam.alpha = 0;
        cam.beta = angles[i]*PI_180;
        cam.gamma = 0;
        cam.s = 1;
        cam.t0 = 0;
        cam.t1 = 0;
        tmp_cam.push_back(cam);
    }
    assert(m_index_low.size() == camera1.size());
    for(int i=0; i<m_index_low.size();i++)
    {
        int num=m_index_low[i];
        tmp_cam[num].s = camera1[i].s;
		tmp_cam[num].alpha = camera1[i].alpha;
		tmp_cam[num].beta = camera1[i].beta;
		tmp_cam[num].gamma = camera1[i].gamma;
		tmp_cam[num].t0 = camera1[i].t0;
		tmp_cam[num].t1 = camera1[i].t1;
    }
    int idx1=m_index_low[0]-1;
    int idx2=m_index_low[0];
    while((idx1)>=0)
    {
//         double s, alpha, beta, gamma, t0, t1;
		double s = tmp_cam[idx2].s;
		double alpha = tmp_cam[idx2].alpha;
		double beta = tmp_cam[idx2].beta;
		double gamma = tmp_cam[idx2].gamma;
		double t0 = tmp_cam[idx2].t0;
		double t1 = tmp_cam[idx2].t1;

		double cos_alpha = cos(alpha);
		double sin_alpha = sin(alpha);
		double cos_beta = cos(beta);
		double sin_beta = sin(beta);
		double cos_gamma = cos(gamma);
		double sin_gamma = sin(gamma);

        double new_alpha;
        double new_gamma;
        h_set* txset = hsets.GetHSetWithIdx(idx1, idx2);
        if(txset){
            cv::Mat txinv = cv::Mat::zeros(3, 3, CV_64FC1);
			cv::invert(txset->h[0], txinv);
            cv::Mat H_gamma = cv::Mat::zeros(3, 3, CV_64FC1);
            cv::Mat H_R = cv::Mat::zeros(3, 4, CV_64FC1);
            cv::Mat H = cv::Mat::zeros(3, 4, CV_64FC1);
            H_gamma.at<double>(0, 0)=cos_gamma; H_gamma.at<double>(0, 1)=-sin_gamma; H_gamma.at<double>(0, 2)=0;
            H_gamma.at<double>(1, 0)=sin_gamma; H_gamma.at<double>(1, 1)=cos_gamma; H_gamma.at<double>(1, 2)=0;
            H_gamma.at<double>(2, 0)=0; H_gamma.at<double>(2, 1)=0; H_gamma.at<double>(2, 2)=1;

            H_R.at<double>(0, 0)=cos_beta; H_R.at<double>(0, 1)=sin_alpha*sin_beta; H_R.at<double>(0, 2)=-cos_alpha*sin_beta; H_R.at<double>(0, 3)=t0;
            H_R.at<double>(1, 0)=0; H_R.at<double>(1, 1)=cos_gamma; H_R.at<double>(1, 2)=sin_alpha; H_R.at<double>(1, 3)=t1;
            H_R.at<double>(2, 0)=0; H_R.at<double>(2, 1)=0; H_R.at<double>(2, 2)=0; H_R.at<double>(2, 3)=1;
//             H=txset->h[0] * H_gamma * H_R;
            H=txinv * H_gamma * H_R;
            new_gamma=atan((H.at<double>(1,0))/(H.at<double>(0,0)));
//             new_beta=acos((H.at<double>(0,0))/cos(new_gamma));

            cv::Mat tmp=cv::Mat::zeros(2,2,CV_64FC1);
            tmp.at<double>(0,0)=cos(new_gamma);tmp.at<double>(1,0)=-sin(new_gamma);
            tmp.at<double>(0,1)=sin(new_gamma);tmp.at<double>(1,1)=cos(new_gamma);
            cv::Mat tmpt = cv::Mat::zeros(2, 2, CV_64FC1);
			cv::invert(tmp, tmpt);
            cv::Mat T=cv::Mat::zeros(2,1,CV_64FC1);
            cv::Mat result=cv::Mat::zeros(2,1,CV_64FC1);
            result.at<double>(0,0)=H.at<double>(0,3);result.at<double>(1,0)=H.at<double>(1,3);
            T=tmpt*result;
            // std::cout<<"gamma:"<<new_gamma<<std::endl;
            // std::cout<<"beta:"<<tmp_cam[idx1].beta<<std::endl;
            double A=H.at<double>(0,2);
            double B=H.at<double>(1,2);
            double cob1=B*cos(new_gamma)-A*sin(new_gamma);
            double cob2=cos(new_gamma)*cos(new_gamma)+sin(new_gamma)*sin(new_gamma);
            new_alpha=asin(cob1/cob2);
            // std::cout<<"alpha:"<<new_alpha<<std::endl;
        }
        else{
            txset = hsets.GetHSetWithIdx(idx2, idx1);

            if(!txset){
                return;
            }
            cv::Mat txinv = cv::Mat::zeros(3, 3, CV_64FC1);
			cv::invert(txset->h[0], txinv);
            cv::Mat H_gamma = cv::Mat::zeros(3, 3, CV_64FC1);
            cv::Mat H_R = cv::Mat::zeros(3, 4, CV_64FC1);
            cv::Mat H = cv::Mat::zeros(3, 4, CV_64FC1);
            H_gamma.at<double>(0, 0)=cos_gamma; H_gamma.at<double>(0, 1)=-sin_gamma; H_gamma.at<double>(0, 2)=0;
            H_gamma.at<double>(1, 0)=sin_gamma; H_gamma.at<double>(1, 1)=cos_gamma; H_gamma.at<double>(1, 2)=0;
            H_gamma.at<double>(2, 0)=0; H_gamma.at<double>(2, 1)=0; H_gamma.at<double>(2, 2)=1;

            H_R.at<double>(0, 0)=cos_beta; H_R.at<double>(0, 1)=sin_alpha*sin_beta; H_R.at<double>(0, 2)=-cos_alpha*sin_beta; H_R.at<double>(0, 3)=t0;
            H_R.at<double>(1, 0)=0; H_R.at<double>(1, 1)=cos_gamma; H_R.at<double>(1, 2)=sin_alpha; H_R.at<double>(1, 3)=t1;
            H_R.at<double>(2, 0)=0; H_R.at<double>(2, 1)=0; H_R.at<double>(2, 2)=0; H_R.at<double>(2, 3)=1;
            H=txset->h[0] * H_gamma * H_R;
//             H=txinv * H_gamma * H_R;
            new_gamma=atan((H.at<double>(1,0))/(H.at<double>(0,0)));
//             new_beta=acos((H.at<double>(0,0))/cos(new_gamma));

            cv::Mat tmp=cv::Mat::zeros(2,2,CV_64FC1);
            tmp.at<double>(0,0)=cos(new_gamma);tmp.at<double>(1,0)=-sin(new_gamma);
            tmp.at<double>(0,1)=sin(new_gamma);tmp.at<double>(1,1)=cos(new_gamma);
            cv::Mat tmpt = cv::Mat::zeros(2, 2, CV_64FC1);
			cv::invert(tmp, tmpt);
            cv::Mat T=cv::Mat::zeros(2,1,CV_64FC1);
            cv::Mat result=cv::Mat::zeros(2,1,CV_64FC1);
            result.at<double>(0,0)=H.at<double>(0,3);result.at<double>(1,0)=H.at<double>(1,3);
            T=tmpt*result;
            double A=H.at<double>(0,2);
            double B=H.at<double>(1,2);
            double cob1=B*cos(new_gamma)-A*sin(new_gamma);
            double cob2=cos(new_gamma)*cos(new_gamma)+sin(new_gamma)*sin(new_gamma);
            new_alpha=asin(cob1/cob2);
        }
        tmp_cam[idx1].alpha=new_alpha;
        tmp_cam[idx1].gamma=new_gamma;
        idx1=idx1-1;
        idx2=idx2-1;
    }

    idx1=m_index_low.back();
    idx2=m_index_low.back()+1;
    while((idx2)<tmp_cam.size())
    {
//         double s, alpha, beta, gamma, t0, t1;
		double s = tmp_cam[idx1].s;
		double alpha = tmp_cam[idx1].alpha;
		double beta = tmp_cam[idx1].beta;
		double gamma = tmp_cam[idx1].gamma;
		double t0 = tmp_cam[idx1].t0;
		double t1 = tmp_cam[idx1].t1;

		double cos_alpha = cos(alpha);
		double sin_alpha = sin(alpha);
		double cos_beta = cos(beta);
		double sin_beta = sin(beta);
		double cos_gamma = cos(gamma);
		double sin_gamma = sin(gamma);

        double new_alpha;
        double new_gamma;
        h_set* txset = hsets.GetHSetWithIdx(idx1, idx2);
        if(txset){
            cv::Mat txinv = cv::Mat::zeros(3, 3, CV_64FC1);
			cv::invert(txset->h[0], txinv);
            cv::Mat H_gamma = cv::Mat::zeros(3, 3, CV_64FC1);
            cv::Mat H_R = cv::Mat::zeros(3, 4, CV_64FC1);
            cv::Mat H = cv::Mat::zeros(3, 4, CV_64FC1);
            H_gamma.at<double>(0, 0)=cos_gamma; H_gamma.at<double>(0, 1)=-sin_gamma; H_gamma.at<double>(0, 2)=0;
            H_gamma.at<double>(1, 0)=sin_gamma; H_gamma.at<double>(1, 1)=cos_gamma; H_gamma.at<double>(1, 2)=0;
            H_gamma.at<double>(2, 0)=0; H_gamma.at<double>(2, 1)=0; H_gamma.at<double>(2, 2)=1;

            H_R.at<double>(0, 0)=cos_beta; H_R.at<double>(0, 1)=sin_alpha*sin_beta; H_R.at<double>(0, 2)=-cos_alpha*sin_beta; H_R.at<double>(0, 3)=t0;
            H_R.at<double>(1, 0)=0; H_R.at<double>(1, 1)=cos_gamma; H_R.at<double>(1, 2)=sin_alpha; H_R.at<double>(1, 3)=t1;
            H_R.at<double>(2, 0)=0; H_R.at<double>(2, 1)=0; H_R.at<double>(2, 2)=0; H_R.at<double>(2, 3)=1;
            H=txset->h[0] * H_gamma * H_R;
//             H=txinv * H_gamma * H_R;
//             std::cout<<"H:"<<H<<std::endl;
            new_gamma=atan((H.at<double>(1,0))/(H.at<double>(0,0)));
//             new_beta=acos((H.at<double>(0,0))/cos(new_gamma));

            cv::Mat tmp=cv::Mat::zeros(2,2,CV_64FC1);
            tmp.at<double>(0,0)=cos(new_gamma);tmp.at<double>(1,0)=-sin(new_gamma);
            tmp.at<double>(0,1)=sin(new_gamma);tmp.at<double>(1,1)=cos(new_gamma);
            cv::Mat tmpt = cv::Mat::zeros(2, 2, CV_64FC1);
			cv::invert(tmp, tmpt);
            cv::Mat T=cv::Mat::zeros(2,1,CV_64FC1);
            cv::Mat result=cv::Mat::zeros(2,1,CV_64FC1);
            result.at<double>(0,0)=H.at<double>(0,3);result.at<double>(1,0)=H.at<double>(1,3);
            T=tmpt*result;
            double A=H.at<double>(0,2);
            double B=H.at<double>(1,2);
            double cob1=B*cos(new_gamma)-A*sin(new_gamma);
            double cob2=cos(new_gamma)*cos(new_gamma)+sin(new_gamma)*sin(new_gamma);
            new_alpha=asin(cob1/cob2);
        }
        else{
            txset = hsets.GetHSetWithIdx(idx2, idx1);
            if(!txset){
                return;
            }
            cv::Mat txinv = cv::Mat::zeros(3, 3, CV_64FC1);
			cv::invert(txset->h[0], txinv);
            cv::Mat H_gamma = cv::Mat::zeros(3, 3, CV_64FC1);
            cv::Mat H_R = cv::Mat::zeros(3, 4, CV_64FC1);
            cv::Mat H = cv::Mat::zeros(3, 4, CV_64FC1);
            H_gamma.at<double>(0, 0)=cos_gamma; H_gamma.at<double>(0, 1)=-sin_gamma; H_gamma.at<double>(0, 2)=0;
            H_gamma.at<double>(1, 0)=sin_gamma; H_gamma.at<double>(1, 1)=cos_gamma; H_gamma.at<double>(1, 2)=0;
            H_gamma.at<double>(2, 0)=0; H_gamma.at<double>(2, 1)=0; H_gamma.at<double>(2, 2)=1;

            H_R.at<double>(0, 0)=cos_beta; H_R.at<double>(0, 1)=sin_alpha*sin_beta; H_R.at<double>(0, 2)=-cos_alpha*sin_beta; H_R.at<double>(0, 3)=t0;
            H_R.at<double>(1, 0)=0; H_R.at<double>(1, 1)=cos_gamma; H_R.at<double>(1, 2)=sin_alpha; H_R.at<double>(1, 3)=t1;
            H_R.at<double>(2, 0)=0; H_R.at<double>(2, 1)=0; H_R.at<double>(2, 2)=0; H_R.at<double>(2, 3)=1;
//             H=txset->h[0] * H_gamma * H_R;
            H=txinv * H_gamma * H_R;

            new_gamma=atan((H.at<double>(1,0))/(H.at<double>(0,0)));
//             new_beta=acos((H.at<double>(0,0))/cos(new_gamma));
            cv::Mat tmp=cv::Mat::zeros(2,2,CV_64FC1);
            tmp.at<double>(0,0)=cos(new_gamma);tmp.at<double>(1,0)=-sin(new_gamma);
            tmp.at<double>(0,1)=sin(new_gamma);tmp.at<double>(1,1)=cos(new_gamma);
            cv::Mat tmpt = cv::Mat::zeros(2, 2, CV_64FC1);
			cv::invert(tmp, tmpt);
            cv::Mat T=cv::Mat::zeros(2,1,CV_64FC1);
            cv::Mat result=cv::Mat::zeros(2,1,CV_64FC1);
            result.at<double>(0,0)=H.at<double>(0,3);result.at<double>(1,0)=H.at<double>(1,3);
            T=tmpt*result;
            double A=H.at<double>(0,2);
            double B=H.at<double>(1,2);
            double cob1=B*cos(new_gamma)-A*sin(new_gamma);
            double cob2=cos(new_gamma)*cos(new_gamma)+sin(new_gamma)*sin(new_gamma);
            new_alpha=asin(cob1/cob2);
        }
        tmp_cam[idx2].alpha=new_alpha;
        tmp_cam[idx2].gamma=new_gamma;
        idx1=idx1+1;
        idx2=idx2+1;
    }
    for(int i=0;i<tmp_cam.size();i++)
    {
        camera2->push_back(tmp_cam[i]);
//         std::cout<<"i:"<<i<<std::endl;
//         std::cout<<"alpha:"<<tmp_cam[i].alpha<<std::endl;
//         std::cout<<"beta:"<<tmp_cam[i].beta<<std::endl;
//         std::cout<<"gamma:"<<tmp_cam[i].gamma<<std::endl;
    }

}
