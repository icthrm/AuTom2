#include <stdio.h>
#include "util.h"
#include "mrc.h"
#include "math.h"
#include "nlopt.h"
#include "Dense"


double loss(unsigned n,const double *x,double *grad,void *data_in)
// data[0]: 照片张数
// data[1]: 灰度均值
// data[2]: 角度（弧度）
// x: [a,b,c,theta0]
{
    double y=0.0;
    double a=x[0];
    double b=x[1];
    double c=x[2];
    double theta0=x[3];
    if(grad)
    {
        grad[0]=0.0;
        grad[1]=0.0;
        grad[2]=0.0;
        grad[3]=0.0;
    }
    double tmp;
    double **data=(double**)data_in;
    for(int i=0;i<int(data[0][0]);i++)
    {
        tmp=data[1][i]-a*exp(-c/cos(data[2][i]-theta0))-b;
        y=y+tmp*tmp;
        if(grad)
        {
            grad[0]=grad[0]-2*tmp*exp(-c/cos(data[2][i]-theta0));
            grad[1]=grad[1]-2*tmp;
            grad[2]=grad[2]+2*tmp*a*exp(-c/cos(data[2][i]-theta0))/cos(data[2][i]-theta0);
            grad[3]=grad[3]-2*tmp*a*c*exp(-c/cos(data[2][i]-theta0))*sin(data[2][i]-theta0)/cos(data[2][i]-theta0)/cos(data[2][i]-theta0);
        }
    }
    return y;
}

double constraint_b(unsigned n,double const *x,double *grad,void *data)
{
    double *I_min=(double*)data;
    grad[0]=0.0;
    grad[1]=-1.0;
    grad[2]=0.0;
    grad[3]=0.0;
    return x[2]-*I_min;
}



