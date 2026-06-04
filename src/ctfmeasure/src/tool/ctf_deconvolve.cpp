#include <cstdio>
#include <iostream>
#include <cstring>
#include <string>
#include "mrc.h"
#include "util.h"
#include "CTF.h"
#include "fftw3.h"
#include "math.h"
using namespace std;

static void buf2fft(float *buf, float *fft, int nx, int ny)
{
    int nxb=nx+2-nx%2;
    int i;
    for(i=0;i<(nx+2-nx%2)*ny;i++)
    {
        fft[i]=0.0;
    }
    for(i=0;i<ny;i++)
    {
        memcpy(fft+i*nxb,buf+i*nx,sizeof(float)*nx);
    }
}

static void fft2buf(float *buf, float *fft, int nx, int ny)
{
    int nxb=nx+2-nx%2;
    int i;
    for(i=0;i<nx*ny;i++)
    {
        buf[i]=0.0;
    }
    for(i=0;i<ny;i++)
    {
        memcpy(buf+i*nx,fft+i*nxb,sizeof(float)*nx);
    }
}

int main(int argc, char **argv)
{
    if(argc<7)
    {
        printf("Usage: \n%6s%s <input defocus file (in CTFMeasure style)> <output tilt series> <pixel size, in Angstrom> <spherical aberration, in mm> <acceleration volatge, in kV> <amplitude contrast>\n\n", "", argv[0]);
        exit(1);
    }

    // string input_tilt_series=argv[1];
    string input_defocus_file=argv[1];
    string output_tilt_series=argv[2];
    float pix=atof(argv[3]);
    float cs=atof(argv[4]);
    float voltage=atof(argv[5]);
    float w=atof(argv[6]);

    ifstream fdefocus_in(input_defocus_file.c_str(),ios::in);
    if(!fdefocus_in)
    {
        cerr << "[Error] Cannot open input defocus file!" << endl;
        abort();
    }

    cout << "Read in CTF parameters..." << endl;
    vector<string> micrograph_name_all;
    vector<int> micrograph_num_all;
    vector<CTF> ctf_para;
    string s;
    int Nz=0;
    int Nx;
    int Ny;
    while(getline(fdefocus_in,s))
    {
        if(s[0]=='#')   // 跳过注释
        {
            continue;
        }
        if(s[0]=='[')   // 跳过注释
        {
            continue;
        }
        if(s[0]=='\0')  // 跳过空行
        {
            continue;
        }

        int n;
        float df_1;
        float df_2;
        float astig;
        float phase_shift;
        float cc;
        float res;
        float theta;
        float psi;
        float phi;
        for(int t=0;t<10;t++)
        {
            size_t pos=s.find_first_of(" ");
            if(pos==string::npos && t<9)
            {
                cerr << "[Error] Wrong format for input defocus file!" << endl;
                abort();
            }
            string current=s.substr(0,pos);
            trim(current);
            s=s.substr(pos+1);
            trim(s);
            if(t==0)
            {
                pos=current.find_first_of("@");
                string num_now=current.substr(0,pos);
                string name_now=current.substr(pos+1);
                trim(num_now);
                trim(name_now);
                micrograph_name_all.push_back(name_now);
                micrograph_num_all.push_back(atoi(num_now.c_str()));
                if(Nz==0)
                {
                    MRC stack_now(micrograph_name_all[0].c_str(),"rb");
                    if(!stack_now.hasFile())
                    {
                        cerr << "[Error] Cannot open input micrograph " << micrograph_name_all[0] << "!" << endl;
                        abort();
                    }
                    Nx=stack_now.getNx();
                    Ny=stack_now.getNy();
                    stack_now.close();
                }
            }
            else if(t==1)
            {
                df_1=atof(current.c_str());
            }
            else if(t==2)
            {
                df_2=atof(current.c_str());
            }
            else if(t==3)
            {
                astig=atof(current.c_str());
            }
            else if(t==4)
            {
                phase_shift=atof(current.c_str());
            }
            else if(t==5)
            {
                psi=atof(current.c_str());
            }
            else if(t==6)
            {
                theta=atof(current.c_str());
            }
            else if(t==7)
            {
                phi=atof(current.c_str());
            }
            else if(t==8)
            {
                cc=atof(current.c_str());
            }
            else if(t==9)
            {
                res=atof(current.c_str());
            }
        }
        CTF ctf_para_now;
        ctf_para_now.setAllImagePara(pix,voltage,cs);
        ctf_para_now.setAllCTFPara(df_1,df_2,astig,phase_shift,w);
        ctf_para.push_back(ctf_para_now);
        Nz++;
    }

    fdefocus_in.close();
    cout << "Done!" << endl << endl;

    cout << "CTF deconvolution for each micrograph..." << endl;
    MRC stack_output(output_tilt_series.c_str(),"wb");
    stack_output.createMRC_empty(Nx,Ny,Nz,2);
    for(int n=0;n<Nz;n++)
    {
        cout << "n=" << n << endl;
        MRC stack_now(micrograph_name_all[n].c_str(),"rb");
        if(!stack_now.hasFile())
        {
            cerr << "[Error] Cannot open input micrograph " << micrograph_name_all[n] << "!" << endl;
            abort();
        }
        float *image_now=new float[stack_now.getNx()*stack_now.getNy()];
        stack_now.read2DIm_32bit(image_now,micrograph_num_all[n]);
        float *buf_fft=new float[(stack_now.getNx()+2-stack_now.getNx()%2)*stack_now.getNy()];
        memset(buf_fft,0,sizeof(float)*(stack_now.getNx()+2-stack_now.getNx()%2)*stack_now.getNy());
        fftwf_plan plan_fft=fftwf_plan_dft_r2c_2d(stack_now.getNy(),stack_now.getNx(),(float*)buf_fft,reinterpret_cast<fftwf_complex*>(buf_fft),FFTW_ESTIMATE);
        fftwf_plan plan_ifft=fftwf_plan_dft_c2r_2d(stack_now.getNy(),stack_now.getNx(),reinterpret_cast<fftwf_complex*>(buf_fft),(float*)buf_fft,FFTW_ESTIMATE);
        buf2fft(image_now,buf_fft,stack_now.getNx(),stack_now.getNy());
        fftwf_execute(plan_fft);
        int *x_coord=new int[stack_now.getNx()];
        int *y_coord=new int[stack_now.getNy()];
        for(int i=0;i<stack_now.getNx();i++)
        {
            if(i<=int(floor(stack_now.getNx()/2)))
            {
                x_coord[i]=i;
            }
            else
            {
                x_coord[i]=i-stack_now.getNx();
            }
        }
        for(int j=0;j<stack_now.getNy();j++)
        {
            if(j<=int(floor(stack_now.getNy()/2)))
            {
                y_coord[j]=j;
            }
            else
            {
                y_coord[j]=j-stack_now.getNy();
            }
        }
        for(int j=0;j<stack_now.getNy();j++)
        {
            for(int i=0;i<stack_now.getNx()+2-stack_now.getNx()%2;i+=2)
            {
                float x_res=float(x_coord[i/2])/float(stack_now.getNx())/ctf_para[n].getPixelSize();
                float y_res=float(y_coord[j])/float(stack_now.getNy())/ctf_para[n].getPixelSize();
                float res2=x_res*x_res+y_res*y_res;
                double df=(ctf_para[n].getDefocus1()+ctf_para[n].getDefocus2()+(ctf_para[n].getDefocus1()-ctf_para[n].getDefocus2())*cos(2*(atan2(y_res,x_res)-ctf_para[n].getAstigmatism())))/2;
                double chi=M_PI*ctf_para[n].getLambda()*df*res2-M_PI_2*ctf_para[n].getCs()*(ctf_para[n].getLambda()*ctf_para[n].getLambda()*ctf_para[n].getLambda())*(double(res2)*double(res2))+ctf_para[n].getW_phase();
                double ctf_now=sin(chi);
                double ssnr_now_inv=exp(sqrt(res2)*1e-10*100.0)/1e3;
                double w_now=ctf_now/(ctf_now*ctf_now+ssnr_now_inv);
                buf_fft[j*(stack_now.getNx()+2-stack_now.getNx()%2)+i]*=w_now;
                buf_fft[j*(stack_now.getNx()+2-stack_now.getNx()%2)+i+1]*=w_now;
            }
        }
        fftwf_execute(plan_ifft);
        fft2buf(image_now,buf_fft,stack_now.getNx(),stack_now.getNy());
        for(int i=0;i<stack_now.getNx()*stack_now.getNy();i++)
        {
            image_now[i]/=float(stack_now.getNx()*stack_now.getNy());
        }
        stack_output.write2DIm(image_now,n);
        fftwf_destroy_plan(plan_fft);
        fftwf_destroy_plan(plan_ifft);
        delete [] x_coord;
        delete [] y_coord;
        delete [] buf_fft;
        delete [] image_now;
        stack_now.close();
    }
    stack_output.close();
    cout << "Done" << endl << endl;

    cout << "Done" << endl << endl;
    cout << "Output CTF deconvolved tilt series written in " << output_tilt_series << endl << endl;

    return 0;
}