int main(int argc, char **argv)
{
    time_t start_time;
    time(&start_time);

    if(argc<3)
    {
        printf("Usage: \n%6s%s <input tilt-series stack> <input tilt angle file> [<input dose file>]\n\n", "", argv[0]);
        exit(1);
    }

    // read input parameters
    string input_mrc=argv[1];
    MRC stack_orig(input_mrc.c_str(),"rb");
    if(!stack_orig.hasFile())
    {
        cerr << "[Error] Cannot open input tilt-series stack!" << endl;
        abort();
    }

    string input_rawtlt=argv[2];
    FILE *rawtlt=fopen(input_rawtlt.c_str(),"r");
    if(rawtlt==NULL)
    {
        cerr << "[Error] Cannot open input tilt angle file!" << endl;
        abort();
    }
    float theta_deg[stack_orig.getNz()],theta_rad[stack_orig.getNz()];
    for(int n=0;n<stack_orig.getNz();n++)
    {
        fscanf(rawtlt,"%f",&theta_deg[n]);
        theta_rad[n]=theta_deg[n]*M_PI/180.0;
    }
    fflush(rawtlt);
    fclose(rawtlt);

    float dose_all[stack_orig.getNz()];
    for(int n=0;n<stack_orig.getNz();n++)
    {
        dose_all[n]=1.0;
    }
    if(argc==4)
    {
        string input_dose=argv[3];
        FILE *fdose=fopen(input_dose.c_str(),"r");
        if(fdose==NULL)
        {
            cerr << "[Error] Cannot open input dose file!" << endl;
            abort();
        }
        for(int n=0;n<stack_orig.getNz();n++)
        {
            fscanf(fdose,"%f",&dose_all[n]);
        }
        fflush(fdose);
        fclose(fdose);
    }
    

    // compute average density
    cout << endl << "################ Step 1: Compute average density ################" << endl << endl;
    float avg[stack_orig.getNz()];
    for(int n=0;n<stack_orig.getNz();n++)
    {
        float *image_now=new float[stack_orig.getNx()*stack_orig.getNy()];
        stack_orig.read2DIm_32bit(image_now,n);
        double sum=0.0;
        for(int i=0;i<stack_orig.getNx()*stack_orig.getNy();i++)
        {
            sum+=image_now[i];
        }
        avg[n]=sum/double(stack_orig.getNx()*stack_orig.getNy());
        // cout << n << ": " << theta_deg[n] << ", " << avg[n] << endl;
        printf("%02d : %9.4f , %9.4f\n",n,theta_deg[n],avg[n]);
        delete [] image_now;
    }

    // Least-Square fit with quadratic polynomial
    cout << endl << "################ Step 2: Normalization with electron dose ################" << endl << endl;
    for(int n=0;n<stack_orig.getNz();n++)
    {
        avg[n]=avg[n]/double(dose_all[n]);
        printf("%02d : %9.4f , %9.4f , %9.4f\n",n,theta_deg[n],avg[n],dose_all[n]);
    }

    // Least-Square fit with quadratic polynomial
    cout << endl << "################ Step 3: Least-square fitting with quadratic polynomial ################" << endl << endl;
    Eigen::MatrixXf A(stack_orig.getNz(),3);
    Eigen::VectorXf b(stack_orig.getNz());
    for(int n=0;n<stack_orig.getNz();n++)
    {
        b(n)=avg[n];
        A(n,0)=1.0;
        A(n,1)=theta_rad[n];
        A(n,2)=theta_rad[n]*theta_rad[n];
    }
    Eigen::MatrixXf ATA(3,3);
    Eigen::VectorXf ATb(3);
    ATA=A.transpose()*A;
    ATb=A.transpose()*b;
    Eigen::VectorXf x(3);
    x=ATA.inverse()*ATb;
    cout << "Result (y-density; x-theta (in radius)): y = " << x(0) << " + " << x(1) << " * x + " << x(2) << " * x ^ 2" << endl;
    cout << "Estimated global tilt offset: " << (-x(1)/(2*x(2)))*180.0/M_PI << endl;
    double theta_offset=(-x(1)/(2*x(2)));

    /*
    // Further optimization with non-linear least-square
    cout << endl << "################ Step 3: Further optimization with non-linear least-square ################" << endl << endl;
    double *data_in[3];
    data_in[0]=new double[1];
    data_in[0][0]=stack_orig.getNz();
    data_in[1]=new double[stack_orig.getNz()];
    data_in[2]=new double[stack_orig.getNz()];
    double I_min=avg[0],I_max=avg[0];
    for(int n=0;n<stack_orig.getNz();n++)
    {
        data_in[1][n]=avg[n];
        data_in[2][n]=theta_rad[n];
        if(I_min>avg[n])
        {
            I_min=avg[n];
        }
        if(I_max<avg[n])
        {
            I_max=avg[n];
        }
    }
    double lb[4]={0,-HUGE_VAL,0,-M_PI_2};
    double ub[4]={HUGE_VAL,I_min,HUGE_VAL,M_PI_2};
    nlopt_opt opt;
    opt=nlopt_create(NLOPT_LN_NELDERMEAD,4);
    // opt=nlopt_create(NLOPT_LD_SLSQP,4);
    nlopt_set_lower_bounds(opt,lb);
    nlopt_set_upper_bounds(opt,ub);
    nlopt_set_min_objective(opt,loss,data_in);
    // nlopt_add_inequality_constraint(opt,constraint_b,&I_min,1e-8);
    nlopt_set_xtol_rel(opt,1e-8);
    double para[4]={I_max*exp(1),0,1,(-x(1)/(2*x(2)))};
    double minf;
    double theta_offset;
    if(nlopt_optimize(opt,para,&minf)<0)
    {
        cout << "nlopt failed!" << endl;
        theta_offset=-x(1)/(2*x(2));
    }
    else
    {
        printf("Found minimum at f( %f , %f , %f , %f ) = %f\n",para[0],para[1],para[2],para[3],minf);
        theta_offset=para[3];
    }
    cout << endl << "Optimized global tilt offset: " << theta_offset*180.0/M_PI << endl;

    delete [] data_in[0];
    delete [] data_in[1];
    delete [] data_in[2];
    */

    stack_orig.close();

    cout << endl << "################ Final result ################" << endl << endl;
    cout << "Estimated absolute tilt angle offset = " << theta_offset*180.0/M_PI << endl;

    cout << endl << "Finish tilt offset estimation successfully!" << endl << endl;

    return 0;
}
