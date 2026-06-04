/*******************************************************************
 *       Filename:  CTFAlgo.cpp                                     
 *                                                                 
 *    Description:                                        
 *                                                                 
 *        Version:  1.0                                            
 *        Created:  07/07/2020 05:37:50 PM                                 
 *       Revision:  none                                           
 *       Compiler:  gcc                                           
 *                                                                 
 *         Author:  Ruan Huabin                                      
 *          Email:  ruanhuabin@tsinghua.edu.cn                                        
 *        Company:  Dep. of CS, Tsinghua Unversity                                      
 *                                                                 
 *******************************************************************/
#include "CTFAlgo_v3.h"
#include "mrc.h"
#include "math.h"
#include "CTF.h"
#include "fftw3.h"
#include "util.h"
#include "nlopt.h"
#include "omp.h"
#include "Dense"



struct Data_opt_epa
{
    int box;
    CTF ctf_para;
    float res_min;
    float res_max;
    float *psd;
    double chi_min;
    double chi_max;
    float *res2_all;
    float *atan_all;
    float *x_fft_1d_res;
};

struct Data_opt_dz_epa
{
    int box;
    CTF ctf_para;
    float res_min;
    float res_max;
    float *psd;
    double chi_min;
    double chi_max;
    float *chi_all;
    float *res2_all;
    float *atan_all;
    float *x_fft_1d_res;
};

struct Data_opt_theta_resolution
{
    int box;
    int N_block;
    int *block_x;
    int *block_y;
    int Nx;
    int Ny;
    CTF ctf_para;
    float res_min;
    float res_max;
    double chi_min;
    double chi_max;
    float **psd_all;
    float *res2_all;
    float *atan_all;
    float *x_fft_1d_res;
    float psi;
    float phi;
    float pix;
    int box_conv;
    fftwf_plan plan_fft;
    fftwf_plan plan_ifft;
    float *fft_buf_in;
    float *fft_buf_out;
    int N_zeros;
    int N_block_x;
    int N_block_y;
};

struct Data_opt_theta_resolution_all
{
    int box;
    int N_block;
    int *block_x;
    int *block_y;
    int Nx;
    int Ny;
    int Nz;
    CTF *ctf_para;
    CTF ctf_para_avg;
    float df_ref;
    float res_min;
    float res_max;
    double chi_min;
    double chi_max;
    float ***psd_all;
    float *res2_all;
    float *atan_all;
    float *x_fft_1d_res;
    float psi;
    float phi;
    float *theta;
    float pix;
    int box_conv;
    fftwf_plan plan_fft;
    fftwf_plan plan_ifft;
    float *fft_buf_in;
    float *fft_buf_out;
    int N_zeros;
    int N_block_x;
    int N_block_y;
    int threads;
    int N_ref;
    int N_avg;
    bool *image_avail;
    bool optimize_with_avg;
};

struct Data_opt_psi_resolution_all
{
    int box;
    int N_block;
    int *block_x;
    int *block_y;
    int Nx;
    int Ny;
    int Nz;
    CTF *ctf_para;
    CTF ctf_para_avg;
    float df_ref;
    float res_min;
    float res_max;
    double chi_min;
    double chi_max;
    float ***psd_all;
    float *res2_all;
    float *atan_all;
    float *x_fft_1d_res;
    float psi;
    float phi;
    float *theta;
    float pix;
    int box_conv;
    fftwf_plan plan_fft;
    fftwf_plan plan_ifft;
    float *fft_buf_in;
    float *fft_buf_out;
    int N_zeros;
    int N_block_x;
    int N_block_y;
    int threads;
    bool optimize_with_avg;
};

struct Data_opt_phi_resolution_all
{
    int box;
    int N_block;
    int *block_x;
    int *block_y;
    int Nx;
    int Ny;
    int Nz;
    CTF *ctf_para;
    CTF ctf_para_avg;
    float df_ref;
    float res_min;
    float res_max;
    double chi_min;
    double chi_max;
    float ***psd_all;
    float *res2_all;
    float *atan_all;
    float *x_fft_1d_res;
    float psi;
    float phi;
    float *theta;
    float pix;
    int box_conv;
    fftwf_plan plan_fft;
    fftwf_plan plan_ifft;
    float *fft_buf_in;
    float *fft_buf_out;
    int N_zeros;
    int N_block_x;
    int N_block_y;
    int threads;
    bool optimize_with_avg;
};




float Sign(float x)
{
    if(x<0.0)
    {
        return -1.0;
    }
    else
    {
        return 1.0;
    }
}



// static string& trim(std::string &s) 
// {
//     if (s.empty()) 
//     {
//         return s;
//     }
 
//     s.erase(0,s.find_first_not_of(" "));
//     s.erase(s.find_last_not_of(" ") + 1);
//     return s;
// }



static void fftshift_2d(float *in,float *out,int Nx,int Ny)
{
    int x_shift[Nx],y_shift[Ny];
    if(Nx%2==0)
    {
        for(int i=0;i<Nx/2;i++)
        {
            x_shift[i]=i+Nx/2;
        }
        for(int i=Nx/2;i<Nx;i++)
        {
            x_shift[i]=i-Nx/2;
        }
    }
    else
    {
        for(int i=0;i<(Nx-1)/2;i++)
        {
            x_shift[i]=i+(Nx+1)/2;
        }
        for(int i=(Nx-1)/2;i<Nx;i++)
        {
            x_shift[i]=i-(Nx-1)/2;
        }
    }
    if(Ny%2==0)
    {
        for(int j=0;j<Ny/2;j++)
        {
            y_shift[j]=j+Ny/2;
        }
        for(int j=Ny/2;j<Ny;j++)
        {
            y_shift[j]=j-Ny/2;
        }
    }
    else
    {
        for(int j=0;j<(Ny-1)/2;j++)
        {
            y_shift[j]=j+(Ny+1)/2;
        }
        for(int j=(Ny-1)/2;j<Ny;j++)
        {
            y_shift[j]=j-(Ny-1)/2;
        }
    }
    for(int j=0;j<Ny;j++)
    {
        for(int i=0;i<Nx;i++)
        {
            out[j*Nx+i]=in[y_shift[j]*Nx+x_shift[i]];
        }
    }
}

static void fftshift_2d_complex(float *in,float *out,int Nx,int Ny)
{
    int x_shift[Nx],y_shift[Ny];
    if(Nx%2==0)
    {
        for(int i=0;i<Nx/2;i++)
        {
            x_shift[i]=i+Nx/2;
        }
        for(int i=Nx/2;i<Nx;i++)
        {
            x_shift[i]=i-Nx/2;
        }
    }
    else
    {
        for(int i=0;i<(Nx-1)/2;i++)
        {
            x_shift[i]=i+(Nx+1)/2;
        }
        for(int i=(Nx-1)/2;i<Nx;i++)
        {
            x_shift[i]=i-(Nx-1)/2;
        }
    }
    if(Ny%2==0)
    {
        for(int j=0;j<Ny/2;j++)
        {
            y_shift[j]=j+Ny/2;
        }
        for(int j=Ny/2;j<Ny;j++)
        {
            y_shift[j]=j-Ny/2;
        }
    }
    else
    {
        for(int j=0;j<(Ny-1)/2;j++)
        {
            y_shift[j]=j+(Ny+1)/2;
        }
        for(int j=(Ny-1)/2;j<Ny;j++)
        {
            y_shift[j]=j-(Ny-1)/2;
        }
    }
    for(int j=0;j<Ny;j++)
    {
        for(int i=0;i<Nx;i++)
        {
            out[j*Nx*2+i*2]=in[y_shift[j]*Nx*2+x_shift[i]*2];
            out[j*Nx*2+i*2+1]=in[y_shift[j]*Nx*2+x_shift[i]*2+1];
        }
    }
}

static void ifftshift_2d_complex(float *in,float *out,int Nx,int Ny)
{
    int x_shift[Nx],y_shift[Ny];
    if(Nx%2==0)
    {
        for(int i=0;i<Nx/2;i++)
        {
            x_shift[i]=i+Nx/2;
        }
        for(int i=Nx/2;i<Nx;i++)
        {
            x_shift[i]=i-Nx/2;
        }
    }
    else
    {
        for(int i=0;i<(Nx+1)/2;i++)
        {
            x_shift[i]=i+(Nx-1)/2;
        }
        for(int i=(Nx+1)/2;i<Nx;i++)
        {
            x_shift[i]=i-(Nx+1)/2;
        }
    }
    if(Ny%2==0)
    {
        for(int j=0;j<Ny/2;j++)
        {
            y_shift[j]=j+Ny/2;
        }
        for(int j=Ny/2;j<Ny;j++)
        {
            y_shift[j]=j-Ny/2;
        }
    }
    else
    {
        for(int j=0;j<(Ny+1)/2;j++)
        {
            y_shift[j]=j+(Ny-1)/2;
        }
        for(int j=(Ny+1)/2;j<Ny;j++)
        {
            y_shift[j]=j-(Ny+1)/2;
        }
    }
    for(int j=0;j<Ny;j++)
    {
        for(int i=0;i<Nx;i++)
        {
            out[j*Nx*2+i*2]=in[y_shift[j]*Nx*2+x_shift[i]*2];
            out[j*Nx*2+i*2+1]=in[y_shift[j]*Nx*2+x_shift[i]*2+1];
        }
    }
}


void get_average_psd_omp(float *image_now,float *psd_float,int box,float df,float pix,float psi,float theta,int Nx,int Ny,fftwf_plan plan_fft,float *fft_buf_in,float *fft_buf_out)  // 切小块求平均功率谱，采用BSoft中的尺度变换统一零点位置，频域直接放缩
{
    // 单位换算到国际单位
    pix=pix*1e-10;  // A --> m
    df=df*1e-10;    // A --> m

    double *psd=new double[box*box];
    for(int i=0;i<box*box;i++)
    {
        psd[i]=0.0;
    }
    float x_fft[box],y_fft[box];
    for(int i=0;i<box;i++)
    {
        x_fft[i]=i-box/2;
        y_fft[i]=i-box/2;
    }

    int N_block=0;
    float x_center=Nx/2;  // box的中心坐标
    float y_center=Ny/2;
    float dz=0;
    float *block=new float[box*box];
    float *block_scaling=new float[box*box];
    float *block_scaling_shift=new float[box*box];
    float *block_scaling_shift_aug=new float[box*box*2];
    float *block_scaling_shift_fft=new float[box*box*2];
    float *block_scaling_shift_fft_shift=new float[box*box*2];
    float *block_scaling_shift_fft_shift_amp=new float[box*box];
    for(int i=0;i<box*box;i++)
    {
        block[i]=0.0;
        block_scaling[i]=0.0;
        block_scaling_shift[i]=0.0;
        block_scaling_shift_fft_shift_amp[i]=0.0;
    }
    for(int i=0;i<box*box*2;i++)
    {
        block_scaling_shift_aug[i]=0.0;
        block_scaling_shift_fft[i]=0.0;
        block_scaling_shift_fft_shift[i]=0.0;
    }
    while(int(floor(x_center)-box/2)>=0 && int(floor(x_center)+box/2)<Nx && int(floor(y_center)-box/2)>=0 && int(floor(y_center)+box/2)<Ny)
    {
        float x_now=x_center;
        float y_now=y_center;
        while(int(floor(x_now)-box/2)>=0 && int(floor(x_now)+box/2)<Nx && int(floor(y_now)-box/2)>=0 && int(floor(y_now)+box/2)<Ny)
        {
            N_block++;
            int t=0;
            for(int j=int(floor(y_now)-box/2);j<int(floor(y_now)+box/2);j++)
            {
                for(int i=int(floor(x_now)-box/2);i<int(floor(x_now)+box/2);i++)
                {
                    block[t]=image_now[j*Nx+i];
                    t++;
                }
            }
            fftshift_2d(block,block_scaling_shift,box,box);
            for(int i=0;i<box*box;i++)
            {
                block_scaling_shift_aug[i*2]=block_scaling_shift[i];
                block_scaling_shift_aug[i*2+1]=0.0;
            }
            for(int i=0;i<box*box*2;i++)
            {
                fft_buf_in[i]=block_scaling_shift_aug[i];
            }
            fftwf_execute(plan_fft);
            for(int i=0;i<box*box*2;i++)
            {
                block_scaling_shift_fft[i]=fft_buf_out[i];
            }
            fftshift_2d_complex(block_scaling_shift_fft,block_scaling_shift_fft_shift,box,box);
            for(int i=0;i<box*box;i++)
            {
                block_scaling[i]=0.0;
                block_scaling_shift_fft_shift_amp[i]=block_scaling_shift_fft_shift[2*i]*block_scaling_shift_fft_shift[2*i]+block_scaling_shift_fft_shift[2*i+1]*block_scaling_shift_fft_shift[2*i+1];
            }
            float df_scale=sqrt(df/(df+dz));
            for(int j=0;j<box;j++)
            {
                for(int i=0;i<box;i++)
                {
                    float res=sqrt(x_fft[i]*x_fft[i]+y_fft[j]*y_fft[j]);
                    float x_scale=x_fft[i]*df_scale+box/2;
                    float y_scale=y_fft[j]*df_scale+box/2;
                    if(x_scale>=0 && x_scale<=box-1 && y_scale>=0 && y_scale<=box-1)
                    {
                        float coeff_x=x_scale-floor(x_scale);
                        float coeff_y=y_scale-floor(y_scale);
                        block_scaling[j*box+i]=(1-coeff_x)*(1-coeff_y)*block_scaling_shift_fft_shift_amp[int(floor(y_scale))*box+int(floor(x_scale))]+(1-coeff_x)*(coeff_y)*block_scaling_shift_fft_shift_amp[int(ceil(y_scale))*box+int(floor(x_scale))]+(coeff_x)*(1-coeff_y)*block_scaling_shift_fft_shift_amp[int(floor(y_scale))*box+int(ceil(x_scale))]+(coeff_x)*(coeff_y)*block_scaling_shift_fft_shift_amp[int(ceil(y_scale))*box+int(ceil(x_scale))];
                    } 
                }
            }
            for(int i=0;i<box*box;i++)
            {
                psd[i]+=block_scaling[i];
            }
            x_now+=(box/2*sin(-psi*M_PI/180.0));
            y_now+=(box/2*cos(-psi*M_PI/180.0));
        }
        x_now=x_center-box/2*sin(-psi*M_PI/180.0);
        y_now=y_center-box/2*cos(-psi*M_PI/180.0);
        while(int(floor(x_now)-box/2)>=0 && int(floor(x_now)+box/2)<Nx && int(floor(y_now)-box/2)>=0 && int(floor(y_now)+box/2)<Ny)
        {
            N_block++;
            int t=0;
            for(int j=int(floor(y_now)-box/2);j<int(floor(y_now)+box/2);j++)
            {
                for(int i=int(floor(x_now)-box/2);i<int(floor(x_now)+box/2);i++)
                {
                    block[t]=image_now[j*Nx+i];
                    t++;
                }
            }
            fftshift_2d(block,block_scaling_shift,box,box);
            for(int i=0;i<box*box;i++)
            {
                block_scaling_shift_aug[i*2]=block_scaling_shift[i];
                block_scaling_shift_aug[i*2+1]=0.0;
            }
            for(int i=0;i<box*box*2;i++)
            {
                fft_buf_in[i]=block_scaling_shift_aug[i];
            }
            fftwf_execute(plan_fft);
            for(int i=0;i<box*box*2;i++)
            {
                block_scaling_shift_fft[i]=fft_buf_out[i];
            }
            fftshift_2d_complex(block_scaling_shift_fft,block_scaling_shift_fft_shift,box,box);
            for(int i=0;i<box*box;i++)
            {
                block_scaling[i]=0.0;
                block_scaling_shift_fft_shift_amp[i]=block_scaling_shift_fft_shift[2*i]*block_scaling_shift_fft_shift[2*i]+block_scaling_shift_fft_shift[2*i+1]*block_scaling_shift_fft_shift[2*i+1];
            }
            float df_scale=sqrt(df/(df+dz));
            for(int j=0;j<box;j++)
            {
                for(int i=0;i<box;i++)
                {
                    float res=sqrt(x_fft[i]*x_fft[i]+y_fft[j]*y_fft[j]);
                    float x_scale=x_fft[i]*df_scale+box/2;
                    float y_scale=y_fft[j]*df_scale+box/2;
                    if(x_scale>=0 && x_scale<=box-1 && y_scale>=0 && y_scale<=box-1)
                    {
                        float coeff_x=x_scale-floor(x_scale);
                        float coeff_y=y_scale-floor(y_scale);
                        block_scaling[j*box+i]=(1-coeff_x)*(1-coeff_y)*block_scaling_shift_fft_shift_amp[int(floor(y_scale))*box+int(floor(x_scale))]+(1-coeff_x)*(coeff_y)*block_scaling_shift_fft_shift_amp[int(ceil(y_scale))*box+int(floor(x_scale))]+(coeff_x)*(1-coeff_y)*block_scaling_shift_fft_shift_amp[int(floor(y_scale))*box+int(ceil(x_scale))]+(coeff_x)*(coeff_y)*block_scaling_shift_fft_shift_amp[int(ceil(y_scale))*box+int(ceil(x_scale))];
                    } 
                }
            }
            for(int i=0;i<box*box;i++)
            {
                psd[i]+=block_scaling[i];
            }
            x_now-=(box/2*sin(-psi*M_PI/180.0));
            y_now-=(box/2*cos(-psi*M_PI/180.0));
        }
        x_center+=(box/2*cos(-psi*M_PI/180.0));
        y_center-=(box/2*sin(-psi*M_PI/180.0));
        dz+=(box/2*tan(theta*M_PI/180.0)*pix);  // 对于高度变化，tan\theta与dz同号！！！
    }
    x_center=Nx/2-(box/2*cos(-psi*M_PI/180.0));
    y_center=Ny/2+(box/2*sin(-psi*M_PI/180.0));
    dz=-(box/2*tan(theta*M_PI/180.0)*pix);
    while(int(floor(x_center)-box/2)>=0 && int(floor(x_center)+box/2)<Nx && int(floor(y_center)-box/2)>=0 && int(floor(y_center)+box/2)<Ny)
    {
        float x_now=x_center;
        float y_now=y_center;
        while(int(floor(x_now)-box/2)>=0 && int(floor(x_now)+box/2)<Nx && int(floor(y_now)-box/2)>=0 && int(floor(y_now)+box/2)<Ny)
        {
            N_block++;
            int t=0;
            for(int j=int(floor(y_now)-box/2);j<int(floor(y_now)+box/2);j++)
            {
                for(int i=int(floor(x_now)-box/2);i<int(floor(x_now)+box/2);i++)
                {
                    block[t]=image_now[j*Nx+i];
                    t++;
                }
            }
            fftshift_2d(block,block_scaling_shift,box,box);
            for(int i=0;i<box*box;i++)
            {
                block_scaling_shift_aug[i*2]=block_scaling_shift[i];
                block_scaling_shift_aug[i*2+1]=0.0;
            }
            for(int i=0;i<box*box*2;i++)
            {
                fft_buf_in[i]=block_scaling_shift_aug[i];
            }
            fftwf_execute(plan_fft);
            for(int i=0;i<box*box*2;i++)
            {
                block_scaling_shift_fft[i]=fft_buf_out[i];
            }
            fftshift_2d_complex(block_scaling_shift_fft,block_scaling_shift_fft_shift,box,box);
            for(int i=0;i<box*box;i++)
            {
                block_scaling[i]=0.0;
                block_scaling_shift_fft_shift_amp[i]=block_scaling_shift_fft_shift[2*i]*block_scaling_shift_fft_shift[2*i]+block_scaling_shift_fft_shift[2*i+1]*block_scaling_shift_fft_shift[2*i+1];
            }
            float df_scale=sqrt(df/(df+dz));
            for(int j=0;j<box;j++)
            {
                for(int i=0;i<box;i++)
                {
                    float res=sqrt(x_fft[i]*x_fft[i]+y_fft[j]*y_fft[j]);
                    float x_scale=x_fft[i]*df_scale+box/2;
                    float y_scale=y_fft[j]*df_scale+box/2;
                    if(x_scale>=0 && x_scale<=box-1 && y_scale>=0 && y_scale<=box-1)
                    {
                        float coeff_x=x_scale-floor(x_scale);
                        float coeff_y=y_scale-floor(y_scale);
                        block_scaling[j*box+i]=(1-coeff_x)*(1-coeff_y)*block_scaling_shift_fft_shift_amp[int(floor(y_scale))*box+int(floor(x_scale))]+(1-coeff_x)*(coeff_y)*block_scaling_shift_fft_shift_amp[int(ceil(y_scale))*box+int(floor(x_scale))]+(coeff_x)*(1-coeff_y)*block_scaling_shift_fft_shift_amp[int(floor(y_scale))*box+int(ceil(x_scale))]+(coeff_x)*(coeff_y)*block_scaling_shift_fft_shift_amp[int(ceil(y_scale))*box+int(ceil(x_scale))];
                    } 
                }
            }
            for(int i=0;i<box*box;i++)
            {
                psd[i]+=block_scaling[i];
            }
            x_now+=(box/2*sin(-psi*M_PI/180.0));
            y_now+=(box/2*cos(-psi*M_PI/180.0));
        }
        x_now=x_center-box/2*sin(-psi*M_PI/180.0);
        y_now=y_center-box/2*cos(-psi*M_PI/180.0);
        while(int(floor(x_now)-box/2)>=0 && int(floor(x_now)+box/2)<Nx && int(floor(y_now)-box/2)>=0 && int(floor(y_now)+box/2)<Ny)
        {
            N_block++;
            int t=0;
            for(int j=int(floor(y_now)-box/2);j<int(floor(y_now)+box/2);j++)
            {
                for(int i=int(floor(x_now)-box/2);i<int(floor(x_now)+box/2);i++)
                {
                    block[t]=image_now[j*Nx+i];
                    t++;
                }
            }
            fftshift_2d(block,block_scaling_shift,box,box);
            for(int i=0;i<box*box;i++)
            {
                block_scaling_shift_aug[i*2]=block_scaling_shift[i];
                block_scaling_shift_aug[i*2+1]=0.0;
            }
            for(int i=0;i<box*box*2;i++)
            {
                fft_buf_in[i]=block_scaling_shift_aug[i];
            }
            fftwf_execute(plan_fft);
            for(int i=0;i<box*box*2;i++)
            {
                block_scaling_shift_fft[i]=fft_buf_out[i];
            }
            fftshift_2d_complex(block_scaling_shift_fft,block_scaling_shift_fft_shift,box,box);
            for(int i=0;i<box*box;i++)
            {
                block_scaling[i]=0.0;
                block_scaling_shift_fft_shift_amp[i]=block_scaling_shift_fft_shift[2*i]*block_scaling_shift_fft_shift[2*i]+block_scaling_shift_fft_shift[2*i+1]*block_scaling_shift_fft_shift[2*i+1];
            }
            float df_scale=sqrt(df/(df+dz));
            for(int j=0;j<box;j++)
            {
                for(int i=0;i<box;i++)
                {
                    float res=sqrt(x_fft[i]*x_fft[i]+y_fft[j]*y_fft[j]);
                    float x_scale=x_fft[i]*df_scale+box/2;
                    float y_scale=y_fft[j]*df_scale+box/2;
                    if(x_scale>=0 && x_scale<=box-1 && y_scale>=0 && y_scale<=box-1)
                    {
                        float coeff_x=x_scale-floor(x_scale);
                        float coeff_y=y_scale-floor(y_scale);
                        block_scaling[j*box+i]=(1-coeff_x)*(1-coeff_y)*block_scaling_shift_fft_shift_amp[int(floor(y_scale))*box+int(floor(x_scale))]+(1-coeff_x)*(coeff_y)*block_scaling_shift_fft_shift_amp[int(ceil(y_scale))*box+int(floor(x_scale))]+(coeff_x)*(1-coeff_y)*block_scaling_shift_fft_shift_amp[int(floor(y_scale))*box+int(ceil(x_scale))]+(coeff_x)*(coeff_y)*block_scaling_shift_fft_shift_amp[int(ceil(y_scale))*box+int(ceil(x_scale))];
                    } 
                }
            }
            for(int i=0;i<box*box;i++)
            {
                psd[i]+=block_scaling[i];
            }
            x_now-=(box/2*sin(-psi*M_PI/180.0));
            y_now-=(box/2*cos(-psi*M_PI/180.0));
        }
        x_center-=(box/2*cos(-psi*M_PI/180.0));
        y_center+=(box/2*sin(-psi*M_PI/180.0));
        dz-=(box/2*tan(theta*M_PI/180.0)*pix);
    }
    for(int i=0;i<box*box;i++)
    {
        psd[i]/=N_block;
        psd_float[i]=float(psd[i]);
    }
    delete [] block_scaling_shift_fft_shift_amp;
    delete [] block_scaling_shift_fft_shift;
    delete [] block_scaling_shift_fft;
    delete [] block_scaling_shift_aug;
    delete [] block_scaling_shift;
    delete [] block_scaling;
    delete [] block;
    delete [] psd;
}

int get_blocks(int box,int *N_block_x,int *N_block_y,int *block_x_min,int *block_x_max,int *block_y_min,int *block_y_max,int Nx,int Ny)
{
    int x_center=Nx/2;  // box的中心坐标
    int y_center=Ny/2;
    *N_block_x=0;
    x_center=Nx/2;
    int x_min=Nx/2;
    int x_max=Nx/2;
    while(x_center-box/2>=0 && x_center+box/2<Nx)
    {
        x_max=x_center;
        *N_block_x=*N_block_x+1;
        x_center+=(box/2);
    }
    x_center=Nx/2-(box/2);
    while(x_center-box/2>=0 && x_center+box/2<Nx)
    {
        x_min=x_center;
        *N_block_x=*N_block_x+1;
        x_center-=(box/2);
    }
    *N_block_y=0;
    y_center=Ny/2;
    int y_min=Ny/2;
    int y_max=Ny/2;
    while(y_center-box/2>=0 && y_center+box/2<Ny)
    {
        y_max=y_center;
        *N_block_y=*N_block_y+1;
        y_center+=(box/2);
    }
    y_center=Ny/2-(box/2);
    while(y_center-box/2>=0 && y_center+box/2<Ny)
    {
        y_min=y_center;
        *N_block_y=*N_block_y+1;
        y_center-=(box/2);
    }
    int N_block=(*N_block_x)*(*N_block_y);
    *block_x_min=x_min;
    *block_x_max=x_max;
    *block_y_min=y_min;
    *block_y_max=y_max;
    return N_block;
}

int get_blocks_tight(int box,int *N_block_x,int *N_block_y,int *block_x_min,int *block_x_max,int *block_y_min,int *block_y_max,int Nx,int Ny)
{
    int x_center=Nx/2;  // box的中心坐标
    int y_center=Ny/2;
    *N_block_x=0;
    x_center=Nx/2;
    int x_min=Nx/2;
    int x_max=Nx/2;
    while(x_center-box/2>=0 && x_center+box/2<Nx)
    {
        x_max=x_center;
        *N_block_x=*N_block_x+1;
        x_center+=(box/2);
    }
    x_center=Nx/2-(box/2);
    while(x_center-box/2>=0 && x_center+box/2<Nx)
    {
        x_min=x_center;
        *N_block_x=*N_block_x+1;
        x_center-=(box/2);
    }
    *N_block_y=0;
    y_center=Ny/2;
    int y_min=Ny/2;
    int y_max=Ny/2;
    while(y_center-box/2>=0 && y_center+box/2<Ny)
    {
        y_max=y_center;
        *N_block_y=*N_block_y+1;
        y_center+=(box/2);
    }
    y_center=Ny/2-(box/2);
    while(y_center-box/2>=0 && y_center+box/2<Ny)
    {
        y_min=y_center;
        *N_block_y=*N_block_y+1;
        y_center-=(box/2);
    }
    *N_block_x-=2;
    *N_block_y-=2;
    int N_block=(*N_block_x)*(*N_block_y);
    *block_x_min=x_min+box/2;
    *block_x_max=x_max-box/2;
    *block_y_min=y_min+box/2;
    *block_y_max=y_max-box/2;
    return N_block;
}

void get_block_coords(int *block_x,int *block_y,int box,int block_x_min,int block_x_max,int block_y_min,int block_y_max,int N_block_x,int N_block_y,int Nx,int Ny)
{
    for(int j=0;j<N_block_y;j++)
    {
        for(int i=0;i<N_block_x;i++)
        {
            block_x[j*N_block_x+i]=block_x_min+i*(box/2);
            block_y[j*N_block_x+i]=block_y_min+j*(box/2);
        }
    }
}

void get_psd_scaling_one_omp(float *image_now,float *psd,int block_x,int block_y,int box,float df,float pix,float psi,float theta,int Nx,int Ny,bool is_scaling,fftwf_plan plan_fft,float *fft_buf_in,float *fft_buf_out)  // 切小块求平均功率谱，采用BSoft中的尺度变换统一零点位置，频域通过padding进行放缩
{
    // 单位换算到国际单位
    pix=pix*1e-10;  // A --> m
    df=df*1e-10;    // A --> m

    for(int i=0;i<box*box;i++)
    {
        psd[i]=0.0;
    }
    float x_fft[box],y_fft[box];
    for(int i=0;i<box;i++)
    {
        x_fft[i]=i-box/2;
        y_fft[i]=i-box/2;
    }

    int x_center=Nx/2;  // box的中心坐标
    int y_center=Ny/2;

    float v_x=float(block_x-x_center);
    float v_y=float(block_y-y_center);
    float nv_x=cos(-psi*M_PI/180.0);
    float nv_y=-sin(-psi*M_PI/180.0);
    float dist_pix=v_x*nv_x+v_y*nv_y;
    float dz=dist_pix*tan(theta*M_PI/180.0)*pix;

    float *block=new float[box*box];
    float *block_scaling=new float[box*box];
    float *block_scaling_shift=new float[box*box];
    float *block_scaling_shift_aug=new float[box*box*2];
    float *block_scaling_shift_fft=new float[box*box*2];
    float *block_scaling_shift_fft_shift=new float[box*box*2];
    float *block_scaling_shift_fft_shift_amp=new float[box*box];
    for(int i=0;i<box*box;i++)
    {
        block[i]=0.0;
        block_scaling[i]=0.0;
        block_scaling_shift[i]=0.0;
        block_scaling_shift_fft_shift_amp[i]=0.0;
    }
    for(int i=0;i<box*box*2;i++)
    {
        block_scaling_shift_aug[i]=0.0;
        block_scaling_shift_fft[i]=0.0;
        block_scaling_shift_fft_shift[i]=0.0;
    }

    int t=0;
    for(int j=block_y-box/2;j<block_y+box/2;j++)
    {
        for(int i=block_x-box/2;i<block_x+box/2;i++)
        {
            block[t]=image_now[j*Nx+i];
            t++;
        }
    }
    fftshift_2d(block,block_scaling_shift,box,box);
    for(int i=0;i<box*box;i++)
    {
        block_scaling_shift_aug[i*2]=block_scaling_shift[i];
        block_scaling_shift_aug[i*2+1]=0.0;
    }
    for(int i=0;i<box*box*2;i++)
    {
        fft_buf_in[i]=block_scaling_shift_aug[i];
    }
    fftwf_execute(plan_fft);
    for(int i=0;i<box*box*2;i++)
    {
        block_scaling_shift_fft[i]=fft_buf_out[i];
    }
    fftshift_2d_complex(block_scaling_shift_fft,block_scaling_shift_fft_shift,box,box);
    for(int i=0;i<box*box;i++)
    {
        block_scaling[i]=0.0;
        block_scaling_shift_fft_shift_amp[i]=block_scaling_shift_fft_shift[2*i]*block_scaling_shift_fft_shift[2*i]+block_scaling_shift_fft_shift[2*i+1]*block_scaling_shift_fft_shift[2*i+1];
    }
    if(is_scaling)
    {
        float df_scale=sqrt(df/(df+dz));
        for(int j=0;j<box;j++)
        {
            for(int i=0;i<box;i++)
            {
                float res=sqrt(x_fft[i]*x_fft[i]+y_fft[j]*y_fft[j]);
                float x_scale=x_fft[i]*df_scale+box/2;
                float y_scale=y_fft[j]*df_scale+box/2;
                if(x_scale>=0 && x_scale<=box-1 && y_scale>=0 && y_scale<=box-1)
                {
                    float coeff_x=x_scale-floor(x_scale);
                    float coeff_y=y_scale-floor(y_scale);
                    block_scaling[j*box+i]=(1-coeff_x)*(1-coeff_y)*block_scaling_shift_fft_shift_amp[int(floor(y_scale))*box+int(floor(x_scale))]+(1-coeff_x)*(coeff_y)*block_scaling_shift_fft_shift_amp[int(ceil(y_scale))*box+int(floor(x_scale))]+(coeff_x)*(1-coeff_y)*block_scaling_shift_fft_shift_amp[int(floor(y_scale))*box+int(ceil(x_scale))]+(coeff_x)*(coeff_y)*block_scaling_shift_fft_shift_amp[int(ceil(y_scale))*box+int(ceil(x_scale))];
                } 
            }
        }
    }
    else
    {
        for(int i=0;i<box*box;i++)
        {
            block_scaling[i]=block_scaling_shift_fft_shift_amp[i];
        }
    }
    for(int i=0;i<box*box;i++)
    {
        psd[i]=block_scaling[i];
    }

    delete [] block_scaling_shift_fft_shift_amp;
    delete [] block_scaling_shift_fft_shift;
    delete [] block_scaling_shift_fft;
    delete [] block_scaling_shift_aug;
    delete [] block_scaling_shift;
    delete [] block_scaling;
    delete [] block;
}

void get_psd_one_padding_omp(float *image_now,float *psd,int block_x,int block_y,int box,float df,float pix,float psi,float theta,int Nx,int Ny,fftwf_plan plan_fft,fftwf_plan plan_ifft_small,float *fft_buf_in,float *fft_buf_out)  // 切小块求平均功率谱，采用BSoft中的尺度变换统一零点位置，频域通过padding进行放缩（box要求为4的倍数！！）
{
    // 单位换算到国际单位
    pix=pix*1e-10;  // A --> m
    df=df*1e-10;    // A --> m

    for(int i=0;i<box*box;i++)
    {
        psd[i]=0.0;
    }
    float x_fft[box],y_fft[box];
    for(int i=0;i<box;i++)
    {
        x_fft[i]=i-box/2;
        y_fft[i]=i-box/2;
    }

    float *block=new float[box*box];
    float *block_shift=new float[box*box];
    float *block_shift_complex=new float[box*box*2];
    float *block_shift_fft=new float[box*box*2];
    float *block_shift_fft_shift=new float[box*box*2];
    float *block_shift_fft_shift_center=new float[box/2*box/2*2];
    float *block_shift_fft_center=new float[box/2*box/2*2];
    float *block_shift_center_complex=new float[box/2*box/2*2];
    float *block_shift_center=new float[box/2*box/2];
    float *block_shift_center_padding=new float[box*box];
    float *block_shift_center_padding_complex=new float[box*box*2];
    float *block_shift_center_padding_fft=new float[box*box*2];
    float *block_shift_center_padding_fft_shift=new float[box*box*2];
    float *block_shift_center_padding_fft_shift_amp=new float[box*box];
    for(int i=0;i<box*box;i++)
    {
        block[i]=0.0;
        block_shift[i]=0.0;
        block_shift_center_padding[i]=0.0;
        block_shift_center_padding_fft_shift_amp[i]=0.0;
    }
    for(int i=0;i<box*box*2;i++)
    {
        block_shift_complex[i]=0.0;
        block_shift_fft[i]=0.0;
        block_shift_fft_shift[i]=0.0;
        block_shift_center_padding_complex[i]=0.0;
        block_shift_center_padding_fft[i]=0.0;
        block_shift_center_padding_fft_shift[i]=0.0;
    }
    for(int i=0;i<box/2*box/2*2;i++)
    {
        block_shift_fft_shift_center[i]=0.0;
        block_shift_fft_center[i]=0.0;
        block_shift_center_complex[i]=0.0;
    }
    for(int i=0;i<box/2*box/2;i++)
    {
        block_shift_center[i]=0.0;
    }

    // 正变换计算频谱
    int t=0;
    for(int j=block_y-box/2;j<block_y+box/2;j++)
    {
        for(int i=block_x-box/2;i<block_x+box/2;i++)
        {
            block[t]=image_now[j*Nx+i];
            t++;
        }
    }
    fftshift_2d(block,block_shift,box,box);
    for(int i=0;i<box*box;i++)
    {
        block_shift_complex[i*2]=block_shift[i];
        block_shift_complex[i*2+1]=0.0;
    }
    for(int i=0;i<box*box*2;i++)
    {
        fft_buf_in[i]=block_shift_complex[i];
    }
    fftwf_execute(plan_fft);
    for(int i=0;i<box*box*2;i++)
    {
        block_shift_fft[i]=fft_buf_out[i];
    }
    fftshift_2d_complex(block_shift_fft,block_shift_fft_shift,box,box);

    // 保留低频频谱（0.5倍）（最高频置零，防止不对称）
    t=0;
    for(int j=box/4;j<box/4*3;j++)
    {
        for(int i=box/4;i<box/4*3;i++)
        {
            block_shift_fft_shift_center[t]=block_shift_fft_shift[j*box*2+2*i];
            block_shift_fft_shift_center[t+1]=block_shift_fft_shift[j*box*2+2*i+1];
            t+=2;
        }
    }
    for(int i=0;i<box/2;i++)
    {
        block_shift_fft_shift_center[2*i]=0.0;
        block_shift_fft_shift_center[2*i+1]=0.0;
    }
    for(int j=0;j<box/2;j++)
    {
        block_shift_fft_shift_center[j*box/2*2]=0.0;
        block_shift_fft_shift_center[j*box/2*2+1]=0.0;
    }

    // 逆变换计算bin后图像
    ifftshift_2d_complex(block_shift_fft_shift_center,block_shift_fft_center,box/2,box/2);
    memcpy(fft_buf_in,block_shift_fft_center,sizeof(float)*box/2*box/2*2);
    fftwf_execute(plan_ifft_small);
    memcpy(block_shift_center_complex,fft_buf_out,sizeof(float)*box/2*box/2*2);
    for(int i=0;i<box/2*box/2*2;i++)    // ifft需要归一化！！
    {
        block_shift_center_complex[i]/=float(box/2*box/2);
    }
    for(int i=0;i<box/2*box/2;i++)
    {
        block_shift_center[i]=block_shift_center_complex[i*2];
    }

    // 时域padding（2倍）（均值padding！！）
    double block_shift_center_sum=0.0;
    for(int i=0;i<box/2*box/2;i++)
    {
        block_shift_center_sum+=double(block_shift_center[i]);
    }
    float block_shift_center_avg=float(block_shift_center_sum/double(box/2*box/2));
    for(int i=0;i<box*box;i++)
    {
        block_shift_center_padding[i]=block_shift_center_avg;
    }
    t=0;
    for(int j=0;j<box/2;j++)
    {
        for(int i=0;i<box/2;i++)
        {
            block_shift_center_padding[j*box+i]=block_shift_center[t];
            t++;
        }
    }

    // 正变换计算功率谱
    for(int i=0;i<box*box;i++)
    {
        block_shift_center_padding_complex[i*2]=block_shift_center_padding[i];
    }
    memcpy(fft_buf_in,block_shift_center_padding_complex,sizeof(float)*box*box*2);
    fftwf_execute(plan_fft);
    memcpy(block_shift_center_padding_fft,fft_buf_out,sizeof(float)*box*box*2);
    fftshift_2d_complex(block_shift_center_padding_fft,block_shift_center_padding_fft_shift,box,box);
    for(int i=0;i<box*box;i++)
    {
        block_shift_center_padding_fft_shift_amp[i]=block_shift_center_padding_fft_shift[2*i]*block_shift_center_padding_fft_shift[2*i]+block_shift_center_padding_fft_shift[2*i+1]*block_shift_center_padding_fft_shift[2*i+1];
    }

    for(int i=0;i<box*box;i++)
    {
        psd[i]=block_shift_center_padding_fft_shift_amp[i];
    }

    delete [] block_shift_center_padding_fft_shift_amp;
    delete [] block_shift_center_padding_fft_shift;
    delete [] block_shift_center_padding_fft;
    delete [] block_shift_center_padding_complex;
    delete [] block_shift_center_padding;
    delete [] block_shift_center;
    delete [] block_shift_center_complex;
    delete [] block_shift_fft_center;
    delete [] block_shift_fft_shift_center;
    delete [] block_shift_fft_shift;
    delete [] block_shift_fft;
    delete [] block_shift_complex;
    delete [] block_shift;
    delete [] block;
}


void get_scaled_psd(float *psd_orig,float *psd_scaled,int block_x,int block_y,int box,float df,float pix,float psi,float theta,int Nx,int Ny)  // 切小块求平均功率谱，采用BSoft中的尺度变换统一零点位置，频域直接放缩
{
    // 单位换算到国际单位
    pix=pix*1e-10;  // A --> m
    df=df*1e-10;    // A --> m

    for(int i=0;i<box*box;i++)
    {
        psd_scaled[i]=0.0;
    }
    float x_fft[box],y_fft[box];
    for(int i=0;i<box;i++)
    {
        x_fft[i]=i-box/2;
        y_fft[i]=i-box/2;
    }

    int x_center=Nx/2;  // box的中心坐标
    int y_center=Ny/2;

    float v_x=float(block_x-x_center);
    float v_y=float(block_y-y_center);
    float nv_x=cos(-psi*M_PI/180.0);
    float nv_y=-sin(-psi*M_PI/180.0);
    float dist_pix=v_x*nv_x+v_y*nv_y;
    float dz=dist_pix*tan(theta*M_PI/180.0)*pix;
    float df_scale=sqrt(df/(df+dz));
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            float res=sqrt(x_fft[i]*x_fft[i]+y_fft[j]*y_fft[j]);
            float x_scale=x_fft[i]*df_scale+box/2;
            float y_scale=y_fft[j]*df_scale+box/2;
            if(x_scale>=0 && x_scale<=box-1 && y_scale>=0 && y_scale<=box-1)
            {
                float coeff_x=x_scale-floor(x_scale);
                float coeff_y=y_scale-floor(y_scale);
                psd_scaled[j*box+i]=(1-coeff_x)*(1-coeff_y)*psd_orig[int(floor(y_scale))*box+int(floor(x_scale))]+(1-coeff_x)*(coeff_y)*psd_orig[int(ceil(y_scale))*box+int(floor(x_scale))]+(coeff_x)*(1-coeff_y)*psd_orig[int(floor(y_scale))*box+int(ceil(x_scale))]+(coeff_x)*(coeff_y)*psd_orig[int(ceil(y_scale))*box+int(ceil(x_scale))];
            } 
        }
    }
}

void get_scaled_psd_phi(float *psd_orig,float *psd_scaled,int block_x,int block_y,int box,float df,float pix,float psi,float theta,float phi,int Nx,int Ny)  // 切小块求平均功率谱，采用BSoft中的尺度变换统一零点位置，频域直接放缩
{
    // 单位换算到国际单位
    pix=pix*1e-10;  // A --> m
    df=df*1e-10;    // A --> m

    for(int i=0;i<box*box;i++)
    {
        psd_scaled[i]=0.0;
    }
    float x_fft[box],y_fft[box];
    for(int i=0;i<box;i++)
    {
        x_fft[i]=i-box/2;
        y_fft[i]=i-box/2;
    }

    int x_center=Nx/2;  // box的中心坐标
    int y_center=Ny/2;

    float v_x=float(block_x-x_center);
    float v_y=float(block_y-y_center);
    float nx_phi=0;
    float ny_phi=sin(phi*M_PI/180.0);
    float nz_phi=cos(phi*M_PI/180.0);
    float nx_theta=nz_phi*sin(theta*M_PI/180.0);
    float ny_theta=ny_phi;
    float nz_theta=nz_phi*cos(theta*M_PI/180.0);
    float nx_psi=nx_theta*cos(psi*M_PI/180.0)-ny_theta*sin(psi*M_PI/180.0);
    float ny_psi=nx_theta*sin(psi*M_PI/180.0)+ny_theta*cos(psi*M_PI/180.0);
    float nz_psi=nz_theta;
    float dz=0.0;
    if(abs(nz_theta)<1e-4)
    {
        dz=0.0;
    }
    else
    {
        dz=(nx_psi*v_x+ny_psi*v_y)/nz_psi*pix;
    }
    float df_scale=sqrt(df/(df+dz));

    // float v_x=float(block_x-x_center);
    // float v_y=float(block_y-y_center);
    // float nv_x=cos(-psi*M_PI/180.0);
    // float nv_y=-sin(-psi*M_PI/180.0);
    // float dist_pix=v_x*nv_x+v_y*nv_y;
    // float dz=dist_pix*tan(theta*M_PI/180.0)*pix;
    // float tv_x=sin(-psi*M_PI/180.0);
    // float tv_y=cos(-psi*M_PI/180.0);
    // float trans_pix=v_x*tv_x+v_y*tv_y;
    // float dh=trans_pix*tan(phi*M_PI/180.0)*pix;
    // float df_scale=sqrt(df/(df+dz+dh));

    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            float res=sqrt(x_fft[i]*x_fft[i]+y_fft[j]*y_fft[j]);
            float x_scale=x_fft[i]*df_scale+box/2;
            float y_scale=y_fft[j]*df_scale+box/2;
            if(x_scale>=0 && x_scale<=box-1 && y_scale>=0 && y_scale<=box-1)
            {
                float coeff_x=x_scale-floor(x_scale);
                float coeff_y=y_scale-floor(y_scale);
                psd_scaled[j*box+i]=(1-coeff_x)*(1-coeff_y)*psd_orig[int(floor(y_scale))*box+int(floor(x_scale))]+(1-coeff_x)*(coeff_y)*psd_orig[int(ceil(y_scale))*box+int(floor(x_scale))]+(coeff_x)*(1-coeff_y)*psd_orig[int(floor(y_scale))*box+int(ceil(x_scale))]+(coeff_x)*(coeff_y)*psd_orig[int(ceil(y_scale))*box+int(ceil(x_scale))];
            } 
        }
    }
}

void get_scaled_psd_all(float *psd_orig,float *psd_scaled,int block_x,int block_y,int box,float df,float df_ref,float pix,float psi,float theta,int Nx,int Ny)  // 切小块求平均功率谱，采用BSoft中的尺度变换统一零点位置，频域直接放缩
{
    // 单位换算到国际单位
    pix=pix*1e-10;  // A --> m
    df=df*1e-10;    // A --> m
    df_ref=df_ref*1e-10;    // A --> m

    for(int i=0;i<box*box;i++)
    {
        psd_scaled[i]=0.0;
    }
    float x_fft[box],y_fft[box];
    for(int i=0;i<box;i++)
    {
        x_fft[i]=i-box/2;
        y_fft[i]=i-box/2;
    }

    int x_center=Nx/2;  // box的中心坐标
    int y_center=Ny/2;

    float v_x=float(block_x-x_center);
    float v_y=float(block_y-y_center);
    float nv_x=cos(-psi*M_PI/180.0);
    float nv_y=-sin(-psi*M_PI/180.0);
    float dist_pix=v_x*nv_x+v_y*nv_y;
    float dz=dist_pix*tan(theta*M_PI/180.0)*pix;
    float df_scale=sqrt(df_ref/(df+dz));
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            float res=sqrt(x_fft[i]*x_fft[i]+y_fft[j]*y_fft[j]);
            float x_scale=x_fft[i]*df_scale+box/2;
            float y_scale=y_fft[j]*df_scale+box/2;
            if(x_scale>=0 && x_scale<=box-1 && y_scale>=0 && y_scale<=box-1)
            {
                float coeff_x=x_scale-floor(x_scale);
                float coeff_y=y_scale-floor(y_scale);
                psd_scaled[j*box+i]=(1-coeff_x)*(1-coeff_y)*psd_orig[int(floor(y_scale))*box+int(floor(x_scale))]+(1-coeff_x)*(coeff_y)*psd_orig[int(ceil(y_scale))*box+int(floor(x_scale))]+(coeff_x)*(1-coeff_y)*psd_orig[int(floor(y_scale))*box+int(ceil(x_scale))]+(coeff_x)*(coeff_y)*psd_orig[int(ceil(y_scale))*box+int(ceil(x_scale))];
            } 
        }
    }
}

void get_scaled_psd_all_phi(float *psd_orig,float *psd_scaled,int block_x,int block_y,int box,float df,float df_ref,float pix,float psi,float theta,float phi,int Nx,int Ny)  // 切小块求平均功率谱，采用BSoft中的尺度变换统一零点位置，频域直接放缩
{
    // 单位换算到国际单位
    pix=pix*1e-10;  // A --> m
    df=df*1e-10;    // A --> m
    df_ref=df_ref*1e-10;    // A --> m

    for(int i=0;i<box*box;i++)
    {
        psd_scaled[i]=0.0;
    }
    float x_fft[box],y_fft[box];
    for(int i=0;i<box;i++)
    {
        x_fft[i]=i-box/2;
        y_fft[i]=i-box/2;
    }

    int x_center=Nx/2;  // box的中心坐标
    int y_center=Ny/2;

    float v_x=float(block_x-x_center);
    float v_y=float(block_y-y_center);
    float nx_phi=0;
    float ny_phi=sin(phi*M_PI/180.0);
    float nz_phi=cos(phi*M_PI/180.0);
    float nx_theta=nz_phi*sin(theta*M_PI/180.0);
    float ny_theta=ny_phi;
    float nz_theta=nz_phi*cos(theta*M_PI/180.0);
    float nx_psi=nx_theta*cos(psi*M_PI/180.0)-ny_theta*sin(psi*M_PI/180.0);
    float ny_psi=nx_theta*sin(psi*M_PI/180.0)+ny_theta*cos(psi*M_PI/180.0);
    float nz_psi=nz_theta;
    float dz=0.0;
    if(abs(nz_theta)<1e-4)
    {
        dz=0.0;
    }
    else
    {
        dz=(nx_psi*v_x+ny_psi*v_y)/nz_psi*pix;
    }
    float df_scale=sqrt(df/(df+dz));

    // float v_x=float(block_x-x_center);
    // float v_y=float(block_y-y_center);
    // float nv_x=cos(-psi*M_PI/180.0);
    // float nv_y=-sin(-psi*M_PI/180.0);
    // float dist_pix=v_x*nv_x+v_y*nv_y;
    // float dz=dist_pix*tan(theta*M_PI/180.0)*pix;
    // float tv_x=sin(-psi*M_PI/180.0);
    // float tv_y=cos(-psi*M_PI/180.0);
    // float trans_pix=v_x*tv_x+v_y*tv_y;
    // float dh=trans_pix*tan(phi*M_PI/180.0)*pix;
    // float df_scale=sqrt(df_ref/(df+dz+dh));

    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            // float res=sqrt(x_fft[i]*x_fft[i]+y_fft[j]*y_fft[j]);
            float x_scale=x_fft[i]*df_scale+box/2;
            float y_scale=y_fft[j]*df_scale+box/2;
            if(x_scale>=0 && x_scale<=box-1 && y_scale>=0 && y_scale<=box-1)
            {
                float floor_x_scale=floor(x_scale);
                float ceil_x_scale=ceil(x_scale);
                float floor_y_scale=floor(y_scale);
                float ceil_y_scale=ceil(y_scale);
                float coeff_x=x_scale-floor_x_scale;
                float coeff_y=y_scale-floor_y_scale;
                psd_scaled[j*box+i]=(1-coeff_x)*(1-coeff_y)*psd_orig[int(floor_y_scale)*box+int(floor_x_scale)]+(1-coeff_x)*(coeff_y)*psd_orig[int(ceil_y_scale)*box+int(floor_x_scale)]+(coeff_x)*(1-coeff_y)*psd_orig[int(floor_y_scale)*box+int(ceil_x_scale)]+(coeff_x)*(coeff_y)*psd_orig[int(ceil_y_scale)*box+int(ceil_x_scale)];
                // float coeff_x=x_scale-floor(x_scale);
                // float coeff_y=y_scale-floor(y_scale);
                // psd_scaled[j*box+i]=(1-coeff_x)*(1-coeff_y)*psd_orig[int(floor(y_scale))*box+int(floor(x_scale))]+(1-coeff_x)*(coeff_y)*psd_orig[int(ceil(y_scale))*box+int(floor(x_scale))]+(coeff_x)*(1-coeff_y)*psd_orig[int(floor(y_scale))*box+int(ceil(x_scale))]+(coeff_x)*(coeff_y)*psd_orig[int(ceil(y_scale))*box+int(ceil(x_scale))];
            } 
        }
    }
}


void get_dose_weighted_psd(float *psd,int box,float dose_acc,float *x_fft_res,float *y_fft_res)
{
    float a=0.245;
    float b=-1.665;
    float c=2.81;

    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            float x_res_angstrom=x_fft_res[i]*1e-10;
            float y_res_angstrom=y_fft_res[j]*1e-10;
            float res_angstrom=sqrt(x_res_angstrom*x_res_angstrom+y_res_angstrom*y_res_angstrom);
            float critical_exposure;
            float weight;
            if(abs(res_angstrom)<1e-6)
            {
                weight=1.0;
            }
            else
            {
                critical_exposure=a*pow(res_angstrom,b)+c;
                weight=exp(-dose_acc/(2*critical_exposure));
            }
            psd[j*box+i]*=(weight*weight);
        }
    }
}


void get_radial_average(float *psd,float *radial,int box,CTF ctf_para,float *res2_all,float *atan_all,float *x_fft_1d_res)  // equal-phase average（沿CTF椭圆求平均）
{
    double *phase=new double[box/2-1];
    for(int i=0;i<box/2-1;i++)
    {
        float df=(ctf_para.getDefocus1()+ctf_para.getDefocus2()+(ctf_para.getDefocus1()-ctf_para.getDefocus2())*cos(2*(-ctf_para.getAstigmatism())))/2;
        phase[i]=M_PI*ctf_para.getLambda()*df*(x_fft_1d_res[i]*x_fft_1d_res[i])-M_PI_2*ctf_para.getCs()*(ctf_para.getLambda()*ctf_para.getLambda()*ctf_para.getLambda())*double(x_fft_1d_res[i]*x_fft_1d_res[i])*double(x_fft_1d_res[i]*x_fft_1d_res[i])+ctf_para.getW_phase();
    }

    float *radial_count=new float[box/2-1];
    for(int i=0;i<box/2-1;i++)
    {
        radial[i]=0.0;
        radial_count[i]=0.0;
    }
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            float df=(ctf_para.getDefocus1()+ctf_para.getDefocus2()+(ctf_para.getDefocus1()-ctf_para.getDefocus2())*cos(2*(atan_all[j*box+i]-ctf_para.getAstigmatism())))/2;
            // double res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
            double chi=M_PI*ctf_para.getLambda()*df*res2_all[j*box+i]-M_PI_2*ctf_para.getCs()*(ctf_para.getLambda()*ctf_para.getLambda()*ctf_para.getLambda())*(double(res2_all[j*box+i])*double(res2_all[j*box+i]))+ctf_para.getW_phase();
            if(chi>=phase[box/2-3])
            {
                radial[box/2-2]+=psd[j*box+i];
                radial_count[box/2-2]++;
            }
            else
            {
                int left=0;
                int right=box/2-3;
                while(left+1<right)
                {
                    int mid=(left+right)/2;
                    if(chi>=phase[mid])
                    {
                        left=mid;
                    }
                    else
                    {
                        right=mid;
                    }
                }
                radial[left]+=psd[j*box+i];
                radial_count[left]++;
            }
            /*
            for(int n=0;n<box/2-2;n++)
            {
                if(chi>=phase[n] && chi<phase[n+1])
                {
                    radial[n]+=psd[j*box+i];
                    radial_count[n]++;
                    // float coeff=(chi-phase[n])/(phase[n+1]-phase[n]);
                    // radial[n]+=(1-coeff)*psd[j*box+i];
                    // radial_count[n]+=(1-coeff);
                    // radial[n+1]+=coeff*psd[j*box+i];
                    // radial_count[n+1]+=coeff;
                    break;
                }
                if(n==box/2-3)  // 所有box/2之外的全部归为最后一项
                {
                    radial[n+1]+=psd[j*box+i];
                    radial_count[n+1]++;
                }
            }
            */
        }
    }

    for(int i=0;i<box/2-1;i++)
    {
        if(radial_count[i]==0)
        {
            radial[i]=0.0;
        }
        else
        {
            radial[i]/=float(radial_count[i]);
        }
    }

    delete [] radial_count;
    delete [] phase;
}

void get_radial_average_phase_shift(float *psd,float *radial,int box,CTF ctf_para)  // equal-phase average（沿CTF椭圆求平均）
{
    float *x_fft=new float[box];
    float *y_fft=new float[box];
    for(int i=0;i<box;i++)
    {
        x_fft[i]=float(i-box/2);
        y_fft[i]=float(i-box/2);
    }

    float *radial_count=new float[box/2-1];
    for(int i=0;i<box/2-1;i++)
    {
        radial[i]=0.0;
        radial_count[i]=0.0;
    }
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            float u=sqrt(x_fft[i]*x_fft[i]+y_fft[j]*y_fft[j]);
            if(u>=0 && u<=box/2-3)
            {
                radial[int(floor(u))]+=psd[j*box+i];
                radial_count[int(floor(u))]++;
            }
            else
            {
                radial[box/2-2]+=psd[j*box+i];
                radial_count[box/2-2]++;
            }
        }
    }

    for(int i=0;i<box/2-1;i++)
    {
        if(radial_count[i]==0)
        {
            radial[i]=0.0;
        }
        else
        {
            radial[i]/=float(radial_count[i]);
        }
    }

    delete [] radial_count;
    delete [] x_fft;
    delete [] y_fft;
}

void get_radial_average_psd(float *psd,float *psd_avg,int box,CTF ctf_para,float *res2_all,float *atan_all,float *x_fft_1d_res)  // equal-phase average（沿CTF椭圆求平均）
{
    double *phase=new double[box/2-1];
    for(int i=0;i<box/2-1;i++)
    {
        float df=(ctf_para.getDefocus1()+ctf_para.getDefocus2()+(ctf_para.getDefocus1()-ctf_para.getDefocus2())*cos(2*(-ctf_para.getAstigmatism())))/2;
        phase[i]=M_PI*ctf_para.getLambda()*df*(x_fft_1d_res[i]*x_fft_1d_res[i])-M_PI_2*ctf_para.getCs()*(ctf_para.getLambda()*ctf_para.getLambda()*ctf_para.getLambda())*double(x_fft_1d_res[i]*x_fft_1d_res[i])*double(x_fft_1d_res[i]*x_fft_1d_res[i])+ctf_para.getW_phase();
    }

    float *radial=new float[box/2-1];
    float *radial_count=new float[box/2-1];
    for(int i=0;i<box/2-1;i++)
    {
        radial[i]=0.0;
        radial_count[i]=0.0;
    }
    int *t_all=new int[box*box];
    double *chi_all=new double[box*box];
    for(int i=0;i<box*box;i++)
    {
        float df=(ctf_para.getDefocus1()+ctf_para.getDefocus2()+(ctf_para.getDefocus1()-ctf_para.getDefocus2())*cos(2*(atan_all[i]-ctf_para.getAstigmatism())))/2;
        // double res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
        double chi=M_PI*ctf_para.getLambda()*df*res2_all[i]-M_PI_2*ctf_para.getCs()*(ctf_para.getLambda()*ctf_para.getLambda()*ctf_para.getLambda())*(double(res2_all[i])*double(res2_all[i]))+ctf_para.getW_phase();
        chi_all[i]=chi;
        if(chi>=phase[box/2-2])
        {
            radial[box/2-2]+=psd[i];
            radial_count[box/2-2]++;
            t_all[i]=box/2-2;
        }
        else
        {
            int left=0;
            int right=box/2-2;
            while(left+1<right)
            {
                int mid=(left+right)/2;
                if(chi>=phase[mid])
                {
                    left=mid;
                }
                else
                {
                    right=mid;
                }
            }
            radial[left]+=psd[i];
            radial_count[left]++;
            t_all[i]=left;
        }
        /*
        for(int n=0;n<box/2-2;n++)
        {
            if(chi>=phase[n] && chi<phase[n+1])
            {
                radial[n]+=psd[j*box+i];
                radial_count[n]++;
                // float coeff=(chi-phase[n])/(phase[n+1]-phase[n]);
                // radial[n]+=(1-coeff)*psd[j*box+i];
                // radial_count[n]+=(1-coeff);
                // radial[n+1]+=coeff*psd[j*box+i];
                // radial_count[n+1]+=coeff;
                break;
            }
            if(n==box/2-3)  // 所有box/2之外的全部归为最后一项
            {
                radial[n+1]+=psd[j*box+i];
                radial_count[n+1]++;
            }
        }*/
    }

    for(int i=0;i<box/2-1;i++)
    {
        if(radial_count[i]==0)
        {
            radial[i]=0.0;
        }
        else
        {
            radial[i]/=float(radial_count[i]);
        }
    }

    memcpy(psd_avg,psd,sizeof(float)*box*box);
    for(int i=0;i<box*box;i++)
    {
        // if(t_all[i]==box/2-2)
        // {
        //     psd_avg[i]=radial[box/2-2];
        // }
        if(t_all[i]>=box/2-3)
        {
            psd_avg[i]=radial[box/2-3];
        }
        else
        {
            float coeff=(chi_all[i]-phase[t_all[i]])/(phase[t_all[i]+1]-phase[t_all[i]]);
            psd_avg[i]=(1-coeff)*radial[t_all[i]]+coeff*radial[t_all[i]+1];
        }
    }
    /*
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            float df=(ctf_para.getDefocus1()+ctf_para.getDefocus2()+(ctf_para.getDefocus1()-ctf_para.getDefocus2())*cos(2*(atan2(y_fft_res[j],x_fft_res[i])-ctf_para.getAstigmatism())))/2;
            double res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
            double chi=M_PI*ctf_para.getLambda()*df*res2-M_PI_2*ctf_para.getCs()*(ctf_para.getLambda()*ctf_para.getLambda()*ctf_para.getLambda())*(double(res2)*double(res2))+atan(ctf_para.getW()/sqrt(1-ctf_para.getW()*ctf_para.getW()));
            if(chi>phase[box/2-3])
            {
                psd_avg[j*box+i]=radial[box/2-2];
            }
            else
            {
                int left=0;
                int right=box/2-3;
                while(left+1<right)
                {
                    int mid=(left+right)/2;
                    if(chi>phase[mid])
                    {
                        left=mid;
                    }
                    else
                    {
                        right=mid;
                    }
                }
                float coeff=(chi-phase[left])/(phase[left+1]-phase[left]);
                psd_avg[j*box+i]=(1-coeff)*radial[left]+coeff*radial[left+1];
            }
            /*
            for(int n=0;n<box/2-2;n++)
            {
                if(chi>=phase[n] && chi<phase[n+1])
                {
                    // psd_avg[j*box+i]=radial[n];
                    float coeff=(chi-phase[n])/(phase[n+1]-phase[n]);
                    psd_avg[j*box+i]=(1-coeff)*radial[n]+coeff*radial[n+1];
                    break;
                }
                if(n==box/2-3)  // 所有box/2之外的全部归为最后一项
                {
                    psd_avg[j*box+i]=radial[n+1];
                }
            }
            */
            /*
        }
    }
    */

    delete [] chi_all;
    delete [] t_all;
    delete [] radial;
    delete [] radial_count;
    delete [] phase;
}

void get_radial_average_psd_phase_shift(float *psd,float *psd_avg,int box,CTF ctf_para,float *res2_all,float *atan_all,float *x_fft_1d_res)  // equal-phase average（沿CTF椭圆求平均）
{
    double *phase=new double[box/2-1];
    for(int i=0;i<box/2-1;i++)
    {
        float df=(ctf_para.getDefocus1()+ctf_para.getDefocus2()+(ctf_para.getDefocus1()-ctf_para.getDefocus2())*cos(2*(-ctf_para.getAstigmatism())))/2;
        phase[i]=M_PI*ctf_para.getLambda()*df*(x_fft_1d_res[i]*x_fft_1d_res[i])-M_PI_2*ctf_para.getCs()*(ctf_para.getLambda()*ctf_para.getLambda()*ctf_para.getLambda())*double(x_fft_1d_res[i]*x_fft_1d_res[i])*double(x_fft_1d_res[i]*x_fft_1d_res[i])+ctf_para.getW_phase()+ctf_para.getPhaseShift();
    }

    float *radial=new float[box/2-1];
    float *radial_count=new float[box/2-1];
    for(int i=0;i<box/2-1;i++)
    {
        radial[i]=0.0;
        radial_count[i]=0.0;
    }
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            float df=(ctf_para.getDefocus1()+ctf_para.getDefocus2()+(ctf_para.getDefocus1()-ctf_para.getDefocus2())*cos(2*(atan_all[j*box+i]-ctf_para.getAstigmatism())))/2;
            // double res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
            double chi=M_PI*ctf_para.getLambda()*df*res2_all[j*box+i]-M_PI_2*ctf_para.getCs()*(ctf_para.getLambda()*ctf_para.getLambda()*ctf_para.getLambda())*(double(res2_all[j*box+i])*double(res2_all[j*box+i]))+ctf_para.getW_phase()+ctf_para.getPhaseShift();
            for(int n=0;n<box/2-2;n++)
            {
                if(chi>=phase[n] && chi<phase[n+1])
                {
                    radial[n]+=psd[j*box+i];
                    radial_count[n]++;
                    // float coeff=(chi-phase[n])/(phase[n+1]-phase[n]);
                    // radial[n]+=(1-coeff)*psd[j*box+i];
                    // radial_count[n]+=(1-coeff);
                    // radial[n+1]+=coeff*psd[j*box+i];
                    // radial_count[n+1]+=coeff;
                    break;
                }
                if(n==box/2-3)  // 所有box/2之外的全部归为最后一项
                {
                    radial[n+1]+=psd[j*box+i];
                    radial_count[n+1]++;
                }
            }
        }
    }

    for(int i=0;i<box/2-1;i++)
    {
        if(radial_count[i]==0)
        {
            radial[i]=0.0;
        }
        else
        {
            radial[i]/=float(radial_count[i]);
        }
    }

    memcpy(psd_avg,psd,sizeof(float)*box*box);
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            float df=(ctf_para.getDefocus1()+ctf_para.getDefocus2()+(ctf_para.getDefocus1()-ctf_para.getDefocus2())*cos(2*(atan_all[j*box+i]-ctf_para.getAstigmatism())))/2;
            // double res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
            double chi=M_PI*ctf_para.getLambda()*df*res2_all[j*box+i]-M_PI_2*ctf_para.getCs()*(ctf_para.getLambda()*ctf_para.getLambda()*ctf_para.getLambda())*(double(res2_all[j*box+i])*double(res2_all[j*box+i]))+ctf_para.getW_phase()+ctf_para.getPhaseShift();
            for(int n=0;n<box/2-2;n++)
            {
                if(chi>=phase[n] && chi<phase[n+1])
                {
                    // psd_avg[j*box+i]=radial[n];
                    float coeff=(chi-phase[n])/(phase[n+1]-phase[n]);
                    psd_avg[j*box+i]=(1-coeff)*radial[n]+coeff*radial[n+1];
                    break;
                }
                if(n==box/2-3)  // 所有box/2之外的全部归为最后一项
                {
                    psd_avg[j*box+i]=radial[n+1];
                }
            }
        }
    }

    delete [] radial;
    delete [] radial_count;
    delete [] phase;
}

void background_estimation(double *fit,float *radial,int box,int N_zeros,CTF ctf_para_avg)
{
    if(N_zeros==0)
    {
        for(int i=0;i<box/2-1;i++)
        {
            fit[i]=0.0;
        }
        return;
    }
    float df=ctf_para_avg.getDefocus1();    // df_1=df_2
    int x_fft_1d[box/2-1];
    for(int i=0;i<box/2-1;i++)
    {
        x_fft_1d[i]=i;
    }
    double x_orig[N_zeros+1];
    double v_orig[N_zeros+1];
    int N_zeros_real=0;
    for(int n=0;n<N_zeros;n++)
    {
        double res2=(M_PI*ctf_para_avg.getLambda()*(df)-sqrt(M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df)*(df)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*((n+1)*M_PI-ctf_para_avg.getW_phase())))/(M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()))*(box*box*ctf_para_avg.getPixelSize()*ctf_para_avg.getPixelSize());
        int t=int(floor(sqrt(res2)));
        if(t<box/2-2)
        {
            double coeff_x=sqrt(res2)-floor(sqrt(res2));
            x_orig[n]=sqrt(res2);
            v_orig[n]=(1-coeff_x)*radial[t]+(coeff_x)*radial[t+1];
            N_zeros_real++;
        }
        /*
        for(int i=0;i<box/2-2;i++)
        {
            if(int(floor(sqrt(res2)))==x_fft_1d[i])
            {
                double coeff_x=sqrt(res2)-floor(sqrt(res2));
                x_orig[n]=sqrt(res2);
                v_orig[n]=(1-coeff_x)*radial[i]+(coeff_x)*radial[i+1];
                N_zeros_real++;
            }
        }
        */
    }
    // x_orig[N_zeros_real]=float(x_fft_1d[box/2-2]);
    // v_orig[N_zeros_real]=radial[box/2-2];
    // N_zeros_real++;
    // x_orig[N_zeros_real]=float(x_fft_1d[box/2-5]);  // 需要额外加一个判断条件，防止前面的零点频率过高，超过这个添加的频点！！！
    // v_orig[N_zeros_real]=radial[box/2-5];
    // v_orig[N_zeros_real]=0.0;
    // N_zeros_real++;
    
    // interpolation with cubic splines
    double xx[N_zeros_real];
    double yy[N_zeros_real];
    for(int n=0;n<N_zeros_real;n++)
    {
        xx[n]=x_orig[n];
        yy[n]=v_orig[n];
    }
    double y2[N_zeros_real];
    double u[N_zeros_real-1];
    y2[0]=0.0;
    u[0]=0.0;
    for(int i=1;i<N_zeros_real-1;i++)
    {
        double sig=(xx[i]-xx[i-1])/(xx[i+1]-xx[i-1]);
		double p=sig*y2[i-1]+2.0;
		y2[i]=(sig-1.0)/p;
		u[i]=(yy[i+1]-yy[i])/(xx[i+1]-xx[i])-(yy[i]-yy[i-1])/(xx[i]-xx[i-1]);
		u[i]=(6.0*u[i]/(xx[i+1]-xx[i-1])-sig*u[i-1])/p;
    }
	y2[N_zeros_real-1]=0.0;
    for(int k=N_zeros_real-2;k>=0;k--)
    {
        y2[k]=y2[k]*y2[k+1]+u[k];
    }
    int flag=0;
    int start=0;
    int N=N_zeros_real;
    for(int n=0;n<box/2-1;n++)
    {
        double x=double(x_fft_1d[n]);
        int t;
        if(flag==1)
        {
            int left=start;
            int mid;
            int right;
            int inc=1;
            bool ascnd=(xx[N-1]>=xx[0]);
            if(left<0 || left>N-1)
            {
                left=0;
                right=n-1;
            }
            else
            {
                if((x>=xx[left])==ascnd)
                {
                    while(1)
                    {
                        right=left+inc;
                        if(right>=N-1)
                        {
                            right=n-1;
                            break;
                        }
                        else if((x<xx[right])==ascnd)
                        {
                            break;
                        }
                        else
                        {
                            left=right;
                            inc+=inc;
                        }
                    }
                }
                else
                {
                    right=left;
                    while(1)
                    {
                        left-=inc;
                        if(left<=0)
                        {
                            left=0;
                            break;
                        }
                        else if((x>=xx[left])==ascnd)
                        {
                            break;
                        }
                        else
                        {
                            right=left;
                            inc+=inc;
                        }
                    }
                }
            }
            while(right-left>1)
            {
                mid=(left+right)/2;
                if((x>=xx[mid])==ascnd)
                {
                    left=mid;
                }
                else
                {
                    right=mid;
                }
            }
            flag=(abs(left-start)>1)?0:1;
            start=left;
            t=max(0,min(N-2,left));
        }
        else
        {
            int left,right,mid;
            bool ascnd=(xx[N-1]>=xx[0]);
            left=0;
            right=n-1;
            while(right-left>1)
            {
                mid=(left+right)/2;
                if((x>=xx[mid])==ascnd)
                {
                    left=mid;
                }
                else
                {
                    right=mid;
                }
            }
            flag=(abs(left-start)>1)?0:1;
            start=left;
            t=max(0,min(N-2,left));
        }
        int left=t;
        int right=t+1;
        double h=xx[right]-xx[left];
        if(h==0.0)
        {
            cerr << "[Error] Interpolation error!" << endl;
            abort();
        }
        double a=(xx[right]-x)/h;
        double b=(x-xx[left])/h;
        double y=a*yy[left]+b*yy[right]+((a*a*a-a)*y2[left]+(b*b*b-b)*y2[right])*(h*h)/6.0;
        fit[n]=y;
    }
    /*
    double xx[N_zeros_real];
    double yy[N_zeros_real];
    for(int n=0;n<N_zeros_real;n++)
    {
        xx[n]=x_orig[n];
        yy[n]=v_orig[n];
    }
    gsl_interp_accel *acc=gsl_interp_accel_alloc();
    gsl_spline *spline=gsl_spline_alloc(gsl_interp_cspline,N_zeros_real);
    gsl_spline_init(spline,xx,yy,N_zeros_real);
    for(int n=0;n<box/2-1;n++)
    {
        fit[n]=gsl_spline_eval(spline,double(x_fft_1d[n]),acc);
    }
    gsl_spline_free(spline);
    gsl_interp_accel_free(acc);
    */
}

void background_subtraction(float *psd,double *background,int box,float *res2_all,float *atan_all,float *x_fft_1d_res,CTF ctf_para)
{
    float phase[box/2-1];
    for(int i=0;i<box/2-1;i++)
    {
        float df=(ctf_para.getDefocus1()+ctf_para.getDefocus2()+(ctf_para.getDefocus1()-ctf_para.getDefocus2())*cos(2*(-ctf_para.getAstigmatism())))/2;
        phase[i]=M_PI*ctf_para.getLambda()*df*(x_fft_1d_res[i]*x_fft_1d_res[i])-M_PI_2*ctf_para.getCs()*(ctf_para.getLambda()*ctf_para.getLambda()*ctf_para.getLambda())*(double(x_fft_1d_res[i]*x_fft_1d_res[i])*double(x_fft_1d_res[i]*x_fft_1d_res[i]))+ctf_para.getW_phase();
    }
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            float df=(ctf_para.getDefocus1()+ctf_para.getDefocus2()+(ctf_para.getDefocus1()-ctf_para.getDefocus2())*cos(2*(atan_all[j*box+i]-ctf_para.getAstigmatism())))/2;
            double chi=M_PI*ctf_para.getLambda()*df*res2_all[j*box+i]-M_PI_2*ctf_para.getCs()*(ctf_para.getLambda()*ctf_para.getLambda()*ctf_para.getLambda())*(double(res2_all[j*box+i])*double(res2_all[j*box+i]))+ctf_para.getW_phase();
            if(chi>=phase[box/2-3])
            {
                psd[j*box+i]-=background[box/2-2];
            }
            else
            {
                int left=0;
                int right=box/2-3;
                while(left+1<right)
                {
                    int mid=(left+right)/2;
                    if(chi>=phase[mid])
                    {
                        left=mid;
                    }
                    else
                    {
                        right=mid;
                    }
                }
                float coeff=(chi-phase[left])/(phase[left+1]-phase[left]);
                psd[j*box+i]=psd[j*box+i]-(1-coeff)*background[left]-coeff*background[left+1];
            }
        }
    }
}


double get_correlation_image_epa_zero_mean(unsigned n,const double *x,double *grad,void *data)
// x: {df_1,df_2,astig}
{
    double df_1=x[0];   // A
    double df_2=x[1];   // A
    double astig=x[2];  // rad
    Data_opt_epa *data_opt=(Data_opt_epa*)data;
    data_opt->ctf_para.setAllCTFPara(df_1,df_2,astig*180.0/M_PI,0,data_opt->ctf_para.getW());  // 特别注意：astig传给CTF的单位是角度，但过来的是弧度！！
    df_1*=1e-10;    // A --> m
    df_2*=1e-10;    // A --> m

    // 二维互相关, 平均功率谱
    float *psd_avg=new float [data_opt->box*data_opt->box];
    get_radial_average_psd(data_opt->psd,psd_avg,data_opt->box,data_opt->ctf_para,data_opt->res2_all,data_opt->atan_all,data_opt->x_fft_1d_res);

    float *psd_ctf=new float[data_opt->box*data_opt->box];
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        psd_ctf[i]=0.0;
    }
    int *mask=new int[data_opt->box*data_opt->box];
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        double df=(df_1+df_2+(df_1-df_2)*cos(2*(data_opt->atan_all[i]-data_opt->ctf_para.getAstigmatism())))/2;
        double chi=M_PI*data_opt->ctf_para.getLambda()*df*data_opt->res2_all[i]-M_PI_2*data_opt->ctf_para.getCs()*(data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda())*(double(data_opt->res2_all[i])*double(data_opt->res2_all[i]))+data_opt->ctf_para.getW_phase();
        if(chi>=data_opt->chi_min && chi<=data_opt->chi_max)
        {
            double ctf=sin(chi);
            psd_ctf[i]=ctf*ctf;
            mask[i]=1;
        }
        else
        {
            mask[i]=0;
        }
    }

    // 零均值化
    float sum_ctf=0.0;
    float sum_psd=0.0;
    int count=0;
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        sum_ctf+=(psd_ctf[i]*float(mask[i]));
        sum_psd+=(psd_avg[i]*float(mask[i]));
        count+=mask[i];
    }
    float avg_ctf=sum_ctf/float(count);
    float avg_psd=sum_psd/float(count);
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        psd_ctf[i]-=avg_ctf;
        psd_avg[i]-=avg_psd;
    }

    double nu=0.0;
    double de_1=0.0;
    double de_2=0.0;
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        nu=nu+(psd_ctf[i]*psd_avg[i])*float(mask[i]);
        de_1=de_1+(psd_ctf[i]*psd_ctf[i])*float(mask[i]);
        de_2=de_2+(psd_avg[i]*psd_avg[i])*float(mask[i]);
    }

    delete [] mask;
    delete [] psd_ctf;
    delete [] psd_avg;
    double cc=nu/sqrt(de_1*de_2);
    return cc;
}

double get_correlation_image_epa_zero_mean_circle(unsigned n,const double *x,double *grad,void *data)
// x: {df_1,df_2,astig}
{
    double df_1=x[0];   // A
    double df_2=x[1];   // A
    double astig=x[2];  // rad
    Data_opt_epa *data_opt=(Data_opt_epa*)data;
    data_opt->ctf_para.setAllCTFPara(df_1,df_2,astig*180.0/M_PI,0,data_opt->ctf_para.getW());  // 特别注意：astig传给CTF的单位是角度，但过来的是弧度！！
    df_1*=1e-10;    // A --> m
    df_2*=1e-10;    // A --> m

    // 二维互相关, 平均功率谱
    float *psd_avg=new float [data_opt->box*data_opt->box];
    get_radial_average_psd(data_opt->psd,psd_avg,data_opt->box,data_opt->ctf_para,data_opt->res2_all,data_opt->atan_all,data_opt->x_fft_1d_res);

    float *psd_ctf=new float[data_opt->box*data_opt->box];
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        psd_ctf[i]=0.0;
    }
    int *mask=new int[data_opt->box*data_opt->box];
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        double df=(df_1+df_2+(df_1-df_2)*cos(2*(data_opt->atan_all[i]-data_opt->ctf_para.getAstigmatism())))/2;
        double chi=M_PI*data_opt->ctf_para.getLambda()*df*data_opt->res2_all[i]-M_PI_2*data_opt->ctf_para.getCs()*(data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda())*(double(data_opt->res2_all[i])*double(data_opt->res2_all[i]))+data_opt->ctf_para.getW_phase();
        double res_now=sqrt(data_opt->res2_all[i]);
        if(res_now>=data_opt->res_min && res_now<=data_opt->res_max)
        {
            double ctf=sin(chi);
            psd_ctf[i]=ctf*ctf;
            mask[i]=1;
        }
        else
        {
            mask[i]=0;
        }
    }

    // 零均值化
    float sum_ctf=0.0;
    float sum_psd=0.0;
    int count=0;
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        sum_ctf+=(psd_ctf[i]*float(mask[i]));
        sum_psd+=(psd_avg[i]*float(mask[i]));
        count+=mask[i];
    }
    float avg_ctf=sum_ctf/float(count);
    float avg_psd=sum_psd/float(count);
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        psd_ctf[i]-=avg_ctf;
        psd_avg[i]-=avg_psd;
    }

    double nu=0.0;
    double de_1=0.0;
    double de_2=0.0;
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        nu=nu+(psd_ctf[i]*psd_avg[i])*float(mask[i]);
        de_1=de_1+(psd_ctf[i]*psd_ctf[i])*float(mask[i]);
        de_2=de_2+(psd_avg[i]*psd_avg[i])*float(mask[i]);
    }

    delete [] mask;
    delete [] psd_ctf;
    delete [] psd_avg;
    double cc=nu/sqrt(de_1*de_2);
    return cc;
}

double get_correlation_image_zero_mean(unsigned n,const double *x,double *grad,void *data)
// x: {df_1,df_2,astig}
{
    double df_1=x[0];   // A
    double df_2=x[1];   // A
    double astig=x[2];  // rad
    Data_opt_epa *data_opt=(Data_opt_epa*)data;
    data_opt->ctf_para.setAllCTFPara(df_1,df_2,astig*180.0/M_PI,0,data_opt->ctf_para.getW());  // 特别注意：astig传给CTF的单位是角度，但过来的是弧度！！
    df_1*=1e-10;    // A --> m
    df_2*=1e-10;    // A --> m

    // 二维互相关, 平均功率谱
    float *psd_avg=new float [data_opt->box*data_opt->box];
    memcpy(psd_avg,data_opt->psd,sizeof(float)*data_opt->box*data_opt->box);
    // get_radial_average_psd(data_opt->psd,psd_avg,data_opt->box,data_opt->ctf_para,data_opt->res2_all,data_opt->atan_all,data_opt->x_fft_1d_res);

    float *psd_ctf=new float[data_opt->box*data_opt->box];
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        psd_ctf[i]=0.0;
    }
    int *mask=new int[data_opt->box*data_opt->box];
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        double df=(df_1+df_2+(df_1-df_2)*cos(2*(data_opt->atan_all[i]-data_opt->ctf_para.getAstigmatism())))/2;
        double chi=M_PI*data_opt->ctf_para.getLambda()*df*data_opt->res2_all[i]-M_PI_2*data_opt->ctf_para.getCs()*(data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda())*(double(data_opt->res2_all[i])*double(data_opt->res2_all[i]))+data_opt->ctf_para.getW_phase();
        if(chi>=data_opt->chi_min && chi<=data_opt->chi_max)
        {
            double ctf=sin(chi);
            psd_ctf[i]=ctf*ctf;
            mask[i]=1;
        }
        else
        {
            mask[i]=0;
        }
    }

    // 零均值化
    float sum_ctf=0.0;
    float sum_psd=0.0;
    int count=0;
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        sum_ctf+=(psd_ctf[i]*float(mask[i]));
        sum_psd+=(psd_avg[i]*float(mask[i]));
        count+=mask[i];
    }
    float avg_ctf=sum_ctf/float(count);
    float avg_psd=sum_psd/float(count);
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        psd_ctf[i]-=avg_ctf;
        psd_avg[i]-=avg_psd;
    }

    double nu=0.0;
    double de_1=0.0;
    double de_2=0.0;
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        nu=nu+(psd_ctf[i]*psd_avg[i])*float(mask[i]);
        de_1=de_1+(psd_ctf[i]*psd_ctf[i])*float(mask[i]);
        de_2=de_2+(psd_avg[i]*psd_avg[i])*float(mask[i]);
    }

    delete [] mask;
    delete [] psd_ctf;
    delete [] psd_avg;
    double cc=nu/sqrt(de_1*de_2);
    return cc;
}

double get_correlation_image_zero_mean_circle(unsigned n,const double *x,double *grad,void *data)
// x: {df_1,df_2,astig}
{
    double df_1=x[0];   // A
    double df_2=x[1];   // A
    double astig=x[2];  // rad
    Data_opt_epa *data_opt=(Data_opt_epa*)data;
    data_opt->ctf_para.setAllCTFPara(df_1,df_2,astig*180.0/M_PI,0,data_opt->ctf_para.getW());  // 特别注意：astig传给CTF的单位是角度，但过来的是弧度！！
    df_1*=1e-10;    // A --> m
    df_2*=1e-10;    // A --> m

    // 二维互相关, 平均功率谱
    float *psd_avg=new float [data_opt->box*data_opt->box];
    memcpy(psd_avg,data_opt->psd,sizeof(float)*data_opt->box*data_opt->box);
    // get_radial_average_psd(data_opt->psd,psd_avg,data_opt->box,data_opt->ctf_para,data_opt->res2_all,data_opt->atan_all,data_opt->x_fft_1d_res);

    float *psd_ctf=new float[data_opt->box*data_opt->box];
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        psd_ctf[i]=0.0;
    }
    int *mask=new int[data_opt->box*data_opt->box];
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        double df=(df_1+df_2+(df_1-df_2)*cos(2*(data_opt->atan_all[i]-data_opt->ctf_para.getAstigmatism())))/2;
        double chi=M_PI*data_opt->ctf_para.getLambda()*df*data_opt->res2_all[i]-M_PI_2*data_opt->ctf_para.getCs()*(data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda())*(double(data_opt->res2_all[i])*double(data_opt->res2_all[i]))+data_opt->ctf_para.getW_phase();
        double res_now=sqrt(data_opt->res2_all[i]);
        if(res_now>=data_opt->res_min && res_now<=data_opt->res_max)
        {
            double ctf=sin(chi);
            psd_ctf[i]=ctf*ctf;
            mask[i]=1;
        }
        else
        {
            mask[i]=0;
        }
    }

    // 零均值化
    float sum_ctf=0.0;
    float sum_psd=0.0;
    int count=0;
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        sum_ctf+=(psd_ctf[i]*float(mask[i]));
        sum_psd+=(psd_avg[i]*float(mask[i]));
        count+=mask[i];
    }
    float avg_ctf=sum_ctf/float(count);
    float avg_psd=sum_psd/float(count);
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        psd_ctf[i]-=avg_ctf;
        psd_avg[i]-=avg_psd;
    }

    double nu=0.0;
    double de_1=0.0;
    double de_2=0.0;
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        nu=nu+(psd_ctf[i]*psd_avg[i])*float(mask[i]);
        de_1=de_1+(psd_ctf[i]*psd_ctf[i])*float(mask[i]);
        de_2=de_2+(psd_avg[i]*psd_avg[i])*float(mask[i]);
    }

    delete [] mask;
    delete [] psd_ctf;
    delete [] psd_avg;
    double cc=nu/sqrt(de_1*de_2);
    return cc;
}

double get_correlation_image_epa_phase_shift_zero_mean(unsigned n,const double *x,double *grad,void *data)
// x: {df_1,df_2,astig}
{
    double df_1=x[0];   // A
    double df_2=x[1];   // A
    double astig=x[2];  // rad
    double ph=x[3]; // rad
    Data_opt_epa *data_opt=(Data_opt_epa*)data;
    data_opt->ctf_para.setAllCTFPara(df_1,df_2,astig*180.0/M_PI,ph,data_opt->ctf_para.getW());  // 特别注意：astig传给CTF的单位是角度，但过来的是弧度！！
    df_1*=1e-10;    // A --> m
    df_2*=1e-10;    // A --> m

    // 二维互相关, 平均功率谱
    float *psd_avg=new float [data_opt->box*data_opt->box];
    get_radial_average_psd_phase_shift(data_opt->psd,psd_avg,data_opt->box,data_opt->ctf_para,data_opt->res2_all,data_opt->atan_all,data_opt->x_fft_1d_res);

    int x_fft[data_opt->box],y_fft[data_opt->box];
    float x_fft_res[data_opt->box],y_fft_res[data_opt->box];
    for(int i=0;i<data_opt->box;i++)
    {
        x_fft[i]=i-data_opt->box/2;
        y_fft[i]=i-data_opt->box/2;
        x_fft_res[i]=float(x_fft[i])/float(data_opt->box)/data_opt->ctf_para.getPixelSize();
        y_fft_res[i]=float(y_fft[i])/float(data_opt->box)/data_opt->ctf_para.getPixelSize();
    }

    float *psd_ctf=new float[data_opt->box*data_opt->box];
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        psd_ctf[i]=0.0;
    }
    for(int j=0;j<data_opt->box;j++)
    {
        for(int i=0;i<data_opt->box;i++)
        {
            double df=(df_1+df_2+(df_1-df_2)*cos(2*(data_opt->atan_all[j*data_opt->box+i]-data_opt->ctf_para.getAstigmatism())))/2;
            // double res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
            double chi=M_PI*data_opt->ctf_para.getLambda()*df*data_opt->res2_all[j*data_opt->box+i]-M_PI_2*data_opt->ctf_para.getCs()*(data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda())*(double(data_opt->res2_all[j*data_opt->box+i])*double(data_opt->res2_all[j*data_opt->box+i]))+data_opt->ctf_para.getW_phase()+data_opt->ctf_para.getPhaseShift();
            if(chi>=data_opt->chi_min && chi<=data_opt->chi_max)
            {
                double ctf=sin(chi);
                psd_ctf[j*data_opt->box+i]=ctf*ctf;
            }
        }
    }

    // 零均值化
    float sum_ctf=0.0;
    float sum_psd=0.0;
    int count=0;
    for(int j=0;j<data_opt->box;j++)
    {
        for(int i=0;i<data_opt->box;i++)
        {
            double df=(df_1+df_2+(df_1-df_2)*cos(2*(data_opt->atan_all[j*data_opt->box+i]-data_opt->ctf_para.getAstigmatism())))/2;
            // double res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
            double chi=M_PI*data_opt->ctf_para.getLambda()*df*data_opt->res2_all[j*data_opt->box+i]-M_PI_2*data_opt->ctf_para.getCs()*(data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda())*(double(data_opt->res2_all[j*data_opt->box+i])*double(data_opt->res2_all[j*data_opt->box+i]))+data_opt->ctf_para.getW_phase()+data_opt->ctf_para.getPhaseShift();
            if(chi>=data_opt->chi_min && chi<=data_opt->chi_max)
            {
                sum_ctf+=psd_ctf[j*data_opt->box+i];
                sum_psd+=psd_avg[j*data_opt->box+i];
                count++;
            }
        }
    }
    float avg_ctf=sum_ctf/float(count);
    float avg_psd=sum_psd/float(count);
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        psd_ctf[i]-=avg_ctf;
        psd_avg[i]-=avg_psd;
    }

    double nu=0.0;
    double de_1=0.0;
    double de_2=0.0;

    for(int j=0;j<data_opt->box;j++)
    {
        for(int i=0;i<data_opt->box;i++)
        {
            double df=(df_1+df_2+(df_1-df_2)*cos(2*(data_opt->atan_all[j*data_opt->box+i]-data_opt->ctf_para.getAstigmatism())))/2;
            // double res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
            double chi=M_PI*data_opt->ctf_para.getLambda()*df*data_opt->res2_all[j*data_opt->box+i]-M_PI_2*data_opt->ctf_para.getCs()*(data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda())*(double(data_opt->res2_all[j*data_opt->box+i])*double(data_opt->res2_all[j*data_opt->box+i]))+data_opt->ctf_para.getW_phase()+data_opt->ctf_para.getPhaseShift();
            if(chi>=data_opt->chi_min && chi<=data_opt->chi_max)
            {
                nu=nu+(psd_ctf[j*data_opt->box+i]*psd_avg[j*data_opt->box+i]);
                de_1=de_1+psd_ctf[j*data_opt->box+i]*psd_ctf[j*data_opt->box+i];
                de_2=de_2+psd_avg[j*data_opt->box+i]*psd_avg[j*data_opt->box+i];
            }
        }
    }
    delete [] psd_ctf;
    delete [] psd_avg;
    double cc=nu/sqrt(de_1*de_2);
    return cc;
}

double get_correlation_image_phase_shift_zero_mean(unsigned n,const double *x,double *grad,void *data)
// x: {df_1,df_2,astig}
{
    double df_1=x[0];   // A
    double df_2=x[1];   // A
    double astig=x[2];  // rad
    double ph=x[3]; // rad
    Data_opt_epa *data_opt=(Data_opt_epa*)data;
    data_opt->ctf_para.setAllCTFPara(df_1,df_2,astig*180.0/M_PI,ph,data_opt->ctf_para.getW());  // 特别注意：astig传给CTF的单位是角度，但过来的是弧度！！
    df_1*=1e-10;    // A --> m
    df_2*=1e-10;    // A --> m

    // 二维互相关, 平均功率谱
    float *psd_avg=new float [data_opt->box*data_opt->box];
    // get_radial_average_psd_phase_shift(data_opt->psd,psd_avg,data_opt->box,data_opt->ctf_para,data_opt->res2_all,data_opt->atan_all,data_opt->x_fft_1d_res);
    memcpy(psd_avg,data_opt->psd,sizeof(float)*data_opt->box*data_opt->box);

    int x_fft[data_opt->box],y_fft[data_opt->box];
    float x_fft_res[data_opt->box],y_fft_res[data_opt->box];
    for(int i=0;i<data_opt->box;i++)
    {
        x_fft[i]=i-data_opt->box/2;
        y_fft[i]=i-data_opt->box/2;
        x_fft_res[i]=float(x_fft[i])/float(data_opt->box)/data_opt->ctf_para.getPixelSize();
        y_fft_res[i]=float(y_fft[i])/float(data_opt->box)/data_opt->ctf_para.getPixelSize();
    }

    float *psd_ctf=new float[data_opt->box*data_opt->box];
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        psd_ctf[i]=0.0;
    }
    for(int j=0;j<data_opt->box;j++)
    {
        for(int i=0;i<data_opt->box;i++)
        {
            double df=(df_1+df_2+(df_1-df_2)*cos(2*(data_opt->atan_all[j*data_opt->box+i]-data_opt->ctf_para.getAstigmatism())))/2;
            // double res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
            double chi=M_PI*data_opt->ctf_para.getLambda()*df*data_opt->res2_all[j*data_opt->box+i]-M_PI_2*data_opt->ctf_para.getCs()*(data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda())*(double(data_opt->res2_all[j*data_opt->box+i])*double(data_opt->res2_all[j*data_opt->box+i]))+data_opt->ctf_para.getW_phase()+data_opt->ctf_para.getPhaseShift();
            if(chi>=data_opt->chi_min && chi<=data_opt->chi_max)
            {
                double ctf=sin(chi);
                psd_ctf[j*data_opt->box+i]=ctf*ctf;
            }
        }
    }

    // 零均值化
    float sum_ctf=0.0;
    float sum_psd=0.0;
    int count=0;
    for(int j=0;j<data_opt->box;j++)
    {
        for(int i=0;i<data_opt->box;i++)
        {
            double df=(df_1+df_2+(df_1-df_2)*cos(2*(data_opt->atan_all[j*data_opt->box+i]-data_opt->ctf_para.getAstigmatism())))/2;
            // double res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
            double chi=M_PI*data_opt->ctf_para.getLambda()*df*data_opt->res2_all[j*data_opt->box+i]-M_PI_2*data_opt->ctf_para.getCs()*(data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda())*(double(data_opt->res2_all[j*data_opt->box+i])*double(data_opt->res2_all[j*data_opt->box+i]))+data_opt->ctf_para.getW_phase()+data_opt->ctf_para.getPhaseShift();
            if(chi>=data_opt->chi_min && chi<=data_opt->chi_max)
            {
                sum_ctf+=psd_ctf[j*data_opt->box+i];
                sum_psd+=psd_avg[j*data_opt->box+i];
                count++;
            }
        }
    }
    float avg_ctf=sum_ctf/float(count);
    float avg_psd=sum_psd/float(count);
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        psd_ctf[i]-=avg_ctf;
        psd_avg[i]-=avg_psd;
    }

    double nu=0.0;
    double de_1=0.0;
    double de_2=0.0;

    for(int j=0;j<data_opt->box;j++)
    {
        for(int i=0;i<data_opt->box;i++)
        {
            double df=(df_1+df_2+(df_1-df_2)*cos(2*(data_opt->atan_all[j*data_opt->box+i]-data_opt->ctf_para.getAstigmatism())))/2;
            // double res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
            double chi=M_PI*data_opt->ctf_para.getLambda()*df*data_opt->res2_all[j*data_opt->box+i]-M_PI_2*data_opt->ctf_para.getCs()*(data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda())*(double(data_opt->res2_all[j*data_opt->box+i])*double(data_opt->res2_all[j*data_opt->box+i]))+data_opt->ctf_para.getW_phase()+data_opt->ctf_para.getPhaseShift();
            if(chi>=data_opt->chi_min && chi<=data_opt->chi_max)
            {
                nu=nu+(psd_ctf[j*data_opt->box+i]*psd_avg[j*data_opt->box+i]);
                de_1=de_1+psd_ctf[j*data_opt->box+i]*psd_ctf[j*data_opt->box+i];
                de_2=de_2+psd_avg[j*data_opt->box+i]*psd_avg[j*data_opt->box+i];
            }
        }
    }
    delete [] psd_ctf;
    delete [] psd_avg;
    double cc=nu/sqrt(de_1*de_2);
    return cc;
}

double get_correlation_astig_epa_zero_mean(unsigned n,const double *x,double *grad,void *data)
// x: {df_1,df_2}
{
    double df_1=x[0];   // A
    double df_2=x[1];   // A
    Data_opt_epa *data_opt=(Data_opt_epa*)data;
    data_opt->ctf_para.setAllCTFPara(df_1,df_2,data_opt->ctf_para.getAstigmatism()*180.0/M_PI,0,data_opt->ctf_para.getW()); // 特别注意：astig传给CTF的单位是角度，但过来的是弧度！！
    df_1*=1e-10;    // A --> m
    df_2*=1e-10;    // A --> m

    // 二维互相关, 平均功率谱
    float *psd_avg=new float[data_opt->box*data_opt->box];
    get_radial_average_psd(data_opt->psd,psd_avg,data_opt->box,data_opt->ctf_para,data_opt->res2_all,data_opt->atan_all,data_opt->x_fft_1d_res);
    // memcpy(psd_avg,data_opt->psd,sizeof(float)*data_opt->box*data_opt->box);

    float *psd_ctf=new float[data_opt->box*data_opt->box];
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        psd_ctf[i]=0.0;
    }
    int *mask=new int[data_opt->box*data_opt->box];
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        double df=(df_1+df_2+(df_1-df_2)*cos(2*(data_opt->atan_all[i]-data_opt->ctf_para.getAstigmatism())))/2;
        double chi=M_PI*data_opt->ctf_para.getLambda()*df*data_opt->res2_all[i]-M_PI_2*data_opt->ctf_para.getCs()*(data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda())*(double(data_opt->res2_all[i])*double(data_opt->res2_all[i]))+data_opt->ctf_para.getW_phase();
        if(chi>=data_opt->chi_min && chi<=data_opt->chi_max)
        {
            double ctf=sin(chi);
            psd_ctf[i]=ctf*ctf;
            mask[i]=1;
        }
        else
        {
            mask[i]=0;
        }
    }

    // 零均值化
    float sum_ctf=0.0;
    float sum_psd=0.0;
    int count=0;
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        sum_ctf+=(psd_ctf[i]*float(mask[i]));
        sum_psd+=(psd_avg[i]*float(mask[i]));
        count+=mask[i];
    }
    float avg_ctf=sum_ctf/float(count);
    float avg_psd=sum_psd/float(count);
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        psd_ctf[i]-=avg_ctf;
        psd_avg[i]-=avg_psd;
    }

    double nu=0.0;
    double de_1=0.0;
    double de_2=0.0;
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        nu=nu+(psd_ctf[i]*psd_avg[i])*float(mask[i]);
        de_1=de_1+(psd_ctf[i]*psd_ctf[i])*float(mask[i]);
        de_2=de_2+(psd_avg[i]*psd_avg[i])*float(mask[i]);
    }

    delete [] mask;
    delete [] psd_ctf;
    delete [] psd_avg;
    double cc=nu/sqrt(de_1*de_2);
    return cc;
}

double get_correlation_astig_epa_zero_mean_circle(unsigned n,const double *x,double *grad,void *data)
// x: {df_1,df_2}
{
    double df_1=x[0];   // A
    double df_2=x[1];   // A
    Data_opt_epa *data_opt=(Data_opt_epa*)data;
    data_opt->ctf_para.setAllCTFPara(df_1,df_2,data_opt->ctf_para.getAstigmatism()*180.0/M_PI,0,data_opt->ctf_para.getW()); // 特别注意：astig传给CTF的单位是角度，但过来的是弧度！！
    df_1*=1e-10;    // A --> m
    df_2*=1e-10;    // A --> m

    // 二维互相关, 平均功率谱
    float *psd_avg=new float[data_opt->box*data_opt->box];
    get_radial_average_psd(data_opt->psd,psd_avg,data_opt->box,data_opt->ctf_para,data_opt->res2_all,data_opt->atan_all,data_opt->x_fft_1d_res);
    // memcpy(psd_avg,data_opt->psd,sizeof(float)*data_opt->box*data_opt->box);

    float *psd_ctf=new float[data_opt->box*data_opt->box];
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        psd_ctf[i]=0.0;
    }
    int *mask=new int[data_opt->box*data_opt->box];
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        double df=(df_1+df_2+(df_1-df_2)*cos(2*(data_opt->atan_all[i]-data_opt->ctf_para.getAstigmatism())))/2;
        double chi=M_PI*data_opt->ctf_para.getLambda()*df*data_opt->res2_all[i]-M_PI_2*data_opt->ctf_para.getCs()*(data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda())*(double(data_opt->res2_all[i])*double(data_opt->res2_all[i]))+data_opt->ctf_para.getW_phase();
        double res_now=sqrt(data_opt->res2_all[i]);
        if(res_now>=data_opt->res_min && res_now<=data_opt->res_max)
        {
            double ctf=sin(chi);
            psd_ctf[i]=ctf*ctf;
            mask[i]=1;
        }
        else
        {
            mask[i]=0;
        }
    }

    // 零均值化
    float sum_ctf=0.0;
    float sum_psd=0.0;
    int count=0;
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        sum_ctf+=(psd_ctf[i]*float(mask[i]));
        sum_psd+=(psd_avg[i]*float(mask[i]));
        count+=mask[i];
    }
    float avg_ctf=sum_ctf/float(count);
    float avg_psd=sum_psd/float(count);
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        psd_ctf[i]-=avg_ctf;
        psd_avg[i]-=avg_psd;
    }

    double nu=0.0;
    double de_1=0.0;
    double de_2=0.0;
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        nu=nu+(psd_ctf[i]*psd_avg[i])*float(mask[i]);
        de_1=de_1+(psd_ctf[i]*psd_ctf[i])*float(mask[i]);
        de_2=de_2+(psd_avg[i]*psd_avg[i])*float(mask[i]);
    }

    delete [] mask;
    delete [] psd_ctf;
    delete [] psd_avg;
    double cc=nu/sqrt(de_1*de_2);
    return cc;
}

double get_correlation_astig_epa_phase_shift_zero_mean(unsigned n,const double *x,double *grad,void *data)
// x: {df_1,df_2}
{
    double df_1=x[0];   // A
    double df_2=x[1];   // A
    Data_opt_epa *data_opt=(Data_opt_epa*)data;
    data_opt->ctf_para.setAllCTFPara(df_1,df_2,data_opt->ctf_para.getAstigmatism()*180.0/M_PI,data_opt->ctf_para.getPhaseShift(),data_opt->ctf_para.getW()); // 特别注意：astig传给CTF的单位是角度，但过来的是弧度！！
    df_1*=1e-10;    // A --> m
    df_2*=1e-10;    // A --> m

    // 二维互相关, 平均功率谱
    float *psd_avg=new float[data_opt->box*data_opt->box];
    get_radial_average_psd_phase_shift(data_opt->psd,psd_avg,data_opt->box,data_opt->ctf_para,data_opt->res2_all,data_opt->atan_all,data_opt->x_fft_1d_res);
    // memcpy(psd_avg,data_opt->psd,sizeof(float)*data_opt->box*data_opt->box);

    int x_fft[data_opt->box],y_fft[data_opt->box];
    float x_fft_res[data_opt->box],y_fft_res[data_opt->box];
    for(int i=0;i<data_opt->box;i++)
    {
        x_fft[i]=i-data_opt->box/2;
        y_fft[i]=i-data_opt->box/2;
        x_fft_res[i]=float(x_fft[i])/float(data_opt->box)/data_opt->ctf_para.getPixelSize();
        y_fft_res[i]=float(y_fft[i])/float(data_opt->box)/data_opt->ctf_para.getPixelSize();
    }

    float *psd_ctf=new float[data_opt->box*data_opt->box];
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        psd_ctf[i]=0.0;
    }
    for(int j=0;j<data_opt->box;j++)
    {
        for(int i=0;i<data_opt->box;i++)
        {
            double df=(df_1+df_2+(df_1-df_2)*cos(2*(data_opt->atan_all[j*data_opt->box+i]-data_opt->ctf_para.getAstigmatism())))/2;
            // double res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
            double chi=M_PI*data_opt->ctf_para.getLambda()*df*data_opt->res2_all[j*data_opt->box+i]-M_PI_2*data_opt->ctf_para.getCs()*(data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda())*(double(data_opt->res2_all[j*data_opt->box+i])*double(data_opt->res2_all[j*data_opt->box+i]))+data_opt->ctf_para.getW_phase()+data_opt->ctf_para.getPhaseShift();
            if(chi>=data_opt->chi_min && chi<=data_opt->chi_max)
            {
                double ctf=sin(chi);
                psd_ctf[j*data_opt->box+i]=ctf*ctf;
            }
        }
    }

    // 零均值化
    float sum_ctf=0.0;
    float sum_psd=0.0;
    int count=0;
    for(int j=0;j<data_opt->box;j++)
    {
        for(int i=0;i<data_opt->box;i++)
        {
            double df=(df_1+df_2+(df_1-df_2)*cos(2*(data_opt->atan_all[j*data_opt->box+i]-data_opt->ctf_para.getAstigmatism())))/2;
            // double res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
            double chi=M_PI*data_opt->ctf_para.getLambda()*df*data_opt->res2_all[j*data_opt->box+i]-M_PI_2*data_opt->ctf_para.getCs()*(data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda())*(double(data_opt->res2_all[j*data_opt->box+i])*double(data_opt->res2_all[j*data_opt->box+i]))+data_opt->ctf_para.getW_phase()+data_opt->ctf_para.getPhaseShift();
            if(chi>=data_opt->chi_min && chi<=data_opt->chi_max)
            {
                sum_ctf+=psd_ctf[j*data_opt->box+i];
                sum_psd+=psd_avg[j*data_opt->box+i];
                count++;
            }
        }
    }
    float avg_ctf=sum_ctf/float(count);
    float avg_psd=sum_psd/float(count);
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        psd_ctf[i]-=avg_ctf;
        psd_avg[i]-=avg_psd;
    }

    double nu=0.0;
    double de_1=0.0;
    double de_2=0.0;

    for(int j=0;j<data_opt->box;j++)
    {
        for(int i=0;i<data_opt->box;i++)
        {
            double df=(df_1+df_2+(df_1-df_2)*cos(2*(data_opt->atan_all[j*data_opt->box+i]-data_opt->ctf_para.getAstigmatism())))/2;
            // double res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
            double chi=M_PI*data_opt->ctf_para.getLambda()*df*data_opt->res2_all[j*data_opt->box+i]-M_PI_2*data_opt->ctf_para.getCs()*(data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda())*(double(data_opt->res2_all[j*data_opt->box+i])*double(data_opt->res2_all[j*data_opt->box+i]))+data_opt->ctf_para.getW_phase()+data_opt->ctf_para.getPhaseShift();
            if(chi>=data_opt->chi_min && chi<=data_opt->chi_max)
            {
                nu=nu+(psd_ctf[j*data_opt->box+i]*psd_avg[j*data_opt->box+i]);
                de_1=de_1+psd_ctf[j*data_opt->box+i]*psd_ctf[j*data_opt->box+i];
                de_2=de_2+psd_avg[j*data_opt->box+i]*psd_avg[j*data_opt->box+i];
            }
        }
    }
    delete [] psd_ctf;
    delete [] psd_avg;
    double cc=nu/sqrt(de_1*de_2);
    return cc;
}

double get_correlation_astig_phase_shift_zero_mean(unsigned n,const double *x,double *grad,void *data)
// x: {df_1,df_2}
{
    double df_1=x[0];   // A
    double df_2=x[1];   // A
    Data_opt_epa *data_opt=(Data_opt_epa*)data;
    data_opt->ctf_para.setAllCTFPara(df_1,df_2,data_opt->ctf_para.getAstigmatism()*180.0/M_PI,data_opt->ctf_para.getPhaseShift(),data_opt->ctf_para.getW()); // 特别注意：astig传给CTF的单位是角度，但过来的是弧度！！
    df_1*=1e-10;    // A --> m
    df_2*=1e-10;    // A --> m

    // 二维互相关, 平均功率谱
    float *psd_avg=new float[data_opt->box*data_opt->box];
    // get_radial_average_psd_phase_shift(data_opt->psd,psd_avg,data_opt->box,data_opt->ctf_para,data_opt->res2_all,data_opt->atan_all,data_opt->x_fft_1d_res);
    memcpy(psd_avg,data_opt->psd,sizeof(float)*data_opt->box*data_opt->box);

    int x_fft[data_opt->box],y_fft[data_opt->box];
    float x_fft_res[data_opt->box],y_fft_res[data_opt->box];
    for(int i=0;i<data_opt->box;i++)
    {
        x_fft[i]=i-data_opt->box/2;
        y_fft[i]=i-data_opt->box/2;
        x_fft_res[i]=float(x_fft[i])/float(data_opt->box)/data_opt->ctf_para.getPixelSize();
        y_fft_res[i]=float(y_fft[i])/float(data_opt->box)/data_opt->ctf_para.getPixelSize();
    }

    float *psd_ctf=new float[data_opt->box*data_opt->box];
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        psd_ctf[i]=0.0;
    }
    for(int j=0;j<data_opt->box;j++)
    {
        for(int i=0;i<data_opt->box;i++)
        {
            double df=(df_1+df_2+(df_1-df_2)*cos(2*(data_opt->atan_all[j*data_opt->box+i]-data_opt->ctf_para.getAstigmatism())))/2;
            // double res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
            double chi=M_PI*data_opt->ctf_para.getLambda()*df*data_opt->res2_all[j*data_opt->box+i]-M_PI_2*data_opt->ctf_para.getCs()*(data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda())*(double(data_opt->res2_all[j*data_opt->box+i])*double(data_opt->res2_all[j*data_opt->box+i]))+data_opt->ctf_para.getW_phase()+data_opt->ctf_para.getPhaseShift();
            if(chi>=data_opt->chi_min && chi<=data_opt->chi_max)
            {
                double ctf=sin(chi);
                psd_ctf[j*data_opt->box+i]=ctf*ctf;
            }
        }
    }

    // 零均值化
    float sum_ctf=0.0;
    float sum_psd=0.0;
    int count=0;
    for(int j=0;j<data_opt->box;j++)
    {
        for(int i=0;i<data_opt->box;i++)
        {
            double df=(df_1+df_2+(df_1-df_2)*cos(2*(data_opt->atan_all[j*data_opt->box+i]-data_opt->ctf_para.getAstigmatism())))/2;
            // double res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
            double chi=M_PI*data_opt->ctf_para.getLambda()*df*data_opt->res2_all[j*data_opt->box+i]-M_PI_2*data_opt->ctf_para.getCs()*(data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda())*(double(data_opt->res2_all[j*data_opt->box+i])*double(data_opt->res2_all[j*data_opt->box+i]))+data_opt->ctf_para.getW_phase()+data_opt->ctf_para.getPhaseShift();
            if(chi>=data_opt->chi_min && chi<=data_opt->chi_max)
            {
                sum_ctf+=psd_ctf[j*data_opt->box+i];
                sum_psd+=psd_avg[j*data_opt->box+i];
                count++;
            }
        }
    }
    float avg_ctf=sum_ctf/float(count);
    float avg_psd=sum_psd/float(count);
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        psd_ctf[i]-=avg_ctf;
        psd_avg[i]-=avg_psd;
    }

    double nu=0.0;
    double de_1=0.0;
    double de_2=0.0;

    for(int j=0;j<data_opt->box;j++)
    {
        for(int i=0;i<data_opt->box;i++)
        {
            double df=(df_1+df_2+(df_1-df_2)*cos(2*(data_opt->atan_all[j*data_opt->box+i]-data_opt->ctf_para.getAstigmatism())))/2;
            // double res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
            double chi=M_PI*data_opt->ctf_para.getLambda()*df*data_opt->res2_all[j*data_opt->box+i]-M_PI_2*data_opt->ctf_para.getCs()*(data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda())*(double(data_opt->res2_all[j*data_opt->box+i])*double(data_opt->res2_all[j*data_opt->box+i]))+data_opt->ctf_para.getW_phase()+data_opt->ctf_para.getPhaseShift();
            if(chi>=data_opt->chi_min && chi<=data_opt->chi_max)
            {
                nu=nu+(psd_ctf[j*data_opt->box+i]*psd_avg[j*data_opt->box+i]);
                de_1=de_1+psd_ctf[j*data_opt->box+i]*psd_ctf[j*data_opt->box+i];
                de_2=de_2+psd_avg[j*data_opt->box+i]*psd_avg[j*data_opt->box+i];
            }
        }
    }
    delete [] psd_ctf;
    delete [] psd_avg;
    double cc=nu/sqrt(de_1*de_2);
    return cc;
}

double get_correlation_astig_dz_epa_zero_mean(unsigned n,const double *x,double *grad,void *data)
// x: {dz}
{
    double dz=x[0];
    Data_opt_dz_epa *data_opt=(Data_opt_dz_epa*)data;
    float df_1=data_opt->ctf_para.getDefocus1()*1e10+float(dz);
    float df_2=data_opt->ctf_para.getDefocus2()*1e10+float(dz);
    CTF ctf_para_now=data_opt->ctf_para;
    ctf_para_now.setAllCTFPara(df_1,df_2,data_opt->ctf_para.getAstigmatism()*180.0/M_PI,0,data_opt->ctf_para.getW()); 
    df_1*=1e-10;
    df_2*=1e-10;

    // 二维互相关, 平均功率谱
    float *psd_avg=new float [data_opt->box*data_opt->box];
    get_radial_average_psd(data_opt->psd,psd_avg,data_opt->box,ctf_para_now,data_opt->res2_all,data_opt->atan_all,data_opt->x_fft_1d_res);
    // memcpy(psd_avg,data_opt->psd,sizeof(float)*data_opt->box*data_opt->box);

    float *psd_ctf=new float[data_opt->box*data_opt->box];
    // float *chi_all=new float[data_opt->box*data_opt->box];
    int *mask=new int[data_opt->box*data_opt->box];
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        psd_ctf[i]=0.0;
        // chi_all[i]=0.0;
    }

    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        float chi=data_opt->chi_all[i]+M_PI*data_opt->ctf_para.getLambda()*(float(dz)*1e-10)*data_opt->res2_all[i];
        if(chi>=data_opt->chi_min && chi<=data_opt->chi_max)
        {
            mask[i]=1;
            float ctf=sin(chi);
            psd_ctf[i]=ctf*ctf;
        }
        else
        {
            mask[i]=0;
        }
    }
    
    /*
    for(int j=0;j<data_opt->box;j++)
    {
        for(int i=0;i<data_opt->box;i++)
        {
            float df=(df_1+df_2+(df_1-df_2)*cos(2*(atan2(y_fft_res[j],x_fft_res[i])-data_opt->ctf_para.getAstigmatism())))/2;
            float res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
            float chi=M_PI*data_opt->ctf_para.getLambda()*df*res2-M_PI_2*data_opt->ctf_para.getCs()*(data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda())*double(res2)*double(res2)+atan(data_opt->ctf_para.getW()/sqrt(1-data_opt->ctf_para.getW()*data_opt->ctf_para.getW()));
            // float chi=data_opt->chi_all[j*data_opt->box+i]+M_PI*data_opt->ctf_para.getLambda()*(float(dz)*1e-10)*data_opt->res2_all[j*data_opt->box+i];
            // float chi=M_PI*data_opt->ctf_para.getLambda()*(df-float(dz)*1e-10)*data_opt->res2_all[j*data_opt->box+i]-M_PI_2*data_opt->ctf_para.getCs()*(data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda()*data_opt->ctf_para.getLambda())*double(data_opt->res2_all[j*data_opt->box+i])*double(data_opt->res2_all[j*data_opt->box+i])+atan(data_opt->ctf_para.getW()/sqrt(1-data_opt->ctf_para.getW()*data_opt->ctf_para.getW()))+M_PI*data_opt->ctf_para.getLambda()*(float(dz)*1e-10)*data_opt->res2_all[j*data_opt->box+i];
            // chi_all[j*data_opt->box+i]=chi;
            if(chi>=data_opt->chi_min && chi<=data_opt->chi_max)
            {
                mask[j*data_opt->box+i]=1;
                float ctf=sin(chi);
                psd_ctf[j*data_opt->box+i]=ctf*ctf;
            }
            else
            {
                mask[j*data_opt->box+i]=0;
            }
        }
    }
    */

    // 零均值化
    double sum_ctf=0.0;
    double sum_psd=0.0;
    int count=0;
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        // if(chi_all[i]>=data_opt->chi_min && chi_all[i]<=data_opt->chi_max)
        // {
        //     sum_ctf+=psd_ctf[i];
        //     sum_psd+=psd_avg[i];
        //     count++;
        // }
        sum_ctf+=(psd_ctf[i]*float(mask[i]));
        sum_psd+=(psd_avg[i]*float(mask[i]));
        count+=mask[i];
    }
    float avg_ctf=float(sum_ctf/double(count));
    float avg_psd=float(sum_psd/double(count));
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        psd_ctf[i]-=avg_ctf;
        psd_avg[i]-=avg_psd;
    }

    double nu=0.0;
    double de_1=0.0;
    double de_2=0.0;
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        // if(chi_all[i]>=data_opt->chi_min && chi_all[i]<=data_opt->chi_max)
        // {
        //     nu=nu+(psd_ctf[i]*psd_avg[i]);
        //     de_1=de_1+psd_ctf[i]*psd_ctf[i];
        //     de_2=de_2+psd_avg[i]*psd_avg[i];
        // }
        nu=nu+(psd_ctf[i]*psd_avg[i])*float(mask[i]);
        de_1=de_1+(psd_ctf[i]*psd_ctf[i])*float(mask[i]);
        de_2=de_2+(psd_avg[i]*psd_avg[i])*float(mask[i]);
    }

    delete [] mask;
    // delete [] chi_all;
    delete [] psd_ctf;
    delete [] psd_avg;
    double cc=nu/sqrt(de_1*de_2);
    return cc;
}

float get_correlation_adaptive_zero_mean(float df,int box,int N_zeros,CTF ctf_para,float res_min,float res_max,float *psd_orig,float *res2_all,float *atan_all,float *x_fft_1d_res)
{
    float *psd=new float[box*box];
    memcpy(psd,psd_orig,sizeof(float)*box*box);

    ctf_para.setAllCTFPara(df,df,0,0,ctf_para.getW()); 
    df*=1e-10;    // A --> m

    float radial[box/2-1];
    get_radial_average(psd,radial,box,ctf_para,res2_all,atan_all,x_fft_1d_res);

    // 一维平均求互相关
    float radial_ctf[box/2-1];
    for(int i=0;i<box/2-1;i++)
    {
        if(x_fft_1d_res[i]>=res_min && x_fft_1d_res[i]<=res_max)
        {
            double chi=M_PI*ctf_para.getLambda()*df*(x_fft_1d_res[i]*x_fft_1d_res[i])-M_PI_2*ctf_para.getCs()*(ctf_para.getLambda()*ctf_para.getLambda()*ctf_para.getLambda())*(double(x_fft_1d_res[i]*x_fft_1d_res[i])*double(x_fft_1d_res[i]*x_fft_1d_res[i]));
            double ctf=ctf_para.getW()*cos(chi)+sqrt(1-ctf_para.getW()*ctf_para.getW())*sin(chi);
            radial_ctf[i]=ctf*ctf;
        }
    }

    float sum_ctf=0.0;
    float sum_psd=0.0;
    int count=0;
    for(int i=0;i<box/2-1;i++)
    {
        if(x_fft_1d_res[i]>=res_min && x_fft_1d_res[i]<=res_max)
        {
            sum_ctf+=radial_ctf[i];
            sum_psd+=radial[i];
            count++;
        }
    }

    float avg_ctf=sum_ctf/float(count);
    float avg_psd=sum_psd/float(count);
    for(int i=0;i<box/2-1;i++)
    {
        if(x_fft_1d_res[i]>=res_min && x_fft_1d_res[i]<=res_max)
        {
            radial_ctf[i]-=avg_ctf;
            radial[i]-=avg_psd;
        }
    }

    double nu=0.0;
    double de_1=0.0;
    double de_2=0.0;
    for(int i=0;i<box/2-1;i++)
    {
        if(x_fft_1d_res[i]>=res_min && x_fft_1d_res[i]<=res_max)
        {
            nu=nu+(radial_ctf[i]*radial[i]);
            de_1=de_1+radial_ctf[i]*radial_ctf[i];
            de_2=de_2+radial[i]*radial[i];
        }
    }

    delete [] psd;
    double cc=nu/sqrt(de_1*de_2);
    return cc;
}

float get_correlation_adaptive_phase_shift_zero_mean(float df,int box,int N_zeros,CTF ctf_para,float res_min,float res_max,float *psd_orig,float ph,float *x_fft_1d_res)
{
    float *psd=new float[box*box];
    memcpy(psd,psd_orig,sizeof(float)*box*box);

    ctf_para.setAllCTFPara(df,df,0,ph,ctf_para.getW()); 
    df*=1e-10;    // A --> m

    float radial[box/2-1];
    get_radial_average_phase_shift(psd,radial,box,ctf_para);

    // 一维平均求互相关
    float radial_ctf[box/2-1];
    for(int i=0;i<box/2-1;i++)
    {
        if(x_fft_1d_res[i]>=res_min && x_fft_1d_res[i]<=res_max)
        {
            double u2=(x_fft_1d_res[i]*x_fft_1d_res[i]);
            double u4=u2*u2;
            double chi=M_PI*ctf_para.getLambda()*df*(x_fft_1d_res[i]*x_fft_1d_res[i])-M_PI_2*ctf_para.getCs()*(ctf_para.getLambda()*ctf_para.getLambda()*ctf_para.getLambda())*(u4)+ctf_para.getPhaseShift();
            double ctf=ctf_para.getW()*cos(chi)+sqrt(1-ctf_para.getW()*ctf_para.getW())*sin(chi);
            radial_ctf[i]=ctf*ctf;
        }
    }

    float sum_ctf=0.0;
    float sum_psd=0.0;
    int count=0;
    for(int i=0;i<box/2-1;i++)
    {
        if(x_fft_1d_res[i]>=res_min && x_fft_1d_res[i]<=res_max)
        {
            sum_ctf+=radial_ctf[i];
            sum_psd+=radial[i];
            count++;
        }
    }

    float avg_ctf=sum_ctf/float(count);
    float avg_psd=sum_psd/float(count);
    for(int i=0;i<box/2-1;i++)
    {
        if(x_fft_1d_res[i]>=res_min && x_fft_1d_res[i]<=res_max)
        {
            radial_ctf[i]-=avg_ctf;
            radial[i]-=avg_psd;
        }
    }

    double nu=0.0;
    double de_1=0.0;
    double de_2=0.0;
    for(int i=0;i<box/2-1;i++)
    {
        if(x_fft_1d_res[i]>=res_min && x_fft_1d_res[i]<=res_max)
        {
            nu=nu+(radial_ctf[i]*radial[i]);
            de_1=de_1+radial_ctf[i]*radial_ctf[i];
            de_2=de_2+radial[i]*radial[i];
        }
    }

    delete [] psd;
    double cc=nu/sqrt(de_1*de_2);
    return cc;
}

void get_psd_fit_show(float *psd_avg,float *psd_show,int box,CTF ctf_para_avg,float res_min,float res_max,float *res2_all,float *atan_all,float *x_fft_1d_res)
{
    /*
    memcpy(psd_show,psd_avg,sizeof(float)*box*box);
    float psd_min=psd_show[0];
    for(int i=1;i<box*box;i++)
    {
        if(psd_min>psd_show[i])
        {
            psd_min=psd_show[i];
        }
    }
    for(int i=0;i<box*box;i++)
    {
        psd_show[i]-=psd_min;
    }
    float psd_max=psd_show[0];
    for(int i=1;i<box*box;i++)
    {
        if(psd_max<psd_show[i])
        {
            psd_max=psd_show[i];
        }
    }
    for(int i=0;i<box*box;i++)
    {
        psd_show[i]/=psd_max;
        psd_show[i]*=255.0;
    }
    int hist[256];  // 通过统计直方图，找到显示的灰度中心
    for(int i=0;i<256;i++)
    {
        hist[i]=0;
    }
    for(int i=0;i<box*box;i++)
    {
        hist[int(floor(psd_show[i]))]++;
    }
    int hist_max=0;
    int hist_max_ind=0;
    for(int i=0;i<256;i++)
    {
        if(hist_max<hist[i])
        {
            hist_max=hist[i];
            hist_max_ind=i;
        }
    }
    int counted=hist[hist_max_ind];
    int interval=0;
    while(float(counted)/float(box*box)<0.5)
    {
        interval++;
        counted+=hist[hist_max_ind+interval];
        counted+=hist[hist_max_ind-interval];
    }
    for(int i=0;i<box*box;i++)
    {
        if(psd_show[i]<float(hist_max_ind-interval))
        {
            psd_show[i]=float(hist_max_ind-interval);
        }
        else if(psd_show[i]>float(hist_max_ind+interval+1))
        {
            psd_show[i]=float(hist_max_ind+interval+1);
        }
        psd_show[i]=255.0*(psd_show[i]-float(hist_max_ind-interval))/float(hist_max_ind+interval+1);
    }
    */

    memcpy(psd_show,psd_avg,sizeof(float)*box*box);
    float *psd_avg_radial=new float[box*box];
    get_radial_average_psd(psd_avg,psd_avg_radial,box,ctf_para_avg,res2_all,atan_all,x_fft_1d_res);
    for(int j=0;j<=box/2-1;j++)
    {
        for(int i=0;i<=box/2-1;i++)
        {
            psd_show[j*box+i]=psd_avg_radial[j*box+i];
        }
    }
    double psd_sum=0.0;
    double psd_sum2=0.0;
    int psd_count=0;
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            if(sqrt(res2_all[j*box+i])>=res_min && sqrt(res2_all[j*box+i])<=res_max)
            {
                psd_sum+=psd_show[j*box+i];
                psd_sum2+=(psd_show[j*box+i]*psd_show[j*box+i]);
                psd_count++;
            }
        }
    }
    double psd_mean=psd_sum/(psd_count);
    double psd_var=psd_sum2/(psd_count)-psd_mean*psd_mean;
    for(int i=0;i<box*box;i++)
    {
        if(psd_show[i]>psd_mean+sqrt(psd_var))
        {
            psd_show[i]=psd_mean+sqrt(psd_var);
        }
        else if(psd_show[i]<psd_mean-sqrt(psd_var))
        {
            psd_show[i]=psd_mean-sqrt(psd_var);
        }
    }
    float psd_min=psd_show[0];
    for(int i=1;i<box*box;i++)
    {
        if(psd_min>psd_show[i])
        {
            psd_min=psd_show[i];
        }
    }
    for(int i=0;i<box*box;i++)
    {
        psd_show[i]-=psd_min;
    }
    float psd_max=psd_show[0];
    for(int i=1;i<box*box;i++)
    {
        if(psd_max<psd_show[i])
        {
            psd_max=psd_show[i];
        }
    }
    for(int i=0;i<box*box;i++)
    {
        psd_show[i]/=psd_max;
    }
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            if(sqrt(res2_all[j*box+i])<res_min)
            {
                psd_show[j*box+i]=0.0;
            }
            /*
            else if(sqrt(x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j])>1.0/2.0/ctf_para_avg.getPixelSize())
            {
                psd_show[j*box+i]=0.0;
            }
            */
        }
    }

    for(int j=box/2;j<box;j++)
    {
        for(int i=0;i<=box/2-1;i++)
        {
            if(sqrt(res2_all[j*box+i])>=res_min && sqrt(res2_all[j*box+i])<=res_max)
            {
                float df=(ctf_para_avg.getDefocus1()+ctf_para_avg.getDefocus2()+(ctf_para_avg.getDefocus1()-ctf_para_avg.getDefocus2())*cos(2*(atan_all[j*box+i]-ctf_para_avg.getAstigmatism())))/2;
                // double res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
                double chi=M_PI*ctf_para_avg.getLambda()*df*res2_all[j*box+i]-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(double(res2_all[j*box+i])*double(res2_all[j*box+i]))+ctf_para_avg.getW_phase();
                float ctf=ctf_para_avg.getW()*cos(chi)+sqrt(1-ctf_para_avg.getW()*ctf_para_avg.getW())*sin(chi);
                psd_show[j*box+i]=abs(ctf);
            }
        }
    }

    delete [] psd_avg_radial;
}

void get_psd_fit_show_epa(float *psd_avg,float *psd_show,int box,CTF ctf_para_avg,float res_min,float res_max,double chi_min,double chi_max,float *res2_all,float *atan_all,float *x_fft_1d_res)
{
    memcpy(psd_show,psd_avg,sizeof(float)*box*box);
    float *psd_avg_radial=new float[box*box];
    get_radial_average_psd(psd_avg,psd_avg_radial,box,ctf_para_avg,res2_all,atan_all,x_fft_1d_res);
    for(int j=0;j<=box/2-1;j++)
    {
        for(int i=0;i<=box/2-1;i++)
        {
            psd_show[j*box+i]=psd_avg_radial[j*box+i];
        }
    }
    double psd_sum=0.0;
    double psd_sum2=0.0;
    int psd_count=0;
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            if(sqrt(res2_all[j*box+i])>=res_min && sqrt(res2_all[j*box+i])<=res_max)
            {
                psd_sum+=psd_show[j*box+i];
                psd_sum2+=(psd_show[j*box+i]*psd_show[j*box+i]);
                psd_count++;
            }
        }
    }
    double psd_mean=psd_sum/(psd_count);
    double psd_var=psd_sum2/(psd_count)-psd_mean*psd_mean;
    for(int i=0;i<box*box;i++)
    {
        if(psd_show[i]>psd_mean+sqrt(psd_var))
        {
            psd_show[i]=psd_mean+sqrt(psd_var);
        }
        else if(psd_show[i]<psd_mean-sqrt(psd_var))
        {
            psd_show[i]=psd_mean-sqrt(psd_var);
        }
    }
    float psd_min=psd_show[0];
    for(int i=1;i<box*box;i++)
    {
        if(psd_min>psd_show[i])
        {
            psd_min=psd_show[i];
        }
    }
    for(int i=0;i<box*box;i++)
    {
        psd_show[i]-=psd_min;
    }
    float psd_max=psd_show[0];
    for(int i=1;i<box*box;i++)
    {
        if(psd_max<psd_show[i])
        {
            psd_max=psd_show[i];
        }
    }
    for(int i=0;i<box*box;i++)
    {
        psd_show[i]/=psd_max;
    }
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            if(sqrt(res2_all[j*box+i])<res_min)
            {
                psd_show[j*box+i]=0.0;
            }
        }
    }

    for(int j=box/2;j<box;j++)
    {
        for(int i=0;i<=box/2-1;i++)
        {
            float df=(ctf_para_avg.getDefocus1()+ctf_para_avg.getDefocus2()+(ctf_para_avg.getDefocus1()-ctf_para_avg.getDefocus2())*cos(2*(atan_all[j*box+i]-ctf_para_avg.getAstigmatism())))/2;
            // double res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
            double chi=M_PI*ctf_para_avg.getLambda()*df*res2_all[j*box+i]-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(double(res2_all[j*box+i])*double(res2_all[j*box+i]))+ctf_para_avg.getW_phase()+ctf_para_avg.getPhaseShift();
            if(chi>=chi_min && chi<=chi_max)
            {
                float ctf=sin(chi);
                psd_show[j*box+i]=abs(ctf);
            }
        }
    }

    delete [] psd_avg_radial;
}

void get_psd_fit_show_epa_raw(float *psd_avg,float *psd_show,int box,CTF ctf_para_avg,float res_min,float res_max,double chi_min,double chi_max,float res_min_orig,float chi_min_orig,float res_max_2,float chi_max_2,float res_min_2,float chi_min_2,float *res2_all,float *atan_all,float *x_fft_1d_res)
{
    memcpy(psd_show,psd_avg,sizeof(float)*box*box);
    for(int i=0;i<box*box;i++)
    {
        psd_show[i]=log(1+psd_show[i]);
    }
    float *psd_avg_radial=new float[box*box];
    get_radial_average_psd(psd_show,psd_avg_radial,box,ctf_para_avg,res2_all,atan_all,x_fft_1d_res);
    for(int j=0;j<=box/2-1;j++)
    {
        for(int i=0;i<=box/2-1;i++)
        {
            // if(sqrt(x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j])>=res_min_orig)
            {
                psd_show[j*box+i]=psd_avg_radial[j*box+i];
            }
        }
    }
    // // Add B-factor
    // float B=1e-20;
    // for(int j=0;j<=box/2-1;j++)
    // {
    //     for(int i=0;i<=box/2-1;i++)
    //     {
    //         psd_show[j*box+i]*=exp(B*res2_all[j*box+i]);
    //     }
    // }

    double psd_sum=0.0;
    double psd_sum2=0.0;
    int psd_count=0;
    for(int j=0;j<box;j++)
    // for(int j=box/2;j<=box/2;j++)
    {
        for(int i=0;i<box;i++)
        {
            // if(sqrt(res2_all[j*box+i])>=res_min && sqrt(res2_all[j*box+i])<=res_max)
            if(sqrt(res2_all[j*box+i])>=res_min && sqrt(res2_all[j*box+i])<=1.0/ctf_para_avg.getPixelSize()/2.0)
            // if(sqrt(res2_all[j*box+i])>=res_max && sqrt(res2_all[j*box+i])<=res_max_2)
            {
                psd_sum+=psd_show[j*box+i];
                psd_sum2+=(psd_show[j*box+i]*psd_show[j*box+i]);
                psd_count++;
                // psd_sum+=psd_avg_radial[j*box+i];
                // psd_sum2+=(psd_avg_radial[j*box+i]*psd_avg_radial[j*box+i]);
                // psd_count++;
            }
        }
    }
    double psd_mean=psd_sum/(psd_count);
    double psd_var=psd_sum2/(psd_count)-psd_mean*psd_mean;
    for(int i=0;i<box*box;i++)
    {
        if(psd_show[i]>psd_mean+3*sqrt(psd_var))
        {
            psd_show[i]=psd_mean+3*sqrt(psd_var);
        }
        else if(psd_show[i]<psd_mean-3*sqrt(psd_var))
        {
            psd_show[i]=psd_mean-3*sqrt(psd_var);
        }
    }
    float psd_min=psd_show[0];
    for(int i=1;i<box*box;i++)
    {
        if(psd_min>psd_show[i])
        {
            psd_min=psd_show[i];
        }
    }
    for(int i=0;i<box*box;i++)
    {
        psd_show[i]-=psd_min;
    }
    float psd_max=psd_show[0];
    for(int i=1;i<box*box;i++)
    {
        if(psd_max<psd_show[i])
        {
            psd_max=psd_show[i];
        }
    }
    for(int i=0;i<box*box;i++)
    {
        psd_show[i]/=psd_max;
    }
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            if(sqrt(res2_all[j*box+i])<res_min_orig)
            {
                // psd_show[j*box+i]=0.0;
            }
        }
    }

    for(int j=box/2;j<box;j++)
    {
        for(int i=0;i<=box/2-1;i++)
        {
            float df=(ctf_para_avg.getDefocus1()+ctf_para_avg.getDefocus2()+(ctf_para_avg.getDefocus1()-ctf_para_avg.getDefocus2())*cos(2*(atan_all[j*box+i]-ctf_para_avg.getAstigmatism())))/2;
            // double res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
            double chi=M_PI*ctf_para_avg.getLambda()*df*res2_all[j*box+i]-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(double(res2_all[j*box+i])*double(res2_all[j*box+i]))+ctf_para_avg.getW_phase()+ctf_para_avg.getPhaseShift();
            // if(chi>=chi_min_orig && chi<=chi_max)
            if(chi<=chi_max_2)
            {
                float ctf=sin(chi);
                psd_show[j*box+i]=abs(ctf);
            }
        }
    }

    delete [] psd_avg_radial;
}

void get_psd_fit_show_epa_raw_avg(float *psd_avg,float *psd_show,int box,CTF ctf_para_avg,float res_min,float res_max,double chi_min,double chi_max,float res_min_orig,float chi_min_orig,float res_max_2,float chi_max_2,float *res2_all,float *atan_all,float *x_fft_1d_res)
{
    memcpy(psd_show,psd_avg,sizeof(float)*box*box);
    // for(int i=0;i<box*box;i++)
    // {
    //     psd_show[i]=log(1+psd_show[i]);
    // }
    float *psd_avg_radial=new float[box*box];
    get_radial_average_psd(psd_show,psd_avg_radial,box,ctf_para_avg,res2_all,atan_all,x_fft_1d_res);
    for(int j=0;j<=box/2-1;j++)
    {
        for(int i=0;i<=box/2-1;i++)
        {
            // if(sqrt(x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j])>=res_min_orig)
            {
                psd_show[j*box+i]=psd_avg_radial[j*box+i];
            }
        }
    }
    // // Add B-factor
    // float B=1e-20;
    // for(int j=0;j<=box/2-1;j++)
    // {
    //     for(int i=0;i<=box/2-1;i++)
    //     {
    //         psd_show[j*box+i]*=exp(B*res2_all[j*box+i]);
    //     }
    // }

    double psd_sum=0.0;
    double psd_sum2=0.0;
    int psd_count=0;
    for(int j=0;j<box;j++)
    // for(int j=box/2;j<=box/2;j++)
    {
        for(int i=0;i<box;i++)
        {
            // if(sqrt(res2_all[j*box+i])>=res_min && sqrt(res2_all[j*box+i])<=res_max)
            if(sqrt(res2_all[j*box+i])>=res_min && sqrt(res2_all[j*box+i])<=1.0/ctf_para_avg.getPixelSize()/2.0)
            // if(sqrt(res2_all[j*box+i])>=res_max && sqrt(res2_all[j*box+i])<=res_max_2)
            {
                psd_sum+=psd_show[j*box+i];
                psd_sum2+=(psd_show[j*box+i]*psd_show[j*box+i]);
                psd_count++;
                // psd_sum+=psd_avg_radial[j*box+i];
                // psd_sum2+=(psd_avg_radial[j*box+i]*psd_avg_radial[j*box+i]);
                // psd_count++;
            }
        }
    }
    double psd_mean=psd_sum/(psd_count);
    double psd_var=psd_sum2/(psd_count)-psd_mean*psd_mean;
    for(int i=0;i<box*box;i++)
    {
        if(psd_show[i]>psd_mean+3*sqrt(psd_var))
        {
            psd_show[i]=psd_mean+3*sqrt(psd_var);
        }
        else if(psd_show[i]<psd_mean-3*sqrt(psd_var))
        {
            psd_show[i]=psd_mean-3*sqrt(psd_var);
        }
    }
    float psd_min=psd_show[0];
    for(int i=1;i<box*box;i++)
    {
        if(psd_min>psd_show[i])
        {
            psd_min=psd_show[i];
        }
    }
    for(int i=0;i<box*box;i++)
    {
        psd_show[i]-=psd_min;
    }
    float psd_max=psd_show[0];
    for(int i=1;i<box*box;i++)
    {
        if(psd_max<psd_show[i])
        {
            psd_max=psd_show[i];
        }
    }
    for(int i=0;i<box*box;i++)
    {
        psd_show[i]/=psd_max;
    }
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            if(sqrt(res2_all[j*box+i])<res_min_orig)
            {
                // psd_show[j*box+i]=0.0;
            }
        }
    }

    for(int j=box/2;j<box;j++)
    {
        for(int i=0;i<=box/2-1;i++)
        {
            float df=(ctf_para_avg.getDefocus1()+ctf_para_avg.getDefocus2()+(ctf_para_avg.getDefocus1()-ctf_para_avg.getDefocus2())*cos(2*(atan_all[j*box+i]-ctf_para_avg.getAstigmatism())))/2;
            // double res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
            double chi=M_PI*ctf_para_avg.getLambda()*df*res2_all[j*box+i]-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(double(res2_all[j*box+i])*double(res2_all[j*box+i]))+ctf_para_avg.getW_phase()+ctf_para_avg.getPhaseShift();
            // if(chi>=chi_min_orig && chi<=chi_max)
            if(chi<=chi_max_2)
            {
                float ctf=sin(chi);
                psd_show[j*box+i]=abs(ctf);
            }
        }
    }

    delete [] psd_avg_radial;
}

void get_psd_fit_show_epa_conv(float *psd_avg,float *psd_show,int box,CTF ctf_para_avg,float res_min,float res_max,double chi_min,double chi_max,float res_min_orig,float chi_min_orig,float res_max_2,float chi_max_2,float res_min_2,float chi_min_2,float *res2_all,float *atan_all,float *x_fft_1d_res)
{
    memcpy(psd_show,psd_avg,sizeof(float)*box*box);
    for(int i=0;i<box*box;i++)
    {
        // psd_show[i]=log(1+psd_show[i]);
    }
    float *psd_avg_radial=new float[box*box];
    get_radial_average_psd(psd_show,psd_avg_radial,box,ctf_para_avg,res2_all,atan_all,x_fft_1d_res);
    for(int j=0;j<=box/2-1;j++)
    {
        for(int i=0;i<=box/2-1;i++)
        {
            // if(sqrt(x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j])>=res_min_orig)
            {
                psd_show[j*box+i]=psd_avg_radial[j*box+i];
            }
        }
    }

    double psd_sum=0.0;
    double psd_sum2=0.0;
    int psd_count=0;
    // for(int j=0;j<box/2;j++)
    for(int j=box/2;j<=box/2;j++)
    {
        for(int i=0;i<box/2;i++)
        {
            if(sqrt(res2_all[j*box+i])>=res_min && sqrt(res2_all[j*box+i])<=res_max_2)
            // if(sqrt(res2_all[j*box+i])>=res_max && sqrt(res2_all[j*box+i])<=1.0/ctf_para_avg.getPixelSize()/2.0)
            // if(sqrt(res2_all[j*box+i])>=res_max && sqrt(res2_all[j*box+i])<=res_max_2)
            {
                psd_sum+=psd_avg_radial[j*box+i];
                psd_sum2+=(psd_avg_radial[j*box+i]*psd_avg_radial[j*box+i]);
                psd_count++;
            }
        }
    }
    double psd_mean=psd_sum/(psd_count);
    double psd_var=psd_sum2/(psd_count)-psd_mean*psd_mean;
    for(int i=0;i<box*box;i++)
    {
        if(psd_show[i]>psd_mean+sqrt(psd_var))
        {
            psd_show[i]=psd_mean+sqrt(psd_var);
        }
        else if(psd_show[i]<psd_mean-sqrt(psd_var))
        {
            psd_show[i]=psd_mean-sqrt(psd_var);
        }
    }
    float psd_min=psd_show[0];
    for(int i=1;i<box*box;i++)
    {
        if(psd_min>psd_show[i])
        {
            psd_min=psd_show[i];
        }
    }
    for(int i=0;i<box*box;i++)
    {
        psd_show[i]-=psd_min;
    }
    float psd_max=psd_show[0];
    for(int i=1;i<box*box;i++)
    {
        if(psd_max<psd_show[i])
        {
            psd_max=psd_show[i];
        }
    }
    for(int i=0;i<box*box;i++)
    {
        psd_show[i]/=psd_max;
    }
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            if(sqrt(res2_all[j*box+i])<res_min_orig)
            {
                // psd_show[j*box+i]=0.0;
            }
        }
    }

    for(int j=box/2;j<box;j++)
    {
        for(int i=0;i<=box/2-1;i++)
        {
            float df=(ctf_para_avg.getDefocus1()+ctf_para_avg.getDefocus2()+(ctf_para_avg.getDefocus1()-ctf_para_avg.getDefocus2())*cos(2*(atan_all[j*box+i]-ctf_para_avg.getAstigmatism())))/2;
            // double res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
            double chi=M_PI*ctf_para_avg.getLambda()*df*res2_all[j*box+i]-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(double(res2_all[j*box+i])*double(res2_all[j*box+i]))+ctf_para_avg.getW_phase()+ctf_para_avg.getPhaseShift();
            // if(chi>=chi_min_orig && chi<=chi_max)
            if(chi<=chi_max_2)
            {
                float ctf=sin(chi);
                psd_show[j*box+i]=abs(ctf);
            }
        }
    }

    delete [] psd_avg_radial;
}

void get_psd_fit_show_epa_combine(float *psd_1,float *psd_2,float *psd_show,int box,CTF ctf_para_avg,float res_min,float res_max,double chi_min,double chi_max,float res_min_orig,float chi_min_orig,float res_max_2,float chi_max_2,float res_min_2,float chi_min_2,float *res2_all,float *atan_all,float *x_fft_1d_res)   // psd_1是抠了背景的，psd_2是只做了box convolution的
{
    for(int j=0;j<=box/2-1;j++)
    {
        for(int i=0;i<box;i++)
        {
            psd_show[j*box+i]=psd_1[j*box+i];
        }
    }
    for(int j=box/2;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            psd_show[j*box+i]=psd_2[j*box+i];
        }
    }

    float *psd_avg_radial=new float[box*box];
    get_radial_average_psd(psd_1,psd_avg_radial,box,ctf_para_avg,res2_all,atan_all,x_fft_1d_res);
    for(int j=0;j<=box/2-1;j++)
    {
        for(int i=j;i<=box/2-1;i++)
        {
            psd_show[j*box+i]=psd_avg_radial[j*box+i];
        }
    }
    // Add B-factor
    float B=25e-20;
    for(int j=0;j<=box/2-1;j++)
    {
        for(int i=j;i<=box/2-1;i++)
        {
            psd_show[j*box+i]*=exp(B*res2_all[j*box+i]);
        }
    }
    get_radial_average_psd(psd_2,psd_avg_radial,box,ctf_para_avg,res2_all,atan_all,x_fft_1d_res);
    for(int j=box/2;j<box;j++)
    {
        for(int i=box-j;i<=box/2-1;i++)
        {
            psd_show[j*box+i]=psd_avg_radial[j*box+i];
        }
    }
    // Add B-factor
    B=10e-20;
    for(int j=box/2;j<box;j++)
    {
        for(int i=box-j;i<=box/2-1;i++)
        {
            psd_show[j*box+i]*=exp(B*res2_all[j*box+i]);
        }
    }

    double psd_sum=0.0;
    double psd_sum2=0.0;
    int psd_count=0;
    /*
    for(int j=box/2;j<=box/2;j++)
    {
        for(int i=0;i<box/2;i++)
        {
            if(sqrt(res2_all[j*box+i])>=res_min && sqrt(res2_all[j*box+i])<=res_max_2)
            // if(sqrt(res2_all[j*box+i])>=res_max && sqrt(res2_all[j*box+i])<=1.0/ctf_para_avg.getPixelSize()/2.0)
            // if(sqrt(res2_all[j*box+i])>=res_max && sqrt(res2_all[j*box+i])<=res_max_2)
            {
                psd_sum+=psd_avg_radial[j*box+i];
                psd_sum2+=(psd_avg_radial[j*box+i]*psd_avg_radial[j*box+i]);
                psd_count++;
            }
        }
    }
    */
    for(int j=0;j<=box/2-1;j++)
    {
        for(int i=box/2;i<box;i++)
        {
            if(sqrt(res2_all[j*box+i])>=res_min && sqrt(res2_all[j*box+i])<=res_max)
            {
                psd_sum+=psd_show[j*box+i];
                psd_sum2+=(psd_show[j*box+i]*psd_show[j*box+i]);
                psd_count++;
            }
        }
    }
    /*
    for(int j=box/2;j<box;j++)
    {
        for(int i=box/2-1;i<=box/2-1;i++)
        {
            if(sqrt(res2_all[j*box+i])>=res_min && sqrt(res2_all[j*box+i])<=res_max_2)
            {
                psd_sum+=psd_show[j*box+i];
                psd_sum2+=(psd_show[j*box+i]*psd_show[j*box+i]);
                psd_count++;
            }
        }
    }
    */
    double psd_mean=psd_sum/(psd_count);
    double psd_var=psd_sum2/(psd_count)-psd_mean*psd_mean;
    for(int i=0;i<box*box;i++)
    {
        if(psd_show[i]>psd_mean+sqrt(psd_var))
        {
            psd_show[i]=psd_mean+sqrt(psd_var);
        }
        else if(psd_show[i]<psd_mean-sqrt(psd_var))
        {
            psd_show[i]=psd_mean-sqrt(psd_var);
        }
    }
    float psd_min=psd_show[0];
    for(int i=1;i<box*box;i++)
    {
        if(psd_min>psd_show[i])
        {
            psd_min=psd_show[i];
        }
    }
    for(int i=0;i<box*box;i++)
    {
        psd_show[i]-=psd_min;
    }
    float psd_max=psd_show[0];
    for(int i=1;i<box*box;i++)
    {
        if(psd_max<psd_show[i])
        {
            psd_max=psd_show[i];
        }
    }
    for(int i=0;i<box*box;i++)
    {
        psd_show[i]/=psd_max;
    }
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            if(sqrt(res2_all[j*box+i])<res_min_orig)
            {
                // psd_show[j*box+i]=0.0;
            }
        }
    }

    // 如果抠背景部分外面是白的，则反成黑的
    int ii;
    int jj=box/2-1;
    float res_cut=res2_all[0];
    for(ii=0;ii<=box/2-1;ii++)
    {
        if(psd_show[jj*box+ii]!=1.0 && psd_show[jj*box+ii]!=0.0)
        {
            res_cut=res2_all[jj*box+ii];
            break;
        }
    }
    for(int j=0;j<=box/2-1;j++)
    {
        for(int i=0;i<box;i++)
        {
            if(res2_all[j*box+i]>res_cut)
            {
                psd_show[j*box+i]=0.0;
            }
        }
    }

    for(int j=box/2;j<box;j++)
    {
        for(int i=0;i<box-j;i++)
        {
            float df=(ctf_para_avg.getDefocus1()+ctf_para_avg.getDefocus2()+(ctf_para_avg.getDefocus1()-ctf_para_avg.getDefocus2())*cos(2*(atan_all[j*box+i]-ctf_para_avg.getAstigmatism())))/2;
            // double res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
            double chi=M_PI*ctf_para_avg.getLambda()*df*res2_all[j*box+i]-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(double(res2_all[j*box+i])*double(res2_all[j*box+i]))+ctf_para_avg.getW_phase()+ctf_para_avg.getPhaseShift();
            // if(chi>=chi_min_orig && chi<=chi_max)
            if(chi<=chi_max_2)
            {
                float ctf=sin(chi);
                psd_show[j*box+i]=abs(ctf);
            }
        }
    }
    for(int j=0;j<=box/2-1;j++)
    {
        for(int i=0;i<j;i++)
        {
            float df=(ctf_para_avg.getDefocus1()+ctf_para_avg.getDefocus2()+(ctf_para_avg.getDefocus1()-ctf_para_avg.getDefocus2())*cos(2*(atan_all[j*box+i]-ctf_para_avg.getAstigmatism())))/2;
            // double res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
            double chi=M_PI*ctf_para_avg.getLambda()*df*res2_all[j*box+i]-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(double(res2_all[j*box+i])*double(res2_all[j*box+i]))+ctf_para_avg.getW_phase()+ctf_para_avg.getPhaseShift();
            // if(chi>=chi_min_orig && chi<=chi_max)
            if(chi<=chi_max_2)
            {
                float ctf=sin(chi);
                psd_show[j*box+i]=abs(ctf);
            }
        }
    }

    delete [] psd_avg_radial;
}

void get_psd_fit_show_epa_combine_2(float *psd_1,float *psd_2,float *psd_show,int box,CTF ctf_para_avg,float res_min,float res_max,double chi_min,double chi_max,float res_min_orig,float chi_min_orig,float res_max_2,float chi_max_2,float res_min_2,float chi_min_2,float *res2_all,float *atan_all,float *x_fft_1d_res)   // psd_1是抠了背景的，psd_2是只做了box convolution的，两部分独立做，再拼起来
{
    for(int j=0;j<=box/2-1;j++)
    {
        for(int i=0;i<box;i++)
        {
            psd_show[j*box+i]=psd_1[j*box+i];
        }
    }
    for(int j=box/2;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            psd_show[j*box+i]=psd_2[j*box+i];
        }
    }

    float *psd_avg_radial=new float[box*box];
    get_radial_average_psd(psd_1,psd_avg_radial,box,ctf_para_avg,res2_all,atan_all,x_fft_1d_res);
    for(int j=0;j<=box/2-1;j++)
    {
        for(int i=j;i<=box/2-1;i++)
        {
            psd_show[j*box+i]=psd_avg_radial[j*box+i];
        }
    }
    // Add B-factor
    float B=25e-20;
    for(int j=0;j<=box/2-1;j++)
    {
        for(int i=j;i<=box/2-1;i++)
        {
            psd_show[j*box+i]*=exp(B*res2_all[j*box+i]);
        }
    }
    get_radial_average_psd(psd_2,psd_avg_radial,box,ctf_para_avg,res2_all,atan_all,x_fft_1d_res);
    for(int j=box/2;j<box;j++)
    {
        for(int i=box-j;i<=box/2-1;i++)
        {
            psd_show[j*box+i]=psd_avg_radial[j*box+i];
        }
    }
    // Add B-factor
    B=2e-20;
    for(int j=box/2;j<box;j++)
    {
        for(int i=box-j;i<=box/2-1;i++)
        {
            psd_show[j*box+i]*=exp(-B*res2_all[j*box+i]);
        }
    }

    double psd_sum=0.0;
    double psd_sum2=0.0;
    int psd_count=0;
    /*
    for(int j=box/2;j<=box/2;j++)
    {
        for(int i=0;i<box/2;i++)
        {
            if(sqrt(res2_all[j*box+i])>=res_min && sqrt(res2_all[j*box+i])<=res_max_2)
            // if(sqrt(res2_all[j*box+i])>=res_max && sqrt(res2_all[j*box+i])<=1.0/ctf_para_avg.getPixelSize()/2.0)
            // if(sqrt(res2_all[j*box+i])>=res_max && sqrt(res2_all[j*box+i])<=res_max_2)
            {
                psd_sum+=psd_avg_radial[j*box+i];
                psd_sum2+=(psd_avg_radial[j*box+i]*psd_avg_radial[j*box+i]);
                psd_count++;
            }
        }
    }
    */
    for(int j=0;j<=box/2-1;j++)
    {
        for(int i=box/2;i<box;i++)
        {
            if(sqrt(res2_all[j*box+i])>=res_min && sqrt(res2_all[j*box+i])<=res_max)
            {
                psd_sum+=psd_show[j*box+i];
                psd_sum2+=(psd_show[j*box+i]*psd_show[j*box+i]);
                psd_count++;
            }
        }
    }
    /*
    for(int j=box/2;j<box;j++)
    {
        for(int i=box/2-1;i<=box/2-1;i++)
        {
            if(sqrt(res2_all[j*box+i])>=res_min && sqrt(res2_all[j*box+i])<=res_max_2)
            {
                psd_sum+=psd_show[j*box+i];
                psd_sum2+=(psd_show[j*box+i]*psd_show[j*box+i]);
                psd_count++;
            }
        }
    }
    */
    double psd_mean=psd_sum/(psd_count);
    double psd_var=psd_sum2/(psd_count)-psd_mean*psd_mean;
    for(int j=0;j<=box/2-1;j++)
    {
        for(int i=0;i<box;i++)
        {
            if(psd_show[j*box+i]>psd_mean+sqrt(psd_var))
            {
                psd_show[j*box+i]=psd_mean+sqrt(psd_var);
            }
            else if(psd_show[j*box+i]<psd_mean-sqrt(psd_var))
            {
                psd_show[j*box+i]=psd_mean-sqrt(psd_var);
            }
        }
    }
    float psd_min=psd_show[0];
    for(int j=0;j<=box/2-1;j++)
    {
        for(int i=0;i<box;i++)
        {
            if(psd_min>psd_show[j*box+i])
            {
                psd_min=psd_show[j*box+i];
            }
        }
    }
    for(int j=0;j<=box/2-1;j++)
    {
        for(int i=0;i<box;i++)
        {
            psd_show[j*box+i]-=psd_min;
        }
    }
    float psd_max=psd_show[0];
    for(int j=0;j<=box/2-1;j++)
    {
        for(int i=0;i<box;i++)
        {
            if(psd_max<psd_show[j*box+i])
            {
                psd_max=psd_show[j*box+i];
            }
        }
    }
    for(int j=0;j<=box/2-1;j++)
    {
        for(int i=0;i<box;i++)
        {
            psd_show[j*box+i]/=psd_max;
        }
    }

    psd_sum=0.0;
    psd_sum2=0.0;
    psd_count=0;
    for(int j=box/2;j<box;j++)
    {
        for(int i=box/2;i<box;i++)
        {
            if(sqrt(res2_all[j*box+i])>=res_min_2 && sqrt(res2_all[j*box+i])<=res_max_2)
            {
                psd_sum+=psd_show[j*box+i];
                psd_sum2+=(psd_show[j*box+i]*psd_show[j*box+i]);
                psd_count++;
            }
        }
    }
    psd_mean=psd_sum/(psd_count);
    psd_var=psd_sum2/(psd_count)-psd_mean*psd_mean;
    for(int j=box/2;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            if(psd_show[j*box+i]>psd_mean+sqrt(psd_var))
            {
                psd_show[j*box+i]=psd_mean+sqrt(psd_var);
            }
            else if(psd_show[j*box+i]<psd_mean-sqrt(psd_var))
            {
                psd_show[j*box+i]=psd_mean-sqrt(psd_var);
            }
        }
    }
    psd_min=psd_show[box/2*box];
    for(int j=box/2;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            if(psd_min>psd_show[j*box+i])
            {
                psd_min=psd_show[j*box+i];
            }
        }
    }
    for(int j=box/2;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            psd_show[j*box+i]-=psd_min;
        }
    }
    psd_max=psd_show[box/2*box];
    for(int j=box/2;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            if(psd_max<psd_show[j*box+i])
            {
                psd_max=psd_show[j*box+i];
            }
        }
    }
    for(int j=box/2;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            psd_show[j*box+i]/=psd_max;
        }
    }



    // 如果抠背景部分外面是白的，则反成黑的
    int ii;
    int jj=box/2-1;
    float res_cut=res2_all[0];
    for(ii=0;ii<=box/2-1;ii++)
    {
        if(psd_show[jj*box+ii]!=1.0 && psd_show[jj*box+ii]!=0.0)
        {
            res_cut=res2_all[jj*box+ii];
            break;
        }
    }
    for(int j=0;j<=box/2-1;j++)
    {
        for(int i=0;i<box;i++)
        {
            if(res2_all[j*box+i]>res_cut)
            {
                psd_show[j*box+i]=0.0;
            }
        }
    }

    for(int j=box/2;j<box;j++)
    {
        for(int i=0;i<box-j;i++)
        {
            float df=(ctf_para_avg.getDefocus1()+ctf_para_avg.getDefocus2()+(ctf_para_avg.getDefocus1()-ctf_para_avg.getDefocus2())*cos(2*(atan_all[j*box+i]-ctf_para_avg.getAstigmatism())))/2;
            // double res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
            double chi=M_PI*ctf_para_avg.getLambda()*df*res2_all[j*box+i]-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(double(res2_all[j*box+i])*double(res2_all[j*box+i]))+ctf_para_avg.getW_phase()+ctf_para_avg.getPhaseShift();
            // if(chi>=chi_min_orig && chi<=chi_max)
            if(chi<=chi_max_2)
            {
                float ctf=sin(chi);
                psd_show[j*box+i]=abs(ctf);
            }
        }
    }
    for(int j=0;j<=box/2-1;j++)
    {
        for(int i=0;i<j;i++)
        {
            float df=(ctf_para_avg.getDefocus1()+ctf_para_avg.getDefocus2()+(ctf_para_avg.getDefocus1()-ctf_para_avg.getDefocus2())*cos(2*(atan_all[j*box+i]-ctf_para_avg.getAstigmatism())))/2;
            // double res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
            double chi=M_PI*ctf_para_avg.getLambda()*df*res2_all[j*box+i]-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(double(res2_all[j*box+i])*double(res2_all[j*box+i]))+ctf_para_avg.getW_phase()+ctf_para_avg.getPhaseShift();
            // if(chi>=chi_min_orig && chi<=chi_max)
            if(chi<=chi_max_2)
            {
                float ctf=sin(chi);
                psd_show[j*box+i]=abs(ctf);
            }
        }
    }

    delete [] psd_avg_radial;
}

void get_psd_fit_show_epa_subtraction(float *psd_avg,float *psd_show,int box,CTF ctf_para_avg,float res_min,float res_max,float res_min_orig,float res_max_2,float chi_max_2,float *res2_all,float *atan_all,float *x_fft_1d_res)
{
    memcpy(psd_show,psd_avg,sizeof(float)*box*box);
    for(int i=0;i<box*box;i++)
    {
        // psd_show[i]=log(1+psd_show[i]);
    }
    float *psd_avg_radial=new float[box*box];
    get_radial_average_psd(psd_avg,psd_avg_radial,box,ctf_para_avg,res2_all,atan_all,x_fft_1d_res);
    for(int j=0;j<=box/2-1;j++)
    {
        for(int i=0;i<=box/2-1;i++)
        {
            // if(sqrt(x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j])>=res_min_orig)
            {
                psd_show[j*box+i]=psd_avg_radial[j*box+i];
            }
        }
    }
    // Add B-factor
    float B=25e-20;
    for(int j=0;j<=box/2-1;j++)
    {
        for(int i=0;i<=box/2-1;i++)
        {
            psd_show[j*box+i]*=exp(B*res2_all[j*box+i]);
        }
    }

    double psd_sum=0.0;
    double psd_sum2=0.0;
    int psd_count=0;
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            if(sqrt(res2_all[j*box+i])>=res_min && sqrt(res2_all[j*box+i])<=res_max)
            {
                psd_sum+=psd_show[j*box+i];
                psd_sum2+=(psd_show[j*box+i]*psd_show[j*box+i]);
                psd_count++;
            }
        }
    }
    double psd_mean=psd_sum/(psd_count);
    double psd_var=psd_sum2/(psd_count)-psd_mean*psd_mean;
    for(int i=0;i<box*box;i++)
    {
        if(psd_show[i]>psd_mean+sqrt(psd_var))
        {
            psd_show[i]=psd_mean+sqrt(psd_var);
        }
        else if(psd_show[i]<psd_mean-sqrt(psd_var))
        {
            psd_show[i]=psd_mean-sqrt(psd_var);
        }
    }
    float psd_min=psd_show[0];
    for(int i=1;i<box*box;i++)
    {
        if(psd_min>psd_show[i])
        {
            psd_min=psd_show[i];
        }
    }
    for(int i=0;i<box*box;i++)
    {
        psd_show[i]-=psd_min;
    }
    float psd_max=psd_show[0];
    for(int i=1;i<box*box;i++)
    {
        if(psd_max<psd_show[i])
        {
            psd_max=psd_show[i];
        }
    }
    for(int i=0;i<box*box;i++)
    {
        psd_show[i]/=psd_max;
    }
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            if(sqrt(res2_all[j*box+i])<res_min_orig)
            {
                // psd_show[j*box+i]=0.0;
            }
        }
    }

    // 如果外面是白的，则反成黑的
    int ii;
    int jj=box/2-1;
    float res_cut=res2_all[jj*box];
    // for(ii=box-1;ii>=box/2;ii--)   // 先找到全是白的或全是黑的地方
    bool start=true;
    for(ii=0;ii<=box/2-1;ii++)
    {
        if(sqrt(res2_all[jj*box+ii])<res_max) // 最多截到res_max
        {
            res_cut=res_max*res_max;
            break;
        }
        if(psd_show[jj*box+ii]!=1.0 && psd_show[jj*box+ii]!=0.0)
        {
            if(start==false && psd_show[jj*box+ii+1]!=1.0 && psd_show[jj*box+ii+1]!=0.0)    // 多判断一个点，防止出现野点；排除边界上最开始几个误差点
            {
                res_cut=res2_all[jj*box+ii];
                break;
            }
            else if(ii>=5)
            {
                start=false;
            }
        }
        else
        {
            start=false;
        }
    }
    for(int i=0;i<box*box;i++)
    {
        if(res2_all[i]>res_cut)
        {
            psd_show[i]=0.0;
        }
    }

    for(int j=box/2;j<box;j++)
    {
        for(int i=0;i<=box/2-1;i++)
        {
            float df=(ctf_para_avg.getDefocus1()+ctf_para_avg.getDefocus2()+(ctf_para_avg.getDefocus1()-ctf_para_avg.getDefocus2())*cos(2*(atan_all[j*box+i]-ctf_para_avg.getAstigmatism())))/2;
            // double res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
            double chi=M_PI*ctf_para_avg.getLambda()*df*res2_all[j*box+i]-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(double(res2_all[j*box+i])*double(res2_all[j*box+i]))+ctf_para_avg.getW_phase()+ctf_para_avg.getPhaseShift();
            // if(chi>=chi_min_orig && chi<=chi_max)
            if(chi<=chi_max_2)
            {
                float ctf=sin(chi);
                psd_show[j*box+i]=abs(ctf);
            }
        }
    }

    delete [] psd_avg_radial;
}

void get_psd_fit_show_epa_subtraction_avg(float *psd_avg,float *psd_show,int box,CTF ctf_para_avg,float res_min,float res_max,double chi_min,double chi_max,float res_min_orig,float chi_min_orig,float res_max_2,float chi_max_2,float *res2_all,float *atan_all,float *x_fft_1d_res)
{
    memcpy(psd_show,psd_avg,sizeof(float)*box*box);
    for(int i=0;i<box*box;i++)
    {
        // psd_show[i]=log(1+psd_show[i]);
    }
    float *psd_avg_radial=new float[box*box];
    get_radial_average_psd(psd_avg,psd_avg_radial,box,ctf_para_avg,res2_all,atan_all,x_fft_1d_res);
    for(int j=0;j<=box/2-1;j++)
    {
        for(int i=0;i<=box/2-1;i++)
        {
            // if(sqrt(x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j])>=res_min_orig)
            {
                psd_show[j*box+i]=psd_avg_radial[j*box+i];
            }
        }
    }
    // Add B-factor
    float B=25e-20;
    for(int j=0;j<=box/2-1;j++)
    {
        for(int i=0;i<=box/2-1;i++)
        {
            psd_show[j*box+i]*=exp(B*res2_all[j*box+i]);
        }
    }

    double psd_sum=0.0;
    double psd_sum2=0.0;
    int psd_count=0;
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            if(sqrt(res2_all[j*box+i])>=res_min && sqrt(res2_all[j*box+i])<=res_max)
            {
                psd_sum+=psd_show[j*box+i];
                psd_sum2+=(psd_show[j*box+i]*psd_show[j*box+i]);
                psd_count++;
            }
        }
    }
    double psd_mean=psd_sum/(psd_count);
    double psd_var=psd_sum2/(psd_count)-psd_mean*psd_mean;
    for(int i=0;i<box*box;i++)
    {
        if(psd_show[i]>psd_mean+1.2*sqrt(psd_var))
        {
            psd_show[i]=psd_mean+1.2*sqrt(psd_var);
        }
        else if(psd_show[i]<psd_mean-1.2*sqrt(psd_var))
        {
            psd_show[i]=psd_mean-1.2*sqrt(psd_var);
        }
    }
    float psd_min=psd_show[0];
    for(int i=1;i<box*box;i++)
    {
        if(psd_min>psd_show[i])
        {
            psd_min=psd_show[i];
        }
    }
    for(int i=0;i<box*box;i++)
    {
        psd_show[i]-=psd_min;
    }
    float psd_max=psd_show[0];
    for(int i=1;i<box*box;i++)
    {
        if(psd_max<psd_show[i])
        {
            psd_max=psd_show[i];
        }
    }
    for(int i=0;i<box*box;i++)
    {
        psd_show[i]/=psd_max;
    }
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            if(sqrt(res2_all[j*box+i])<res_min_orig)
            {
                // psd_show[j*box+i]=0.0;
            }
        }
    }

    // 如果外面是白的，则反成黑的
    int ii;
    int jj=box/2-1;
    float res_cut=res2_all[jj*box];
    // for(ii=box-1;ii>=box/2;ii--)   // 先找到全是白的或全是黑的地方
    bool start=true;
    for(ii=0;ii<=box/2-1;ii++)
    {
        if(sqrt(res2_all[jj*box+ii])<res_max) // 最多截到res_max
        {
            res_cut=res_max*res_max;
            break;
        }
        if(psd_show[jj*box+ii]!=1.0 && psd_show[jj*box+ii]!=0.0)
        {
            if(start==false && psd_show[jj*box+ii+1]!=1.0 && psd_show[jj*box+ii+1]!=0.0)    // 多判断一个点，防止出现野点；排除边界上最开始几个误差点
            {
                res_cut=res2_all[jj*box+ii];
                break;
            }
            else if(ii>=5)
            {
                start=false;
            }
        }
        else
        {
            start=false;
        }
    }
    for(int i=0;i<box*box;i++)
    {
        if(res2_all[i]>res_cut)
        {
            psd_show[i]=0.0;
        }
    }

    for(int j=box/2;j<box;j++)
    {
        for(int i=0;i<=box/2-1;i++)
        {
            float df=(ctf_para_avg.getDefocus1()+ctf_para_avg.getDefocus2()+(ctf_para_avg.getDefocus1()-ctf_para_avg.getDefocus2())*cos(2*(atan_all[j*box+i]-ctf_para_avg.getAstigmatism())))/2;
            // double res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
            double chi=M_PI*ctf_para_avg.getLambda()*df*res2_all[j*box+i]-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(double(res2_all[j*box+i])*double(res2_all[j*box+i]))+ctf_para_avg.getW_phase()+ctf_para_avg.getPhaseShift();
            // if(chi>=chi_min_orig && chi<=chi_max)
            if(chi<=chi_max_2)
            {
                float ctf=sin(chi);
                psd_show[j*box+i]=abs(ctf);
            }
        }
    }

    delete [] psd_avg_radial;
}

void get_psd_fit_show_epa_subtraction_avg_log(float *psd_avg,float *psd_show,int box,float res_min,float res_max,float *res2_all)
{
    memcpy(psd_show,psd_avg,sizeof(float)*box*box);
    double psd_sum=0.0;
    double psd_sum2=0.0;
    int psd_count=0;
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            if(sqrt(res2_all[j*box+i])>=res_min && sqrt(res2_all[j*box+i])<=res_max)
            {
                psd_sum+=psd_show[j*box+i];
                psd_sum2+=(psd_show[j*box+i]*psd_show[j*box+i]);
                psd_count++;
            }
        }
    }
    double psd_mean=psd_sum/(psd_count);
    double psd_var=psd_sum2/(psd_count)-psd_mean*psd_mean;
    for(int i=0;i<box*box;i++)
    {
        if(psd_show[i]>psd_mean+1.2*sqrt(psd_var))
        {
            psd_show[i]=psd_mean+1.2*sqrt(psd_var);
        }
        else if(psd_show[i]<psd_mean-1.2*sqrt(psd_var))
        {
            psd_show[i]=psd_mean-1.2*sqrt(psd_var);
        }
    }
    float psd_min=psd_show[0];
    for(int i=1;i<box*box;i++)
    {
        if(psd_min>psd_show[i])
        {
            psd_min=psd_show[i];
        }
    }
    for(int i=0;i<box*box;i++)
    {
        psd_show[i]-=psd_min;
    }
    float psd_max=psd_show[0];
    for(int i=1;i<box*box;i++)
    {
        if(psd_max<psd_show[i])
        {
            psd_max=psd_show[i];
        }
    }
    for(int i=0;i<box*box;i++)
    {
        psd_show[i]/=psd_max;
    }
}

void get_psd_fit_show_epa_subtraction_conv(float *psd_avg,float *psd_show,int box,CTF ctf_para_avg,float res_min,float res_max,double chi_min,double chi_max,float res_min_orig,float chi_min_orig,float res_max_2,float chi_max_2,float *res2_all,float *atan_all,float *x_fft_1d_res,int box_conv)
{
    memcpy(psd_show,psd_avg,sizeof(float)*box*box);
    // 把边界受box conv影响的区域置零不输出
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            if(j<box_conv/2 || j>box-box_conv/2-1 || i<box_conv/2 || i>box-box_conv/2-1)
            {
                psd_show[j*box+i]=0.0;
            }
        }
    }
    for(int i=0;i<box*box;i++)
    {
        // psd_show[i]=log(1+psd_show[i]);
    }
    float *psd_avg_radial=new float[box*box];
    get_radial_average_psd(psd_show,psd_avg_radial,box,ctf_para_avg,res2_all,atan_all,x_fft_1d_res);
    for(int j=0;j<=box/2-1;j++)
    {
        for(int i=0;i<=box/2-1;i++)
        {
            // if(sqrt(x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j])>=res_min_orig)
            {
                psd_show[j*box+i]=psd_avg_radial[j*box+i];
            }
        }
    }
    // Add B-factor
    float B=25e-20;
    for(int j=0;j<=box/2-1;j++)
    {
        for(int i=0;i<=box/2-1;i++)
        {
            psd_show[j*box+i]*=exp(B*res2_all[j*box+i]);
        }
    }

    double psd_sum=0.0;
    double psd_sum2=0.0;
    int psd_count=0;
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            if(sqrt(res2_all[j*box+i])>=res_min && sqrt(res2_all[j*box+i])<=res_max)
            {
                psd_sum+=psd_show[j*box+i];
                psd_sum2+=(psd_show[j*box+i]*psd_show[j*box+i]);
                psd_count++;
            }
        }
    }
    double psd_mean=psd_sum/(psd_count);
    double psd_var=psd_sum2/(psd_count)-psd_mean*psd_mean;
    for(int i=0;i<box*box;i++)
    {
        if(psd_show[i]>psd_mean+sqrt(psd_var))
        {
            psd_show[i]=psd_mean+sqrt(psd_var);
        }
        else if(psd_show[i]<psd_mean-sqrt(psd_var))
        {
            psd_show[i]=psd_mean-sqrt(psd_var);
        }
    }
    float psd_min=psd_show[0];
    for(int i=1;i<box*box;i++)
    {
        if(psd_min>psd_show[i])
        {
            psd_min=psd_show[i];
        }
    }
    for(int i=0;i<box*box;i++)
    {
        psd_show[i]-=psd_min;
    }
    float psd_max=psd_show[0];
    for(int i=1;i<box*box;i++)
    {
        if(psd_max<psd_show[i])
        {
            psd_max=psd_show[i];
        }
    }
    for(int i=0;i<box*box;i++)
    {
        psd_show[i]/=psd_max;
    }
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            if(sqrt(res2_all[j*box+i])<res_min_orig)
            {
                // psd_show[j*box+i]=0.0;
            }
        }
    }

    // 如果外面是白的，则反成黑的
    int ii;
    int jj=box/2-1;
    float res_cut=res2_all[0];
    // for(ii=box-1;ii>=box/2;ii--)   // 先找到全是白的或全是黑的地方
    for(ii=0;ii<=box/2-1;ii++)
    {
        if(psd_show[jj*box+ii]!=1.0 && psd_show[jj*box+ii]!=0.0)
        {
            res_cut=res2_all[jj*box+ii];
            break;
        }
    }
    for(int i=0;i<box*box;i++)
    {
        if(res2_all[i]>res_cut)
        {
            psd_show[i]=0.0;
        }
    }

    for(int j=box/2;j<box;j++)
    {
        for(int i=0;i<=box/2-1;i++)
        {
            float df=(ctf_para_avg.getDefocus1()+ctf_para_avg.getDefocus2()+(ctf_para_avg.getDefocus1()-ctf_para_avg.getDefocus2())*cos(2*(atan_all[j*box+i]-ctf_para_avg.getAstigmatism())))/2;
            // double res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
            double chi=M_PI*ctf_para_avg.getLambda()*df*res2_all[j*box+i]-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(double(res2_all[j*box+i])*double(res2_all[j*box+i]))+ctf_para_avg.getW_phase()+ctf_para_avg.getPhaseShift();
            // if(chi>=chi_min_orig && chi<=chi_max)
            if(chi<=chi_max_2)
            {
                float ctf=sin(chi);
                psd_show[j*box+i]=abs(ctf);
            }
        }
    }

    delete [] psd_avg_radial;
}


void get_psd_conv_fft(float *psd,int box,int box_conv,fftwf_plan plan_fft,fftwf_plan plan_ifft,float *fft_buf_in,float *fft_buf_out)
{
    float *psd_aug=new float[box*box*2];
    float *psd_fft=new float[box*box*2];
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            if(j>=int(box/2)-2 && j<=int(box/2)+2 && i>=int(box/2)-2 && i<=int(box/2)+2)
            {
                psd_aug[j*box*2+i*2]=0.0;
                psd_aug[j*box*2+i*2+1]=0.0;
            }
            else
            {
                psd_aug[j*box*2+i*2]=psd[j*box+i];
                psd_aug[j*box*2+i*2+1]=0.0;
            }
        }
    }
    memcpy(fft_buf_in,psd_aug,sizeof(float)*box*box*2);
    fftwf_execute(plan_fft);
    memcpy(psd_fft,fft_buf_out,sizeof(float)*box*box*2);

    float *conv_box_aug=new float[box*box*2];
    float *conv_box_fft=new float[box*box*2];
    for(int i=0;i<box*box*2;i++)
    {
        conv_box_aug[i]=0.0;
    }
    int dx_min,dx_max;
    if(box_conv%2==0)
    {
        dx_min=box_conv/2-1;
        dx_max=box_conv/2;
    }
    else
    {
        dx_min=(box_conv-1)/2;
        dx_max=(box_conv-1)/2;
    }
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            if((j<=dx_max && i<=dx_max) || (j<=dx_max && i>=box-dx_min) || (j>=box-dx_min && i<=dx_max) || (j>=box-dx_min && i>=box-dx_min))
            {
                conv_box_aug[j*box*2+2*i]=1.0;
            }
        }
    }
    memcpy(fft_buf_in,conv_box_aug,sizeof(float)*box*box*2);
    fftwf_execute(plan_fft);
    memcpy(conv_box_fft,fft_buf_out,sizeof(float)*box*box*2);

    float *psd_conv_fft=new float[box*box*2];
    for(int i=0;i<box*box;i++)
    {
        psd_conv_fft[i*2]=psd_fft[i*2]-(psd_fft[i*2]*(conv_box_fft[i*2])-psd_fft[i*2+1]*(conv_box_fft[i*2+1]))/float(box_conv*box_conv);
        psd_conv_fft[i*2+1]=psd_fft[i*2+1]-(psd_fft[i*2]*(conv_box_fft[i*2+1])+psd_fft[i*2+1]*(conv_box_fft[i*2]))/float(box_conv*box_conv);
    }
    memcpy(fft_buf_in,psd_conv_fft,sizeof(float)*box*box*2);
    fftwf_execute(plan_ifft);
    for(int i=0;i<box*box;i++)
    {
        psd[i]=fft_buf_out[2*i]/float(box*box);
    }

    delete [] psd_conv_fft;
    delete [] conv_box_fft;
    delete [] conv_box_aug;
    delete [] psd_fft;
    delete [] psd_aug;
}


int get_number_of_rings_fit(float *psd_avg,int box,CTF ctf_para_avg,float *res2_all,float *atan_all,float *x_fft_1d_res)
{
    int n=1;
    float df=(ctf_para_avg.getDefocus1()+ctf_para_avg.getDefocus2()+(ctf_para_avg.getDefocus1()-ctf_para_avg.getDefocus2())*cos(2*(-ctf_para_avg.getAstigmatism())))/2;
    float res_min=sqrt((M_PI*ctf_para_avg.getLambda()*(df)-sqrt(M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df)*(df)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(n*M_PI-ctf_para_avg.getW_phase())))/(M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())));
    float res_max=sqrt((M_PI*ctf_para_avg.getLambda()*(df)-sqrt(M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df)*(df)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*((n+1)*M_PI-ctf_para_avg.getW_phase())))/(M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())));
    double chi_min;
    double chi_max;
    while(res_max<1.0/ctf_para_avg.getPixelSize()/2.0)
    {
        // 求一圈thon ring的互相关
        Data_opt_epa data_opt;
        data_opt.box=box;
        data_opt.ctf_para=ctf_para_avg;
        data_opt.psd=psd_avg;
        data_opt.res_min=res_min;
        data_opt.res_max=res_max;
        data_opt.chi_min=M_PI*ctf_para_avg.getLambda()*df*(res_min*res_min)-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*double(res_min*res_min)*double(res_min*res_min)+ctf_para_avg.getW_phase();
        data_opt.chi_max=M_PI*ctf_para_avg.getLambda()*df*(res_max*res_max)-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*double(res_max*res_max)*double(res_max*res_max)+ctf_para_avg.getW_phase();
        data_opt.res2_all=res2_all;
        data_opt.atan_all=atan_all;
        data_opt.x_fft_1d_res=x_fft_1d_res;
        double x[3]={ctf_para_avg.getDefocus1()*1e10,ctf_para_avg.getDefocus2()*1e10,ctf_para_avg.getAstigmatism()};

        double cc=-1.0;
        cc=get_correlation_image_epa_zero_mean(3,x,NULL,&data_opt);
        if(cc<0.7)  // 互相关太小，拟合有偏差，截取到上一个零点
        {
            break;
        }

        // 下一个零点
        n++;
        res_min=sqrt((M_PI*ctf_para_avg.getLambda()*(df)-sqrt(M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df)*(df)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(n*M_PI-ctf_para_avg.getW_phase())))/(M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())));
        res_max=sqrt((M_PI*ctf_para_avg.getLambda()*(df)-sqrt(M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df)*(df)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*((n+1)*M_PI-ctf_para_avg.getW_phase())))/(M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())));
    }
    // n--;
    return n;
}

double get_theta_resolution_all_omp(unsigned n,const double *x,double *grad,void *data) // 优化的bottleneck在尺度变换上（要对每一张图每一个小块做缩放）！！
{
    Data_opt_theta_resolution_all *data_opt=(Data_opt_theta_resolution_all*)data;
    float theta_offset=x[0];
    // CTF ctf_para_scaling=data_opt->ctf_para_avg;
    // ctf_para_scaling.setAllCTFPara(ctf_para_scaling.getDefocus1()*1e10/4.0,ctf_para_scaling.getDefocus2()*1e10/4.0,ctf_para_scaling.getAstigmatism()*180.0/M_PI,ctf_para_scaling.getPhaseShift(),ctf_para_scaling.getW());

    // 平均功率谱的互相关最大化
    double *psd_sum_omp[data_opt->threads];
    for(int th=0;th<data_opt->threads;th++)
    {
        psd_sum_omp[th]=new double[data_opt->box*data_opt->box];
        for(int i=0;i<data_opt->box*data_opt->box;i++)
        {
            psd_sum_omp[th][i]=0.0;
        }
    }
    int N_avail=0;
    for(int n=0;n<data_opt->Nz;n++)
    {
        if(data_opt->image_avail[n]==true)
        {
            N_avail++;
        }
    }
    #pragma omp parallel for num_threads(data_opt->threads)
    for(int n=0;n<data_opt->Nz;n++)
    // for(int n=data_opt->N_ref-data_opt->N_avg;n<=data_opt->N_ref+data_opt->N_avg;n++)
    {
        if(data_opt->image_avail[n])
        {
            float *psd_now=new float[data_opt->box*data_opt->box];
            float df=(data_opt->ctf_para[n].getDefocus1()+data_opt->ctf_para[n].getDefocus2()+(data_opt->ctf_para[n].getDefocus1()-data_opt->ctf_para[n].getDefocus2())*cos(2*(-data_opt->ctf_para[n].getAstigmatism())))/2;
            for(int i=0;i<data_opt->box*data_opt->box;i++)
            {
                psd_now[i]=0.0;
            }
            // #pragma omp parallel for num_threads(data_opt->threads)
            for(int m=0;m<data_opt->N_block;m++)
            {
                // get_scaled_psd_all(data_opt->psd_all[n][m],psd_now,data_opt->block_x[m],data_opt->block_y[m],data_opt->box,df*1e10,data_opt->df_ref*1e10,data_opt->pix,data_opt->psi,data_opt->theta[n]+theta_offset,data_opt->Nx,data_opt->Ny);
                // get_scaled_psd_all_phi(data_opt->psd_all[n][m],psd_now,data_opt->block_x[m],data_opt->block_y[m],data_opt->box,df*1e10,data_opt->df_ref*1e10,data_opt->pix,data_opt->psi,data_opt->theta[n]+theta_offset,data_opt->phi,data_opt->Nx,data_opt->Ny);
                get_scaled_psd_all_phi(data_opt->psd_all[n][m],psd_now,data_opt->block_x[m],data_opt->block_y[m],data_opt->box,df*1e10,data_opt->df_ref*1e10,data_opt->pix,data_opt->psi,data_opt->theta[n]+theta_offset,data_opt->phi,data_opt->Nx,data_opt->Ny);
                for(int i=0;i<data_opt->box*data_opt->box;i++)
                {
                    psd_sum_omp[omp_get_thread_num()][i]+=double(psd_now[i]);
                }
            }
            delete [] psd_now;
        }
    }
    float *psd_now=new float[data_opt->box*data_opt->box];
    #pragma omp parallel for num_threads(data_opt->threads)
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        double psd_sum_now=0.0;
        for(int th=0;th<data_opt->threads;th++)
        {
            psd_sum_now+=psd_sum_omp[th][i];
        }
        // psd_now[i]=float(psd_sum_now/double(data_opt->N_block*data_opt->Nz));
        psd_now[i]=float(psd_sum_now/double(data_opt->N_block*N_avail));
        // psd_now[i]=float(psd_sum_now/double(data_opt->N_block*(2*data_opt->N_avg+1)));
        psd_now[i]=log(1.0+psd_now[i]);
    }
    for(int th=0;th<data_opt->threads;th++)
    {
        delete [] psd_sum_omp[th];
    }

    // box convolution
    if(data_opt->box_conv>0)
    {
        // get_psd_conv_fft(psd_now,data_opt->box,data_opt->box_conv,data_opt->plan_fft,data_opt->plan_ifft,data_opt->fft_buf_in,data_opt->fft_buf_out);
    }

    // 通过优化方法估背景
    float radial_now[data_opt->box/2-1];
    get_radial_average(psd_now,radial_now,data_opt->box,data_opt->ctf_para_avg,data_opt->res2_all,data_opt->atan_all,data_opt->x_fft_1d_res);
    // get_radial_average(psd_now,radial_now,data_opt->box,ctf_para_scaling,data_opt->res2_all,data_opt->atan_all,data_opt->x_fft_1d_res);
    double fit_now[data_opt->box/2-1];
    background_estimation(fit_now,radial_now,data_opt->box,data_opt->N_zeros,data_opt->ctf_para_avg);
    // background_estimation(fit_now,radial_now,data_opt->box,data_opt->N_zeros,ctf_para_scaling);

    // 回到二维抠背景
    // background_subtraction(psd_now,fit_now,data_opt->box,data_opt->res2_all,data_opt->atan_all,data_opt->x_fft_1d_res,data_opt->ctf_para_avg);
    // background_subtraction(psd_now,fit_now,data_opt->box,data_opt->res2_all,data_opt->atan_all,data_opt->x_fft_1d_res,ctf_para_scaling);

    // 计算高频互相关，先优化一次CTF参数
    // 先再局域优化一次CTF
    Data_opt_epa data_opt_image;
    data_opt_image.box=data_opt->box;
    data_opt_image.ctf_para=data_opt->ctf_para_avg;
    // data_opt_image.ctf_para=ctf_para_scaling;
    data_opt_image.psd=psd_now;
    data_opt_image.res_min=data_opt->res_min;
    data_opt_image.res_max=data_opt->res_max;
    data_opt_image.chi_min=M_PI*data_opt->ctf_para_avg.getLambda()*data_opt->df_ref*(data_opt->res_min*data_opt->res_min)-M_PI_2*data_opt->ctf_para_avg.getCs()*(data_opt->ctf_para_avg.getLambda()*data_opt->ctf_para_avg.getLambda()*data_opt->ctf_para_avg.getLambda())*double(data_opt->res_min*data_opt->res_min)*double(data_opt->res_min*data_opt->res_min)+data_opt->ctf_para_avg.getW_phase();
    data_opt_image.chi_max=M_PI*data_opt->ctf_para_avg.getLambda()*data_opt->df_ref*(data_opt->res_max*data_opt->res_max)-M_PI_2*data_opt->ctf_para_avg.getCs()*(data_opt->ctf_para_avg.getLambda()*data_opt->ctf_para_avg.getLambda()*data_opt->ctf_para_avg.getLambda())*double(data_opt->res_max*data_opt->res_max)*double(data_opt->res_max*data_opt->res_max)+data_opt->ctf_para_avg.getW_phase();
    data_opt_image.res2_all=data_opt->res2_all;
    data_opt_image.atan_all=data_opt->atan_all;
    data_opt_image.x_fft_1d_res=data_opt->x_fft_1d_res;

    nlopt_opt opt_image;
    opt_image=nlopt_create(NLOPT_LN_NELDERMEAD,3);
    if(data_opt->optimize_with_avg)
    {
        nlopt_set_max_objective(opt_image,get_correlation_image_epa_zero_mean,&data_opt_image);
    }
    else
    {
        nlopt_set_max_objective(opt_image,get_correlation_image_zero_mean,&data_opt_image);
    }
    nlopt_set_xtol_rel(opt_image,1e-4);
    nlopt_set_ftol_rel(opt_image,1e-4);
    double step[3]={1e1,1e1,0.1*M_PI/180.0};
    nlopt_set_initial_step(opt_image,step);
    double x_ctf[3]={data_opt->ctf_para_avg.getDefocus1()*1e10,data_opt->ctf_para_avg.getDefocus2()*1e10,data_opt->ctf_para_avg.getAstigmatism()};
    // double x_ctf[3]={ctf_para_scaling.getDefocus1()*1e10,ctf_para_scaling.getDefocus2()*1e10,ctf_para_scaling.getAstigmatism()};
    double cc_max;  // 单位: (A,A,rad)
    if(nlopt_optimize(opt_image,x_ctf,&cc_max)<0)
    {
        x_ctf[0]=data_opt->ctf_para_avg.getDefocus1()*1e10;
        x_ctf[1]=data_opt->ctf_para_avg.getDefocus2()*1e10;
        x_ctf[2]=data_opt->ctf_para_avg.getAstigmatism();
        // x_ctf[0]=ctf_para_scaling.getDefocus1()*1e10;
        // x_ctf[1]=ctf_para_scaling.getDefocus2()*1e10;
        // x_ctf[2]=ctf_para_scaling.getAstigmatism();
    }
    CTF ctf_para_now=data_opt->ctf_para_avg;
    // CTF ctf_para_now=ctf_para_scaling;
    ctf_para_now.setAllCTFPara(x_ctf[0],x_ctf[1],x_ctf[2]*180.0/M_PI,0,ctf_para_now.getW());
    float df_refine=(ctf_para_now.getDefocus1()+ctf_para_now.getDefocus2()+(ctf_para_now.getDefocus1()-ctf_para_now.getDefocus2())*cos(2*(-ctf_para_now.getAstigmatism())))/2;
    double chi_min=M_PI*ctf_para_now.getLambda()*data_opt->df_ref*(data_opt->res_min*data_opt->res_min)-M_PI_2*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*double(data_opt->res_min*data_opt->res_min)*double(data_opt->res_min*data_opt->res_min)+ctf_para_now.getW_phase();
    double chi_max=M_PI*ctf_para_now.getLambda()*data_opt->df_ref*(data_opt->res_max*data_opt->res_max)-M_PI_2*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*double(data_opt->res_max*data_opt->res_max)*double(data_opt->res_max*data_opt->res_max)+ctf_para_now.getW_phase();

    // 二维互相关, 平均功率谱
    float *psd_avg=new float [data_opt->box*data_opt->box];
    if(data_opt->optimize_with_avg)
    {
        get_radial_average_psd(psd_now,psd_avg,data_opt->box,ctf_para_now,data_opt->res2_all,data_opt->atan_all,data_opt->x_fft_1d_res);
    }
    else
    {
        memcpy(psd_avg,psd_now,sizeof(float)*data_opt->box*data_opt->box);
    }

    float *psd_ctf=new float[data_opt->box*data_opt->box];
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        psd_ctf[i]=0.0;
    }
    int *mask=new int[data_opt->box*data_opt->box];
    #pragma omp parallel for num_threads(data_opt->threads)
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        double df=(ctf_para_now.getDefocus1()+ctf_para_now.getDefocus2()+(ctf_para_now.getDefocus1()-ctf_para_now.getDefocus2())*cos(2*(data_opt->atan_all[i]-ctf_para_now.getAstigmatism())))/2;
        double chi=M_PI*ctf_para_now.getLambda()*df*data_opt->res2_all[i]-M_PI_2*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*(double(data_opt->res2_all[i])*double(data_opt->res2_all[i]))+ctf_para_now.getW_phase();
        if(chi>=chi_min && chi<=chi_max)
        {
            double ctf=sin(chi);
            psd_ctf[i]=ctf*ctf;
            mask[i]=1;
        }
        else
        {
            mask[i]=0;
        }
    }

    // 零均值化
    float sum_ctf=0.0;
    float sum_psd=0.0;
    int count=0;
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        sum_ctf+=(psd_ctf[i]*float(mask[i]));
        sum_psd+=(psd_avg[i]*float(mask[i]));
        count+=mask[i];
    }
    float avg_ctf=sum_ctf/float(count);
    float avg_psd=sum_psd/float(count);
    #pragma omp parallel for num_threads(data_opt->threads)
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        psd_ctf[i]-=avg_ctf;
        psd_avg[i]-=avg_psd;
    }

    double nu=0.0;
    double de_1=0.0;
    double de_2=0.0;
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        nu=nu+(psd_ctf[i]*psd_avg[i])*float(mask[i]);
        de_1=de_1+(psd_ctf[i]*psd_ctf[i])*float(mask[i]);
        de_2=de_2+(psd_avg[i]*psd_avg[i])*float(mask[i]);
    }

    // cout << endl << "[TEST] nu = " << nu << " ; de_1 = " << de_1 << " ; de_2 = " << de_2 << endl;
    // cout << "[TEST] psd_avg = " << psd_avg[128250] << " ; psd_0 = " << data_opt->psd_all[20][136][128250] << endl << endl;

    delete [] mask;
    delete [] psd_ctf;
    delete [] psd_avg;
    delete [] psd_now;

    double cc=nu/sqrt(de_1*de_2);
    return cc;
}

double get_psi_resolution_all_omp(unsigned n,const double *x,double *grad,void *data)
{
    Data_opt_psi_resolution_all *data_opt=(Data_opt_psi_resolution_all*)data;
    float psi_now=x[0];

    // 平均功率谱的互相关最大化
    double *psd_sum_omp[data_opt->threads];
    for(int th=0;th<data_opt->threads;th++)
    {
        psd_sum_omp[th]=new double[data_opt->box*data_opt->box];
        for(int i=0;i<data_opt->box*data_opt->box;i++)
        {
            psd_sum_omp[th][i]=0.0;
        }
    }
    #pragma omp parallel for num_threads(data_opt->threads)
    for(int n=0;n<data_opt->Nz;n++)
    {
        float *psd_now=new float[data_opt->box*data_opt->box];
        float df=(data_opt->ctf_para[n].getDefocus1()+data_opt->ctf_para[n].getDefocus2()+(data_opt->ctf_para[n].getDefocus1()-data_opt->ctf_para[n].getDefocus2())*cos(2*(-data_opt->ctf_para[n].getAstigmatism())))/2;
        for(int i=0;i<data_opt->box*data_opt->box;i++)
        {
            psd_now[i]=0.0;
        }
        for(int m=0;m<data_opt->N_block;m++)
        {
            get_scaled_psd_all(data_opt->psd_all[n][m],psd_now,data_opt->block_x[m],data_opt->block_y[m],data_opt->box,df*1e10,data_opt->df_ref*1e10,data_opt->pix,psi_now,data_opt->theta[n],data_opt->Nx,data_opt->Ny);
            for(int i=0;i<data_opt->box*data_opt->box;i++)
            {
                psd_sum_omp[omp_get_thread_num()][i]+=double(psd_now[i]);
            }
        }
        delete [] psd_now;
    }
    float *psd_now=new float[data_opt->box*data_opt->box];
    #pragma omp parallel for num_threads(data_opt->threads)
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        double psd_sum_now=0.0;
        for(int th=0;th<data_opt->threads;th++)
        {
            psd_sum_now+=psd_sum_omp[th][i];
        }
        psd_now[i]=float(psd_sum_now/double(data_opt->N_block*data_opt->Nz));
        psd_now[i]=log(1.0+psd_now[i]);
    }
    for(int th=0;th<data_opt->threads;th++)
    {
        delete [] psd_sum_omp[th];
    }

    // box convolution
    if(data_opt->box_conv>0)
    {
        // get_psd_conv_fft(psd_now,data_opt->box,data_opt->box_conv,data_opt->plan_fft,data_opt->plan_ifft,data_opt->fft_buf_in,data_opt->fft_buf_out);
    }

    // 通过优化方法估背景
    float radial_now[data_opt->box/2-1];
    get_radial_average(psd_now,radial_now,data_opt->box,data_opt->ctf_para_avg,data_opt->res2_all,data_opt->atan_all,data_opt->x_fft_1d_res);
    double fit_now[data_opt->box/2-1];
    background_estimation(fit_now,radial_now,data_opt->box,data_opt->N_zeros,data_opt->ctf_para_avg);

    // 回到二维抠背景
    // background_subtraction(psd_now,fit_now,data_opt->box,data_opt->res2_all,data_opt->atan_all,data_opt->x_fft_1d_res,data_opt->ctf_para_avg);

    // 计算高频互相关，先优化一次CTF参数
    // 先再局域优化一次CTF
    Data_opt_epa data_opt_image;
    data_opt_image.box=data_opt->box;
    data_opt_image.ctf_para=data_opt->ctf_para_avg;
    data_opt_image.psd=psd_now;
    data_opt_image.res_min=data_opt->res_min;
    data_opt_image.res_max=data_opt->res_max;
    data_opt_image.chi_min=M_PI*data_opt->ctf_para_avg.getLambda()*data_opt->df_ref*(data_opt->res_min*data_opt->res_min)-M_PI_2*data_opt->ctf_para_avg.getCs()*(data_opt->ctf_para_avg.getLambda()*data_opt->ctf_para_avg.getLambda()*data_opt->ctf_para_avg.getLambda())*double(data_opt->res_min*data_opt->res_min)*double(data_opt->res_min*data_opt->res_min)+data_opt->ctf_para_avg.getW_phase();
    data_opt_image.chi_max=M_PI*data_opt->ctf_para_avg.getLambda()*data_opt->df_ref*(data_opt->res_max*data_opt->res_max)-M_PI_2*data_opt->ctf_para_avg.getCs()*(data_opt->ctf_para_avg.getLambda()*data_opt->ctf_para_avg.getLambda()*data_opt->ctf_para_avg.getLambda())*double(data_opt->res_max*data_opt->res_max)*double(data_opt->res_max*data_opt->res_max)+data_opt->ctf_para_avg.getW_phase();
    data_opt_image.res2_all=data_opt->res2_all;
    data_opt_image.atan_all=data_opt->atan_all;
    data_opt_image.x_fft_1d_res=data_opt->x_fft_1d_res;

    nlopt_opt opt_image;
    opt_image=nlopt_create(NLOPT_LN_NELDERMEAD,3);
    if(data_opt->optimize_with_avg)
    {
        nlopt_set_max_objective(opt_image,get_correlation_image_epa_zero_mean,&data_opt_image);
    }
    else
    {
        nlopt_set_max_objective(opt_image,get_correlation_image_zero_mean,&data_opt_image);
    }
    nlopt_set_xtol_rel(opt_image,1e-4);
    nlopt_set_ftol_rel(opt_image,1e-4);
    double step[3]={1e1,1e1,0.1*M_PI/180.0};
    nlopt_set_initial_step(opt_image,step);
    double x_ctf[3]={data_opt->ctf_para_avg.getDefocus1()*1e10,data_opt->ctf_para_avg.getDefocus2()*1e10,data_opt->ctf_para_avg.getAstigmatism()};
    double cc_max;  // 单位: (A,A,rad)
    if(nlopt_optimize(opt_image,x_ctf,&cc_max)<0)
    {
        x_ctf[0]=data_opt->ctf_para_avg.getDefocus1()*1e10;
        x_ctf[1]=data_opt->ctf_para_avg.getDefocus2()*1e10;
        x_ctf[2]=data_opt->ctf_para_avg.getAstigmatism();
    }
    CTF ctf_para_now=data_opt->ctf_para_avg;
    ctf_para_now.setAllCTFPara(x_ctf[0],x_ctf[1],x_ctf[2]*180.0/M_PI,0,ctf_para_now.getW());
    float df_refine=(ctf_para_now.getDefocus1()+ctf_para_now.getDefocus2()+(ctf_para_now.getDefocus1()-ctf_para_now.getDefocus2())*cos(2*(-ctf_para_now.getAstigmatism())))/2;
    double chi_min=M_PI*ctf_para_now.getLambda()*data_opt->df_ref*(data_opt->res_min*data_opt->res_min)-M_PI_2*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*double(data_opt->res_min*data_opt->res_min)*double(data_opt->res_min*data_opt->res_min)+ctf_para_now.getW_phase();
    double chi_max=M_PI*ctf_para_now.getLambda()*data_opt->df_ref*(data_opt->res_max*data_opt->res_max)-M_PI_2*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*double(data_opt->res_max*data_opt->res_max)*double(data_opt->res_max*data_opt->res_max)+ctf_para_now.getW_phase();

    // 二维互相关, 平均功率谱
    float *psd_avg=new float [data_opt->box*data_opt->box];
    if(data_opt->optimize_with_avg)
    {
        get_radial_average_psd(psd_now,psd_avg,data_opt->box,ctf_para_now,data_opt->res2_all,data_opt->atan_all,data_opt->x_fft_1d_res);
    }
    else
    {
        memcpy(psd_avg,psd_now,sizeof(float)*data_opt->box*data_opt->box);
    }

    float *psd_ctf=new float[data_opt->box*data_opt->box];
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        psd_ctf[i]=0.0;
    }
    int *mask=new int[data_opt->box*data_opt->box];
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        double df=(ctf_para_now.getDefocus1()+ctf_para_now.getDefocus2()+(ctf_para_now.getDefocus1()-ctf_para_now.getDefocus2())*cos(2*(data_opt->atan_all[i]-ctf_para_now.getAstigmatism())))/2;
        double chi=M_PI*ctf_para_now.getLambda()*df*data_opt->res2_all[i]-M_PI_2*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*(double(data_opt->res2_all[i])*double(data_opt->res2_all[i]))+ctf_para_now.getW_phase();
        if(chi>=chi_min && chi<=chi_max)
        {
            double ctf=sin(chi);
            psd_ctf[i]=ctf*ctf;
            mask[i]=1;
        }
        else
        {
            mask[i]=0;
        }
    }

    // 零均值化
    float sum_ctf=0.0;
    float sum_psd=0.0;
    int count=0;
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        sum_ctf+=(psd_ctf[i]*float(mask[i]));
        sum_psd+=(psd_avg[i]*float(mask[i]));
        count+=mask[i];
    }
    float avg_ctf=sum_ctf/float(count);
    float avg_psd=sum_psd/float(count);
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        psd_ctf[i]-=avg_ctf;
        psd_avg[i]-=avg_psd;
    }

    double nu=0.0;
    double de_1=0.0;
    double de_2=0.0;
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        nu=nu+(psd_ctf[i]*psd_avg[i])*float(mask[i]);
        de_1=de_1+(psd_ctf[i]*psd_ctf[i])*float(mask[i]);
        de_2=de_2+(psd_avg[i]*psd_avg[i])*float(mask[i]);
    }

    delete [] mask;
    delete [] psd_ctf;
    delete [] psd_avg;
    delete [] psd_now;

    double cc=nu/sqrt(de_1*de_2);
    return cc;
}

double get_psi_theta_resolution_all_omp(unsigned n,const double *x,double *grad,void *data)
{
    Data_opt_psi_resolution_all *data_opt=(Data_opt_psi_resolution_all*)data;
    float psi_now=x[0];
    float theta_offset_now=x[1];

    // 平均功率谱的互相关最大化
    double *psd_sum_omp[data_opt->threads];
    for(int th=0;th<data_opt->threads;th++)
    {
        psd_sum_omp[th]=new double[data_opt->box*data_opt->box];
        for(int i=0;i<data_opt->box*data_opt->box;i++)
        {
            psd_sum_omp[th][i]=0.0;
        }
    }
    #pragma omp parallel for num_threads(data_opt->threads)
    for(int n=0;n<data_opt->Nz;n++)
    {
        float *psd_now=new float[data_opt->box*data_opt->box];
        float df=(data_opt->ctf_para[n].getDefocus1()+data_opt->ctf_para[n].getDefocus2()+(data_opt->ctf_para[n].getDefocus1()-data_opt->ctf_para[n].getDefocus2())*cos(2*(-data_opt->ctf_para[n].getAstigmatism())))/2;
        for(int i=0;i<data_opt->box*data_opt->box;i++)
        {
            psd_now[i]=0.0;
        }
        for(int m=0;m<data_opt->N_block;m++)
        {
            get_scaled_psd_all(data_opt->psd_all[n][m],psd_now,data_opt->block_x[m],data_opt->block_y[m],data_opt->box,df*1e10,data_opt->df_ref*1e10,data_opt->pix,psi_now,data_opt->theta[n]+theta_offset_now,data_opt->Nx,data_opt->Ny);
            for(int i=0;i<data_opt->box*data_opt->box;i++)
            {
                psd_sum_omp[omp_get_thread_num()][i]+=double(psd_now[i]);
            }
        }
        delete [] psd_now;
    }
    float *psd_now=new float[data_opt->box*data_opt->box];
    #pragma omp parallel for num_threads(data_opt->threads)
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        double psd_sum_now=0.0;
        for(int th=0;th<data_opt->threads;th++)
        {
            psd_sum_now+=psd_sum_omp[th][i];
        }
        psd_now[i]=float(psd_sum_now/double(data_opt->N_block*data_opt->Nz));
        psd_now[i]=log(1.0+psd_now[i]);
    }
    for(int th=0;th<data_opt->threads;th++)
    {
        delete [] psd_sum_omp[th];
    }

    // box convolution
    if(data_opt->box_conv>0)
    {
        // get_psd_conv_fft(psd_now,data_opt->box,data_opt->box_conv,data_opt->plan_fft,data_opt->plan_ifft,data_opt->fft_buf_in,data_opt->fft_buf_out);
    }

    // 通过优化方法估背景
    float radial_now[data_opt->box/2-1];
    get_radial_average(psd_now,radial_now,data_opt->box,data_opt->ctf_para_avg,data_opt->res2_all,data_opt->atan_all,data_opt->x_fft_1d_res);
    double fit_now[data_opt->box/2-1];
    background_estimation(fit_now,radial_now,data_opt->box,data_opt->N_zeros,data_opt->ctf_para_avg);

    // 回到二维抠背景
    background_subtraction(psd_now,fit_now,data_opt->box,data_opt->res2_all,data_opt->atan_all,data_opt->x_fft_1d_res,data_opt->ctf_para_avg);

    // 计算高频互相关，先优化一次CTF参数
    // 先再局域优化一次CTF
    Data_opt_epa data_opt_image;
    data_opt_image.box=data_opt->box;
    data_opt_image.ctf_para=data_opt->ctf_para_avg;
    data_opt_image.psd=psd_now;
    data_opt_image.res_min=data_opt->res_min;
    data_opt_image.res_max=data_opt->res_max;
    data_opt_image.chi_min=M_PI*data_opt->ctf_para_avg.getLambda()*data_opt->df_ref*(data_opt->res_min*data_opt->res_min)-M_PI_2*data_opt->ctf_para_avg.getCs()*(data_opt->ctf_para_avg.getLambda()*data_opt->ctf_para_avg.getLambda()*data_opt->ctf_para_avg.getLambda())*double(data_opt->res_min*data_opt->res_min)*double(data_opt->res_min*data_opt->res_min)+data_opt->ctf_para_avg.getW_phase();
    data_opt_image.chi_max=M_PI*data_opt->ctf_para_avg.getLambda()*data_opt->df_ref*(data_opt->res_max*data_opt->res_max)-M_PI_2*data_opt->ctf_para_avg.getCs()*(data_opt->ctf_para_avg.getLambda()*data_opt->ctf_para_avg.getLambda()*data_opt->ctf_para_avg.getLambda())*double(data_opt->res_max*data_opt->res_max)*double(data_opt->res_max*data_opt->res_max)+data_opt->ctf_para_avg.getW_phase();
    data_opt_image.res2_all=data_opt->res2_all;
    data_opt_image.atan_all=data_opt->atan_all;
    data_opt_image.x_fft_1d_res=data_opt->x_fft_1d_res;

    nlopt_opt opt_image;
    opt_image=nlopt_create(NLOPT_LN_NELDERMEAD,3);
    if(data_opt->optimize_with_avg)
    {
        nlopt_set_max_objective(opt_image,get_correlation_image_epa_zero_mean,&data_opt_image);
    }
    else
    {
        nlopt_set_max_objective(opt_image,get_correlation_image_zero_mean,&data_opt_image);
    }
    nlopt_set_xtol_rel(opt_image,1e-4);
    nlopt_set_ftol_rel(opt_image,1e-4);
    double step[3]={1e1,1e1,0.1*M_PI/180.0};
    nlopt_set_initial_step(opt_image,step);
    double x_ctf[3]={data_opt->ctf_para_avg.getDefocus1()*1e10,data_opt->ctf_para_avg.getDefocus2()*1e10,data_opt->ctf_para_avg.getAstigmatism()};
    double cc_max;  // 单位: (A,A,rad)
    if(nlopt_optimize(opt_image,x_ctf,&cc_max)<0)
    {
        x_ctf[0]=data_opt->ctf_para_avg.getDefocus1()*1e10;
        x_ctf[1]=data_opt->ctf_para_avg.getDefocus2()*1e10;
        x_ctf[2]=data_opt->ctf_para_avg.getAstigmatism();
    }
    CTF ctf_para_now=data_opt->ctf_para_avg;
    ctf_para_now.setAllCTFPara(x_ctf[0],x_ctf[1],x_ctf[2]*180.0/M_PI,0,ctf_para_now.getW());
    float df_refine=(ctf_para_now.getDefocus1()+ctf_para_now.getDefocus2()+(ctf_para_now.getDefocus1()-ctf_para_now.getDefocus2())*cos(2*(-ctf_para_now.getAstigmatism())))/2;
    double chi_min=M_PI*ctf_para_now.getLambda()*data_opt->df_ref*(data_opt->res_min*data_opt->res_min)-M_PI_2*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*double(data_opt->res_min*data_opt->res_min)*double(data_opt->res_min*data_opt->res_min)+ctf_para_now.getW_phase();
    double chi_max=M_PI*ctf_para_now.getLambda()*data_opt->df_ref*(data_opt->res_max*data_opt->res_max)-M_PI_2*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*double(data_opt->res_max*data_opt->res_max)*double(data_opt->res_max*data_opt->res_max)+ctf_para_now.getW_phase();

    // 二维互相关, 平均功率谱
    float *psd_avg=new float [data_opt->box*data_opt->box];
    if(data_opt->optimize_with_avg)
    {
        get_radial_average_psd(psd_now,psd_avg,data_opt->box,ctf_para_now,data_opt->res2_all,data_opt->atan_all,data_opt->x_fft_1d_res);
    }
    else
    {
        memcpy(psd_avg,psd_now,sizeof(float)*data_opt->box*data_opt->box);
    }

    float *psd_ctf=new float[data_opt->box*data_opt->box];
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        psd_ctf[i]=0.0;
    }
    int *mask=new int[data_opt->box*data_opt->box];
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        double df=(ctf_para_now.getDefocus1()+ctf_para_now.getDefocus2()+(ctf_para_now.getDefocus1()-ctf_para_now.getDefocus2())*cos(2*(data_opt->atan_all[i]-ctf_para_now.getAstigmatism())))/2;
        double chi=M_PI*ctf_para_now.getLambda()*df*data_opt->res2_all[i]-M_PI_2*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*(double(data_opt->res2_all[i])*double(data_opt->res2_all[i]))+ctf_para_now.getW_phase();
        if(chi>=chi_min && chi<=chi_max)
        {
            double ctf=sin(chi);
            psd_ctf[i]=ctf*ctf;
            mask[i]=1;
        }
        else
        {
            mask[i]=0;
        }
    }

    // 零均值化
    float sum_ctf=0.0;
    float sum_psd=0.0;
    int count=0;
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        sum_ctf+=(psd_ctf[i]*float(mask[i]));
        sum_psd+=(psd_avg[i]*float(mask[i]));
        count+=mask[i];
    }
    float avg_ctf=sum_ctf/float(count);
    float avg_psd=sum_psd/float(count);
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        psd_ctf[i]-=avg_ctf;
        psd_avg[i]-=avg_psd;
    }

    double nu=0.0;
    double de_1=0.0;
    double de_2=0.0;
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        nu=nu+(psd_ctf[i]*psd_avg[i])*float(mask[i]);
        de_1=de_1+(psd_ctf[i]*psd_ctf[i])*float(mask[i]);
        de_2=de_2+(psd_avg[i]*psd_avg[i])*float(mask[i]);
    }

    delete [] mask;
    delete [] psd_ctf;
    delete [] psd_avg;
    delete [] psd_now;

    double cc=nu/sqrt(de_1*de_2);
    return cc;
}

double get_phi_resolution_all_omp(unsigned n,const double *x,double *grad,void *data)
{
    Data_opt_phi_resolution_all *data_opt=(Data_opt_phi_resolution_all*)data;
    float phi_now=x[0];

    // 平均功率谱的互相关最大化
    double *psd_sum_omp[data_opt->threads];
    for(int th=0;th<data_opt->threads;th++)
    {
        psd_sum_omp[th]=new double[data_opt->box*data_opt->box];
        for(int i=0;i<data_opt->box*data_opt->box;i++)
        {
            psd_sum_omp[th][i]=0.0;
        }
    }
    #pragma omp parallel for num_threads(data_opt->threads)
    for(int n=0;n<data_opt->Nz;n++)
    {
        float *psd_now=new float[data_opt->box*data_opt->box];
        float df=(data_opt->ctf_para[n].getDefocus1()+data_opt->ctf_para[n].getDefocus2()+(data_opt->ctf_para[n].getDefocus1()-data_opt->ctf_para[n].getDefocus2())*cos(2*(-data_opt->ctf_para[n].getAstigmatism())))/2;
        for(int i=0;i<data_opt->box*data_opt->box;i++)
        {
            psd_now[i]=0.0;
        }
        for(int m=0;m<data_opt->N_block;m++)
        {
            get_scaled_psd_all_phi(data_opt->psd_all[n][m],psd_now,data_opt->block_x[m],data_opt->block_y[m],data_opt->box,df*1e10,data_opt->df_ref*1e10,data_opt->pix,data_opt->psi,data_opt->theta[n],phi_now,data_opt->Nx,data_opt->Ny);
            for(int i=0;i<data_opt->box*data_opt->box;i++)
            {
                psd_sum_omp[omp_get_thread_num()][i]+=double(psd_now[i]);
            }
        }
        delete [] psd_now;
    }
    float *psd_now=new float[data_opt->box*data_opt->box];
    #pragma omp parallel for num_threads(data_opt->threads)
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        double psd_sum_now=0.0;
        for(int th=0;th<data_opt->threads;th++)
        {
            psd_sum_now+=psd_sum_omp[th][i];
        }
        psd_now[i]=float(psd_sum_now/double(data_opt->N_block*data_opt->Nz));
        psd_now[i]=log(1.0+psd_now[i]);
    }
    for(int th=0;th<data_opt->threads;th++)
    {
        delete [] psd_sum_omp[th];
    }

    // box convolution
    if(data_opt->box_conv>0)
    {
        // get_psd_conv_fft(psd_now,data_opt->box,data_opt->box_conv,data_opt->plan_fft,data_opt->plan_ifft,data_opt->fft_buf_in,data_opt->fft_buf_out);
    }

    // 通过优化方法估背景
    float radial_now[data_opt->box/2-1];
    get_radial_average(psd_now,radial_now,data_opt->box,data_opt->ctf_para_avg,data_opt->res2_all,data_opt->atan_all,data_opt->x_fft_1d_res);
    double fit_now[data_opt->box/2-1];
    background_estimation(fit_now,radial_now,data_opt->box,data_opt->N_zeros,data_opt->ctf_para_avg);

    // 回到二维抠背景
    // background_subtraction(psd_now,fit_now,data_opt->box,data_opt->res2_all,data_opt->atan_all,data_opt->x_fft_1d_res,data_opt->ctf_para_avg);

    // 计算高频互相关，先优化一次CTF参数
    // 先再局域优化一次CTF
    Data_opt_epa data_opt_image;
    data_opt_image.box=data_opt->box;
    data_opt_image.ctf_para=data_opt->ctf_para_avg;
    data_opt_image.psd=psd_now;
    data_opt_image.res_min=data_opt->res_min;
    data_opt_image.res_max=data_opt->res_max;
    data_opt_image.chi_min=M_PI*data_opt->ctf_para_avg.getLambda()*data_opt->df_ref*(data_opt->res_min*data_opt->res_min)-M_PI_2*data_opt->ctf_para_avg.getCs()*(data_opt->ctf_para_avg.getLambda()*data_opt->ctf_para_avg.getLambda()*data_opt->ctf_para_avg.getLambda())*double(data_opt->res_min*data_opt->res_min)*double(data_opt->res_min*data_opt->res_min)+data_opt->ctf_para_avg.getW_phase();
    data_opt_image.chi_max=M_PI*data_opt->ctf_para_avg.getLambda()*data_opt->df_ref*(data_opt->res_max*data_opt->res_max)-M_PI_2*data_opt->ctf_para_avg.getCs()*(data_opt->ctf_para_avg.getLambda()*data_opt->ctf_para_avg.getLambda()*data_opt->ctf_para_avg.getLambda())*double(data_opt->res_max*data_opt->res_max)*double(data_opt->res_max*data_opt->res_max)+data_opt->ctf_para_avg.getW_phase();
    data_opt_image.res2_all=data_opt->res2_all;
    data_opt_image.atan_all=data_opt->atan_all;
    data_opt_image.x_fft_1d_res=data_opt->x_fft_1d_res;

    nlopt_opt opt_image;
    opt_image=nlopt_create(NLOPT_LN_NELDERMEAD,3);
    if(data_opt->optimize_with_avg)
    {
        nlopt_set_max_objective(opt_image,get_correlation_image_epa_zero_mean,&data_opt_image);
    }
    else
    {
        nlopt_set_max_objective(opt_image,get_correlation_image_zero_mean,&data_opt_image);
    }
    nlopt_set_xtol_rel(opt_image,1e-4);
    nlopt_set_ftol_rel(opt_image,1e-4);
    double step[3]={1e1,1e1,0.1*M_PI/180.0};
    nlopt_set_initial_step(opt_image,step);
    double x_ctf[3]={data_opt->ctf_para_avg.getDefocus1()*1e10,data_opt->ctf_para_avg.getDefocus2()*1e10,data_opt->ctf_para_avg.getAstigmatism()};
    double cc_max;  // 单位: (A,A,rad)
    if(nlopt_optimize(opt_image,x_ctf,&cc_max)<0)
    {
        x_ctf[0]=data_opt->ctf_para_avg.getDefocus1()*1e10;
        x_ctf[1]=data_opt->ctf_para_avg.getDefocus2()*1e10;
        x_ctf[2]=data_opt->ctf_para_avg.getAstigmatism();
    }
    CTF ctf_para_now=data_opt->ctf_para_avg;
    ctf_para_now.setAllCTFPara(x_ctf[0],x_ctf[1],x_ctf[2]*180.0/M_PI,0,ctf_para_now.getW());
    float df_refine=(ctf_para_now.getDefocus1()+ctf_para_now.getDefocus2()+(ctf_para_now.getDefocus1()-ctf_para_now.getDefocus2())*cos(2*(-ctf_para_now.getAstigmatism())))/2;
    double chi_min=M_PI*ctf_para_now.getLambda()*data_opt->df_ref*(data_opt->res_min*data_opt->res_min)-M_PI_2*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*double(data_opt->res_min*data_opt->res_min)*double(data_opt->res_min*data_opt->res_min)+ctf_para_now.getW_phase();
    double chi_max=M_PI*ctf_para_now.getLambda()*data_opt->df_ref*(data_opt->res_max*data_opt->res_max)-M_PI_2*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*double(data_opt->res_max*data_opt->res_max)*double(data_opt->res_max*data_opt->res_max)+ctf_para_now.getW_phase();

    // 二维互相关, 平均功率谱
    float *psd_avg=new float [data_opt->box*data_opt->box];
    if(data_opt->optimize_with_avg)
    {
        get_radial_average_psd(psd_now,psd_avg,data_opt->box,ctf_para_now,data_opt->res2_all,data_opt->atan_all,data_opt->x_fft_1d_res);
    }
    else
    {
        memcpy(psd_avg,psd_now,sizeof(float)*data_opt->box*data_opt->box);
    }

    float *psd_ctf=new float[data_opt->box*data_opt->box];
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        psd_ctf[i]=0.0;
    }
    int *mask=new int[data_opt->box*data_opt->box];
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        double df=(ctf_para_now.getDefocus1()+ctf_para_now.getDefocus2()+(ctf_para_now.getDefocus1()-ctf_para_now.getDefocus2())*cos(2*(data_opt->atan_all[i]-ctf_para_now.getAstigmatism())))/2;
        double chi=M_PI*ctf_para_now.getLambda()*df*data_opt->res2_all[i]-M_PI_2*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*(double(data_opt->res2_all[i])*double(data_opt->res2_all[i]))+ctf_para_now.getW_phase();
        if(chi>=chi_min && chi<=chi_max)
        {
            double ctf=sin(chi);
            psd_ctf[i]=ctf*ctf;
            mask[i]=1;
        }
        else
        {
            mask[i]=0;
        }
    }

    // 零均值化
    float sum_ctf=0.0;
    float sum_psd=0.0;
    int count=0;
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        sum_ctf+=(psd_ctf[i]*float(mask[i]));
        sum_psd+=(psd_avg[i]*float(mask[i]));
        count+=mask[i];
    }
    float avg_ctf=sum_ctf/float(count);
    float avg_psd=sum_psd/float(count);
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        psd_ctf[i]-=avg_ctf;
        psd_avg[i]-=avg_psd;
    }

    double nu=0.0;
    double de_1=0.0;
    double de_2=0.0;
    for(int i=0;i<data_opt->box*data_opt->box;i++)
    {
        nu=nu+(psd_ctf[i]*psd_avg[i])*float(mask[i]);
        de_1=de_1+(psd_ctf[i]*psd_ctf[i])*float(mask[i]);
        de_2=de_2+(psd_avg[i]*psd_avg[i])*float(mask[i]);
    }

    delete [] mask;
    delete [] psd_ctf;
    delete [] psd_avg;
    delete [] psd_now;

    double cc=nu/sqrt(de_1*de_2);
    return cc;
}


void get_psd_fit_show_image(float *psd_avg,float *psd_show,int box,CTF ctf_para_avg,float res_min,float res_max,float *res2_all,float *atan_all)
{
    memcpy(psd_show,psd_avg,sizeof(float)*box*box);
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            if(sqrt(res2_all[j*box+i])<res_min)
            {
                psd_show[j*box+i]=0.0;
            }
        }
    }
    double psd_sum=0.0;
    double psd_sum2=0.0;
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            // if(sqrt(x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j])>=res_min && sqrt(x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j])<=res_max)
            {
                psd_sum+=psd_show[j*box+i];
                psd_sum2+=(psd_show[j*box+i]*psd_show[j*box+i]);
            }
        }
    }
    double psd_mean=psd_sum/double(box*box);
    double psd_var=psd_sum2/double(box*box)-psd_mean*psd_mean;
    for(int i=0;i<box*box;i++)
    {
        if(psd_show[i]>float(psd_mean+sqrt(psd_var)))
        {
            psd_show[i]=float(psd_mean+sqrt(psd_var));
        }
        else if(psd_show[i]<float(psd_mean-sqrt(psd_var)))
        {
            psd_show[i]=float(psd_mean-sqrt(psd_var));
        }
    }
    float psd_min=psd_show[0];
    for(int i=1;i<box*box;i++)
    {
        if(psd_min>psd_show[i])
        {
            psd_min=psd_show[i];
        }
    }
    for(int i=0;i<box*box;i++)
    {
        psd_show[i]-=psd_min;
    }
    float psd_max=psd_show[0];
    for(int i=1;i<box*box;i++)
    {
        if(psd_max<psd_show[i])
        {
            psd_max=psd_show[i];
        }
    }
    for(int i=0;i<box*box;i++)
    {
        psd_show[i]/=psd_max;
    }
    /*
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            if(sqrt(x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j])<res_min)
            {
                psd_show[j*box+i]=0.0;
            }
            /*
            else if(sqrt(x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j])>1.0/2.0/ctf_para_avg.getPixelSize())
            {
                psd_show[j*box+i]=0.0;
            }
            */
    /*
        }
    }
    */

    for(int j=box/2;j<box;j++)
    {
        for(int i=0;i<=box/2-1;i++)
        {
            if(sqrt(res2_all[j*box+i])>=res_min && sqrt(res2_all[j*box+i])<=res_max)
            {
                float df=(ctf_para_avg.getDefocus1()+ctf_para_avg.getDefocus2()+(ctf_para_avg.getDefocus1()-ctf_para_avg.getDefocus2())*cos(2*(atan_all[j*box+i]-ctf_para_avg.getAstigmatism())))/2;
                // double res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
                double chi=M_PI*ctf_para_avg.getLambda()*df*res2_all[j*box+i]-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(double(res2_all[j*box+i])*double(res2_all[j*box+i]))+ctf_para_avg.getPhaseShift();
                float ctf=ctf_para_avg.getW()*cos(chi)+sqrt(1-ctf_para_avg.getW()*ctf_para_avg.getW())*sin(chi);
                psd_show[j*box+i]=abs(ctf);
            }
        }
    }
}


void get_psd_show_image(float *psd_avg,float *psd_show,int box,float res_min,float res_max,float *res2_all,float *atan_all)
{
    memcpy(psd_show,psd_avg,sizeof(float)*box*box);
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            if(sqrt(res2_all[j*box+i])<res_min)
            {
                psd_show[j*box+i]=0.0;
            }
        }
    }
    double psd_sum=0.0;
    double psd_sum2=0.0;
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            // if(sqrt(x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j])>=res_min && sqrt(x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j])<=res_max)
            {
                psd_sum+=psd_show[j*box+i];
                psd_sum2+=(psd_show[j*box+i]*psd_show[j*box+i]);
            }
        }
    }
    double psd_mean=psd_sum/double(box*box);
    double psd_var=psd_sum2/double(box*box)-psd_mean*psd_mean;
    for(int i=0;i<box*box;i++)
    {
        if(psd_show[i]>float(psd_mean+sqrt(psd_var)))
        {
            psd_show[i]=float(psd_mean+sqrt(psd_var));
        }
        else if(psd_show[i]<float(psd_mean-sqrt(psd_var)))
        {
            psd_show[i]=float(psd_mean-sqrt(psd_var));
        }
    }
    float psd_min=psd_show[0];
    for(int i=1;i<box*box;i++)
    {
        if(psd_min>psd_show[i])
        {
            psd_min=psd_show[i];
        }
    }
    for(int i=0;i<box*box;i++)
    {
        psd_show[i]-=psd_min;
    }
    float psd_max=psd_show[0];
    for(int i=1;i<box*box;i++)
    {
        if(psd_max<psd_show[i])
        {
            psd_max=psd_show[i];
        }
    }
    for(int i=0;i<box*box;i++)
    {
        psd_show[i]/=psd_max;
    }
}

void get_psd_show_image_2(float *psd_avg,float *psd_show,int box,float res_min,float res_max,float *res2_all,float *atan_all)
{
    memcpy(psd_show,psd_avg,sizeof(float)*box*box);
    /*
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            if(sqrt(res2_all[j*box+i])<res_min)
            {
                psd_show[j*box+i]=0.0;
            }
        }
    }
    */
    double psd_sum=0.0;
    double psd_sum2=0.0;
    int psd_count=0;
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            if(sqrt(res2_all[j*box+i])>=res_min && sqrt(res2_all[j*box+i])<=res_max)
            {
                psd_sum+=psd_show[j*box+i];
                psd_sum2+=(psd_show[j*box+i]*psd_show[j*box+i]);
                psd_count++;
            }
        }
    }
    double psd_mean=psd_sum/double(psd_count);
    double psd_var=psd_sum2/double(psd_count)-psd_mean*psd_mean;
    for(int i=0;i<box*box;i++)
    {
        if(psd_show[i]>float(psd_mean+sqrt(psd_var)))
        {
            psd_show[i]=float(psd_mean+sqrt(psd_var));
        }
        else if(psd_show[i]<float(psd_mean-sqrt(psd_var)))
        {
            psd_show[i]=float(psd_mean-sqrt(psd_var));
        }
    }
    float psd_min=psd_show[0];
    for(int i=1;i<box*box;i++)
    {
        if(psd_min>psd_show[i])
        {
            psd_min=psd_show[i];
        }
    }
    for(int i=0;i<box*box;i++)
    {
        psd_show[i]-=psd_min;
    }
    float psd_max=psd_show[0];
    for(int i=1;i<box*box;i++)
    {
        if(psd_max<psd_show[i])
        {
            psd_max=psd_show[i];
        }
    }
    for(int i=0;i<box*box;i++)
    {
        psd_show[i]/=psd_max;
    }
}

void get_one_block_defocus_epa_omp(float *psd_orig,int block_x,int block_y,float *block_z,bool *block_avail,float *block_dz,int box,float pix,float psi,float theta,float phi,int Nx,int Ny,int N_zeros,CTF ctf_para,float res_min,float res_max,double chi_min,double chi_max,int box_conv,int m,float *res2_all,float *chi_all,float *atan_all,float *x_fft_1d_res,fftwf_plan plan_fft,fftwf_plan plan_ifft,float *fft_buf_in,float *fft_buf_out)  // 切小块求平均功率谱，采用BSoft中的尺度变换统一零点位置，频域直接放缩 （block_z单位为A！！）
{
    // 单位换算到国际单位
    pix=pix*1e-10;  // A --> m
    // df=df*1e-10;    // A --> m

    float *psd=new float[box*box];
    memcpy(psd,psd_orig,sizeof(float)*box*box);

    int x_center=Nx/2;  // box的中心坐标
    int y_center=Ny/2;

    float v_x=float(block_x-x_center);
    float v_y=float(block_y-y_center);
    float nv_x=cos(-psi*M_PI/180.0);
    float nv_y=-sin(-psi*M_PI/180.0);
    float dist_pix=v_x*nv_x+v_y*nv_y;
    float dz=dist_pix*tan(theta*M_PI/180.0)*pix;
    float tv_x=sin(-psi*M_PI/180.0);
    float tv_y=cos(-psi*M_PI/180.0);
    float trans_pix=v_x*tv_x+v_y*tv_y;
    float dh=trans_pix*tan(phi*M_PI/180.0)*pix;

    CTF ctf_para_orig=ctf_para;
    ctf_para.setAllCTFPara((ctf_para.getDefocus1()+dz+dh)*1e10,(ctf_para.getDefocus2()+dz+dh)*1e10,ctf_para.getAstigmatism()*180.0/M_PI,ctf_para.getPhaseShift(),ctf_para.getW());
    // ctf_para.setAllCTFPara((ctf_para.getDefocus1())*1e10,(ctf_para.getDefocus2())*1e10,ctf_para.getAstigmatism()*180.0/M_PI,ctf_para.getPhaseShift(),ctf_para.getW());

    // box convolution
    if(box_conv>0)
    {
        // get_psd_conv_fft(psd,box,box_conv,plan_fft,plan_ifft,fft_buf_in,fft_buf_out);
    }

    // 估背景
    float radial_now[box/2-1];
    // get_radial_average(psd,radial_now,box,ctf_para,res2_all,atan_all,x_fft_1d_res);
    get_radial_average(psd,radial_now,box,ctf_para_orig,res2_all,atan_all,x_fft_1d_res);
    double fit_now[box/2-1];
    // background_estimation(fit_now,radial_now,box,N_zeros,ctf_para);
    background_estimation(fit_now,radial_now,box,N_zeros,ctf_para_orig);

    // 回到二维抠背景
    // background_subtraction(psd,fit_now,box,res2_all,atan_all,x_fft_1d_res,ctf_para);
    background_subtraction(psd,fit_now,box,res2_all,atan_all,x_fft_1d_res,ctf_para_orig);

    // Nelder-Mead算法局部精优化
    Data_opt_dz_epa data_opt_image;
    data_opt_image.box=box;
    data_opt_image.ctf_para=ctf_para_orig;
    data_opt_image.psd=psd;
    data_opt_image.res_min=res_min;
    data_opt_image.res_max=res_max;
    data_opt_image.chi_min=chi_min;
    data_opt_image.chi_max=chi_max;
    data_opt_image.res2_all=res2_all;
    data_opt_image.chi_all=chi_all;
    data_opt_image.atan_all=atan_all;
    data_opt_image.x_fft_1d_res=x_fft_1d_res;
    
    nlopt_opt opt_image;
    opt_image=nlopt_create(NLOPT_LN_NELDERMEAD,1);
    nlopt_set_max_objective(opt_image,get_correlation_astig_dz_epa_zero_mean,&data_opt_image);
    nlopt_set_xtol_abs1(opt_image,1e-4);
    // nlopt_set_ftol_rel(opt_image,1e-4);
    double step[1]={2.0};
    // double step[1]={1.0};
    nlopt_set_initial_step(opt_image,step);
    double x[1]={-1.0};
    // double x[1]={(dz+dh)*1e10-1.0};
    // double x[1]={0.0};
    double cc_max;  // 单位: (A,A,rad)
    if(nlopt_optimize(opt_image,x,&cc_max)<0)   // NLOPT failed
    {
        x[0]=0.0;
    }

    // x[0]=0.0;
    
    block_z[m]=(dz+dh)*1e10+x[0];
    block_dz[m]=x[0];

    // block_z[m]=x[0];
    // block_dz[m]=x[0]-(dz+dh)*1e10;

    // block_z[m]=(ctf_para_orig.getDefocus1()+ctf_para_orig.getDefocus2()+(ctf_para_orig.getDefocus1()-ctf_para_orig.getDefocus2())*cos(2*(-ctf_para_orig.getAstigmatism())))/2*1e10+x[0]+dz*1e10;
    // block_z[m]=(ctf_para_orig.getDefocus1()+ctf_para_orig.getDefocus2()+(ctf_para_orig.getDefocus1()-ctf_para_orig.getDefocus2())*cos(2*(-ctf_para_orig.getAstigmatism())))/2*1e10+dz*1e10;

    block_avail[m]=1;
    // // if(abs(x[0])<2e1)
    // if(abs(x[0])<1e1/cos(theta*M_PI/180.0)/cos(theta*M_PI/180.0))
    // {
    //     block_avail[m]=1;
    // }
    // else
    // {
    //     block_avail[m]=0;
    //     // block_avail[m]=1;
    //     // block_z[m]=(ctf_para_orig.getDefocus1()+ctf_para_orig.getDefocus2()+(ctf_para_orig.getDefocus1()-ctf_para_orig.getDefocus2())*cos(2*(-ctf_para_orig.getAstigmatism())))/2*1e10+dz*1e10;
    //     // block_z[m]=dz*1e10;
    // }

    delete [] psd;
}


void get_psd_center_padding(float *psd_now,float *psd_padding,int box,fftwf_plan plan_fft,fftwf_plan plan_ifft,float *fft_buf_in,float *fft_buf_out)    // 要求box为4的倍数！！！
{
    // 取原功率谱中间低频部分
    float *psd_center=new float[(box/2)*(box/2)];
    int t=0;
    for(int j=box/4;j<box/4*3;j++)
    {
        for(int i=box/4;i<box/4*3;i++)
        {
            psd_center[t]=psd_now[j*box+i];
            t++;
        }
    }

    // 正向傅里叶变换，box/2大小
    for(int i=0;i<(box*2)*(box/2);i++)
    {
        fft_buf_in[i*2]=psd_center[i];
        fft_buf_in[i*2+1]=0.0;
    }
    fftwf_execute(plan_fft);

    // 高频zero padding
    float *psd_fft_padding_shift=new float[box*box*2];
    for(int i=0;i<box*box*2;i++)
    {
        psd_fft_padding_shift[i]=0.0;
    }
    t=0;
    for(int j=box/4;j<box/4*3;j++)
    {
        for(int i=box/4;i<box/4*3;i++)
        {
            psd_fft_padding_shift[j*box*2+i*2]=fft_buf_out[t*2];
            psd_fft_padding_shift[j*box*2+i*2+1]=fft_buf_out[t*2+1];
            t++;
        }
    }
    float *psd_fft_padding=new float[box*box*2];
    ifftshift_2d_complex(psd_fft_padding_shift,psd_fft_padding,box,box);

    // 逆变换得到加密结果
    memcpy(fft_buf_in,psd_fft_padding,sizeof(float)*box*box*2);
    fftwf_execute(plan_ifft);
    for(int i=0;i<box*box;i++)
    {
        psd_padding[i]=fft_buf_out[i*2]/float(box*box);
    }

    delete [] psd_fft_padding_shift;
    delete [] psd_fft_padding;
    delete [] psd_center;
}



double loss_theta_offset(unsigned n,const double *x,double *grad,void *data_in)
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





CTFAlgo_v3::~CTFAlgo_v3()
{
}

void CTFAlgo_v3::doCTF(map<string, string> &inputPara, map<string, string> &outputPara)
{
    // cout<<"Run doCTF() in CTFAlgo(V3)"<<endl;

    bool log_avail=false;

    time_t current_time;
    time(&current_time);
    char time_buf[1024];
    strftime(time_buf,1024,"%a %b %d %T %Y",localtime(&current_time));
    cout << endl << "[" << time_buf << "] " << "################ Read parameters ################" << endl;

    // Input
    time(&current_time);
    strftime(time_buf,1024,"%a %b %d %T %Y",localtime(&current_time));
    cout << endl << "[" << time_buf << "] " << "######## Basic parameters ########" << endl;

    map<string,string>::iterator it=inputPara.find("workpath");
    string path;
    if(it!=inputPara.end())
    {
        path=it->second;
        cout << "Work path: " << path << endl;
    }
    else
    {
        char *tmp;
        tmp=getcwd(NULL,0);
        path=tmp;
        free(tmp);
        cout << "Work path: {Current path} = " << path << endl;
        path=".";
    }

    it=inputPara.find("single_particle");
    bool single_particle;
    if(it!=inputPara.end())
    {
        single_particle=atoi(it->second.c_str());
    }

    string s;
    int Nz=0;
    vector<float> theta_all;
    vector<float> psi_all;
    vector<string> micrograph_name_all;
    vector<int> micrograph_num_all;
    string input_list;
    string input_micrograph;
    if(single_particle)
    {
        cout << "Estimate single particle micrograph!" << endl;
        
        Nz=1;

        it=inputPara.find("input_micrograph");
        if(it!=inputPara.end())
        {
            input_micrograph=it->second;
            cout << "Input micrograph: " << input_micrograph << endl;
            micrograph_name_all.push_back(input_micrograph);
            micrograph_num_all.push_back(0);
        }
        else
        {
            cerr << "[Error] No input micrograph file!" << endl;
            abort();
        }

        it=inputPara.find("theta");
        if(it!=inputPara.end())
        {
            float theta_now=atof(it->second.c_str());
            cout << "Tilt angle: " << theta_now << endl;
            theta_all.push_back(theta_now);
        }
        else
        {
            cout << "[Warning] No tilt angle, set 0 by default!" << endl;
            theta_all.push_back(0);
        }

        it=inputPara.find("psi");
        if(it!=inputPara.end())
        {
            float psi_now=atof(it->second.c_str());
            cout << "Tilt axis angle: " << psi_now << endl;
            psi_all.push_back(psi_now);
        }
        else
        {
            cout << "[Warning] No tilt axis angle, set 0 by default!" << endl;
            psi_all.push_back(0);
        }
    }
    else
    {
        it=inputPara.find("input_micrograph_list");
        if(it!=inputPara.end())
        {
            // input_list=path+"/"+it->second;
            input_list=it->second;
            cout << "Input micrograph list: " << input_list << endl;
        }
        else
        {
            cerr << "[Error] No input micrograph list!" << endl;
            abort();
        }
        ifstream fin(input_list,ios::in);
        if(!fin)
        {
            cerr << "[Error] Cannot open input micrograph list!" << endl;
            abort();
        }
        while(getline(fin,s))
        {
            if(s[0]=='#')
            {
                continue;
            }
            if(s[0]=='[')
            {
                continue;
            }
            if(s[0]=='\0')
            {
                continue;
            }
            Nz++;
            for(int t=0;t<3;t++)
            {
                size_t pos=s.find_first_of(" ");
                if(pos==string::npos && t<2)
                {
                    cerr << "[Error] Wrong format for input micrograph list!" << endl;
                    cerr << "# Column 1: Frame number@Micrograph name" << endl;
                    cerr << "# Column 2: Tilt angle (in degree)" << endl;
                    cerr << "# Column 3: Tilt axis angle (in degree)" << endl;
                    cerr << "e.g." << endl;
                    cerr << "0@tomo_001.ali -60.000000 175.000000" << endl;
                    cerr << "1@tomo_001.ali -57.000000 175.000000" << endl;
                    cerr << "..." << endl;
                    cerr << "40@tomo_001.ali 60.000000 175.000000" << endl;
                    abort();
                }
                string current=s.substr(0,pos);
                trim(current);
                s=s.substr(pos+1);
                if(t==0)
                {
                    pos=current.find_first_of("@");
                    string num_now=current.substr(0,pos);
                    string name_now=current.substr(pos+1);
                    trim(num_now);
                    trim(name_now);
                    micrograph_name_all.push_back(name_now);
                    micrograph_num_all.push_back(atoi(num_now.c_str()));
                }
                else if(t==1)
                {
                    float theta_now=atof(current.c_str());
                    theta_all.push_back(theta_now);
                }
                else
                {
                    float psi_now=atof(current.c_str());
                    psi_all.push_back(psi_now);
                }
            }
        }
    }

    it=inputPara.find("tlt_offset");
    float tlt_offset=0.0;
    if(it!=inputPara.end())
    {
        tlt_offset=atof(it->second.c_str());
        cout << "Absolute tilt angle offset: " << tlt_offset << endl;
    }
    else
    {
        cout << "Absolute tilt angle offset: 0.0" << endl;
        tlt_offset=0.0;
    }

    CTF ctf_para[Nz];
    CTF ctf_para_avg;
    float Cs,pix,volt,w_cos;
    it=inputPara.find("Cs");
    if(it!=inputPara.end())
    {
        Cs=atof(it->second.c_str());
        cout << "Cs (mm): " << Cs << endl;
    }
    else
    {
        cerr << "[Error] No Cs!" << endl;
        abort();
    }
    it=inputPara.find("pixel_size");
    if(it!=inputPara.end())
    {
        pix=atof(it->second.c_str());
        cout << "Pixel size (A): " << pix << endl;
    }
    else
    {
        cerr << "[Error] No pixel size!" << endl;
        abort();
    }
    it=inputPara.find("voltage");
    if(it!=inputPara.end())
    {
        volt=atof(it->second.c_str());
        cout << "Accelerating voltage (kV): " << volt << endl;
    }
    else
    {
        cerr << "[Error] No accelerating voltage!" << endl;
        abort();
    }
    it=inputPara.find("w");
    if(it!=inputPara.end())
    {
        w_cos=atof(it->second.c_str());
        cout << "Amplitude contrast: " << w_cos << endl;
    }
    else
    {
        cerr << "[Error] No amplitude contrast!" << endl;
        abort();
    }
    for(int n=0;n<Nz;n++)
    {
        ctf_para[n].setAllImagePara(pix,volt,Cs);
        ctf_para[n].setAllCTFPara(0,0,0,0,w_cos);
    }
    ctf_para_avg.setAllImagePara(pix,volt,Cs);
    ctf_para_avg.setAllCTFPara(0,0,0,0,w_cos);

    float psi_sum=0.0;
    float psi_sum2=0.0;
    float theta[Nz];
    float theta_min=90.0;
    int theta_min_index=0;
    float theta_min_orig=90.0;
    int theta_min_index_orig=0;
    for(int n=0;n<Nz;n++)
    {
        psi_sum+=psi_all[n];
        psi_sum2+=(psi_all[n]*psi_all[n]);
        theta[n]=theta_all[n];
        if(fabs(theta[n])<theta_min_orig)
        {
            theta_min_orig=fabs(theta[n]);
            theta_min_index_orig=n;
        }
        theta[n]+=tlt_offset;
        // theta[n]=-theta[n]; // 手性相反
        if(fabs(theta[n])<theta_min)
        {
            theta_min=fabs(theta[n]);
            theta_min_index=n;
        }
    }
    float psi=psi_sum/float(Nz);
    float psi_std=sqrt(psi_sum2/float(Nz)-psi*psi);
    cout << "Psi: " << psi << endl;
    cout << "Standard deviation of psi: " << psi_std << endl;
    if(psi_std>10)
    {
        cout << "[Warning] The standard deviation of tilt axis angle is larger than 10 degrees, which may cause problems in angle estimation! (CTFMeasure estimates only one tilt axis angle for the entire tilt-series.)" << endl;
    }

    /*
    it=inputPara.find("input_mrc");
    string input_mrc;
    if(it!=inputPara.end())
    {
        input_mrc=path+"/"+it->second;
        cout << "Input file name: " << input_mrc << endl;
    }
    else
    {
        cerr << "No input file name!" << endl;
        abort();
    }
    MRC stack_orig(input_mrc.c_str(),"rb");
    if(!stack_orig.hasFile())
    {
        cerr << "Cannot open input mrc stack!" << endl;
        abort();
    }
    */

    /*
    it=inputPara.find("psi");
    float psi=0.0;
    if(it!=inputPara.end())
    {
        psi=atof(it->second.c_str());
        cout << "psi: " << psi << endl;
    }
    else
    {
        cout << "No tilt axis angle psi, set default: 0" << endl;
        psi=0.0;
    }
    */

    /*
    it=inputPara.find("input_tlt");
    string input_tlt;
    float theta[Nz];
    float theta_min=90.0;
    int theta_min_index=0;
    float theta_min_orig=90.0;
    int theta_min_index_orig=0;
    if(it!=inputPara.end())
    {
        input_tlt=path+"/"+it->second;
        cout << "Input tilt angle file name: " << input_tlt << endl;
        FILE *ftlt=fopen(input_tlt.c_str(),"r");
        if(ftlt==NULL)
        {
            cerr << "Cannot open tlt file!" << endl;
            abort();
        }
        for(int n=0;n<Nz;n++)
        {
            fscanf(ftlt,"%f",&theta[n]);
            if(fabs(theta[n])<theta_min_orig)
            {
                theta_min_orig=fabs(theta[n]);
                theta_min_index_orig=n;
            }
            theta[n]+=tlt_offset;
            // theta[n]=-theta[n]; // 手性相反
            if(fabs(theta[n])<theta_min)
            {
                theta_min=fabs(theta[n]);
                theta_min_index=n;
            }
        }
        fflush(ftlt);
        fclose(ftlt);
    }
    else
    {
        cout << "No input tilt angle file name, set all tilt angles to 0!" << endl;
        for(int n=0;n<Nz;n++)
        {
            theta[n]=0.0;
        }
        theta_min=0.0;
        theta_min_orig=0.0;
        theta_min_index=Nz/2;
        theta_min_index_orig=Nz/2;
    }
    */

    it=inputPara.find("prfx");
    string prfx;
    if(it!=inputPara.end())
    {
        prfx=it->second;
        cout << "Prefix: " << prfx << endl;
    }
    else
    {
        map<string,string>::iterator it_tmp;
        if(single_particle)
        {
            it_tmp=inputPara.find("input_micrograph");
        }
        else
        {
            it_tmp=inputPara.find("input_micrograph_list");
        }
        string tmp=it_tmp->second;
        size_t t=tmp.find_last_of(".");
        tmp=tmp.substr(0,t);
        trim(tmp);
        t=tmp.find_last_of("/");
        prfx=tmp.substr(t+1);
        trim(prfx);
        cout << "Prefix: " << prfx << endl;
    }

    it=inputPara.find("average_mrc");
    string average_mrc;
    if(it!=inputPara.end())
    {
        if(it->second.substr(0,1)=="/")
        {
            average_mrc=it->second;
        }
        else
        {
            average_mrc=path+"/"+it->second;
        }
        // cout << "Average spectrum name: " << average_mrc << endl;
        cout << "Average spectrum name: " << it->second << endl;
    }
    else
    {
        average_mrc=path+"/"+prfx+"_avg.mrc";
        // cout << "Average spectrum name: " << average_mrc << endl;
        cout << "Average spectrum name: " << prfx+"_avg.mrc" << endl;
    }
    it=inputPara.find("output_mrc");
    string output_mrc;
    if(it!=inputPara.end())
    {
        if(it->second.substr(0,1)=="/")
        {
            output_mrc=it->second;
        }
        else
        {
            output_mrc=path+"/"+it->second;
        }
        // cout << "Output spectrum name: " << output_mrc << endl;
        cout << "Output spectrum name: " << it->second << endl;
    }
    else
    {
        output_mrc=path+"/"+prfx+"_diag.st";
        // cout << "Output spectrum name: " << output_mrc << endl;
        cout << "Output spectrum name: " << prfx+"_diag.st" << endl;
    }

    it=inputPara.find("output_file");
    string output_file;
    if(it!=inputPara.end())
    {
        if(it->second.substr(0,1)=="/")
        {
            output_file=it->second;
        }
        else
        {
            output_file=path+"/"+it->second;
        }
        // cout << "Output file name: " << output_file << endl;
        cout << "Output file name: " << it->second << endl;
    }
    else
    {
        output_file=path+"/"+prfx+"_ctf.txt";
        // cout << "Output file name: " << output_file << endl;
        cout << "Output file name: " << prfx+"_ctf.txt" << endl;
    }

    /*
    string output_mrc_raw=output_mrc+".raw";
    string output_mrc_conv=output_mrc+".conv";
    string output_mrc_combine=output_mrc+".combine";
    it=inputPara.find("output_tlt");
    string output_tlt;
    if(it!=inputPara.end())
    {
        output_tlt=path+"/"+it->second;
        cout << "Output tilt file name: " << output_tlt << endl;
    }
    else
    {
        output_tlt=path+"/"+prfx+"_ctf_theta.tlt";
        cout << "No output tilt file name, set default: " << output_tlt << endl;
    }

    it=inputPara.find("defocus_file");
    string defocus_file;
    if(it!=inputPara.end())
    {
        defocus_file=path+"/"+it->second;
        cout << "Defocus file name: " << defocus_file << endl;
    }
    else
    {
        cout << "No defocus file name, set default: defocus_file.txt" << endl;
        defocus_file="defocus_file.txt";
    }
    */

    it=inputPara.find("j");
    int threads;
    if(it!=inputPara.end())
    {
        threads=atoi(it->second.c_str());
        cout << "Threads: " << threads << endl;
    }
    else
    {
        cout << "Threads: 10" << endl;
        threads=10;
    }

    cout << endl << "#### Input micrograph list ####" << endl;
    for(int n=0;n<Nz;n++)
    {
        // cout << micrograph_num_all[n] << "@" << micrograph_name_all[n] << " " << theta_all[n] << " " << psi_all[n] << endl;
        printf("%02d@%s %7.2f %7.2f\n",micrograph_num_all[n],micrograph_name_all[n].c_str(),theta_all[n],psi_all[n]);
    }
    // cout << endl;


    time(&current_time);
    strftime(time_buf,1024,"%a %b %d %T %Y",localtime(&current_time));
    cout << endl << "[" << time_buf << "] " << "######## Parameters for estimation ########" << endl;

    it=inputPara.find("N_zeros");
    int N_zeros=10;
    if(it!=inputPara.end())
    {
        N_zeros=atoi(it->second.c_str());
        cout << "Number of CTF zeros to fit: " << N_zeros << endl;
    }
    else
    {
        cout << "Number of CTF zeros to fit: 10" << endl;
        N_zeros=10;
    }
    
    int box;
    float res_max,res_min;
    float df_max,df_min,df_step;
    int res_ada=0;
    it=inputPara.find("box");
    if(it!=inputPara.end())
    {
        box=atoi(it->second.c_str());
        if(box%2==1)  // 默认为偶数
        {
            box++;
        }
        cout << "Box size: " << box << endl;
    }
    else
    {
        cout << "Box size: 512" << endl;
        box=512;
    }
    it=inputPara.find("resolution_max");
    if(it!=inputPara.end())
    {
        res_max=atof(it->second.c_str());
        cout << "Maximum resolution (A): " << res_max << endl;
    }
    else
    {
        cout << "Maximum resolution (A): 4*{pixel_size} = " << 4*pix << endl;
        res_max=4*pix;
    }
    it=inputPara.find("resolution_min");
    if(it!=inputPara.end())
    {
        res_min=atof(it->second.c_str());
        cout << "Minimum resolution (A): " << res_min << endl;
    }
    else
    {
        cout << "Minimum resolution (A): 30" << endl;
        res_min=30.0;
    }
    if(res_max>res_min)
    {
        float tmp=res_max;
        res_max=res_min;
        res_min=tmp;
        cout << "[Warning] Maximum resolution is smaller than minimum resolution! Interchange these two parameters for estimation." << endl;
        cout << "Minimum resolution after modification (A): " << res_min << endl;
        cout << "Maximum resolution after modification (A): " << res_max << endl;
    }
    res_min=1/(res_min*1e-10);  // 单位A^-1换算成m
    res_max=1/(res_max*1e-10);
    float res_max_orig=res_max;
    float res_min_orig=res_min;
    float res_max_2=res_max;
    float res_min_2=res_min;
    it=inputPara.find("defocus_min");
    if(it!=inputPara.end())
    {
        df_min=atof(it->second.c_str());
        cout << "Minimum defocus (A): " << df_min << endl;
    }
    else
    {
        cout << "Minimum defocus (A): 10000" << endl;
        df_min=10000.0;
    }
    it=inputPara.find("defocus_max");
    if(it!=inputPara.end())
    {
        df_max=atof(it->second.c_str());
        cout << "Maximum defocus (A): " << df_max << endl;
    }
    else
    {
        cout << "Maximum defocus (A): 100000" << endl;
        df_max=100000.0;
    }
    if(df_max<df_min)
    {
        float tmp=df_max;
        df_max=df_min;
        df_min=tmp;
        cout << "[Warning] Maximum defocus is smaller than minimum defocus! Interchange these two parameters for estimation." << endl;
        cout << "Maximum defocus after modification (A): " << df_max << endl;
        cout << "Minimum defocus after modification (A): " << df_min << endl;
    }
    it=inputPara.find("defocus_step");
    if(it!=inputPara.end())
    {
        df_step=atof(it->second.c_str());
        cout << "Defocus searching step (A): " << df_step << endl;
    }
    else
    {
        cout << "Defocus searching step (A): 100" << endl;
        df_step=100.0;
    }
    it=inputPara.find("resolution_adaptive");
    if(it!=inputPara.end())
    {
        res_ada=atoi(it->second.c_str());
        cout << "Using adaptive resolution range for searching: " << res_ada << endl;
    }
    else
    {
        cout << "Using adaptive resolution range for searching: 1" << endl;
        res_ada=1;
    }

    bool adaptive_range=1;
    it=inputPara.find("adaptive_range");
    if(it!=inputPara.end())
    {
        adaptive_range=atoi(it->second.c_str());
        cout << "Apply adaptive fitting range adjustment: " << adaptive_range << endl;
    }
    else
    {
        cout << "Apply adaptive fitting range adjustment: 1" << endl;
        adaptive_range=1;
    }

    int N_it=3;
    it=inputPara.find("it");
    if(it!=inputPara.end())
    {
        N_it=atoi(it->second.c_str());
        cout << "Number of iterations: " << N_it << endl;
    }
    else
    {
        N_it=3;
        cout << "Number of iterations: 3" << endl;
    }


    time(&current_time);
    strftime(time_buf,1024,"%a %b %d %T %Y",localtime(&current_time));
    cout << endl << "[" << time_buf << "] " << "######## Advanced parameters ########" << endl;

    int N_avg=0;
    it=inputPara.find("N_avg");
    if(it!=inputPara.end())
    {
        N_avg=atoi(it->second.c_str());
        cout << "Number of adjacent micrographs averaging for coarse search: " << N_avg << endl;
    }
    else
    {
        cout << "Number of adjacent micrographs averaging for coarse search: 0" << endl;
        N_avg=0;
    }

    it=inputPara.find("box_conv");
    int box_conv=box/20;
    if(it!=inputPara.end())
    {
        box_conv=atoi(it->second.c_str());
        cout << "Box size for box-convolution: " << box_conv << endl;
    }
    else
    {
        box_conv=box/20;
        /*
        if(is_resampling)
        {
            box_conv=box_conv*2;
        }
        */
        cout << "Box size for box-convolution: {box} / 20 = " << box_conv << endl;
    }
    // box_conv=0;

    /*
    it=inputPara.find("skip_3dctf");
    bool skip_3dctf;
    if(it!=inputPara.end())
    {
        skip_3dctf=atoi(it->second.c_str());
        cout << "Skip 3D-CTF estimation: " << skip_3dctf << endl;
    }
    else
    {
        cout << "Skip 3D-CTF estimation (default): 1" << endl;
        skip_3dctf=1;
    }

    it=inputPara.find("h");
    int h=0;
    if(it!=inputPara.end())
    {
        h=atoi(it->second.c_str());
        cout << "Height (Pixels): " << h << endl;
    }
    else
    {
        cout << "No height infomation, skip 3D-CTF estimation" << endl;
        skip_3dctf=1;
        h=0;
    }
    */

    it=inputPara.find("search_phase_shift");
    bool search_phase_shift=0;
    if(it!=inputPara.end())
    {
        search_phase_shift=atoi(it->second.c_str());
        cout << "Search for phase shift (using phase plate): " << search_phase_shift << endl;
    }
    else
    {
        cout << "Search for phase shift (using phase plate): 0" << endl;
        search_phase_shift=0;
    }

    it=inputPara.find("N_ref");
    int N_ref=theta_min_index_orig;
    if(it!=inputPara.end())
    {
        N_ref=atoi(it->second.c_str());
        cout << "Reference for coarse search: " << N_ref << endl;
    }
    else
    {
        N_ref=theta_min_index_orig;
        cout << "Reference for coarse search: " << N_ref << " the micrograph with minimum tilt angle" << endl;
    }

    it=inputPara.find("resampling");
    bool is_resampling=false;
    if(it!=inputPara.end())
    {
        is_resampling=atoi(it->second.c_str());
        cout << "Perform resampling: " << is_resampling << endl;
        if(is_resampling)
        {
            pix*=2;
            cout << "Perform resampling, modified pixel size (A): " << pix << endl;
        }
    }
    else
    {
        cout << "Performing resampling: 0" << endl;
        is_resampling=false;
    }

    bool box_conv_first=1;
    it=inputPara.find("box_conv_first");
    if(it!=inputPara.end())
    {
        box_conv_first=atoi(it->second.c_str());
        cout << "Box convolution before background subtraction: " << box_conv_first << endl;
    }
    else
    {
        cout << "Box convolution before background subtraction: 1" << endl;
        box_conv_first=1;
    }

    bool skip_box_conv_coarse=0;
    it=inputPara.find("skip_box_conv_coarse");
    if(it!=inputPara.end())
    {
        skip_box_conv_coarse=atoi(it->second.c_str());
        cout << "Skip box convolution in coarse estimation: " << skip_box_conv_coarse << endl;
    }
    else
    {
        cout << "Skip box convolution in coarse estimation: 0" << endl;
        skip_box_conv_coarse=0;
    }

    bool hard_restriction=0;
    it=inputPara.find("hard_restriction");
    if(it!=inputPara.end())
    {
        hard_restriction=atoi(it->second.c_str());
        cout << "Hard restriction for per-tilt refinement: " << hard_restriction << endl;
    }
    else
    {
        cout << "Hard restriction for per-tilt refinement: 0" << endl;
        hard_restriction=0;
    }

    bool loose_restriction=0;
    it=inputPara.find("loose_restriction");
    if(it!=inputPara.end())
    {
        loose_restriction=atoi(it->second.c_str());
        cout << "Loose restriction for per-tilt refinement: " << loose_restriction << endl;
    }
    else
    {
        cout << "No parameter for loose restriction, set default: 0" << endl;
        loose_restriction=0;
    }

    float astig_angstrom=2e3;
    it=inputPara.find("astigmatism_angstrom");
    if(it!=inputPara.end())
    {
        astig_angstrom=atof(it->second.c_str());
        cout << "Expected astigmatism (A): " << astig_angstrom << endl;
    }
    else
    {
        cout << "Expected astigmatism (A): 2000" << endl;
        astig_angstrom=2e3;
    }

    bool skip_unified=0;
    it=inputPara.find("skip_unified");
    if(it!=inputPara.end())
    {
        skip_unified=atoi(it->second.c_str());
        cout << "Skip unified CTF estimation: " << skip_unified << endl;
    }
    else
    {
        cout << "Skip unified CTF estimation: 0" << endl;
        skip_unified=0;
    }

    bool optimize_with_avg=0;
    it=inputPara.find("optimize_with_average");
    if(it!=inputPara.end())
    {
        optimize_with_avg=atoi(it->second.c_str());
        cout << "Local optimization with average PSD: " << optimize_with_avg << endl;
    }
    else
    {
        cout << "Local optimization with average PSD: 0" << endl;
        optimize_with_avg=0;
    }

    bool per_tilt_estimation=0;
    it=inputPara.find("per_tilt_estimation");
    if(it!=inputPara.end())
    {
        per_tilt_estimation=atoi(it->second.c_str());
        cout << "Estimate per-tilt independently: " << per_tilt_estimation << endl;
    }
    else
    {
        cout << "Estimate per-tilt independently: 0" << endl;
        per_tilt_estimation=0;
    }

    bool tight_blocks=0;
    it=inputPara.find("tight_blocks");
    if(it!=inputPara.end())
    {
        tight_blocks=atoi(it->second.c_str());
        cout << "PSD estimation with a tight region (neglect the outermost blocks): " << tight_blocks << endl;
    }
    else
    {
        cout << "PSD estimation with a tight region (neglect the outermost blocks): 0" << endl;
        tight_blocks=0;
    }

    bool dose_weighting=0;
    it=inputPara.find("dose_weighting");
    if(it!=inputPara.end())
    {
        dose_weighting=atoi(it->second.c_str());
        cout << "Perform dose weighting during estimaton: " << dose_weighting << endl;
    }
    else
    {
        cout << "Perform dose weighting during estimaton: 0" << endl;
        dose_weighting=0;
    }

    it=inputPara.find("dose_acc_file");
    string dose_acc_file;
    bool flag_dose_acc_file=false;
    if(it!=inputPara.end())
    {
        flag_dose_acc_file=true;
        if(it->second.substr(0,1)=="/")
        {
            dose_acc_file=it->second;
        }
        else
        {
            dose_acc_file=path+"/"+it->second;
        }
        cout << "Input accumulated dose file name: " << it->second << endl;
    }
    else
    {
        cout << "Input accumulated dose file name: " << endl;
    }

    float dose_acc_all[Nz];
    for(int n=0;n<Nz;n++)
    {
        dose_acc_all[n]=0.0;
    }
    if(flag_dose_acc_file==true)
    {
        FILE *fdose=fopen(dose_acc_file.c_str(),"r");
        if(fdose==NULL)
        {
            cerr << "[Error] Cannot open input accumulated dose file! Skip dose weighting!" << endl;
        }
        else
        {
            for(int n=0;n<Nz;n++)
            {
                fscanf(fdose,"%f",&dose_acc_all[n]);
            }
            fflush(fdose);
            fclose(fdose);
        }
    }
    else if(dose_weighting && !flag_dose_acc_file)
    {
        cerr << "[Error] No input accumulated dose file specified! Skip dose weighting!" << endl;
        dose_weighting=false;
    }

    float convergence_df=100.0;
    float convergence_astig=1.0;
    float convergence_angle=0.1;
    it=inputPara.find("convergence_defocus");
    if(it!=inputPara.end())
    {
        convergence_df=atof(it->second.c_str());
        cout << "Convergence criteria for defocus values (A): " << convergence_df << endl;
    }
    else
    {
        cout << "Convergence criteria for defocus values (A): 100" << endl;
        convergence_df=100.0;
    }
    it=inputPara.find("convergence_astigmatism");
    if(it!=inputPara.end())
    {
        convergence_astig=atof(it->second.c_str());
        cout << "Convergence criteria for astigmatism angle (degree): " << convergence_astig << endl;
    }
    else
    {
        cout << "Convergence criteria for astigmatism angle (degree): 1" << endl;
        convergence_astig=1.0;
    }
    it=inputPara.find("convergence_angle");
    if(it!=inputPara.end())
    {
        convergence_angle=atof(it->second.c_str());
        cout << "Convergence criteria for angle estimation (A): " << convergence_angle << endl;
    }
    else
    {
        cout << "Convergence criteria for angle estimation (A): 0.1" << endl;
        convergence_angle=0.1;
    }

    bool output_raw=false;
    it=inputPara.find("output_raw");
    if(it!=inputPara.end())
    {
        output_raw=atoi(it->second.c_str());
        cout << "Write out power spectrums before background subtraction: " << output_raw << endl;
    }
    else
    {
        cout << "Write out power spectrums before background subtraction: 0" << endl;
        output_raw=0;
    }

    time(&current_time);
    strftime(time_buf,1024,"%a %b %d %T %Y",localtime(&current_time));
    cout << endl << "[" << time_buf << "] " << "######## Parameters for angle estimation ########" << endl;

    int N_avg_block=2;
    it=inputPara.find("N_avg_block");
    if(it!=inputPara.end())
    {
        N_avg_block=atoi(it->second.c_str());
        cout << "Number of blocks to average in angle refinement: " << N_avg_block << endl;
    }
    else
    {
        N_avg_block=2;
        cout << "Number of blocks to average in angle refinement: 2" << endl;
    }

    bool pre_offset_estimation=0;
    it=inputPara.find("pre_offset_estimation");
    if(it!=inputPara.end())
    {
        pre_offset_estimation=atoi(it->second.c_str());
        cout << "Estimate tilt offset before CTF estimation: " << pre_offset_estimation << endl;
    }
    else
    {
        cout << "Estimate tilt offset before CTF estimation: 0" << endl;
        pre_offset_estimation=0;
    }

    it=inputPara.find("dose_file");
    string dose_file;
    bool flag_dose_file=false;
    if(it!=inputPara.end())
    {
        flag_dose_file=true;
        if(it->second.substr(0,1)=="/")
        {
            dose_file=it->second;
        }
        else
        {
            dose_file=path+"/"+it->second;
        }
        cout << "Input dose file name: " << it->second << endl;
    }
    else
    {
        cout << "Input dose file name: " << endl;
    }

    float dose_all[Nz];
    for(int n=0;n<Nz;n++)
    {
        dose_all[n]=1.0;
    }
    if(flag_dose_file==true)
    {
        FILE *fdose=fopen(dose_file.c_str(),"r");
        if(fdose==NULL)
        {
            cerr << "[Error] Cannot open input dose file! Skip dose normalization for coarse tilt offset estimation!" << endl;
        }
        else
        {
            for(int n=0;n<Nz;n++)
            {
                fscanf(fdose,"%f",&dose_all[n]);
            }
            fflush(fdose);
            fclose(fdose);
        }
    }

    bool skip_offset_estimation=1;
    it=inputPara.find("skip_offset_estimation");
    if(it!=inputPara.end())
    {
        skip_offset_estimation=atoi(it->second.c_str());
        cout << "Skip tilt offset estimation during CTF estimation: " << skip_offset_estimation << endl;
    }
    else
    {
        cout << "Skip tilt offset estimation during CTF estimation: 1" << endl;
        skip_offset_estimation=1;
    }

    bool skip_offset_refinement=1;
    it=inputPara.find("skip_offset_refinement");
    if(it!=inputPara.end())
    {
        skip_offset_refinement=atoi(it->second.c_str());
        cout << "Skip tilt offset refinement during CTF estimation: " << skip_offset_refinement << endl;
    }
    else
    {
        cout << "Skip tilt offset refinement during CTF estimation: 1" << endl;
        skip_offset_refinement=1;
    }
    
    bool skip_offset_estimation_x=1;
    it=inputPara.find("skip_offset_estimation_x");
    if(it!=inputPara.end())
    {
        skip_offset_estimation_x=atoi(it->second.c_str());
        cout << "Skip tilt offset estimation for x-axis: " << skip_offset_estimation_x << endl;
    }
    else
    {
        cout << "Skip tilt offset estimation for x-axis: 1" << endl;
        skip_offset_estimation_x=1;
    }

    bool skip_offset_refinement_x=1;
    it=inputPara.find("skip_offset_refinement_x");
    if(it!=inputPara.end())
    {
        skip_offset_refinement_x=atoi(it->second.c_str());
        cout << "Skip tilt offset refinement for x-axis: " << skip_offset_refinement_x << endl;
    }
    else
    {
        cout << "Skip tilt offset refinement for x-axis: 1" << endl;
        skip_offset_refinement_x=1;
    }

    bool flag_phi_initial=false;
    float phi_initial=0.0;
    it=inputPara.find("xtilt");
    if(it!=inputPara.end())
    {
        phi_initial=atof(it->second.c_str());
        flag_phi_initial=true;
        cout << "Initial tilt offset for x-axis: " << phi_initial << endl;
    }
    else
    {
        cout << "Initial tilt offset for x-axis: 0.0" << endl;
        flag_phi_initial=false;
        phi_initial=0.0;
    }

    bool skip_axis_refinement=1;
    it=inputPara.find("skip_axis_refinement");
    if(it!=inputPara.end())
    {
        skip_axis_refinement=atoi(it->second.c_str());
        cout << "Skip tilt axis refinement: " << skip_axis_refinement << endl;
    }
    else
    {
        cout << "Skip tilt axis refinement: 1" << endl;
        skip_axis_refinement=1;
    }

    bool skip_angle_refinement=1;
    it=inputPara.find("skip_angle_refinement");
    if(it!=inputPara.end())
    {
        skip_angle_refinement=atoi(it->second.c_str());
        cout << "Skip tilt angle refinement: " << skip_angle_refinement << endl;
    }
    else
    {
        cout << "Skip tilt angle refinement: 1" << endl;
        skip_angle_refinement=1;
    }



    int Nx,Ny;
    float *image_all[Nz];
    MRC stack_now;
    for(int n=0;n<Nz;n++)
    {
        // string micrograph_name_now=path+"/"+micrograph_name_all[n];
        string micrograph_name_now=micrograph_name_all[n];
        if(micrograph_name_now.substr(0,1)!="/")    // 相对路径
        {
            micrograph_name_now=path+"/"+micrograph_name_all[n];
        }
        stack_now.open(micrograph_name_now.c_str(),"rb");
        if(!stack_now.hasFile())
        {
            cerr << "[Error] Cannot open input micrograph " << micrograph_name_all[n] << "!" << endl;
            abort();
        }
        image_all[n]=new float[stack_now.getNx()*stack_now.getNy()];
        stack_now.read2DIm_32bit(image_all[n],micrograph_num_all[n]);
        if(n==0)
        {
            Nx=stack_now.getNx();
            Ny=stack_now.getNy();
        }
        else
        {
            if(Nx!=stack_now.getNx() || Ny!=stack_now.getNy())
            {
                cerr << "[Error] The sizes of the input micrographs are different! Cannot perform CTF estimation on this dataset!" << endl;
                abort();
            }
        }
        stack_now.close();
    }

    time(&current_time);
    strftime(time_buf,1024,"%a %b %d %T %Y",localtime(&current_time));
    cout << endl << "[" << time_buf << "] " << "################ Start estimation ################" << endl;

    if(pre_offset_estimation)
    {
        // compute average density
        cout << endl << "Estimate absolute tilt angle offset before CTF estimation" << endl;
        cout << endl << "Compute average density:" << endl;
        float avg[Nz];
        #pragma omp parallel for num_threads(threads)
        for(int n=0;n<Nz;n++)
        {
            double sum=0.0;
            for(int i=0;i<Nx*Ny;i++)
            {
                sum+=image_all[n][i];
            }
            avg[n]=sum/double(Nx*Ny);
        }
        for(int n=0;n<Nz;n++)
        {
            cout << n << " : " << theta[n] << " , " << avg[n] << endl;
        }
        if(flag_dose_file)
        {
            cout << endl << "Normalize average density with dose:" << endl;
            #pragma omp parallel for num_threads(threads)
            for(int n=0;n<Nz;n++)
            {
                avg[n]=avg[n]/dose_all[n];
            }
            for(int n=0;n<Nz;n++)
            {
                cout << n << " : " << theta[n] << " , " << dose_all[n] << " , " << avg[n] << endl;
            }
        }

        // Least-Square fit with quadratic polynomial
        cout << endl << "Least-Square fit with quadratic polynomial:" << endl;
        Eigen::MatrixXf A(Nz,3);
        Eigen::VectorXf b(Nz);
        for(int n=0;n<Nz;n++)
        {
            b(n)=avg[n];
            A(n,0)=1.0;
            A(n,1)=theta[n]*M_PI/180.0;
            A(n,2)=(theta[n]*M_PI/180.0)*(theta[n]*M_PI/180.0);
        }
        Eigen::MatrixXf ATA(3,3);
        Eigen::VectorXf ATb(3);
        ATA=A.transpose()*A;
        ATb=A.transpose()*b;
        Eigen::VectorXf x(3);
        x=ATA.inverse()*ATb;
        cout << "Result (y-density; x-theta (in radius)): y = " << x(0) << " + " << x(1) << " * x + " << x(2) << " * x ^ 2" << endl;
        // cout << "Estimated global tilt offset: " << (-x(1)/(2*x(2)))*180.0/M_PI << endl;
        double theta_offset=(-x(1)/(2*x(2)))*180.0/M_PI;
        printf("Estimated global tilt offset: %7.2f\n",theta_offset);
        for(int n=0;n<Nz;n++)   // update tilt angle
        {
            theta[n]-=theta_offset;
        }

        /*
        // Further optimization with non-linear least-square
        cout << endl << "Further optimization with non-linear least-square:" << endl;
        double *data_in[3];
        data_in[0]=new double[1];
        data_in[0][0]=Nz;
        data_in[1]=new double[Nz];
        data_in[2]=new double[Nz];
        double I_min=avg[0],I_max=avg[0];
        for(int n=0;n<Nz;n++)
        {
            data_in[1][n]=avg[n];
            data_in[2][n]=theta[n]*M_PI/180.0;
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
        nlopt_set_min_objective(opt,loss_theta_offset,data_in);
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
            printf("Found minimum at f(%f,%f,%f,%f) = %f\n",para[0],para[1],para[2],para[3],minf);
            theta_offset=para[3];
        }
        theta_offset=theta_offset*180.0/M_PI;   // change back to degree
        cout << endl << "Optimized global tilt offset: " << theta_offset << endl;

        for(int n=0;n<Nz;n++)   // update tilt angle
        {
            theta[n]-=theta_offset;
        }

        delete [] data_in[0];
        delete [] data_in[1];
        delete [] data_in[2];
        */
    }


    if(per_tilt_estimation)
    {
        if(search_phase_shift)    // Search for phase shift (using phase plate)
        {
            cout << endl << "Search for phase shift (using phase plate), we can not apply unified CTF estimation, as phase shift varies through the tilt-seires!" << endl << endl;

            int x_fft[box],y_fft[box];
            float x_fft_res[box],y_fft_res[box];
            for(int i=0;i<box;i++)
            {
                x_fft[i]=i-box/2;
                y_fft[i]=i-box/2;
                x_fft_res[i]=float(x_fft[i])/float(box)/ctf_para_avg.getPixelSize();
                y_fft_res[i]=float(y_fft[i])/float(box)/ctf_para_avg.getPixelSize();
            }
            float x_fft_1d_res[box/2-1];
            for(int i=0;i<box/2-1;i++)
            {
                x_fft_1d_res[i]=float(i)/float(box)/ctf_para_avg.getPixelSize();
            }
            float *res2_all=new float[box*box];
            for(int j=0;j<box;j++)
            {
                for(int i=0;i<box;i++)
                {
                    res2_all[j*box+i]=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
                }
            }
            float *atan_all=new float[box*box];
            for(int j=0;j<box;j++)
            {
                for(int i=0;i<box;i++)
                {
                    atan_all[j*box+i]=atan2(y_fft_res[j],x_fft_res[i]);
                }
            }

            MRC stack_psd(output_mrc.c_str(),"w");
            if(!stack_psd.hasFile())
            {
                cerr << "[Error] Cannot open output file " << output_mrc << "!" << endl;
                abort();
            }
            stack_psd.createMRC_empty(box,box,Nz,2);

            int N_block_x,N_block_y;
            int block_x_min,block_x_max;
            int block_y_min,block_y_max;
            int N_block;
            if(tight_blocks)
            {
                N_block_x=Nx/box*2-2;
                N_block_y=Ny/box*2-2;
                N_block=get_blocks_tight(box,&N_block_x,&N_block_y,&block_x_min,&block_x_max,&block_y_min,&block_y_max,Nx,Ny);
            }
            else
            {
                N_block_x=Nx/box*2;
                N_block_y=Ny/box*2;
                N_block=get_blocks(box,&N_block_x,&N_block_y,&block_x_min,&block_x_max,&block_y_min,&block_y_max,Nx,Ny);
            }
            int *block_x=new int[N_block];
            int *block_y=new int[N_block];
            get_block_coords(block_x,block_y,box,block_x_min,block_x_max,block_y_min,block_y_max,N_block_x,N_block_y,Nx,Ny);
            if(tight_blocks)
            {
                cout << "Number of blocks: " << N_block << " = (" << N_block_x+2 << " - 2) * (" << N_block_y+2 << " - 2)" << endl;
            }
            else
            {
                cout << "Number of blocks: " << N_block << " = " << N_block_x << " * " << N_block_y << endl;
            }

            double *psd_avg_omp[threads];
            float *fft_buf_in_omp[threads];
            float *fft_buf_out_omp[threads];
            fftwf_plan plan_fft_omp[threads];
            fftwf_plan plan_fft_omp_half[threads];
            fftwf_plan plan_ifft_omp[threads];
            fftwf_plan plan_ifft_omp_half[threads];
            for(int th=0;th<threads;th++)
            {
                psd_avg_omp[th]=new double[box*box];
                fft_buf_in_omp[th]=new float[box*box*2];
                fft_buf_out_omp[th]=new float[box*box*2];
                for(int i=0;i<box*box;i++)
                {
                    psd_avg_omp[th][i]=0.0;
                    fft_buf_in_omp[th][2*i]=0.0;
                    fft_buf_in_omp[th][2*i+1]=0.0;
                    fft_buf_out_omp[th][2*i]=0.0;
                    fft_buf_out_omp[th][2*i+1]=0.0;
                }
                plan_fft_omp[th]=fftwf_plan_dft_2d(box,box,reinterpret_cast<fftwf_complex*>(fft_buf_in_omp[th]),reinterpret_cast<fftwf_complex*>(fft_buf_out_omp[th]),-1,FFTW_ESTIMATE);
                plan_fft_omp_half[th]=fftwf_plan_dft_2d(box/2,box/2,reinterpret_cast<fftwf_complex*>(fft_buf_in_omp[th]),reinterpret_cast<fftwf_complex*>(fft_buf_out_omp[th]),-1,FFTW_ESTIMATE);
                plan_ifft_omp[th]=fftwf_plan_dft_2d(box,box,reinterpret_cast<fftwf_complex*>(fft_buf_in_omp[th]),reinterpret_cast<fftwf_complex*>(fft_buf_out_omp[th]),1,FFTW_ESTIMATE);
                plan_ifft_omp_half[th]=fftwf_plan_dft_2d(box/2,box/2,reinterpret_cast<fftwf_complex*>(fft_buf_in_omp[th]),reinterpret_cast<fftwf_complex*>(fft_buf_out_omp[th]),1,FFTW_ESTIMATE);
            }

            float **psd_block_all[Nz];
            #pragma omp parallel for num_threads(threads)
            for(int n=0;n<Nz;n++)
            {
                psd_block_all[n]=new float*[N_block];
                for(int m=0;m<N_block;m++)
                {
                    psd_block_all[n][m]=new float[box*box];
                    if(is_resampling)
                    {
                        get_psd_one_padding_omp(image_all[n],psd_block_all[n][m],block_x[m],block_y[m],box,(df_max+df_min)/2,pix,psi,theta[n],Nx,Ny,plan_fft_omp[omp_get_thread_num()],plan_ifft_omp_half[omp_get_thread_num()],fft_buf_in_omp[omp_get_thread_num()],fft_buf_out_omp[omp_get_thread_num()]);
                    }
                    else
                    {
                        get_psd_scaling_one_omp(image_all[n],psd_block_all[n][m],block_x[m],block_y[m],box,(df_max+df_min)/2,pix,psi,theta[n],Nx,Ny,false,plan_fft_omp[omp_get_thread_num()],fft_buf_in_omp[omp_get_thread_num()],fft_buf_out_omp[omp_get_thread_num()]);
                    }
                }
            }

            CTF ctf_para_all[Nz];
            float cc_max_all[Nz];
            float *psd_show_all[Nz];
            for(int n=0;n<Nz;n++)
            {
                psd_show_all[n]=new float[box*box];
            }
            #pragma omp parallel for num_threads(threads)
            for(int n=0;n<Nz;n++)
            {
                cout << n << " : Coarse estimation" << endl;
                float *psd_0=new float[box*box];
                for(int i=0;i<box*box;i++)
                {
                    psd_0[i]=0.0;
                }
                for(int m=0;m<N_block;m++)  // 相位板无法联合估计
                {
                    for(int i=0;i<box*box;i++)
                    {
                        psd_0[i]+=psd_block_all[n][m][i];
                    }
                }
                for(int i=0;i<box*box;i++)
                {
                    psd_0[i]/=float(N_block);
                }

                if(box_conv>0 && !skip_box_conv_coarse)
                {
                    get_psd_conv_fft(psd_0,box,box_conv,plan_fft_omp[omp_get_thread_num()],plan_ifft_omp[omp_get_thread_num()],fft_buf_in_omp[omp_get_thread_num()],fft_buf_out_omp[omp_get_thread_num()]);
                }
                
                int angle_interval=5;
                float cc_max_df[180/angle_interval+1];
                float cc_max[180/angle_interval+1];

                for(int m=0;m<180/angle_interval+1;m++)  // 遍历相位差
                {
                    float ph_deg=m*angle_interval;
                    float ph=ph_deg*M_PI/180.0;

                    float res_min_now=res_min;
                    float res_max_now=res_max;

                    // coarse 1d search
                    float cc_0[int(ceil((df_max-df_min)/df_step))+2];
                    float cc_0_max=-1.0;
                    float df_cc_max=df_min;
                    for(int t=0;t<=int(floor((df_max-df_min)/df_step));t++)
                    {
                        CTF ctf_para_now=ctf_para_avg;
                        ctf_para_now.setAllCTFPara(df_min+t*df_step,df_min+t*df_step,0,ph,ctf_para_now.getW());

                        if(res_ada)
                        {
                            int k=2;
                            float delta=M_PI*M_PI*ctf_para_now.getLambda()*ctf_para_now.getLambda()*((df_min+t*df_step)*1e-10)*((df_min+t*df_step)*1e-10)-2*M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*((k)*M_PI-ctf_para_now.getW_phase()-ctf_para_now.getPhaseShift());
                            if(delta<0)
                            {
                                while(delta<0)
                                {
                                    k--;
                                    delta=M_PI*M_PI*ctf_para_now.getLambda()*ctf_para_now.getLambda()*((df_min+t*df_step)*1e-10)*((df_min+t*df_step)*1e-10)-2*M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*(k*M_PI-ctf_para_now.getW_phase()-ctf_para_now.getPhaseShift());
                                }
                                res_min_now=sqrt((M_PI*ctf_para_now.getLambda()*((df_min+t*df_step)*1e-10)+sqrt(M_PI*M_PI*ctf_para_now.getLambda()*ctf_para_now.getLambda()*((df_min+t*df_step)*1e-10)*((df_min+t*df_step)*1e-10)-2*M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*((k)*M_PI-ctf_para_now.getW_phase()-ctf_para_now.getPhaseShift())))/(M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())));
                                float res_max_tmp=sqrt((M_PI*ctf_para_now.getLambda()*((df_min+t*df_step)*1e-10)+sqrt(M_PI*M_PI*ctf_para_now.getLambda()*ctf_para_now.getLambda()*((df_min+t*df_step)*1e-10)*((df_min+t*df_step)*1e-10)-2*M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*((k-ceil(float(N_zeros)/2.0))*M_PI-ctf_para_now.getW_phase()-ctf_para_now.getPhaseShift())))/(M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())));
                                if(res_max_tmp<1.0/(pix*1e-10)/2.0) // 没有超过Nyquist频率
                                {
                                    res_max_now=res_max_tmp;
                                }
                                else
                                {
                                    res_max_now=1.0/(pix*1e-10)/2.0;
                                }
                            }
                            else
                            {
                                res_min_now=sqrt((M_PI*ctf_para_now.getLambda()*((df_min+t*df_step)*1e-10)-sqrt(M_PI*M_PI*ctf_para_now.getLambda()*ctf_para_now.getLambda()*((df_min+t*df_step)*1e-10)*((df_min+t*df_step)*1e-10)-2*M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*((k)*M_PI-ctf_para_now.getW_phase()-ctf_para_now.getPhaseShift())))/(M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())));
                                float res_max_tmp=sqrt((M_PI*ctf_para_now.getLambda()*((df_min+t*df_step)*1e-10)-sqrt(M_PI*M_PI*ctf_para_now.getLambda()*ctf_para_now.getLambda()*((df_min+t*df_step)*1e-10)*((df_min+t*df_step)*1e-10)-2*M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*((k+ceil(float(N_zeros)/2.0))*M_PI-ctf_para_now.getW_phase()-ctf_para_now.getPhaseShift())))/(M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())));
                                if(res_max_tmp<1.0/(pix*1e-10)/2.0) // 没有超过Nyquist频率
                                {
                                    res_max_now=res_max_tmp;
                                }
                                else
                                {
                                    res_max_now=1.0/(pix*1e-10)/2.0;
                                }
                            }
                        }
                        if(res_min_now>=res_max_now)    // 拟合的最小频率比最大频率要高，无法完成拟合
                        {
                            cc_0[t]=-1.0;
                        }
                        else
                        {
                            cc_0[t]=get_correlation_adaptive_phase_shift_zero_mean(df_min+t*df_step,box,N_zeros,ctf_para_avg,res_min_now,res_max_now,psd_0,ph,x_fft_1d_res);
                        }
                    }
                    for(int t=0;t<=int(floor((df_max-df_min)/df_step));t++)
                    {
                        if(cc_0[t]>cc_0_max)
                        {
                            cc_0_max=cc_0[t];
                            df_cc_max=df_min+t*df_step;
                        }
                    }
                    cc_max[m]=cc_0_max;
                    cc_max_df[m]=df_cc_max;

                    cout << n << " : " << ph_deg << " degree: df = " << cc_max_df[m] << ", with cc = " << cc_max[m] << endl;

                    float *psd_show=new float[box*box];
                    CTF ctf_para_show=ctf_para_avg;
                    ctf_para_show.setAllCTFPara(df_cc_max,df_cc_max,0,ph,ctf_para_show.getW());
                    if(res_ada)
                    {
                        int k=2;
                        float delta=M_PI*M_PI*ctf_para_show.getLambda()*ctf_para_show.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_show.getCs()*(ctf_para_show.getLambda()*ctf_para_show.getLambda()*ctf_para_show.getLambda())*((k)*M_PI-ctf_para_show.getW_phase()-ctf_para_show.getPhaseShift());
                        if(delta<0)
                        {
                            while(delta<0)
                            {
                                k--;
                                delta=M_PI*M_PI*ctf_para_show.getLambda()*ctf_para_show.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_show.getCs()*(ctf_para_show.getLambda()*ctf_para_show.getLambda()*ctf_para_show.getLambda())*(k*M_PI-ctf_para_show.getW_phase()-ctf_para_show.getPhaseShift());
                            }
                            res_min_now=sqrt((M_PI*ctf_para_show.getLambda()*(df_cc_max*1e-10)+sqrt(M_PI*M_PI*ctf_para_show.getLambda()*ctf_para_show.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_show.getCs()*(ctf_para_show.getLambda()*ctf_para_show.getLambda()*ctf_para_show.getLambda())*((k)*M_PI-ctf_para_show.getW_phase()-ctf_para_show.getPhaseShift())))/(M_PI*ctf_para_show.getCs()*(ctf_para_show.getLambda()*ctf_para_show.getLambda()*ctf_para_show.getLambda())));
                            float res_max_tmp=sqrt((M_PI*ctf_para_show.getLambda()*(df_cc_max*1e-10)+sqrt(M_PI*M_PI*ctf_para_show.getLambda()*ctf_para_show.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_show.getCs()*(ctf_para_show.getLambda()*ctf_para_show.getLambda()*ctf_para_show.getLambda())*((k-ceil(float(N_zeros)/2.0))*M_PI-ctf_para_show.getW_phase()-ctf_para_show.getPhaseShift())))/(M_PI*ctf_para_show.getCs()*(ctf_para_show.getLambda()*ctf_para_show.getLambda()*ctf_para_show.getLambda())));
                            if(res_max_tmp<1.0/(pix*1e-10)/2.0) // 没有超过Nyquist频率
                            {
                                res_max_now=res_max_tmp;
                            }
                            else
                            {
                                res_max_now=1.0/(pix*1e-10)/2.0;
                            }
                        }
                        else
                        {
                            res_min_now=sqrt((M_PI*ctf_para_show.getLambda()*(df_cc_max*1e-10)-sqrt(M_PI*M_PI*ctf_para_show.getLambda()*ctf_para_show.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_show.getCs()*(ctf_para_show.getLambda()*ctf_para_show.getLambda()*ctf_para_show.getLambda())*((k)*M_PI-ctf_para_show.getW_phase()-ctf_para_show.getPhaseShift())))/(M_PI*ctf_para_show.getCs()*(ctf_para_show.getLambda()*ctf_para_show.getLambda()*ctf_para_show.getLambda())));
                            float res_max_tmp=sqrt((M_PI*ctf_para_show.getLambda()*(df_cc_max*1e-10)-sqrt(M_PI*M_PI*ctf_para_show.getLambda()*ctf_para_show.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_show.getCs()*(ctf_para_show.getLambda()*ctf_para_show.getLambda()*ctf_para_show.getLambda())*((k+ceil(float(N_zeros)/2.0))*M_PI-ctf_para_show.getW_phase()-ctf_para_show.getPhaseShift())))/(M_PI*ctf_para_show.getCs()*(ctf_para_show.getLambda()*ctf_para_show.getLambda()*ctf_para_show.getLambda())));
                            if(res_max_tmp<1.0/(pix*1e-10)/2.0) // 没有超过Nyquist频率
                            {
                                res_max_now=res_max_tmp;
                            }
                            else
                            {
                                res_max_now=1.0/(pix*1e-10)/2.0;
                            }
                        }
                    }
                    get_psd_fit_show_image(psd_0,psd_show,box,ctf_para_show,res_min_now,res_max_now,res2_all,atan_all);
                    // mlog_0.write2DIm(psd_show,m);
                    delete [] psd_show;
                }
                // mlog_0.close();
                float cc_max_ph=-1.0;
                float df_cc_max=cc_max_df[0];
                float ph_cc_max;
                for(int m=0;m<180/angle_interval+1;m++)
                {
                    if(cc_max[m]>cc_max_ph)
                    {
                        cc_max_ph=cc_max[m];
                        df_cc_max=cc_max_df[m];
                        ph_cc_max=m*angle_interval;
                    }
                }
                cout << n << " : " << "Find maximum at " << ph_cc_max << " degree phase shift, df = " << df_cc_max << ", with cc = " << cc_max_ph << endl;
                CTF ctf_para_avg_now=ctf_para_avg;
                ctf_para_avg_now.setAllCTFPara(df_cc_max,df_cc_max,0,ph_cc_max*M_PI/180.0,ctf_para_avg_now.getW());

                if(res_ada)
                {
                    int k=2;
                    float delta=M_PI*M_PI*ctf_para_avg_now.getLambda()*ctf_para_avg_now.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_avg_now.getCs()*(ctf_para_avg_now.getLambda()*ctf_para_avg_now.getLambda()*ctf_para_avg_now.getLambda())*(k*M_PI-ctf_para_avg_now.getW_phase()-ctf_para_avg_now.getPhaseShift());
                    if(delta<0)
                    {
                        while(delta<0)
                        {
                            k--;
                            delta=M_PI*M_PI*ctf_para_avg_now.getLambda()*ctf_para_avg_now.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_avg_now.getCs()*(ctf_para_avg_now.getLambda()*ctf_para_avg_now.getLambda()*ctf_para_avg_now.getLambda())*(k*M_PI-ctf_para_avg_now.getW_phase()-ctf_para_avg_now.getPhaseShift());
                        }
                        res_min=sqrt((M_PI*ctf_para_avg_now.getLambda()*(df_cc_max*1e-10)+sqrt(M_PI*M_PI*ctf_para_avg_now.getLambda()*ctf_para_avg_now.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_avg_now.getCs()*(ctf_para_avg_now.getLambda()*ctf_para_avg_now.getLambda()*ctf_para_avg_now.getLambda())*((k)*M_PI-ctf_para_avg_now.getW_phase()-ctf_para_avg_now.getPhaseShift())))/(M_PI*ctf_para_avg_now.getCs()*(ctf_para_avg_now.getLambda()*ctf_para_avg_now.getLambda()*ctf_para_avg_now.getLambda())));
                        float res_max_tmp=sqrt((M_PI*ctf_para_avg_now.getLambda()*(df_cc_max*1e-10)+sqrt(M_PI*M_PI*ctf_para_avg_now.getLambda()*ctf_para_avg_now.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_avg_now.getCs()*(ctf_para_avg_now.getLambda()*ctf_para_avg_now.getLambda()*ctf_para_avg_now.getLambda())*((k-ceil(float(N_zeros)/2.0))*M_PI-ctf_para_avg_now.getW_phase()-ctf_para_avg_now.getPhaseShift())))/(M_PI*ctf_para_avg_now.getCs()*(ctf_para_avg_now.getLambda()*ctf_para_avg_now.getLambda()*ctf_para_avg_now.getLambda())));
                        if(res_max_tmp<1.0/(pix*1e-10)/2.0) // 没有超过Nyquist频率
                        {
                            res_max=res_max_tmp;
                        }
                        else
                        {
                            res_max=1.0/(pix*1e-10)/2.0;
                        }
                    }
                    else
                    {
                        res_min=sqrt((M_PI*ctf_para_avg_now.getLambda()*(df_cc_max*1e-10)-sqrt(M_PI*M_PI*ctf_para_avg_now.getLambda()*ctf_para_avg_now.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_avg_now.getCs()*(ctf_para_avg_now.getLambda()*ctf_para_avg_now.getLambda()*ctf_para_avg_now.getLambda())*((k)*M_PI-ctf_para_avg_now.getW_phase()-ctf_para_avg_now.getPhaseShift())))/(M_PI*ctf_para_avg_now.getCs()*(ctf_para_avg_now.getLambda()*ctf_para_avg_now.getLambda()*ctf_para_avg_now.getLambda())));
                        float res_max_tmp=sqrt((M_PI*ctf_para_avg_now.getLambda()*(df_cc_max*1e-10)-sqrt(M_PI*M_PI*ctf_para_avg_now.getLambda()*ctf_para_avg_now.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_avg_now.getCs()*(ctf_para_avg_now.getLambda()*ctf_para_avg_now.getLambda()*ctf_para_avg_now.getLambda())*((k+ceil(float(N_zeros)/2.0))*M_PI-ctf_para_avg_now.getW_phase()-ctf_para_avg_now.getPhaseShift())))/(M_PI*ctf_para_avg_now.getCs()*(ctf_para_avg_now.getLambda()*ctf_para_avg_now.getLambda()*ctf_para_avg_now.getLambda())));
                        if(res_max_tmp<1.0/(pix*1e-10)/2.0) // 没有超过Nyquist频率
                        {
                            res_max=res_max_tmp;
                        }
                        else
                        {
                            res_max=1.0/(pix*1e-10)/2.0;
                        }
                    }
                    cout << n << " : " << "Use adaptive resolution boundary:" << endl;
                    cout << n << " : " << "Modified res_min (first zero): " << 1/res_min*1e10 << endl;
                    cout << n << " : " << "Modified res_max (N/2-th zero): " << 1/res_max*1e10 << endl;
                }

                // coarse search for astigmatism
                // get psd with scaling
                // get_average_psd_omp(image_now,psd_0,box,df_cc_max,pix,psi,0.0,Nx,Ny,plan_fft_omp[0],fft_buf_in_omp[0],fft_buf_out_omp[0]);
                for(int i=0;i<box*box;i++)
                {
                    psd_0[i]=0.0;
                }
                float *psd_tmp=new float[box*box];
                for(int m=0;m<N_block;m++)
                {
                    get_scaled_psd_phi(psd_block_all[n][m],psd_tmp,block_x[m],block_y[m],box,df_cc_max,pix,psi,theta[n],phi_initial,Nx,Ny);
                    for(int i=0;i<box*box;i++)
                    {
                        psd_0[i]+=double(psd_tmp[i]);
                    }
                }
                for(int i=0;i<box*box;i++)
                {
                    psd_0[i]/=float(N_block);
                }
                delete [] psd_tmp;
                // box convolution
                if(box_conv>0)
                {
                    get_psd_conv_fft(psd_0,box,box_conv,plan_fft_omp[omp_get_thread_num()],plan_ifft_omp[omp_get_thread_num()],fft_buf_in_omp[omp_get_thread_num()],fft_buf_out_omp[omp_get_thread_num()]);
                }

                int astig_interval=5;
                int N_angles=180/astig_interval+1;
                float cc_astig[N_angles];
                float df_1_astig[N_angles];
                float df_2_astig[N_angles];
                float cc_astig_max=0;
                int index=0;
                float res_min_astig=res_min,res_max_astig=res_max;
                cout << n << " : " << "Search for astigmatism" << endl;
                for(int m=0;m<N_angles;m++)
                {
                    CTF ctf_para_astig=ctf_para_avg_now;
                    ctf_para_astig.setAllCTFPara(df_cc_max,df_cc_max,m*astig_interval,ph_cc_max*M_PI/180.0,ctf_para_astig.getW());
                    Data_opt_epa data_opt;
                    data_opt.box=box;
                    data_opt.ctf_para=ctf_para_astig;
                    data_opt.psd=psd_0;
                    data_opt.res_min=res_min_astig;
                    data_opt.res_max=res_max_astig;
                    data_opt.chi_min=M_PI*ctf_para_avg_now.getLambda()*df_cc_max*1e-10*(res_min_astig*res_min_astig)-M_PI_2*ctf_para_avg_now.getCs()*(ctf_para_avg_now.getLambda()*ctf_para_avg_now.getLambda()*ctf_para_avg_now.getLambda())*double(res_min_astig*res_min_astig)*double(res_min_astig*res_min_astig)+ctf_para_avg_now.getW_phase()+ctf_para_avg_now.getPhaseShift();
                    data_opt.chi_max=M_PI*ctf_para_avg_now.getLambda()*df_cc_max*1e-10*(res_max_astig*res_max_astig)-M_PI_2*ctf_para_avg_now.getCs()*(ctf_para_avg_now.getLambda()*ctf_para_avg_now.getLambda()*ctf_para_avg_now.getLambda())*double(res_max_astig*res_max_astig)*double(res_max_astig*res_max_astig)+ctf_para_avg_now.getW_phase()+ctf_para_avg_now.getPhaseShift();
                    data_opt.res2_all=res2_all;
                    data_opt.atan_all=atan_all;
                    data_opt.x_fft_1d_res=x_fft_1d_res;

                    nlopt_opt opt;
                    opt=nlopt_create(NLOPT_LN_NELDERMEAD,2);
                    if(optimize_with_avg)
                    {
                        nlopt_set_max_objective(opt,get_correlation_astig_epa_phase_shift_zero_mean,&data_opt);
                    }
                    else
                    {
                        nlopt_set_max_objective(opt,get_correlation_astig_phase_shift_zero_mean,&data_opt);
                    }
                    nlopt_set_xtol_rel(opt,1e-4);
                    nlopt_set_ftol_rel(opt,1e-4);
                    // double step[2]={1e3,1e3};
                    double step[2]={astig_angstrom/2,astig_angstrom/2};
                    nlopt_set_initial_step(opt,step);
                    nlopt_set_lower_bounds1(opt,0);
                    // nlopt_set_upper_bounds1(opt,df_max);
                    // double x[2]={df_cc_max+1e3,df_cc_max-1e3};
                    double x[2]={df_cc_max+astig_angstrom/2,df_cc_max-astig_angstrom/2};
                    if(df_cc_max-astig_angstrom/2<0)
                    {
                        x[1]=0.0;
                    }
                    double cc_max;
                    if(nlopt_optimize(opt,x,&cc_max)<0)
                    {
                        cout << n << " : " << m*astig_interval << " degrees: " << "nlopt failed!" << endl;
                        df_1_astig[m]=df_cc_max;
                        df_2_astig[m]=df_cc_max;
                        cc_astig[m]=0.0;
                    }
                    else
                    {
                        if(x[0]==df_min || x[0]==df_max || x[1]==df_min || x[2]==df_max)    // optimization failed
                        {
                            x[0]=df_cc_max;
                            x[1]=df_cc_max;
                            cc_max=-1.0;
                        }
                        cout << n << " : " << m*astig_interval << " degrees: " << "Found maximum at (df_1,df_2): (" << x[0] << "," << x[1] << "), with cc = " << cc_max << endl;
                        cc_astig[m]=cc_max;
                        df_1_astig[m]=x[0];
                        df_2_astig[m]=x[1];
                    }
                }
                for(int m=0;m<N_angles;m++)
                {
                    if(cc_astig[m]>cc_astig_max)
                    {
                        cc_astig_max=cc_astig[m];
                        index=m;
                    }
                }
                cout << n << " : " << "Best fit at (df_1,df_2,astig): (" << df_1_astig[index] << "," << df_2_astig[index] << "," << index*astig_interval*M_PI/180.0 << ")" << endl;
                ctf_para_avg_now.setAllCTFPara(df_1_astig[index],df_2_astig[index],index*astig_interval,ctf_para_avg_now.getPhaseShift(),w_cos);
                
                // local refinement
                CTF ctf_para_now=ctf_para_avg_now;
                float df=(ctf_para_now.getDefocus1()+ctf_para_now.getDefocus2()+(ctf_para_now.getDefocus1()-ctf_para_now.getDefocus2())*cos(2*(-ctf_para_now.getAstigmatism())))/2;

                float *psd_now=new float[box*box];
                psd_tmp=new float[box*box];
                for(int i=0;i<box*box;i++)
                {
                    psd_now[i]=0.0;
                }
                for(int m=0;m<N_block;m++)
                {
                    get_scaled_psd_phi(psd_block_all[n][m],psd_tmp,block_x[m],block_y[m],box,df*1e10,pix,psi,theta[n],phi_initial,Nx,Ny);
                    for(int i=0;i<box*box;i++)
                    {
                        psd_now[i]+=psd_tmp[i];
                    }
                }
                for(int i=0;i<box*box;i++)
                {
                    psd_now[i]/=float(N_block);
                }
                delete [] psd_tmp;

                // box convolution
                if(box_conv>0)
                {
                    get_psd_conv_fft(psd_now,box,box_conv,plan_fft_omp[omp_get_thread_num()],plan_ifft_omp[omp_get_thread_num()],fft_buf_in_omp[omp_get_thread_num()],fft_buf_out_omp[omp_get_thread_num()]);
                }

                // local refinement of all CTF parameters
                Data_opt_epa data_opt;
                data_opt.box=box;
                data_opt.ctf_para=ctf_para_now;
                data_opt.psd=psd_now;
                data_opt.res_min=res_min;
                data_opt.res_max=res_max;
                df=(ctf_para_now.getDefocus1()+ctf_para_now.getDefocus2()+(ctf_para_now.getDefocus1()-ctf_para_now.getDefocus2())*cos(2*(-ctf_para_now.getAstigmatism())))/2;
                data_opt.chi_min=M_PI*ctf_para_now.getLambda()*df*(res_min*res_min)-M_PI_2*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*double(res_min*res_min)*double(res_min*res_min)+ctf_para_now.getW_phase()+ctf_para_now.getPhaseShift();
                data_opt.chi_max=M_PI*ctf_para_now.getLambda()*df*(res_max*res_max)-M_PI_2*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*double(res_max*res_max)*double(res_max*res_max)+ctf_para_now.getW_phase()+ctf_para_now.getPhaseShift();
                data_opt.res2_all=res2_all;
                data_opt.atan_all=atan_all;
                data_opt.x_fft_1d_res=x_fft_1d_res;
                
                nlopt_opt opt;
                opt=nlopt_create(NLOPT_LN_NELDERMEAD,4);
                if(optimize_with_avg)
                {
                    nlopt_set_max_objective(opt,get_correlation_image_epa_phase_shift_zero_mean,&data_opt);
                }
                else
                {
                    nlopt_set_max_objective(opt,get_correlation_image_phase_shift_zero_mean,&data_opt);
                }
                nlopt_set_xtol_rel(opt,1e-4);
                nlopt_set_ftol_rel(opt,1e-4);
                double step_all[4]={1e2,1e2,M_PI/180.0,M_PI/180.0};
                nlopt_set_initial_step(opt,step_all);
                double x[4]={df_1_astig[index],df_2_astig[index],index*astig_interval*M_PI/180.0,ph_cc_max*M_PI/180.0};
                double cc_max_d;
                if(nlopt_optimize(opt,x,&cc_max_d)<0)
                {
                    cout << n << " : " << "[Error] nlopt failed!" << endl;
                }
                else
                {
                    cout << n << " : " << "Found maximum at (df_1,df_2,astig,phase_shift): (" << x[0] << "," << x[1] << "," << x[2]*180.0/M_PI << "," << x[3]*180.0/M_PI << ")" << endl;
                }
                ctf_para_now.setAllCTFPara(x[0],x[1],x[2]*180.0/M_PI,x[3],ctf_para_now.getW());   // 特别注意：astig传给CTF的单位是角度，但过来的是弧度！！
                
                // 输出拟合结果
                df=(ctf_para_now.getDefocus1()+ctf_para_now.getDefocus2()+(ctf_para_now.getDefocus1()-ctf_para_now.getDefocus2())*cos(2*(-ctf_para_now.getAstigmatism())))/2;
                double chi_min=M_PI*ctf_para_now.getLambda()*df*(res_min_astig*res_min_astig)-M_PI_2*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*double(res_min_astig*res_min_astig)*double(res_min_astig*res_min_astig)+ctf_para_now.getW_phase()+ctf_para_now.getPhaseShift();
                double chi_max=M_PI*ctf_para_now.getLambda()*df*(res_max_astig*res_max_astig)-M_PI_2*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*double(res_max_astig*res_max_astig)*double(res_max_astig*res_max_astig)+ctf_para_now.getW_phase()+ctf_para_now.getPhaseShift();
                get_psd_fit_show_epa(psd_now,psd_show_all[n],box,ctf_para_now,res_min,res_max,chi_min,chi_max,res2_all,atan_all,x_fft_1d_res);
                ctf_para_all[n]=ctf_para_now;
                cc_max_all[n]=cc_max_d;

                delete [] psd_now;
                delete [] psd_0;
            }

            // Write out final results
            cout << endl << "Final estimation results:" << endl;
            for(int n=0;n<Nz;n++)
            {
                cout << micrograph_num_all[n] << "@" << micrograph_name_all[n] << ": (df_1,df_2,astig,phase_shift) = (" << ctf_para_all[n].getDefocus1()*1e10 << "," << ctf_para_all[n].getDefocus2()*1e10 << "," << ctf_para_all[n].getAstigmatism()*180.0/M_PI << "," << ctf_para_all[n].getPhaseShift() << "), with cc = " << cc_max_all[n] << endl;
            }

            for(int n=0;n<Nz;n++)
            {
                stack_psd.write2DIm(psd_show_all[n],n);
                delete [] psd_show_all[n];
            }
            stack_psd.close();

            // Write equation for calculating defocus of every pixel
            cout << endl;
            cout << "# Single micrograph power spectrum and Thon ring fitting diagnosis in " << output_mrc << endl;
            cout << "# Estimation results saved in " << output_file << endl;
            cout << endl;

            FILE *fall=fopen(output_file.c_str(),"w");
            if(!fall)
            {
                cerr << "[Error] Cannot open output file!" << endl;
            }
            else
            {
                fprintf(fall,"# Column 1: Frame number@Micrograph name\n");
                fprintf(fall,"# Column 2: Defocus1 (in Angstrom)\n");
                fprintf(fall,"# Column 3: Defocus2 (in Angstrom)\n");
                fprintf(fall,"# Column 4: Astigmatism angle (in degree)\n");
                fprintf(fall,"# Column 5: Phase shift (in radian)\n");
                fprintf(fall,"# Column 6: Tilt axis angle (in degree)\n");
                fprintf(fall,"# Column 7: Tilt angle (in degree)\n");
                fprintf(fall,"# Column 8: Tilt offset around x-axis (x-tilt) (in degree)\n");
                fprintf(fall,"# Column 9: Correlation coefficient\n");
                fprintf(fall,"# Column 10: Well fitted resolution\n\n");

                for(int n=0;n<Nz;n++)
                {
                    fprintf(fall,"%02d@%s %9.3f %9.3f %7.2f %7.2f %7.2f %7.2f %7.2f %9.6f %7.4f\n",micrograph_num_all[n],micrograph_name_all[n].c_str(),ctf_para_all[n].getDefocus1()*1e10,ctf_para_all[n].getDefocus2()*1e10,ctf_para_all[n].getAstigmatism()*180.0/M_PI,ctf_para_all[n].getPhaseShift(),psi,theta[n],phi_initial,cc_max_all[n],0.0);
                }

                fprintf(fall,"\n");
                fprintf(fall,"# Equation for calculating defocus values of every pixel:\n");
                fprintf(fall,"# PSI = Tilt axis angle = %7.2f\n",psi);
                fprintf(fall,"# THETA = Tilt angle = (in output file)\n");
                fprintf(fall,"# PHI = Tilt offset around x-axis (x-tilt) = %7.2f\n",phi_initial);
                fprintf(fall,"# PIX = Pixel size = %f (In Angstrom)\n",pix);
                fprintf(fall,"# NX = Micrograph size in X dimension = %d\n",Nx);
                fprintf(fall,"# NY = Micrograph size in Y dimension = %d\n",Ny);
                fprintf(fall,"\n");
                fprintf(fall,"# For pixel (X,Y) (start from 0, X stands for the first (fastest changing) dimension):\n");
                fprintf(fall,"# DX(X) = X - X_CENTER = X - %d\n",int(Nx/2));
                fprintf(fall,"# DY(Y) = Y - Y_CENTER = Y - %d\n",int(Ny/2));
                // fprintf(fall,"# DELTA_Z(X,Y) = (DX(X) * Cos(PSI) + DY(Y) * Sin(PSI)) * Tan(THETA) * PIX\n");
                fprintf(fall,"# DELTA_Z(X,Y) = (DX(X) * (Cos(PHI) * Sin(THETA) * Cos(PSI) - Sin(PHI) * Sin(PSI)) + DY(Y) * (Cos(PHI) * Sin(THETA) * Sin(PSI) + Sin(PHI) * Cos(PSI))) / (Cos(PHI) * Cos(THETA)) * PIX\n");
                fprintf(fall,"# DEFOCUS_1(X,Y) = DEFOCUS_1 + DELTA_Z(X,Y)\n");
                fprintf(fall,"# DEFOCUS_2(X,Y) = DEFOCUS_2 + DELTA_Z(X,Y)\n");
                fprintf(fall,"\n");
                if(single_particle)
                {
                    fprintf(fall,"# Input micrograph in %s\n",input_micrograph.c_str());
                }
                else
                {
                    fprintf(fall,"# Input micrograph list in %s\n",input_list.c_str());
                }
                fprintf(fall,"# Tilt-series average power spectrum and Thon ring fitting diagnosis in %s\n",average_mrc.c_str());
                fprintf(fall,"# Single micrograph power spectrum and Thon ring fitting diagnosis in %s\n",output_mrc.c_str());
                fprintf(fall,"# Estimation results saved in %s\n",output_file.c_str());
                fprintf(fall,"\n");
                
                fflush(fall);
                fclose(fall);
            }

            for(int n=0;n<Nz;n++)
            {
                delete [] image_all[n];
            }

            for(int th=0;th<threads;th++)
            {
                delete [] psd_avg_omp[th];
                delete [] fft_buf_in_omp[th];
                delete [] fft_buf_out_omp[th];
                fftwf_destroy_plan(plan_fft_omp_half[th]);
                fftwf_destroy_plan(plan_fft_omp[th]);
                fftwf_destroy_plan(plan_ifft_omp[th]);
                fftwf_destroy_plan(plan_ifft_omp_half[th]);
            }
            delete [] res2_all;
            delete [] atan_all;

            delete [] block_x;
            delete [] block_y;

            for(int n=0;n<Nz;n++)
            {
                for(int m=0;m<N_block;m++)
                {
                    delete [] psd_block_all[n][m];
                }
                delete [] psd_block_all[n];
            }

            stack_psd.close();

            return;
        }

        int x_fft[box],y_fft[box];
        float x_fft_res[box],y_fft_res[box];
        for(int i=0;i<box;i++)
        {
            x_fft[i]=i-box/2;
            y_fft[i]=i-box/2;
            x_fft_res[i]=float(x_fft[i])/float(box)/ctf_para_avg.getPixelSize();
            y_fft_res[i]=float(y_fft[i])/float(box)/ctf_para_avg.getPixelSize();
        }
        float x_fft_1d_res[box/2-1];
        for(int i=0;i<box/2-1;i++)
        {
            x_fft_1d_res[i]=float(i)/float(box)/ctf_para_avg.getPixelSize();
        }
        float *res2_all=new float[box*box];
        for(int j=0;j<box;j++)
        {
            for(int i=0;i<box;i++)
            {
                res2_all[j*box+i]=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
            }
        }
        float *atan_all=new float[box*box];
        for(int j=0;j<box;j++)
        {
            for(int i=0;i<box;i++)
            {
                atan_all[j*box+i]=atan2(y_fft_res[j],x_fft_res[i]);
            }
        }

        int N_block_x,N_block_y;
        int block_x_min,block_x_max;
        int block_y_min,block_y_max;
        int N_block;
        if(tight_blocks)
        {
            N_block_x=Nx/box*2-2;
            N_block_y=Ny/box*2-2;
            N_block=get_blocks_tight(box,&N_block_x,&N_block_y,&block_x_min,&block_x_max,&block_y_min,&block_y_max,Nx,Ny);
        }
        else
        {
            N_block_x=Nx/box*2;
            N_block_y=Ny/box*2;
            N_block=get_blocks(box,&N_block_x,&N_block_y,&block_x_min,&block_x_max,&block_y_min,&block_y_max,Nx,Ny);
        }
        int *block_x=new int[N_block];
        int *block_y=new int[N_block];
        get_block_coords(block_x,block_y,box,block_x_min,block_x_max,block_y_min,block_y_max,N_block_x,N_block_y,Nx,Ny);
        if(tight_blocks)
        {
            cout << "Number of blocks: " << N_block << " = (" << N_block_x+2 << " - 2) * (" << N_block_y+2 << " - 2)" << endl;
        }
        else
        {
            cout << "Number of blocks: " << N_block << " = " << N_block_x << " * " << N_block_y << endl;
        }

        double *psd_avg_omp[threads];
        float *fft_buf_in_omp[threads];
        float *fft_buf_out_omp[threads];
        fftwf_plan plan_fft_omp[threads];
        fftwf_plan plan_fft_omp_half[threads];
        fftwf_plan plan_ifft_omp[threads];
        fftwf_plan plan_ifft_omp_half[threads];
        for(int th=0;th<threads;th++)
        {
            psd_avg_omp[th]=new double[box*box];
            fft_buf_in_omp[th]=new float[box*box*2];
            fft_buf_out_omp[th]=new float[box*box*2];
            for(int i=0;i<box*box;i++)
            {
                psd_avg_omp[th][i]=0.0;
                fft_buf_in_omp[th][2*i]=0.0;
                fft_buf_in_omp[th][2*i+1]=0.0;
                fft_buf_out_omp[th][2*i]=0.0;
                fft_buf_out_omp[th][2*i+1]=0.0;
            }
            plan_fft_omp[th]=fftwf_plan_dft_2d(box,box,reinterpret_cast<fftwf_complex*>(fft_buf_in_omp[th]),reinterpret_cast<fftwf_complex*>(fft_buf_out_omp[th]),-1,FFTW_ESTIMATE);
            plan_fft_omp_half[th]=fftwf_plan_dft_2d(box/2,box/2,reinterpret_cast<fftwf_complex*>(fft_buf_in_omp[th]),reinterpret_cast<fftwf_complex*>(fft_buf_out_omp[th]),-1,FFTW_ESTIMATE);
            plan_ifft_omp[th]=fftwf_plan_dft_2d(box,box,reinterpret_cast<fftwf_complex*>(fft_buf_in_omp[th]),reinterpret_cast<fftwf_complex*>(fft_buf_out_omp[th]),1,FFTW_ESTIMATE);
            plan_ifft_omp_half[th]=fftwf_plan_dft_2d(box/2,box/2,reinterpret_cast<fftwf_complex*>(fft_buf_in_omp[th]),reinterpret_cast<fftwf_complex*>(fft_buf_out_omp[th]),1,FFTW_ESTIMATE);
        }

        float **psd_block_all[Nz];
        #pragma omp parallel for num_threads(threads)
        for(int n=0;n<Nz;n++)
        {
            psd_block_all[n]=new float*[N_block];
            for(int m=0;m<N_block;m++)
            {
                psd_block_all[n][m]=new float[box*box];
                if(is_resampling)
                {
                    get_psd_one_padding_omp(image_all[n],psd_block_all[n][m],block_x[m],block_y[m],box,(df_max+df_min)/2,pix,psi,theta[n],Nx,Ny,plan_fft_omp[omp_get_thread_num()],plan_ifft_omp_half[omp_get_thread_num()],fft_buf_in_omp[omp_get_thread_num()],fft_buf_out_omp[omp_get_thread_num()]);
                }
                else
                {
                    get_psd_scaling_one_omp(image_all[n],psd_block_all[n][m],block_x[m],block_y[m],box,(df_max+df_min)/2,pix,psi,theta[n],Nx,Ny,false,plan_fft_omp[omp_get_thread_num()],fft_buf_in_omp[omp_get_thread_num()],fft_buf_out_omp[omp_get_thread_num()]);
                }
            }
        }

        float ctf_all[Nz][4];   // {df_1 (A),df_2 (A),astig (degree),cc}
        CTF ctf_para_all[Nz];
        int ctf_all_eva_n[Nz];
        float ctf_all_eva_res[Nz];

        for(int th=0;th<threads;th++)
        {
            for(int i=0;i<box*box;i++)
            {
                psd_avg_omp[th][i]=0.0;
                fft_buf_in_omp[th][2*i]=0.0;
                fft_buf_in_omp[th][2*i+1]=0.0;
                fft_buf_out_omp[th][2*i]=0.0;
                fft_buf_out_omp[th][2*i+1]=0.0;
            }
        }
        
        float *psd_show_all[Nz];
        for(int n=0;n<Nz;n++)
        {
            psd_show_all[n]=new float[box*box];
        }

        #pragma omp parallel for num_threads(threads)
        for(int n=0;n<Nz;n++)
        {
            // Coarse 1D search
            float *psd_0=new float[box*box];
            for(int i=0;i<box*box;i++)
            {
                psd_0[i]=0.0;
            }
            for(int m=0;m<N_block;m++)
            {
                for(int i=0;i<box*box;i++)
                {
                    psd_0[i]+=double(psd_block_all[n][m][i]);
                }
            }
            for(int i=0;i<box*box;i++)
            {
                psd_0[i]/=double(N_block);
            }

            if(box_conv>0 && !skip_box_conv_coarse)
            {
                get_psd_conv_fft(psd_0,box,box_conv,plan_fft_omp[omp_get_thread_num()],plan_ifft_omp[omp_get_thread_num()],fft_buf_in_omp[omp_get_thread_num()],fft_buf_out_omp[omp_get_thread_num()]);
            }

            float cc_0[int(ceil((df_max-df_min)/df_step))+2];
            float cc_0_max=0.0;
            float df_cc_max=df_min;
            int t=0;
            for(int m=0;m<=int(floor((df_max-df_min)/df_step));m++)
            {
                cc_0[m]=get_correlation_adaptive_zero_mean(df_min+m*df_step,box,N_zeros,ctf_para_avg,res_min,res_max,psd_0,res2_all,atan_all,x_fft_1d_res);
                // cout << "df=" << df_min+m*df_step << ": cc=" << cc_0[m] << endl;
            }
            for(int m=0;m<=int(floor((df_max-df_min)/df_step));m++)
            {
                if(cc_0[m]>cc_0_max)
                {
                    cc_0_max=cc_0[m];
                    df_cc_max=df_min+m*df_step;
                }
            }
            cout << n << ": Best fit at df = " << df_cc_max << " with cc = " << cc_0_max << endl;
            
            CTF ctf_para_now=ctf_para_avg;
            ctf_para_now.setAllCTFPara(df_cc_max,df_cc_max,0,0,w_cos);
            
            // 遍历角度，搜索astigmatism
            float *psd_now=new float[box*box];
            float *psd=new float[box*box];
            float radial_now[box/2-1];
            double fit_now[box/2-1];
            if(astig_angstrom>0)
            {
                // float df=(ctf_para_now.getDefocus1()+ctf_para_now.getDefocus2()+(ctf_para_now.getDefocus1()-ctf_para_now.getDefocus2())*cos(2*(-ctf_para_now.getAstigmatism())))/2;
                float df=df_cc_max;
                for(int i=0;i<box*box;i++)
                {
                    psd_0[i]=0.0;
                    psd_now[i]=0.0;
                    psd[i]=0.0;
                }
                for(int m=0;m<N_block;m++)
                {
                    get_scaled_psd_phi(psd_block_all[n][m],psd_now,block_x[m],block_y[m],box,df,pix,psi,theta[n],phi_initial,Nx,Ny);
                    for(int i=0;i<box*box;i++)
                    {
                        psd_0[i]+=psd_now[i];
                    }
                }
                // delete [] psd_now;

                // 把平均功率谱作为一个先验放着，再加上当前照片的功率谱
                for(int i=0;i<box*box;i++)
                {
                    // psd[i]=psd_add[i]+psd_image[n][i];
                    // psd[i]=log(psd_add[i]+psd_image[n][i]);
                    // psd[i]=log(psd_image[n][i]);
                    psd[i]=psd_0[i];
                }

                // box convolution
                if(box_conv>0 && box_conv_first)
                {
                    get_psd_conv_fft(psd,box,box_conv,plan_fft_omp[omp_get_thread_num()],plan_ifft_omp[omp_get_thread_num()],fft_buf_in_omp[omp_get_thread_num()],fft_buf_out_omp[omp_get_thread_num()]);
                }

                // 通过优化方法估背景
                get_radial_average(psd,radial_now,box,ctf_para_now,res2_all,atan_all,x_fft_1d_res);
                background_estimation(fit_now,radial_now,box,N_zeros,ctf_para_now);

                // 回到二维抠背景
                background_subtraction(psd,fit_now,box,res2_all,atan_all,x_fft_1d_res,ctf_para_now);
                // background_subtraction(psd,fit_avg,box,res2_all,atan_all,x_fft_1d_res,ctf_para_now);

                // box convolution
                if(box_conv>0 && !box_conv_first)
                {
                    get_psd_conv_fft(psd,box,box_conv,plan_fft_omp[omp_get_thread_num()],plan_ifft_omp[omp_get_thread_num()],fft_buf_in_omp[omp_get_thread_num()],fft_buf_out_omp[omp_get_thread_num()]);
                }

                // 遍历astig，每个角度自己精搜，找最大互相关
                int astig_interval=5;
                int N_angles=180/astig_interval+1;
                float cc_astig[N_angles];
                float df_1_astig[N_angles];
                float df_2_astig[N_angles];
                float cc_astig_max=-1.0;
                int index=0;
                float res_min_astig=res_min,res_max_astig=res_max;
                for(int m=0;m<N_angles;m++)
                {
                    CTF ctf_para_astig=ctf_para_avg;
                    ctf_para_astig.setAllCTFPara(df,df,m*astig_interval,0,ctf_para_astig.getW());

                    Data_opt_epa data_opt;
                    data_opt.box=box;
                    data_opt.ctf_para=ctf_para_astig;
                    data_opt.psd=psd;
                    data_opt.res_min=res_min_astig;
                    data_opt.res_max=res_max_astig;
                    data_opt.chi_min=M_PI*ctf_para_avg.getLambda()*df*1e-10*(res_min_astig*res_min_astig)-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*double(res_min_astig*res_min_astig)*double(res_min_astig*res_min_astig)+ctf_para_avg.getW_phase();
                    data_opt.chi_max=M_PI*ctf_para_avg.getLambda()*df*1e-10*(res_max_astig*res_max_astig)-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*double(res_max_astig*res_max_astig)*double(res_max_astig*res_max_astig)+ctf_para_avg.getW_phase();
                    data_opt.res2_all=res2_all;
                    data_opt.atan_all=atan_all;
                    data_opt.x_fft_1d_res=x_fft_1d_res;

                    nlopt_opt opt;
                    opt=nlopt_create(NLOPT_LN_NELDERMEAD,2);
                    if(adaptive_range)
                    {
                        nlopt_set_max_objective(opt,get_correlation_astig_epa_zero_mean,&data_opt);
                    }
                    else
                    {
                        nlopt_set_max_objective(opt,get_correlation_astig_epa_zero_mean_circle,&data_opt);
                    }
                    nlopt_set_xtol_rel(opt,1e-4);
                    nlopt_set_ftol_rel(opt,1e-4);
                    // double step[2]={1e4,1e4};
                    // double step[2]={df*1e-1,df*1e-1};
                    // double step[2]={1e3,1e3};
                    double step[2]={astig_angstrom/2.0,astig_angstrom/2.0};
                    if(astig_angstrom<2)
                    {
                        step[0]=1;
                        step[1]=1;
                    }
                    nlopt_set_initial_step(opt,step);
                    // nlopt_set_lower_bounds1(opt,df_min);
                    // nlopt_set_upper_bounds1(opt,df_max);
                    // double x[2]={df,df};
                    // double x[2]={df+1e3,df-1e3};
                    double x[2]={df+astig_angstrom/2.0,df-astig_angstrom/2.0};
                    double cc_max;
                    if(nlopt_optimize(opt,x,&cc_max)<0)
                    {
                        // cout << m*astig_interval << " degrees: " << "nlopt failed!" << endl;
                        df_1_astig[m]=df;
                        df_2_astig[m]=df;
                        cc_astig[m]=0.0;
                    }
                    else
                    {
                        if(x[0]==df_min || x[0]==df_max || x[1]==df_min || x[2]==df_max)    // optimization failed
                        {
                            x[0]=df;
                            x[1]=df;
                            cc_max=0.0;
                        }
                        // cout << m*astig_interval << " degrees: " << "Found maximum at (df_1,df_2): (" << x[0] << "," << x[1] << "), with cc = " << cc_max << endl;
                        cc_astig[m]=cc_max;
                        df_1_astig[m]=x[0];
                        df_2_astig[m]=x[1];
                    }
                }
                for(int m=0;m<N_angles;m++)
                {
                    if(cc_astig[m]>cc_astig_max)
                    {
                        cc_astig_max=cc_astig[m];
                        index=m;
                    }
                }
                cout << n << ": Best fit at (df_1,df_2,astig): (" << df_1_astig[index] << "," << df_2_astig[index] << "," << index*astig_interval << ")" << endl;
                ctf_para_avg.setAllCTFPara(df_1_astig[index],df_2_astig[index],index*astig_interval,0,w_cos);
            }

            /*
            // evaluation
            int n_eva=0;
            n_eva=get_number_of_rings_fit(psd,box,ctf_para_avg,res2_all,atan_all,x_fft_1d_res);
            float res_eva=sqrt((M_PI*ctf_para_avg.getLambda()*(df)-sqrt(M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df)*(df)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(n_eva*M_PI-ctf_para_avg.getW_phase())))/(M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())));
            cout << "Evaluation of fitting: Fitting to the " << n_eva << "-th Thon ring, with resolution = " << 1/(res_eva*1e-10) << endl;
            */

            // Local refine
            // 切小块估计功率谱
            float df=(ctf_para_now.getDefocus1()+ctf_para_now.getDefocus2()+(ctf_para_now.getDefocus1()-ctf_para_now.getDefocus2())*cos(2*(-ctf_para_now.getAstigmatism())))/2;
            // float *psd_now=new float[box*box];
            // float *psd=new float[box*box];
            for(int i=0;i<box*box;i++)
            {
                psd_0[i]=0.0;
                psd_now[i]=0.0;
                psd[i]=0.0;
            }
            for(int m=0;m<N_block;m++)
            {
                get_scaled_psd_phi(psd_block_all[n][m],psd_now,block_x[m],block_y[m],box,df*1e10,pix,psi,theta[n],phi_initial,Nx,Ny);
                for(int i=0;i<box*box;i++)
                {
                    psd_0[i]+=psd_now[i];
                }
            }
            delete [] psd_now;

            // 把平均功率谱作为一个先验放着，再加上当前照片的功率谱
            for(int i=0;i<box*box;i++)
            {
                // psd[i]=psd_add[i]+psd_image[n][i];
                // psd[i]=log(psd_add[i]+psd_image[n][i]);
                // psd[i]=log(psd_image[n][i]);
                psd[i]=psd_0[i];
            }

            // box convolution
            if(box_conv>0 && box_conv_first)
            {
                get_psd_conv_fft(psd,box,box_conv,plan_fft_omp[omp_get_thread_num()],plan_ifft_omp[omp_get_thread_num()],fft_buf_in_omp[omp_get_thread_num()],fft_buf_out_omp[omp_get_thread_num()]);
            }

            // 通过优化方法估背景
            // float radial_now[box/2-1];
            get_radial_average(psd,radial_now,box,ctf_para_now,res2_all,atan_all,x_fft_1d_res);
            // double fit_now[box/2-1];
            background_estimation(fit_now,radial_now,box,N_zeros,ctf_para_now);

            // 回到二维抠背景
            background_subtraction(psd,fit_now,box,res2_all,atan_all,x_fft_1d_res,ctf_para_now);
            // background_subtraction(psd,fit_avg,box,res2_all,atan_all,x_fft_1d_res,ctf_para_now);

            // box convolution
            if(box_conv>0 && !box_conv_first)
            {
                get_psd_conv_fft(psd,box,box_conv,plan_fft_omp[omp_get_thread_num()],plan_ifft_omp[omp_get_thread_num()],fft_buf_in_omp[omp_get_thread_num()],fft_buf_out_omp[omp_get_thread_num()]);
            }

            df=(ctf_para_now.getDefocus1()+ctf_para_now.getDefocus2()+(ctf_para_now.getDefocus1()-ctf_para_now.getDefocus2())*cos(2*(-ctf_para_now.getAstigmatism())))/2;
            Data_opt_epa data_opt_image;
            data_opt_image.box=box;
            data_opt_image.ctf_para=ctf_para_now;
            data_opt_image.psd=psd;
            data_opt_image.res_min=res_min;
            data_opt_image.res_max=res_max;
            data_opt_image.chi_min=M_PI*ctf_para_now.getLambda()*df*(res_min*res_min)-M_PI_2*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*double(res_min*res_min)*double(res_min*res_min)+ctf_para_now.getW_phase();
            data_opt_image.chi_max=M_PI*ctf_para_now.getLambda()*df*(res_max*res_max)-M_PI_2*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*double(res_max*res_max)*double(res_max*res_max)+ctf_para_now.getW_phase();
            data_opt_image.res2_all=res2_all;
            data_opt_image.atan_all=atan_all;
            data_opt_image.x_fft_1d_res=x_fft_1d_res;

            nlopt_opt opt_image;
            opt_image=nlopt_create(NLOPT_LN_NELDERMEAD,3);
            if(optimize_with_avg)
            {
                if(adaptive_range)
                {
                    nlopt_set_max_objective(opt_image,get_correlation_image_epa_zero_mean,&data_opt_image);
                }
                else
                {
                    nlopt_set_max_objective(opt_image,get_correlation_image_epa_zero_mean_circle,&data_opt_image);
                }
            }
            else
            {
                if(adaptive_range)
                {
                    nlopt_set_max_objective(opt_image,get_correlation_image_zero_mean,&data_opt_image);
                }
                else
                {
                    nlopt_set_max_objective(opt_image,get_correlation_image_zero_mean_circle,&data_opt_image);
                }
            }
            nlopt_set_xtol_rel(opt_image,1e-4);
            nlopt_set_ftol_rel(opt_image,1e-4);
            double step[3]={astig_angstrom/2.0,astig_angstrom/2.0,1.0*M_PI/180.0};
            if(astig_angstrom>0)
            {
                if(astig_angstrom<100)
                {
                    step[0]=50;
                    step[1]=50;
                }
            }
            else
            {
                step[0]=50;
                step[1]=50;
            }
            // double step[3]={1e2,1e2,1.0*M_PI/180.0};
            // double step[3]={astig_angstrom/2.0,astig_angstrom/2.0,1.0*M_PI/180.0};
            // if(astig_angstrom<2e2)
            // {
            //     step[0]=1e2;
            //     step[1]=1e2;
            // }
            nlopt_set_initial_step(opt_image,step);
            double x[3]={ctf_para_now.getDefocus1()*1e10,ctf_para_now.getDefocus2()*1e10,ctf_para_now.getAstigmatism()};
            double cc_max;  // 单位: (A,A,rad)
            if(nlopt_optimize(opt_image,x,&cc_max)<0)
            {
                x[0]=ctf_para_now.getDefocus1()*1e10;
                x[1]=ctf_para_now.getDefocus2()*1e10;
                x[2]=ctf_para_now.getAstigmatism();
                // cout << n << ": nlopt failed! Use unified CTF instead: (" << x[0] << "," << x[1] << "," << x[2]*180.0/M_PI << ")" << endl;
            }
            ctf_all[n][0]=x[0];
            ctf_all[n][1]=x[1];
            ctf_all[n][2]=x[2]*180.0/M_PI;
            ctf_all[n][3]=cc_max;
            ctf_para_now.setAllCTFPara(x[0],x[1],x[2]*180.0/M_PI,0,w_cos);
            ctf_para_all[n]=ctf_para_now;
            cout << n << ": Found maximum at (df_1,df_2,astig): (" << x[0] << "," << x[1] << "," << x[2]*180.0/M_PI << "), with cc = " << cc_max << endl;

            // evaluation
            int n_eva=0;
            n_eva=get_number_of_rings_fit(psd,box,ctf_para_now,res2_all,atan_all,x_fft_1d_res);
            float df_now=(ctf_para_now.getDefocus1()+ctf_para_now.getDefocus2()+(ctf_para_now.getDefocus1()-ctf_para_now.getDefocus2())*cos(2*(-ctf_para_now.getAstigmatism())))/2;
            float res_eva=sqrt((M_PI*ctf_para_now.getLambda()*(df_now)-sqrt(M_PI*M_PI*ctf_para_now.getLambda()*ctf_para_now.getLambda()*(df_now)*(df_now)-2*M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*(n_eva*M_PI-ctf_para_now.getW_phase())))/(M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())));
            cout << n << ": Evaluation of fitting: Fitting to the " << n_eva << "-th Thon ring, with resolution = " << 1/(res_eva*1e-10) << endl;
            ctf_all_eva_n[n]=n_eva;
            ctf_all_eva_res[n]=1/(res_eva*1e-10);

            res_eva=sqrt((M_PI*ctf_para_now.getLambda()*(df_now)-sqrt(M_PI*M_PI*ctf_para_now.getLambda()*ctf_para_now.getLambda()*(df_now)*(df_now)-2*M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*(ctf_all_eva_n[n]*M_PI-ctf_para_now.getW_phase())))/(M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())));
            double chi_eva=M_PI*ctf_para_now.getLambda()*df_now*(res_eva*res_eva)-M_PI_2*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*double(res_eva*res_eva)*double(res_eva*res_eva)+ctf_para_now.getW_phase();
            get_psd_fit_show_epa_subtraction(psd,psd_show_all[n],box,ctf_para_now,res_min,res_max,res_min_orig,res_eva,chi_eva,res2_all,atan_all,x_fft_1d_res);

            delete [] psd;
            delete [] psd_0;
        }

        for(int th=0;th<threads;th++)
        {
            delete [] psd_avg_omp[th];
            delete [] fft_buf_in_omp[th];
            delete [] fft_buf_out_omp[th];
            fftwf_destroy_plan(plan_fft_omp_half[th]);
            fftwf_destroy_plan(plan_fft_omp[th]);
            fftwf_destroy_plan(plan_ifft_omp[th]);
            fftwf_destroy_plan(plan_ifft_omp_half[th]);
        }
        
        /*
        FILE *fdefocus=fopen(defocus_file.c_str(),"w");
        for(int n=0;n<Nz;n++)
        {
            fprintf(fdefocus,"%d %f %f %f 0.000000 %f %f\n",n,ctf_all[n][0],ctf_all[n][1],ctf_all[n][2],ctf_all[n][3],ctf_all_eva_res[n]);
        }
        fclose(fdefocus);
        */

        // Write out final results
        cout << endl << "Final estimation results:" << endl;
        for(int n=0;n<Nz;n++)
        {
            cout << micrograph_num_all[n] << "@" << micrograph_name_all[n] << ": (df_1,df_2,astig) = (" << ctf_all[n][0] << "," << ctf_all[n][1] << "," << ctf_all[n][2] << "), with cc = " << ctf_all[n][3] << ", fitting to the " << ctf_all_eva_n[n] << "-th Thon ring, with resolution = " << ctf_all_eva_res[n] << ", tilt angle = " << theta[n] << ", and tilt axis angle = " << psi << endl;
        }

        MRC stack_psd(output_mrc.c_str(),"w");
        if(!stack_psd.hasFile())
        {
            cerr << "[Error] Cannot open output file " << output_mrc << "!" << endl;
            abort();
        }
        stack_psd.createMRC_empty(box,box,Nz,2);
        for(int n=0;n<Nz;n++)
        {
            // fprintf(fdefocus,"%d %f %f %f 0.000000 %f %f\n",n,ctf_all[n][0],ctf_all[n][1],ctf_all[n][2],ctf_all[n][3],ctf_all_eva_res[n]);
            stack_psd.write2DIm(psd_show_all[n],n);
            delete [] psd_show_all[n];
        }
        stack_psd.close();

        // Write equation for calculating defocus of every pixel
        cout << endl;
        cout << "# Equation for calculating defocus values of every pixel:" << endl;
        cout << "# PSI = Tilt axis angle = " << psi << endl;
        cout << "# THETA = Tilt angle = (in tlt file)" << endl;
        cout << "# PHI = Tilt offset around x-axis (x-tilt) = " << phi_initial << endl;
        cout << "# PIX = Pixel size = " << pix << " (In Angstrom)" << endl;
        cout << "# NX = Micrograph size in X dimension = " << Nx << endl;
        cout << "# NY = Micrograph size in Y dimension = " << Ny << endl;
        cout << endl;
        cout << "# For pixel (X,Y) (start from 0, X stands for the first (fastest changing) dimension):" << endl;
        cout << "# DX(X) = X - X_CENTER = X - " << int(Nx/2) << endl;
        cout << "# DY(Y) = Y - Y_CENTER = Y - " << int(Ny/2) << endl;
        // cout << "# DELTA_Z(X,Y) = (DX(X) * Cos(PSI) + DY(Y) * Sin(PSI)) * Tan(THETA) * PIX" << endl;
        cout << "# DELTA_Z(X,Y) = (DX(X) * (Cos(PHI) * Sin(THETA) * Cos(PSI) - Sin(PHI) * Sin(PSI)) + DY(Y) * (Cos(PHI) * Sin(THETA) * Sin(PSI) + Sin(PHI) * Cos(PSI))) / (Cos(PHI) * Cos(THETA)) * PIX" << endl;
        cout << "# DEFOCUS_1(X,Y) = DEFOCUS_1 + DELTA_Z(X,Y)" << endl;
        cout << "# DEFOCUS_2(X,Y) = DEFOCUS_2 + DELTA_Z(X,Y)" << endl;
        cout << endl;
        cout << "# Tilt-series average power spectrum and Thon ring fitting diagnosis in " << average_mrc << endl;
        cout << "# Single micrograph power spectrum and Thon ring fitting diagnosis in " << output_mrc << endl;
        cout << "# Estimation results saved in " << output_file << endl;
        cout << endl;

        FILE *fall=fopen(output_file.c_str(),"w");
        if(!fall)
        {
            cerr << "[Error] Cannot open output file!" << endl;
        }
        else
        {
            fprintf(fall,"# Column 1: Frame number@Micrograph name\n");
            fprintf(fall,"# Column 2: Defocus1 (in Angstrom)\n");
            fprintf(fall,"# Column 3: Defocus2 (in Angstrom)\n");
            fprintf(fall,"# Column 4: Astigmatism angle (in degree)\n");
            fprintf(fall,"# Column 5: Phase shift (in radian)\n");
            fprintf(fall,"# Column 6: Tilt axis angle (in degree)\n");
            fprintf(fall,"# Column 7: Tilt angle (in degree)\n");
            fprintf(fall,"# Column 8: Tilt offset around x-axis (x-tilt) (in degree)\n");
            fprintf(fall,"# Column 9: Correlation coefficient\n");
            fprintf(fall,"# Column 10: Well fitted resolution\n\n");

            for(int n=0;n<Nz;n++)
            {
                fprintf(fall,"%02d@%s %9.3f %9.3f %7.2f 0.000000 %7.2f %7.2f %7.2f %9.6f %7.4f\n",micrograph_num_all[n],micrograph_name_all[n].c_str(),ctf_all[n][0],ctf_all[n][1],ctf_all[n][2],psi,theta[n],phi_initial,ctf_all[n][3],ctf_all_eva_res[n]);
            }

            fprintf(fall,"\n");
            fprintf(fall,"# Equation for calculating defocus values of every pixel:\n");
            fprintf(fall,"# PSI = Tilt axis angle = %7.2f\n",psi);
            fprintf(fall,"# THETA = Tilt angle = (in output file)\n");
            fprintf(fall,"# PHI = Tilt offset around x-axis (x-tilt) = %7.2f\n",phi_initial);
            fprintf(fall,"# PIX = Pixel size = %f (In Angstrom)\n",pix);
            fprintf(fall,"# NX = Micrograph size in X dimension = %d\n",Nx);
            fprintf(fall,"# NY = Micrograph size in Y dimension = %d\n",Ny);
            fprintf(fall,"\n");
            fprintf(fall,"# For pixel (X,Y) (start from 0, X stands for the first (fastest changing) dimension):\n");
            fprintf(fall,"# DX(X) = X - X_CENTER = X - %d\n",int(Nx/2));
            fprintf(fall,"# DY(Y) = Y - Y_CENTER = Y - %d\n",int(Ny/2));
            // fprintf(fall,"# DELTA_Z(X,Y) = (DX(X) * Cos(PSI) + DY(Y) * Sin(PSI)) * Tan(THETA) * PIX\n");
            fprintf(fall,"# DELTA_Z(X,Y) = (DX(X) * (Cos(PHI) * Sin(THETA) * Cos(PSI) - Sin(PHI) * Sin(PSI)) + DY(Y) * (Cos(PHI) * Sin(THETA) * Sin(PSI) + Sin(PHI) * Cos(PSI))) / (Cos(PHI) * Cos(THETA)) * PIX\n");
            fprintf(fall,"# DEFOCUS_1(X,Y) = DEFOCUS_1 + DELTA_Z(X,Y)\n");
            fprintf(fall,"# DEFOCUS_2(X,Y) = DEFOCUS_2 + DELTA_Z(X,Y)\n");
            fprintf(fall,"\n");
            if(single_particle)
            {
                fprintf(fall,"# Input micrograph in %s\n",input_micrograph.c_str());
            }
            else
            {
                fprintf(fall,"# Input micrograph list in %s\n",input_list.c_str());
            }
            fprintf(fall,"# Tilt-series average power spectrum and Thon ring fitting diagnosis in %s\n",average_mrc.c_str());
            fprintf(fall,"# Single micrograph power spectrum and Thon ring fitting diagnosis in %s\n",output_mrc.c_str());
            fprintf(fall,"# Estimation results saved in %s\n",output_file.c_str());
            fprintf(fall,"\n");
            
            fflush(fall);
            fclose(fall);
        }

        return;
    }


    // 新的相位板估计：先选低角度整体估离焦量初值和像散，再每一张精搜
    if(search_phase_shift)    // Search for phase shift (using phase plate)
    {
        cout << endl << "Search for phase shift (using phase plate), we can not apply unified CTF estimation, as phase shift varies through the tilt-seires!" << endl << endl;

        int x_fft[box],y_fft[box];
        float x_fft_res[box],y_fft_res[box];
        for(int i=0;i<box;i++)
        {
            x_fft[i]=i-box/2;
            y_fft[i]=i-box/2;
            x_fft_res[i]=float(x_fft[i])/float(box)/ctf_para_avg.getPixelSize();
            y_fft_res[i]=float(y_fft[i])/float(box)/ctf_para_avg.getPixelSize();
        }
        float x_fft_1d_res[box/2-1];
        for(int i=0;i<box/2-1;i++)
        {
            x_fft_1d_res[i]=float(i)/float(box)/ctf_para_avg.getPixelSize();
        }
        float *res2_all=new float[box*box];
        for(int j=0;j<box;j++)
        {
            for(int i=0;i<box;i++)
            {
                res2_all[j*box+i]=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
            }
        }
        float *atan_all=new float[box*box];
        for(int j=0;j<box;j++)
        {
            for(int i=0;i<box;i++)
            {
                atan_all[j*box+i]=atan2(y_fft_res[j],x_fft_res[i]);
            }
        }

        MRC stack_psd(output_mrc.c_str(),"w");
        if(!stack_psd.hasFile())
        {
            cerr << "[Error] Cannot open output file " << output_mrc << "!" << endl;
            abort();
        }
        stack_psd.createMRC_empty(box,box,Nz,2);
        
        /*
        FILE *fdefocus=fopen(defocus_file.c_str(),"w");
        if(!fdefocus)
        {
            cerr << endl << "Cannot open output defocus file!" << endl;
            abort();
        }
        */
        
        // int N_block_x=Nx/box*2;
        // int N_block_y=Ny/box*2;
        // int block_x_min,block_x_max;
        // int block_y_min,block_y_max;
        // int N_block=get_blocks(box,&N_block_x,&N_block_y,&block_x_min,&block_x_max,&block_y_min,&block_y_max,Nx,Ny);
        // int *block_x=new int[N_block];
        // int *block_y=new int[N_block];
        // get_block_coords(block_x,block_y,box,block_x_min,block_x_max,block_y_min,block_y_max,N_block_x,N_block_y,Nx,Ny);
        // cout << "Number of blocks: " << N_block << " = " << N_block_x << " * " << N_block_y << endl;

        int N_block_x,N_block_y;
        int block_x_min,block_x_max;
        int block_y_min,block_y_max;
        int N_block;
        if(tight_blocks)
        {
            N_block_x=Nx/box*2-2;
            N_block_y=Ny/box*2-2;
            N_block=get_blocks_tight(box,&N_block_x,&N_block_y,&block_x_min,&block_x_max,&block_y_min,&block_y_max,Nx,Ny);
        }
        else
        {
            N_block_x=Nx/box*2;
            N_block_y=Ny/box*2;
            N_block=get_blocks(box,&N_block_x,&N_block_y,&block_x_min,&block_x_max,&block_y_min,&block_y_max,Nx,Ny);
        }
        int *block_x=new int[N_block];
        int *block_y=new int[N_block];
        get_block_coords(block_x,block_y,box,block_x_min,block_x_max,block_y_min,block_y_max,N_block_x,N_block_y,Nx,Ny);
        if(tight_blocks)
        {
            cout << "Number of blocks: " << N_block << " = (" << N_block_x+2 << " - 2) * (" << N_block_y+2 << " - 2)" << endl;
        }
        else
        {
            cout << "Number of blocks: " << N_block << " = " << N_block_x << " * " << N_block_y << endl;
        }

        double *psd_avg_omp[threads];
        float *fft_buf_in_omp[threads];
        float *fft_buf_out_omp[threads];
        fftwf_plan plan_fft_omp[threads];
        fftwf_plan plan_fft_omp_half[threads];
        fftwf_plan plan_ifft_omp[threads];
        fftwf_plan plan_ifft_omp_half[threads];
        for(int th=0;th<threads;th++)
        {
            psd_avg_omp[th]=new double[box*box];
            fft_buf_in_omp[th]=new float[box*box*2];
            fft_buf_out_omp[th]=new float[box*box*2];
            for(int i=0;i<box*box;i++)
            {
                psd_avg_omp[th][i]=0.0;
                fft_buf_in_omp[th][2*i]=0.0;
                fft_buf_in_omp[th][2*i+1]=0.0;
                fft_buf_out_omp[th][2*i]=0.0;
                fft_buf_out_omp[th][2*i+1]=0.0;
            }
            plan_fft_omp[th]=fftwf_plan_dft_2d(box,box,reinterpret_cast<fftwf_complex*>(fft_buf_in_omp[th]),reinterpret_cast<fftwf_complex*>(fft_buf_out_omp[th]),-1,FFTW_ESTIMATE);
            plan_fft_omp_half[th]=fftwf_plan_dft_2d(box/2,box/2,reinterpret_cast<fftwf_complex*>(fft_buf_in_omp[th]),reinterpret_cast<fftwf_complex*>(fft_buf_out_omp[th]),-1,FFTW_ESTIMATE);
            plan_ifft_omp[th]=fftwf_plan_dft_2d(box,box,reinterpret_cast<fftwf_complex*>(fft_buf_in_omp[th]),reinterpret_cast<fftwf_complex*>(fft_buf_out_omp[th]),1,FFTW_ESTIMATE);
            plan_ifft_omp_half[th]=fftwf_plan_dft_2d(box/2,box/2,reinterpret_cast<fftwf_complex*>(fft_buf_in_omp[th]),reinterpret_cast<fftwf_complex*>(fft_buf_out_omp[th]),1,FFTW_ESTIMATE);
        }

        float **psd_block_all[Nz];
        #pragma omp parallel for num_threads(threads)
        for(int n=0;n<Nz;n++)
        {
            psd_block_all[n]=new float*[N_block];
            for(int m=0;m<N_block;m++)
            {
                psd_block_all[n][m]=new float[box*box];
                if(is_resampling)
                {
                    get_psd_one_padding_omp(image_all[n],psd_block_all[n][m],block_x[m],block_y[m],box,(df_max+df_min)/2,pix,psi,theta[n],Nx,Ny,plan_fft_omp[omp_get_thread_num()],plan_ifft_omp_half[omp_get_thread_num()],fft_buf_in_omp[omp_get_thread_num()],fft_buf_out_omp[omp_get_thread_num()]);
                }
                else
                {
                    get_psd_scaling_one_omp(image_all[n],psd_block_all[n][m],block_x[m],block_y[m],box,(df_max+df_min)/2,pix,psi,theta[n],Nx,Ny,false,plan_fft_omp[omp_get_thread_num()],fft_buf_in_omp[omp_get_thread_num()],fft_buf_out_omp[omp_get_thread_num()]);
                }
            }
        }

        cout << "Coarse estimation with 0-degree micrograph:" << endl;
        float *psd_0=new float[box*box];
        for(int i=0;i<box*box;i++)
        {
            psd_0[i]=0.0;
        }
        for(int m=0;m<N_block;m++)  // 相位板无法联合估计
        {
            for(int i=0;i<box*box;i++)
            {
                psd_0[i]+=psd_block_all[N_ref][m][i];
            }
        }
        for(int i=0;i<box*box;i++)
        {
            psd_0[i]/=float(N_block);
        }

        if(box_conv>0 && !skip_box_conv_coarse)
        {
            get_psd_conv_fft(psd_0,box,box_conv,plan_fft_omp[0],plan_ifft_omp[0],fft_buf_in_omp[0],fft_buf_out_omp[0]);
        }
        
        int angle_interval=5;
        float cc_max_df[180/angle_interval+1];
        float cc_max[180/angle_interval+1];

        // MRC mlog_0("log_0.mrc","w");
        // mlog_0.createMRC_empty(box,box,180/angle_interval+1,2);
        #pragma omp parallel for num_threads(threads)
        for(int m=0;m<180/angle_interval+1;m++)  // 遍历相位差
        {
            float ph_deg=m*angle_interval;
            float ph=ph_deg*M_PI/180.0;

            float res_min_now=res_min;
            float res_max_now=res_max;

            // coarse 1d search
            float cc_0[int(ceil((df_max-df_min)/df_step))+2];
            float cc_0_max=-1.0;
            float df_cc_max=df_min;
            for(int t=0;t<=int(floor((df_max-df_min)/df_step));t++)
            {
                CTF ctf_para_now=ctf_para_avg;
                ctf_para_now.setAllCTFPara(df_min+t*df_step,df_min+t*df_step,0,ph,ctf_para_now.getW());

                if(res_ada)
                {
                    int k=2;
                    float delta=M_PI*M_PI*ctf_para_now.getLambda()*ctf_para_now.getLambda()*((df_min+t*df_step)*1e-10)*((df_min+t*df_step)*1e-10)-2*M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*((k)*M_PI-ctf_para_now.getW_phase()-ctf_para_now.getPhaseShift());
                    if(delta<0)
                    {
                        while(delta<0)
                        {
                            k--;
                            delta=M_PI*M_PI*ctf_para_now.getLambda()*ctf_para_now.getLambda()*((df_min+t*df_step)*1e-10)*((df_min+t*df_step)*1e-10)-2*M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*(k*M_PI-ctf_para_now.getW_phase()-ctf_para_now.getPhaseShift());
                        }
                        res_min_now=sqrt((M_PI*ctf_para_now.getLambda()*((df_min+t*df_step)*1e-10)+sqrt(M_PI*M_PI*ctf_para_now.getLambda()*ctf_para_now.getLambda()*((df_min+t*df_step)*1e-10)*((df_min+t*df_step)*1e-10)-2*M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*((k)*M_PI-ctf_para_now.getW_phase()-ctf_para_now.getPhaseShift())))/(M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())));
                        float res_max_tmp=sqrt((M_PI*ctf_para_now.getLambda()*((df_min+t*df_step)*1e-10)+sqrt(M_PI*M_PI*ctf_para_now.getLambda()*ctf_para_now.getLambda()*((df_min+t*df_step)*1e-10)*((df_min+t*df_step)*1e-10)-2*M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*((k-ceil(float(N_zeros)/2.0))*M_PI-ctf_para_now.getW_phase()-ctf_para_now.getPhaseShift())))/(M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())));
                        if(res_max_tmp<1.0/(pix*1e-10)/2.0) // 没有超过Nyquist频率
                        {
                            res_max_now=res_max_tmp;
                        }
                        else
                        {
                            res_max_now=1.0/(pix*1e-10)/2.0;
                        }
                    }
                    else
                    {
                        res_min_now=sqrt((M_PI*ctf_para_now.getLambda()*((df_min+t*df_step)*1e-10)-sqrt(M_PI*M_PI*ctf_para_now.getLambda()*ctf_para_now.getLambda()*((df_min+t*df_step)*1e-10)*((df_min+t*df_step)*1e-10)-2*M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*((k)*M_PI-ctf_para_now.getW_phase()-ctf_para_now.getPhaseShift())))/(M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())));
                        float res_max_tmp=sqrt((M_PI*ctf_para_now.getLambda()*((df_min+t*df_step)*1e-10)-sqrt(M_PI*M_PI*ctf_para_now.getLambda()*ctf_para_now.getLambda()*((df_min+t*df_step)*1e-10)*((df_min+t*df_step)*1e-10)-2*M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*((k+ceil(float(N_zeros)/2.0))*M_PI-ctf_para_now.getW_phase()-ctf_para_now.getPhaseShift())))/(M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())));
                        if(res_max_tmp<1.0/(pix*1e-10)/2.0) // 没有超过Nyquist频率
                        {
                            res_max_now=res_max_tmp;
                        }
                        else
                        {
                            res_max_now=1.0/(pix*1e-10)/2.0;
                        }
                    }
                }
                if(res_min_now>=res_max_now)    // 拟合的最小频率比最大频率要高，无法完成拟合
                {
                    cc_0[t]=-1.0;
                }
                else
                {
                    cc_0[t]=get_correlation_adaptive_phase_shift_zero_mean(df_min+t*df_step,box,N_zeros,ctf_para_avg,res_min_now,res_max_now,psd_0,ph,x_fft_1d_res);
                }
            }
            for(int t=0;t<=int(floor((df_max-df_min)/df_step));t++)
            {
                if(cc_0[t]>cc_0_max)
                {
                    cc_0_max=cc_0[t];
                    df_cc_max=df_min+t*df_step;
                }
            }
            cc_max[m]=cc_0_max;
            cc_max_df[m]=df_cc_max;

            cout << ph_deg << " degree: df = " << cc_max_df[m] << ", with cc = " << cc_max[m] << endl;

            float *psd_show=new float[box*box];
            CTF ctf_para_show=ctf_para_avg;
            ctf_para_show.setAllCTFPara(df_cc_max,df_cc_max,0,ph,ctf_para_show.getW());
            if(res_ada)
            {
                int k=2;
                float delta=M_PI*M_PI*ctf_para_show.getLambda()*ctf_para_show.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_show.getCs()*(ctf_para_show.getLambda()*ctf_para_show.getLambda()*ctf_para_show.getLambda())*((k)*M_PI-ctf_para_show.getW_phase()-ctf_para_show.getPhaseShift());
                if(delta<0)
                {
                    while(delta<0)
                    {
                        k--;
                        delta=M_PI*M_PI*ctf_para_show.getLambda()*ctf_para_show.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_show.getCs()*(ctf_para_show.getLambda()*ctf_para_show.getLambda()*ctf_para_show.getLambda())*(k*M_PI-ctf_para_show.getW_phase()-ctf_para_show.getPhaseShift());
                    }
                    res_min_now=sqrt((M_PI*ctf_para_show.getLambda()*(df_cc_max*1e-10)+sqrt(M_PI*M_PI*ctf_para_show.getLambda()*ctf_para_show.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_show.getCs()*(ctf_para_show.getLambda()*ctf_para_show.getLambda()*ctf_para_show.getLambda())*((k)*M_PI-ctf_para_show.getW_phase()-ctf_para_show.getPhaseShift())))/(M_PI*ctf_para_show.getCs()*(ctf_para_show.getLambda()*ctf_para_show.getLambda()*ctf_para_show.getLambda())));
                    float res_max_tmp=sqrt((M_PI*ctf_para_show.getLambda()*(df_cc_max*1e-10)+sqrt(M_PI*M_PI*ctf_para_show.getLambda()*ctf_para_show.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_show.getCs()*(ctf_para_show.getLambda()*ctf_para_show.getLambda()*ctf_para_show.getLambda())*((k-ceil(float(N_zeros)/2.0))*M_PI-ctf_para_show.getW_phase()-ctf_para_show.getPhaseShift())))/(M_PI*ctf_para_show.getCs()*(ctf_para_show.getLambda()*ctf_para_show.getLambda()*ctf_para_show.getLambda())));
                    if(res_max_tmp<1.0/(pix*1e-10)/2.0) // 没有超过Nyquist频率
                    {
                        res_max_now=res_max_tmp;
                    }
                    else
                    {
                        res_max_now=1.0/(pix*1e-10)/2.0;
                    }
                }
                else
                {
                    res_min_now=sqrt((M_PI*ctf_para_show.getLambda()*(df_cc_max*1e-10)-sqrt(M_PI*M_PI*ctf_para_show.getLambda()*ctf_para_show.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_show.getCs()*(ctf_para_show.getLambda()*ctf_para_show.getLambda()*ctf_para_show.getLambda())*((k)*M_PI-ctf_para_show.getW_phase()-ctf_para_show.getPhaseShift())))/(M_PI*ctf_para_show.getCs()*(ctf_para_show.getLambda()*ctf_para_show.getLambda()*ctf_para_show.getLambda())));
                    float res_max_tmp=sqrt((M_PI*ctf_para_show.getLambda()*(df_cc_max*1e-10)-sqrt(M_PI*M_PI*ctf_para_show.getLambda()*ctf_para_show.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_show.getCs()*(ctf_para_show.getLambda()*ctf_para_show.getLambda()*ctf_para_show.getLambda())*((k+ceil(float(N_zeros)/2.0))*M_PI-ctf_para_show.getW_phase()-ctf_para_show.getPhaseShift())))/(M_PI*ctf_para_show.getCs()*(ctf_para_show.getLambda()*ctf_para_show.getLambda()*ctf_para_show.getLambda())));
                    if(res_max_tmp<1.0/(pix*1e-10)/2.0) // 没有超过Nyquist频率
                    {
                        res_max_now=res_max_tmp;
                    }
                    else
                    {
                        res_max_now=1.0/(pix*1e-10)/2.0;
                    }
                }
            }
            get_psd_fit_show_image(psd_0,psd_show,box,ctf_para_show,res_min_now,res_max_now,res2_all,atan_all);
            // mlog_0.write2DIm(psd_show,m);
            delete [] psd_show;
        }
        // mlog_0.close();
        float cc_max_ph=-1.0;
        float df_cc_max=cc_max_df[0];
        float ph_cc_max;
        for(int m=0;m<180/angle_interval+1;m++)
        {
            if(cc_max[m]>cc_max_ph)
            {
                cc_max_ph=cc_max[m];
                df_cc_max=cc_max_df[m];
                ph_cc_max=m*angle_interval;
            }
        }
        cout << "Find maximum at " << ph_cc_max << " degree phase shift, df = " << df_cc_max << ", with cc = " << cc_max_ph << endl;
        ctf_para_avg.setAllCTFPara(df_cc_max,df_cc_max,0,ph_cc_max*M_PI/180.0,ctf_para_avg.getW());

        if(res_ada)
        {
            int k=2;
            float delta=M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(k*M_PI-ctf_para_avg.getW_phase()-ctf_para_avg.getPhaseShift());
            if(delta<0)
            {
                while(delta<0)
                {
                    k--;
                    delta=M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(k*M_PI-ctf_para_avg.getW_phase()-ctf_para_avg.getPhaseShift());
                }
                res_min=sqrt((M_PI*ctf_para_avg.getLambda()*(df_cc_max*1e-10)+sqrt(M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*((k)*M_PI-ctf_para_avg.getW_phase()-ctf_para_avg.getPhaseShift())))/(M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())));
                float res_max_tmp=sqrt((M_PI*ctf_para_avg.getLambda()*(df_cc_max*1e-10)+sqrt(M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*((k-ceil(float(N_zeros)/2.0))*M_PI-ctf_para_avg.getW_phase()-ctf_para_avg.getPhaseShift())))/(M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())));
                if(res_max_tmp<1.0/(pix*1e-10)/2.0) // 没有超过Nyquist频率
                {
                    res_max=res_max_tmp;
                }
                else
                {
                    res_max=1.0/(pix*1e-10)/2.0;
                }
            }
            else
            {
                res_min=sqrt((M_PI*ctf_para_avg.getLambda()*(df_cc_max*1e-10)-sqrt(M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*((k)*M_PI-ctf_para_avg.getW_phase()-ctf_para_avg.getPhaseShift())))/(M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())));
                float res_max_tmp=sqrt((M_PI*ctf_para_avg.getLambda()*(df_cc_max*1e-10)-sqrt(M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*((k+ceil(float(N_zeros)/2.0))*M_PI-ctf_para_avg.getW_phase()-ctf_para_avg.getPhaseShift())))/(M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())));
                if(res_max_tmp<1.0/(pix*1e-10)/2.0) // 没有超过Nyquist频率
                {
                    res_max=res_max_tmp;
                }
                else
                {
                    res_max=1.0/(pix*1e-10)/2.0;
                }
            }
            cout << "Use adaptive resolution boundary:" << endl;
            cout << "Modified res_min (first zero): " << 1/res_min*1e10 << endl;
            cout << "Modified res_max (N/2-th zero): " << 1/res_max*1e10 << endl;
        }

        // coarse search for astigmatism
        // get psd with scaling
        // get_average_psd_omp(image_now,psd_0,box,df_cc_max,pix,psi,0.0,Nx,Ny,plan_fft_omp[0],fft_buf_in_omp[0],fft_buf_out_omp[0]);
        for(int i=0;i<box*box;i++)
        {
            psd_0[i]=0.0;
        }
        float *psd_tmp=new float[box*box];
        for(int m=0;m<N_block;m++)
        {
            get_scaled_psd(psd_block_all[N_ref][m],psd_tmp,block_x[m],block_y[m],box,df_cc_max,pix,psi,theta[N_ref],Nx,Ny);
            for(int i=0;i<box*box;i++)
            {
                psd_0[i]+=double(psd_tmp[i]);
            }
        }
        for(int i=0;i<box*box;i++)
        {
            psd_0[i]/=float(N_block);
        }
        delete [] psd_tmp;
        // box convolution
        if(box_conv>0)
        {
            get_psd_conv_fft(psd_0,box,box_conv,plan_fft_omp[0],plan_ifft_omp[0],fft_buf_in_omp[0],fft_buf_out_omp[0]);
        }

        int astig_interval=5;
        int N_angles=180/astig_interval+1;
        float cc_astig[N_angles];
        float df_1_astig[N_angles];
        float df_2_astig[N_angles];
        float cc_astig_max=0;
        int index=0;
        float res_min_astig=res_min,res_max_astig=res_max;
        cout << endl << "Search for astigmatism:" << endl;
        #pragma omp parallel for num_threads(threads)
        for(int m=0;m<N_angles;m++)
        {
            CTF ctf_para_astig=ctf_para_avg;
            ctf_para_astig.setAllCTFPara(df_cc_max,df_cc_max,m*astig_interval,ph_cc_max*M_PI/180.0,ctf_para_astig.getW());
            Data_opt_epa data_opt;
            data_opt.box=box;
            data_opt.ctf_para=ctf_para_astig;
            data_opt.psd=psd_0;
            data_opt.res_min=res_min_astig;
            data_opt.res_max=res_max_astig;
            data_opt.chi_min=M_PI*ctf_para_avg.getLambda()*df_cc_max*1e-10*(res_min_astig*res_min_astig)-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*double(res_min_astig*res_min_astig)*double(res_min_astig*res_min_astig)+ctf_para_avg.getW_phase()+ctf_para_avg.getPhaseShift();
            data_opt.chi_max=M_PI*ctf_para_avg.getLambda()*df_cc_max*1e-10*(res_max_astig*res_max_astig)-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*double(res_max_astig*res_max_astig)*double(res_max_astig*res_max_astig)+ctf_para_avg.getW_phase()+ctf_para_avg.getPhaseShift();
            data_opt.res2_all=res2_all;
            data_opt.atan_all=atan_all;
            data_opt.x_fft_1d_res=x_fft_1d_res;

            nlopt_opt opt;
            opt=nlopt_create(NLOPT_LN_NELDERMEAD,2);
            nlopt_set_max_objective(opt,get_correlation_astig_epa_phase_shift_zero_mean,&data_opt);
            nlopt_set_xtol_rel(opt,1e-4);
            nlopt_set_ftol_rel(opt,1e-4);
            double step[2]={1e3,1e3};
            nlopt_set_initial_step(opt,step);
            nlopt_set_lower_bounds1(opt,0);
            // nlopt_set_upper_bounds1(opt,df_max);
            double x[2]={df_cc_max+1e3,df_cc_max-1e3};
            if(df_cc_max-1e3<0)
            {
                x[1]=0.0;
            }
            double cc_max;
            if(nlopt_optimize(opt,x,&cc_max)<0)
            {
                cout << m*astig_interval << " degrees: " << "nlopt failed!" << endl;
                df_1_astig[m]=df_cc_max;
                df_2_astig[m]=df_cc_max;
                cc_astig[m]=0.0;
            }
            else
            {
                if(x[0]==df_min || x[0]==df_max || x[1]==df_min || x[2]==df_max)    // optimization failed
                {
                    x[0]=df_cc_max;
                    x[1]=df_cc_max;
                    cc_max=-1.0;
                }
                cout << m*astig_interval << " degrees: " << "Found maximum at (df_1,df_2): (" << x[0] << "," << x[1] << "), with cc = " << cc_max << endl;
                cc_astig[m]=cc_max;
                df_1_astig[m]=x[0];
                df_2_astig[m]=x[1];
            }
        }
        for(int m=0;m<N_angles;m++)
        {
            if(cc_astig[m]>cc_astig_max)
            {
                cc_astig_max=cc_astig[m];
                index=m;
            }
        }
        cout << "Best fit at (df_1,df_2,astig): (" << df_1_astig[index] << "," << df_2_astig[index] << "," << index*astig_interval*M_PI/180.0 << ")" << endl;
        ctf_para_avg.setAllCTFPara(df_1_astig[index],df_2_astig[index],index*astig_interval,ctf_para_avg.getPhaseShift(),w_cos);

        // local refinement
        Data_opt_epa data_opt;
        data_opt.box=box;
        data_opt.ctf_para=ctf_para_avg;
        data_opt.psd=psd_0;
        data_opt.res_min=res_min;
        data_opt.res_max=res_max;
        float df=(ctf_para_avg.getDefocus1()+ctf_para_avg.getDefocus2()+(ctf_para_avg.getDefocus1()-ctf_para_avg.getDefocus2())*cos(2*(-ctf_para_avg.getAstigmatism())))/2;
        data_opt.chi_min=M_PI*ctf_para_avg.getLambda()*df*(res_min*res_min)-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*double(res_min*res_min)*double(res_min*res_min)+ctf_para_avg.getW_phase()+ctf_para_avg.getPhaseShift();
        data_opt.chi_max=M_PI*ctf_para_avg.getLambda()*df*(res_max*res_max)-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*double(res_max*res_max)*double(res_max*res_max)+ctf_para_avg.getW_phase()+ctf_para_avg.getPhaseShift();
        data_opt.res2_all=res2_all;
        data_opt.atan_all=atan_all;
        data_opt.x_fft_1d_res=x_fft_1d_res;
        
        nlopt_opt opt;
        opt=nlopt_create(NLOPT_LN_NELDERMEAD,4);
        nlopt_set_max_objective(opt,get_correlation_image_epa_phase_shift_zero_mean,&data_opt);
        nlopt_set_xtol_rel(opt,1e-4);
        nlopt_set_ftol_rel(opt,1e-4);
        double step[4]={1e2,1e2,M_PI/180.0,M_PI/180.0};
        nlopt_set_initial_step(opt,step);
        double x[4]={df_1_astig[index],df_2_astig[index],index*astig_interval*M_PI/180.0,ph_cc_max*M_PI/180.0};
        double cc_max_d;
        if(nlopt_optimize(opt,x,&cc_max_d)<0)
        {
            cout << "[Error] nlopt failed!" << endl;
        }
        else
        {
            cout << "Found maximum at (df_1,df_2,astig,phase_shift): (" << x[0] << "," << x[1] << "," << x[2]*180.0/M_PI << "," << x[3]*180.0/M_PI << ")" << endl;
        }
        ctf_para_avg.setAllCTFPara(x[0],x[1],x[2]*180.0/M_PI,x[3],ctf_para_avg.getW());   // 特别注意：astig传给CTF的单位是角度，但过来的是弧度！！

        cout << endl << "Refine for every micrograph: " << endl;
        float *psd_show_all[Nz];
        for(int n=0;n<Nz;n++)
        {
            psd_show_all[n]=new float[box*box];
        }
        CTF ctf_para_all[Nz];
        float cc_max_all[Nz];

        #pragma omp parallel for num_threads(threads)
        for(int n=0;n<Nz;n++)
        {
            cout << n << ": " << endl;

            CTF ctf_para_now=ctf_para_avg;

            float *psd_now=new float[box*box];
            float *psd_tmp=new float[box*box];
            for(int i=0;i<box*box;i++)
            {
                psd_now[i]=0.0;
            }
            for(int m=0;m<N_block;m++)
            {
                get_scaled_psd(psd_block_all[n][m],psd_tmp,block_x[m],block_y[m],box,df*1e10,pix,psi,theta[n],Nx,Ny);
                for(int i=0;i<box*box;i++)
                {
                    psd_now[i]+=psd_tmp[i];
                }
            }
            for(int i=0;i<box*box;i++)
            {
                psd_now[i]/=float(N_block);
            }
            delete [] psd_tmp;

            // box convolution
            if(box_conv>0)
            {
                get_psd_conv_fft(psd_now,box,box_conv,plan_fft_omp[omp_get_thread_num()],plan_ifft_omp[omp_get_thread_num()],fft_buf_in_omp[omp_get_thread_num()],fft_buf_out_omp[omp_get_thread_num()]);
            }

            df=(ctf_para_now.getDefocus1()+ctf_para_now.getDefocus2()+(ctf_para_now.getDefocus1()-ctf_para_now.getDefocus2())*cos(2*(-ctf_para_now.getAstigmatism())))/2;

            // local refinement
            Data_opt_epa data_opt;
            data_opt.box=box;
            data_opt.ctf_para=ctf_para_now;
            data_opt.psd=psd_now;
            data_opt.res_min=res_min;
            data_opt.res_max=res_max;
            float df=(ctf_para_now.getDefocus1()+ctf_para_now.getDefocus2()+(ctf_para_now.getDefocus1()-ctf_para_now.getDefocus2())*cos(2*(-ctf_para_now.getAstigmatism())))/2;
            data_opt.chi_min=M_PI*ctf_para_now.getLambda()*df*(res_min*res_min)-M_PI_2*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*double(res_min*res_min)*double(res_min*res_min)+ctf_para_now.getW_phase()+ctf_para_now.getPhaseShift();
            data_opt.chi_max=M_PI*ctf_para_now.getLambda()*df*(res_max*res_max)-M_PI_2*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*double(res_max*res_max)*double(res_max*res_max)+ctf_para_now.getW_phase()+ctf_para_now.getPhaseShift();
            data_opt.res2_all=res2_all;
            data_opt.atan_all=atan_all;
            data_opt.x_fft_1d_res=x_fft_1d_res;
            
            nlopt_opt opt;
            opt=nlopt_create(NLOPT_LN_NELDERMEAD,4);
            if(optimize_with_avg)
            {
                nlopt_set_max_objective(opt,get_correlation_image_epa_phase_shift_zero_mean,&data_opt);
            }
            else
            {
                nlopt_set_max_objective(opt,get_correlation_image_phase_shift_zero_mean,&data_opt);
            }
            nlopt_set_xtol_rel(opt,1e-4);
            nlopt_set_ftol_rel(opt,1e-4);
            double step[4]={1e2,1e2,M_PI/180.0,M_PI/180.0};
            nlopt_set_initial_step(opt,step);
            double x[4]={df_1_astig[index],df_2_astig[index],index*astig_interval*M_PI/180.0,ph_cc_max*M_PI/180.0};
            double cc_max_d;
            if(nlopt_optimize(opt,x,&cc_max_d)<0)
            {
                cout << "[Error] nlopt failed!" << endl;
            }
            else
            {
                cout << "Found maximum at (df_1,df_2,astig,phase_shift): (" << x[0] << "," << x[1] << "," << x[2]*180.0/M_PI << "," << x[3]*180.0/M_PI << ")" << endl;
            }
            ctf_para_now.setAllCTFPara(x[0],x[1],x[2]*180.0/M_PI,x[3],ctf_para_now.getW());   // 特别注意：astig传给CTF的单位是角度，但过来的是弧度！！
            
            // 输出拟合结果
            df=(ctf_para_now.getDefocus1()+ctf_para_now.getDefocus2()+(ctf_para_now.getDefocus1()-ctf_para_now.getDefocus2())*cos(2*(-ctf_para_now.getAstigmatism())))/2;
            double chi_min=M_PI*ctf_para_now.getLambda()*df*(res_min_astig*res_min_astig)-M_PI_2*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*double(res_min_astig*res_min_astig)*double(res_min_astig*res_min_astig)+ctf_para_now.getW_phase()+ctf_para_now.getPhaseShift();
            double chi_max=M_PI*ctf_para_now.getLambda()*df*(res_max_astig*res_max_astig)-M_PI_2*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*double(res_max_astig*res_max_astig)*double(res_max_astig*res_max_astig)+ctf_para_now.getW_phase()+ctf_para_now.getPhaseShift();
            get_psd_fit_show_epa(psd_now,psd_show_all[n],box,ctf_para_now,res_min,res_max,chi_min,chi_max,res2_all,atan_all,x_fft_1d_res);
            ctf_para_all[n]=ctf_para_now;
            cc_max_all[n]=cc_max_d;

            delete [] psd_now;
        }

        /*
        for(int n=0;n<Nz;n++)
        {
            stack_psd.write2DIm(psd_show_all[n],n);
            fprintf(fdefocus,"%d %f %f %f %f %f 0.000000\n",n,ctf_para_all[n].getDefocus1()*1e10,ctf_para_all[n].getDefocus2()*1e10,ctf_para_all[n].getAstigmatism()*180.0/M_PI,ctf_para_all[n].getPhaseShift(),cc_max_all[n]);
            delete [] psd_show_all[n];
        }
        fflush(fdefocus);
        fclose(fdefocus);
        */

        // Write out final results
        cout << endl << "Final estimation results:" << endl;
        for(int n=0;n<Nz;n++)
        {
            cout << micrograph_num_all[n] << "@" << micrograph_name_all[n] << ": (df_1,df_2,astig,phase_shift) = (" << ctf_para_all[n].getDefocus1()*1e10 << "," << ctf_para_all[n].getDefocus2()*1e10 << "," << ctf_para_all[n].getAstigmatism()*180.0/M_PI << "," << ctf_para_all[n].getPhaseShift() << "), with cc = " << cc_max_all[n] << endl;
        }

        for(int n=0;n<Nz;n++)
        {
            stack_psd.write2DIm(psd_show_all[n],n);
            delete [] psd_show_all[n];
        }
        stack_psd.close();

        // Write equation for calculating defocus of every pixel
        cout << endl;
        cout << "# Single micrograph power spectrum and Thon ring fitting diagnosis in " << output_mrc << endl;
        cout << "# Estimation results saved in " << output_file << endl;
        cout << endl;

        FILE *fall=fopen(output_file.c_str(),"w");
        if(!fall)
        {
            cerr << "[Error] Cannot open output file!" << endl;
        }
        else
        {
            fprintf(fall,"# Column 1: Frame number@Micrograph name\n");
            fprintf(fall,"# Column 2: Defocus1 (in Angstrom)\n");
            fprintf(fall,"# Column 3: Defocus2 (in Angstrom)\n");
            fprintf(fall,"# Column 4: Astigmatism angle (in degree)\n");
            fprintf(fall,"# Column 5: Phase shift (in radian)\n");
            fprintf(fall,"# Column 6: Tilt axis angle (in degree)\n");
            fprintf(fall,"# Column 7: Tilt angle (in degree)\n");
            fprintf(fall,"# Column 8: Tilt offset around x-axis (x-tilt) (in degree)\n");
            fprintf(fall,"# Column 9: Correlation coefficient\n");
            fprintf(fall,"# Column 10: Well fitted resolution\n\n");

            for(int n=0;n<Nz;n++)
            {
                fprintf(fall,"%02d@%s %9.3f %9.3f %7.2f %7.2f %7.2f %7.2f %7.2f %9.6f %7.4f\n",micrograph_num_all[n],micrograph_name_all[n].c_str(),ctf_para_all[n].getDefocus1()*1e10,ctf_para_all[n].getDefocus2()*1e10,ctf_para_all[n].getAstigmatism()*180.0/M_PI,ctf_para_all[n].getPhaseShift(),psi,theta[n],phi_initial,cc_max_all[n],0.0);
            }

            fprintf(fall,"\n");
            fprintf(fall,"# Equation for calculating defocus values of every pixel:\n");
            fprintf(fall,"# PSI = Tilt axis angle = %7.2f\n",psi);
            fprintf(fall,"# THETA = Tilt angle = (in output file)\n");
            fprintf(fall,"# PHI = Tilt offset around x-axis (x-tilt) = %7.2f\n",phi_initial);
            fprintf(fall,"# PIX = Pixel size = %f (In Angstrom)\n",pix);
            fprintf(fall,"# NX = Micrograph size in X dimension = %d\n",Nx);
            fprintf(fall,"# NY = Micrograph size in Y dimension = %d\n",Ny);
            fprintf(fall,"\n");
            fprintf(fall,"# For pixel (X,Y) (start from 0, X stands for the first (fastest changing) dimension):\n");
            fprintf(fall,"# DX(X) = X - X_CENTER = X - %d\n",int(Nx/2));
            fprintf(fall,"# DY(Y) = Y - Y_CENTER = Y - %d\n",int(Ny/2));
            // fprintf(fall,"# DELTA_Z(X,Y) = (DX(X) * Cos(PSI) + DY(Y) * Sin(PSI)) * Tan(THETA) * PIX\n");
            fprintf(fall,"# DELTA_Z(X,Y) = (DX(X) * (Cos(PHI) * Sin(THETA) * Cos(PSI) - Sin(PHI) * Sin(PSI)) + DY(Y) * (Cos(PHI) * Sin(THETA) * Sin(PSI) + Sin(PHI) * Cos(PSI))) / (Cos(PHI) * Cos(THETA)) * PIX\n");
            fprintf(fall,"# DEFOCUS_1(X,Y) = DEFOCUS_1 + DELTA_Z(X,Y)\n");
            fprintf(fall,"# DEFOCUS_2(X,Y) = DEFOCUS_2 + DELTA_Z(X,Y)\n");
            fprintf(fall,"\n");
            if(single_particle)
            {
                fprintf(fall,"# Input micrograph in %s\n",input_micrograph.c_str());
            }
            else
            {
                fprintf(fall,"# Input micrograph list in %s\n",input_list.c_str());
            }
            fprintf(fall,"# Tilt-series average power spectrum and Thon ring fitting diagnosis in %s\n",average_mrc.c_str());
            fprintf(fall,"# Single micrograph power spectrum and Thon ring fitting diagnosis in %s\n",output_mrc.c_str());
            fprintf(fall,"# Estimation results saved in %s\n",output_file.c_str());
            fprintf(fall,"\n");
            
            fflush(fall);
            fclose(fall);
        }

        for(int n=0;n<Nz;n++)
        {
            delete [] image_all[n];
        }

        for(int th=0;th<threads;th++)
        {
            delete [] psd_avg_omp[th];
            delete [] fft_buf_in_omp[th];
            delete [] fft_buf_out_omp[th];
            fftwf_destroy_plan(plan_fft_omp_half[th]);
            fftwf_destroy_plan(plan_fft_omp[th]);
            fftwf_destroy_plan(plan_ifft_omp[th]);
            fftwf_destroy_plan(plan_ifft_omp_half[th]);
        }
        delete [] res2_all;
        delete [] atan_all;

        delete [] block_x;
        delete [] block_y;

        for(int n=0;n<Nz;n++)
        {
            for(int m=0;m<N_block;m++)
            {
                delete [] psd_block_all[n][m];
            }
            delete [] psd_block_all[n];
        }

        delete [] psd_0;

        stack_psd.close();

        return;
    }



    // CTF estimation
    // 对0度图单独估计CTF初值，选取角度计示数最小的而非校正系统误差后最小的（选取第一张拍摄的，辐照损伤最小）
    time(&current_time);
    strftime(time_buf,1024,"%a %b %d %T %Y",localtime(&current_time));
    cout << endl << "[" << time_buf << "] " << "######## Coarse estimation with 0-degree micrograph ########" << endl;

    int x_fft[box],y_fft[box];
    float x_fft_res[box],y_fft_res[box];
    for(int i=0;i<box;i++)
    {
        x_fft[i]=i-box/2;
        y_fft[i]=i-box/2;
        x_fft_res[i]=float(x_fft[i])/float(box)/ctf_para_avg.getPixelSize();
        y_fft_res[i]=float(y_fft[i])/float(box)/ctf_para_avg.getPixelSize();
    }
    float x_fft_1d_res[box/2-1];
    for(int i=0;i<box/2-1;i++)
    {
        x_fft_1d_res[i]=float(i)/float(box)/ctf_para_avg.getPixelSize();
    }
    float *res2_all=new float[box*box];
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            res2_all[j*box+i]=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
        }
    }
    float *atan_all=new float[box*box];
    for(int j=0;j<box;j++)
    {
        for(int i=0;i<box;i++)
        {
            atan_all[j*box+i]=atan2(y_fft_res[j],x_fft_res[i]);
        }
    }

    // int N_block_x=Nx/box*2;
    // int N_block_y=Ny/box*2;
    // int block_x_min,block_x_max;
    // int block_y_min,block_y_max;
    // int N_block=get_blocks(box,&N_block_x,&N_block_y,&block_x_min,&block_x_max,&block_y_min,&block_y_max,Nx,Ny);
    // int *block_x=new int[N_block];
    // int *block_y=new int[N_block];
    // get_block_coords(block_x,block_y,box,block_x_min,block_x_max,block_y_min,block_y_max,N_block_x,N_block_y,Nx,Ny);
    // cout << "Micrograph size: " << Nx << " * " << Ny << endl;
    // cout << "Number of blocks: " << N_block << " = " << N_block_x << " * " << N_block_y << endl;

    int N_block_x,N_block_y;
    int block_x_min,block_x_max;
    int block_y_min,block_y_max;
    int N_block;
    if(tight_blocks)
    {
        N_block_x=Nx/box*2-2;
        N_block_y=Ny/box*2-2;
        N_block=get_blocks_tight(box,&N_block_x,&N_block_y,&block_x_min,&block_x_max,&block_y_min,&block_y_max,Nx,Ny);
    }
    else
    {
        N_block_x=Nx/box*2;
        N_block_y=Ny/box*2;
        N_block=get_blocks(box,&N_block_x,&N_block_y,&block_x_min,&block_x_max,&block_y_min,&block_y_max,Nx,Ny);
    }
    int *block_x=new int[N_block];
    int *block_y=new int[N_block];
    get_block_coords(block_x,block_y,box,block_x_min,block_x_max,block_y_min,block_y_max,N_block_x,N_block_y,Nx,Ny);
    if(tight_blocks)
    {
        cout << "Number of blocks: " << N_block << " = (" << N_block_x+2 << " - 2) * (" << N_block_y+2 << " - 2)" << endl;
    }
    else
    {
        cout << "Number of blocks: " << N_block << " = " << N_block_x << " * " << N_block_y << endl;
    }

    double *psd_avg_omp[threads];
    float *fft_buf_in_omp[threads];
    float *fft_buf_out_omp[threads];
    fftwf_plan plan_fft_omp[threads];
    fftwf_plan plan_fft_omp_half[threads];
    fftwf_plan plan_ifft_omp[threads];
    fftwf_plan plan_ifft_omp_half[threads];
    for(int th=0;th<threads;th++)
    {
        psd_avg_omp[th]=new double[box*box];
        fft_buf_in_omp[th]=new float[box*box*2];
        fft_buf_out_omp[th]=new float[box*box*2];
        for(int i=0;i<box*box;i++)
        {
            psd_avg_omp[th][i]=0.0;
            fft_buf_in_omp[th][2*i]=0.0;
            fft_buf_in_omp[th][2*i+1]=0.0;
            fft_buf_out_omp[th][2*i]=0.0;
            fft_buf_out_omp[th][2*i+1]=0.0;
        }
        plan_fft_omp[th]=fftwf_plan_dft_2d(box,box,reinterpret_cast<fftwf_complex*>(fft_buf_in_omp[th]),reinterpret_cast<fftwf_complex*>(fft_buf_out_omp[th]),-1,FFTW_ESTIMATE);
        plan_fft_omp_half[th]=fftwf_plan_dft_2d(box/2,box/2,reinterpret_cast<fftwf_complex*>(fft_buf_in_omp[th]),reinterpret_cast<fftwf_complex*>(fft_buf_out_omp[th]),-1,FFTW_ESTIMATE);
        plan_ifft_omp[th]=fftwf_plan_dft_2d(box,box,reinterpret_cast<fftwf_complex*>(fft_buf_in_omp[th]),reinterpret_cast<fftwf_complex*>(fft_buf_out_omp[th]),1,FFTW_ESTIMATE);
        plan_ifft_omp_half[th]=fftwf_plan_dft_2d(box/2,box/2,reinterpret_cast<fftwf_complex*>(fft_buf_in_omp[th]),reinterpret_cast<fftwf_complex*>(fft_buf_out_omp[th]),1,FFTW_ESTIMATE);
    }

    float **psd_block_all[Nz];
    #pragma omp parallel for num_threads(threads)
    for(int n=0;n<Nz;n++)
    {
        psd_block_all[n]=new float*[N_block];
        for(int m=0;m<N_block;m++)
        {
            psd_block_all[n][m]=new float[box*box];
            if(is_resampling)
            {
                get_psd_one_padding_omp(image_all[n],psd_block_all[n][m],block_x[m],block_y[m],box,(df_max+df_min)/2,pix,psi,theta[n],Nx,Ny,plan_fft_omp[omp_get_thread_num()],plan_ifft_omp_half[omp_get_thread_num()],fft_buf_in_omp[omp_get_thread_num()],fft_buf_out_omp[omp_get_thread_num()]);
            }
            else
            {
                get_psd_scaling_one_omp(image_all[n],psd_block_all[n][m],block_x[m],block_y[m],box,(df_max+df_min)/2,pix,psi,theta[n],Nx,Ny,false,plan_fft_omp[omp_get_thread_num()],fft_buf_in_omp[omp_get_thread_num()],fft_buf_out_omp[omp_get_thread_num()]);
            }
        }
    }

    float *psd_0=new float[box*box];
    for(int i=0;i<box*box;i++)
    {
        psd_0[i]=0.0;
    }
    #pragma omp parallel for num_threads(threads)
    for(int n=max(0,N_ref-N_avg);n<=min(N_ref+N_avg,Nz-1);n++)
    {
        for(int m=0;m<N_block;m++)
        {
            for(int i=0;i<box*box;i++)
            {
                psd_avg_omp[omp_get_thread_num()][i]+=double(psd_block_all[n][m][i]);
            }
        }
        for(int i=0;i<box*box;i++)
        {
            psd_avg_omp[omp_get_thread_num()][i]/=double(N_block);
        }
        // cout << n << ": Done" << endl;
        printf("%02d : Done\n",n);
    }
    for(int th=0;th<threads;th++)
    {
        for(int i=0;i<box*box;i++)
        {
            psd_0[i]+=float(psd_avg_omp[th][i]);
        }
    }
    for(int i=0;i<box*box;i++)
    {
        psd_0[i]/=float(2*N_avg+1);
    }

    float *psd_0_nosubtraction=new float[box*box];
    memcpy(psd_0_nosubtraction,psd_0,sizeof(float)*box*box);

    if(box_conv>0 && !skip_box_conv_coarse)
    {
        get_psd_conv_fft(psd_0,box,box_conv,plan_fft_omp[0],plan_ifft_omp[0],fft_buf_in_omp[0],fft_buf_out_omp[0]);
    }

    float cc_0[int(ceil((df_max-df_min)/df_step))+2];
    float cc_0_max=0.0;
    float df_cc_max=df_min;
    int t=0;
    #pragma omp parallel for num_threads(threads)
    for(int n=0;n<=int(floor((df_max-df_min)/df_step));n++)
    {
        cc_0[n]=get_correlation_adaptive_zero_mean(df_min+n*df_step,box,N_zeros,ctf_para_avg,res_min,res_max,psd_0,res2_all,atan_all,x_fft_1d_res);
    }
    for(int n=0;n<=int(floor((df_max-df_min)/df_step));n++)
    {
        // cout << "df = " << df_min+n*df_step << " : cc = " << cc_0[n] << endl;
        printf("df = %8.1f : cc = %9.6f\n",df_min+n*df_step,cc_0[n]);
    }
    for(int n=0;n<=int(floor((df_max-df_min)/df_step));n++)
    {
        if(cc_0[n]>cc_0_max)
        {
            cc_0_max=cc_0[n];
            df_cc_max=df_min+n*df_step;
        }
    }
    cout << "Best fit at df = " << df_cc_max << " with cc = " << cc_0_max << endl;

    if(log_avail)
    {
        float *psd_0_show=new float[box*box];
        ctf_para_avg.setAllCTFPara(df_cc_max,df_cc_max,0,0,w_cos);
        get_psd_fit_show_image(psd_0,psd_0_show,box,ctf_para_avg,res_min,res_max,res2_all,atan_all);
        MRC mlog_0("log_0.mrc","w");
        mlog_0.createMRC_empty(box,box,1,2);
        mlog_0.write2DIm(psd_0_show,0);
        mlog_0.close();
        delete [] psd_0_show;

        psd_0_show=new float[box*box];
        get_psd_show_image(psd_0,psd_0_show,box,res_min,res_max,res2_all,atan_all);
        mlog_0.open("log_0_nofitting.mrc","w");
        mlog_0.createMRC_empty(box,box,1,2);
        mlog_0.write2DIm(psd_0_show,0);
        mlog_0.close();
        delete [] psd_0_show;

        psd_0_show=new float[box*box];
        get_psd_show_image(psd_0_nosubtraction,psd_0_show,box,res_min,res_max,res2_all,atan_all);
        mlog_0.open("log_0_nosubtraction.mrc","w");
        mlog_0.createMRC_empty(box,box,1,2);
        mlog_0.write2DIm(psd_0_show,0);
        mlog_0.close();
        delete [] psd_0_show;
        delete [] psd_0_nosubtraction;

        float log_radial[box/2-1];
        get_radial_average(psd_0,log_radial,box,ctf_para_avg,res2_all,atan_all,x_fft_1d_res);
        FILE *flog_radial_0=fopen("log_0_radial.txt","w");
        for(int i=0;i<box/2-1;i++)
        {
            fprintf(flog_radial_0,"%f\n",log_radial[i]);
        }
        fclose(flog_radial_0);
    }

    if(res_ada)
    {
        res_min=sqrt((M_PI*ctf_para_avg.getLambda()*(df_cc_max*1e-10)-sqrt(M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(M_PI-ctf_para_avg.getW_phase())))/(M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())));
        res_min_2=sqrt((M_PI*ctf_para_avg.getLambda()*(df_cc_max*1e-10)-sqrt(M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(3*M_PI-ctf_para_avg.getW_phase())))/(M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())));   // 显示时第一圈还是太强了，跳过
        float res_max_tmp=sqrt((M_PI*ctf_para_avg.getLambda()*(df_cc_max*1e-10)-sqrt(M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(ceil(N_zeros/2)*M_PI-ctf_para_avg.getW_phase())))/(M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())));
        if(res_max_tmp<1.0/(pix*1e-10)/2.0) // 没有超过Nyquist频率
        {
            res_max=res_max_tmp;
        }
        else
        {
            res_max=1.0/(pix*1e-10)/2.0;
        }
        res_max_tmp=sqrt((M_PI*ctf_para_avg.getLambda()*(df_cc_max*1e-10)-sqrt(M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(float(N_zeros)*M_PI-ctf_para_avg.getW_phase())))/(M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())));
        if(res_max_tmp<1.0/(pix*1e-10)/2.0) // 没有超过Nyquist频率
        {
            res_max_2=res_max_tmp;
        }
        else
        {
            res_max_2=1.0/(pix*1e-10)/2.0;
        }
        cout << "Use adaptive resolution boundary:" << endl;
        cout << "Modified res_min (first zero): " << 1/res_min*1e10 << endl;
        cout << "Modified res_max (N/2-th zero): " << 1/res_max*1e10 << endl;
    }
    // float res_min_high=sqrt((M_PI*ctf_para_avg.getLambda()*(df_cc_max*1e-10)-sqrt(M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(ceil(N_zeros/2)*M_PI-ctf_para_avg.getW_phase())))/(M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())));
    // float res_min_high=sqrt((M_PI*ctf_para_avg.getLambda()*(df_cc_max*1e-10)-sqrt(M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(ceil(N_zeros-4)*M_PI-ctf_para_avg.getW_phase())))/(M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())));
    // float res_min_high=sqrt((M_PI*ctf_para_avg.getLambda()*(df_cc_max*1e-10)-sqrt(M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(3.0*M_PI-ctf_para_avg.getW_phase())))/(M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())));
    // int N_zeros_now=ceil(N_zeros/2);
    // while(res_min_high>1.0/(pix*1e-10)/2.0 && N_zeros_now>=2)
    // {
    //     N_zeros_now--;
    //     res_min_high=sqrt((M_PI*ctf_para_avg.getLambda()*(df_cc_max*1e-10)-sqrt(M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(float(N_zeros_now)*M_PI-ctf_para_avg.getW_phase())))/(M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())));
    // }
    // if(N_zeros_now==1)
    // {
    //     cerr << "No CTF zeros available under the given pixel size!" << endl;
    //     abort();
    // }
    // else
    // {
    //     N_zeros_now--;
    //     res_min_high=sqrt((M_PI*ctf_para_avg.getLambda()*(df_cc_max*1e-10)-sqrt(M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(float(N_zeros_now)*M_PI-ctf_para_avg.getW_phase())))/(M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())));
    // }
    // float res_max_high=sqrt((M_PI*ctf_para_avg.getLambda()*(df_cc_max*1e-10)-sqrt(M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(float(N_zeros-1)*M_PI-ctf_para_avg.getW_phase())))/(M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())));
    // float res_max_high=sqrt((M_PI*ctf_para_avg.getLambda()*(df_cc_max*1e-10)-sqrt(M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*((ceil(N_zeros/2))*M_PI-ctf_para_avg.getW_phase())))/(M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())));
    // if(N_zeros-2<=N_zeros_now)
    // {
    //     res_max_high=sqrt((M_PI*ctf_para_avg.getLambda()*(df_cc_max*1e-10)-sqrt(M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(float(N_zeros_now+1)*M_PI-ctf_para_avg.getW_phase())))/(M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())));
    // }
    // if(res_max_high>1.0/(pix*1e-10)/2.0)
    // {
    //     res_max_high=1.0/(pix*1e-10)/2.0;
    // }

    double chi_min,chi_max;
    double chi_min_2,chi_max_2;
    double chi_min_orig;
    float res_min_high,res_max_high;
    if(!skip_unified)
    {
        time(&current_time);
        strftime(time_buf,1024,"%a %b %d %T %Y",localtime(&current_time));
        cout << endl << "[" << time_buf << "] " << "######## Tilt-seires joint CTF estimation ########" << endl;
        
        // 切小块求平均功率谱
        time(&current_time);
        strftime(time_buf,1024,"%a %b %d %T %Y",localtime(&current_time));
        cout << endl << "[" << time_buf << "] " << "#### Split into small patches to obtain tilt-series PSD ####" << endl;
        float df=df_cc_max;
        // float df=46100; // 单位：A（特别注意单位！！）
        ctf_para_avg.setAllCTFPara(df,df,0,0,w_cos);

        for(int th=0;th<threads;th++)
        {
            for(int i=0;i<box*box;i++)
            {
                psd_avg_omp[th][i]=0.0;
                fft_buf_in_omp[th][2*i]=0.0;
                fft_buf_in_omp[th][2*i+1]=0.0;
                fft_buf_out_omp[th][2*i]=0.0;
                fft_buf_out_omp[th][2*i+1]=0.0;
            }
        }
        #pragma omp parallel for num_threads(threads)
        for(int n=0;n<Nz;n++)
        {
            float *psd_now=new float[box*box];
            for(int m=0;m<N_block;m++)
            {
                // get_scaled_psd(psd_block_all[n][m],psd_now,block_x[m],block_y[m],box,df,pix,psi,theta[n],Nx,Ny);
                get_scaled_psd_phi(psd_block_all[n][m],psd_now,block_x[m],block_y[m],box,df,pix,psi,theta[n],phi_initial,Nx,Ny);
                for(int i=0;i<box*box;i++)
                {
                    psd_avg_omp[omp_get_thread_num()][i]+=double(psd_now[i]);
                }
            }
            // cout << n << ": Done" << endl;
            printf("%02d : Done\n",n);
            delete [] psd_now;
        }
        float *psd_avg=new float[box*box];
        for(int i=0;i<box*box;i++)
        {
            psd_avg[i]=0.0;
        }
        for(int th=0;th<threads;th++)
        {
            for(int i=0;i<box*box;i++)
            {
                psd_avg[i]+=float(psd_avg_omp[th][i]);
            }
        }
        for(int j=0;j<box;j++)
        {
            for(int i=0;i<box;i++)
            {
                psd_avg[j*box+i]/=float(Nz);
                psd_avg[j*box+i]=log(1.0+psd_avg[j*box+i]);  // 取log功率谱
            }
        }

        float *psd_avg_raw=new float[box*box];
        memcpy(psd_avg_raw,psd_avg,sizeof(float)*box*box);

        if(log_avail)
        {
            MRC stack_log("log_1.mrc","w");
            // stack_log.open("log.mrc","w");
            stack_log.createMRC_empty(box,box,1,2);
            stack_log.write2DIm(psd_avg,0);
            stack_log.close();
        }

        if(log_avail)
        {
            float *psd_0_show=new float[box*box];
            // get_psd_show_image_2(psd_avg,psd_0_show,box,res_min,res_max,res2_all,atan_all);
            get_psd_fit_show_epa_subtraction_avg_log(psd_avg,psd_0_show,box,res_min,res_max,res2_all);
            MRC mlog_0;
            mlog_0.open("log_1_before.mrc","w");
            mlog_0.createMRC_empty(box,box,1,2);
            mlog_0.write2DIm(psd_0_show,0);
            mlog_0.close();
            delete [] psd_0_show;

            FILE *flog_radial_avg=fopen("log_radial_avg.txt","w");
            float radial_log[box/2-1];
            get_radial_average(psd_avg,radial_log,box,ctf_para_avg,res2_all,atan_all,x_fft_1d_res);
            for(int i=0;i<box/2-1;i++)
            {
                fprintf(flog_radial_avg,"%f ",radial_log[i]);
            }
            fprintf(flog_radial_avg,"\n");
            fclose(flog_radial_avg);
        }

        // box convolution
        if(box_conv>0 && box_conv_first)
        // if(0)
        {
            get_psd_conv_fft(psd_avg,box,box_conv,plan_fft_omp[0],plan_ifft_omp[0],fft_buf_in_omp[0],fft_buf_out_omp[0]);
            if(log_avail)
            {
                float radial_log[box/2-1];
                get_radial_average(psd_avg,radial_log,box,ctf_para_avg,res2_all,atan_all,x_fft_1d_res);
                FILE *flog_radial_avg=fopen("log_radial_avg.txt","a+");
                for(int i=0;i<box/2-1;i++)
                {
                    fprintf(flog_radial_avg,"%f ",radial_log[i]);
                }
                fprintf(flog_radial_avg,"\n");
                fclose(flog_radial_avg);
            }
        }
        
        // Add B-factor
        float B=100e-20;
        /*
        int x_fft_1d[box/2-1];
        for(int i=0;i<box/2-1;i++)
        {
            x_fft_1d[i]=i;
        }
        get_radial_average(psd_avg,radial,box,ctf_para_avg);
        float psd_max=1.0,psd_min=1.0;
        float psd_max_res2=1.0,psd_min_res2=1.0;
        for(int n=0;n<2;n++)
        {
            double res2=(M_PI*ctf_para_avg.getLambda()*(df*1e-10)-sqrt(M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df*1e-10)*(df*1e-10)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*((float(n)+0.5)*M_PI-atan(ctf_para_avg.getW()/sqrt(1-ctf_para_avg.getW()*ctf_para_avg.getW())))))/(M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()))*(box*box*ctf_para_avg.getPixelSize()*ctf_para_avg.getPixelSize());
            for(int i=0;i<box/2-2;i++)
            {
                if(int(floor(sqrt(res2)))==x_fft_1d[i])
                {
                    if(n==0)
                    {
                        psd_max=radial[i];
                        psd_max_res2=res2/(box*box*ctf_para_avg.getPixelSize()*ctf_para_avg.getPixelSize());
                        psd_min=radial[i];
                        psd_min_res2=res2/(box*box*ctf_para_avg.getPixelSize()*ctf_para_avg.getPixelSize());
                    }
                    else
                    {
                        psd_min=radial[i];
                        psd_min_res2=res2/(box*box*ctf_para_avg.getPixelSize()*ctf_para_avg.getPixelSize());
                    }
                }
            }
        }
        if(psd_max>0 && psd_min>0)
        {
            B=log(psd_max/psd_min)/(psd_min_res2-psd_max_res2);
        }
        else
        {
            B=200e-20;
        }
        
        for(int j=0;j<box;j++)
        {
            for(int i=0;i<box;i++)
            {
                float res2=x_fft_res[i]*x_fft_res[i]+y_fft_res[j]*y_fft_res[j];
                if(res2>=res_min*res_min && res2<=res_max*res_max)
                {
                    psd_avg[j*box+i]*=exp(B*res2);
                }
            }
        }

        stack_log.open("log_3.mrc","w");
        stack_log.createMRC_empty(box,box,1,2);
        stack_log.write2DIm(psd_avg,0);
        stack_log.close();
        */

        // 取radial average
        float *radial=new float[box/2-1];
        get_radial_average(psd_avg,radial,box,ctf_para_avg,res2_all,atan_all,x_fft_1d_res);

        // 通过零点位置拟合估背景
        double fit[box/2-1];
        background_estimation(fit,radial,box,N_zeros,ctf_para_avg);
        if(log_avail)
        {
            FILE *flog_radial_avg=fopen("log_radial_avg.txt","a+");
            for(int i=0;i<box/2-1;i++)
            {
                fprintf(flog_radial_avg,"%f ",fit[i]);
            }
            fprintf(flog_radial_avg,"\n");
            fclose(flog_radial_avg);
        }

        // 回到二维抠背景
        for(int j=0;j<box;j++)
        {
            for(int i=0;i<box;i++)
            {
                if(sqrt(x_fft[i]*x_fft[i]+y_fft[j]*y_fft[j])<box/2-2)
                {
                    int k=int(floor(sqrt(x_fft[i]*x_fft[i]+y_fft[j]*y_fft[j])));
                    float coeff_k=sqrt(x_fft[i]*x_fft[i]+y_fft[j]*y_fft[j])-float(k);
                    psd_avg[j*box+i]=psd_avg[j*box+i]-(1.0-coeff_k)*fit[k]-coeff_k*fit[k+1];
                    // psd_avg[j*box+i]-=fit[k];
                }
                else
                {
                    psd_avg[j*box+i]-=fit[box/2-2];
                }
            }
        }
        if(log_avail)
        {
            float radial_log[box/2-1];
            get_radial_average(psd_avg,radial_log,box,ctf_para_avg,res2_all,atan_all,x_fft_1d_res);
            FILE *flog_radial_avg=fopen("log_radial_avg.txt","a+");
            for(int i=0;i<box/2-1;i++)
            {
                fprintf(flog_radial_avg,"%f ",radial_log[i]);
            }
            fprintf(flog_radial_avg,"\n");
            fflush(flog_radial_avg);
            fclose(flog_radial_avg);
        }

        if(box_conv>0 && !box_conv_first)
        {
            get_psd_conv_fft(psd_avg,box,box_conv,plan_fft_omp[0],plan_ifft_omp[0],fft_buf_in_omp[0],fft_buf_out_omp[0]);
            if(log_avail)
            {
                float radial_log[box/2-1];
                get_radial_average(psd_avg,radial_log,box,ctf_para_avg,res2_all,atan_all,x_fft_1d_res);
                FILE *flog_radial_avg=fopen("log_radial_avg.txt","a+");
                for(int i=0;i<box/2-1;i++)
                {
                    fprintf(flog_radial_avg,"%f ",radial_log[i]);
                }
                fprintf(flog_radial_avg,"\n");
                fclose(flog_radial_avg);
            }
        }

        if(log_avail)
        {
            float *psd_0_show=new float[box*box];
            // get_psd_show_image_2(psd_avg,psd_0_show,box,res_min,res_max,res2_all,atan_all);
            get_psd_fit_show_epa_subtraction_avg_log(psd_avg,psd_0_show,box,res_min,res_max,res2_all);
            MRC mlog_0;
            mlog_0.open("log_1_after.mrc","w");
            mlog_0.createMRC_empty(box,box,1,2);
            mlog_0.write2DIm(psd_0_show,0);
            mlog_0.close();
            delete [] psd_0_show;
        }

        // 遍历astig，每个角度自己精搜，找最大互相关
        time(&current_time);
        strftime(time_buf,1024,"%a %b %d %T %Y",localtime(&current_time));
        cout << endl << "[" << time_buf << "] " << "#### Coarse search for astigmatism ####" << endl;
        int astig_interval=5;
        int N_angles=180/astig_interval+1;
        float cc_astig[N_angles];
        float df_1_astig[N_angles];
        float df_2_astig[N_angles];
        float cc_astig_max=-1.0;
        int index=0;
        float res_min_astig=res_min,res_max_astig=res_max;
        #pragma omp parallel for num_threads(threads)
        for(int n=0;n<N_angles;n++)
        {
            CTF ctf_para_astig=ctf_para_avg;
            ctf_para_astig.setAllCTFPara(df,df,n*astig_interval,0,ctf_para_astig.getW());

            Data_opt_epa data_opt;
            data_opt.box=box;
            data_opt.ctf_para=ctf_para_astig;
            data_opt.psd=psd_avg;
            data_opt.res_min=res_min_astig;
            data_opt.res_max=res_max_astig;
            data_opt.chi_min=M_PI*ctf_para_avg.getLambda()*df*1e-10*(res_min_astig*res_min_astig)-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*double(res_min_astig*res_min_astig)*double(res_min_astig*res_min_astig)+ctf_para_avg.getW_phase();
            data_opt.chi_max=M_PI*ctf_para_avg.getLambda()*df*1e-10*(res_max_astig*res_max_astig)-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*double(res_max_astig*res_max_astig)*double(res_max_astig*res_max_astig)+ctf_para_avg.getW_phase();
            data_opt.res2_all=res2_all;
            data_opt.atan_all=atan_all;
            data_opt.x_fft_1d_res=x_fft_1d_res;

            nlopt_opt opt;
            opt=nlopt_create(NLOPT_LN_NELDERMEAD,2);
            if(adaptive_range)
            {
                nlopt_set_max_objective(opt,get_correlation_astig_epa_zero_mean,&data_opt);
            }
            else
            {
                nlopt_set_max_objective(opt,get_correlation_astig_epa_zero_mean_circle,&data_opt);
            }
            nlopt_set_xtol_rel(opt,1e-4);
            nlopt_set_ftol_rel(opt,1e-4);
            // double step[2]={1e4,1e4};
            // double step[2]={df*1e-1,df*1e-1};
            // double step[2]={1e3,1e3};
            double step[2]={astig_angstrom/2.0,astig_angstrom/2.0};
            if(astig_angstrom<2e2)
            {
                step[0]=1e2;
                step[1]=1e2;
            }
            nlopt_set_initial_step(opt,step);
            // nlopt_set_lower_bounds1(opt,df_min);
            // nlopt_set_upper_bounds1(opt,df_max);
            // double x[2]={df,df};
            // double x[2]={df+1e3,df-1e3};
            double x[2]={df+astig_angstrom/2.0,df-astig_angstrom/2.0};
            double cc_max;
            if(nlopt_optimize(opt,x,&cc_max)<0)
            {
                cout << n*astig_interval << " degrees: " << "nlopt failed!" << endl;
                df_1_astig[n]=df;
                df_2_astig[n]=df;
                cc_astig[n]=0.0;
            }
            else
            {
                if(x[0]==df_min || x[0]==df_max || x[1]==df_min || x[2]==df_max)    // optimization failed
                {
                    x[0]=df;
                    x[1]=df;
                    cc_max=0.0;
                }
                // cout << n*astig_interval << " degrees: " << "Found maximum at (df_1,df_2): (" << x[0] << "," << x[1] << "), with cc = " << cc_max << endl;
                cc_astig[n]=cc_max;
                df_1_astig[n]=x[0];
                df_2_astig[n]=x[1];
            }
        }
        for(int n=0;n<N_angles;n++)
        {
            printf("%3d degrees : Found maximum at ( df_1 , df_2 ) : ( %9.3f , %9.3f ) , with cc = %9.6f\n",n*astig_interval,df_1_astig[n],df_2_astig[n],cc_astig[n]);
            if(cc_astig[n]>cc_astig_max)
            {
                cc_astig_max=cc_astig[n];
                index=n;
            }
        }
        // cout << "Best fit at (df_1,df_2,astig): (" << df_1_astig[index] << "," << df_2_astig[index] << "," << index*astig_interval*M_PI/180.0 << ")" << endl;
        printf("Best fit at ( df_1 , df_2 , astig ) : ( %9.3f , %9.3f , %7.2f )\n",df_1_astig[index],df_2_astig[index],float(index*astig_interval));
        ctf_para_avg.setAllCTFPara(df_1_astig[index],df_2_astig[index],index*astig_interval,0,w_cos);

        // Nelder-Mead算法局部精优化
        time(&current_time);
        strftime(time_buf,1024,"%a %b %d %T %Y",localtime(&current_time));
        cout << endl << "[" << time_buf << "] " << "#### Refinement around the best fitted astigmatism angle ####" << endl;
        Data_opt_epa data_opt;
        data_opt.box=box;
        data_opt.ctf_para=ctf_para_avg;
        data_opt.psd=psd_avg;
        data_opt.res_min=res_min;
        data_opt.res_max=res_max;
        df=(ctf_para_avg.getDefocus1()+ctf_para_avg.getDefocus2()+(ctf_para_avg.getDefocus1()-ctf_para_avg.getDefocus2())*cos(2*(-ctf_para_avg.getAstigmatism())))/2;
        data_opt.chi_min=M_PI*ctf_para_avg.getLambda()*df*(res_min*res_min)-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*double(res_min*res_min)*double(res_min*res_min)+ctf_para_avg.getW_phase();
        data_opt.chi_max=M_PI*ctf_para_avg.getLambda()*df*(res_max*res_max)-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*double(res_max*res_max)*double(res_max*res_max)+ctf_para_avg.getW_phase();
        data_opt.res2_all=res2_all;
        data_opt.atan_all=atan_all;
        data_opt.x_fft_1d_res=x_fft_1d_res;
        
        nlopt_opt opt;
        opt=nlopt_create(NLOPT_LN_NELDERMEAD,3);
        if(adaptive_range)
        {
            nlopt_set_max_objective(opt,get_correlation_image_epa_zero_mean,&data_opt);
        }
        else
        {
            nlopt_set_max_objective(opt,get_correlation_image_epa_zero_mean_circle,&data_opt);
        }
        nlopt_set_xtol_rel(opt,1e-4);
        nlopt_set_ftol_rel(opt,1e-4);
        // double step[3]={df*1e-1,df*1e-1,M_PI_4};
        // double step[3]={1e4,1e4,M_PI_4};
        // double step[3]={1e3,1e3,M_PI/180};
        // double step[3]={1e3,1e3,astig_interval*M_PI/180.0};
        double step[3]={1e2,1e2,M_PI/180.0};
        // double step[3]={df*1e-2,df*1e-2,astig_interval*M_PI/180.0};
        nlopt_set_initial_step(opt,step);
        // double x[3]={df,df,0};
        double x[3]={df_1_astig[index],df_2_astig[index],index*astig_interval*M_PI/180.0};
        double cc_max;
        if(nlopt_optimize(opt,x,&cc_max)<0)
        {
            cout << "[Error] nlopt failed!" << endl;
        }
        else
        {
            // cout << "Found maximum at (df_1,df_2,astig): (" << x[0] << "," << x[1] << "," << x[2]*180.0/M_PI << ")" << endl;
            printf("Found maximum at ( df_1 , df_2 , astig ) : ( %9.3f , %9.3f , %7.2f )\n",x[0],x[1],x[2]*180.0/M_PI);
        }
        ctf_para_avg.setAllCTFPara(x[0],x[1],x[2]*180.0/M_PI,0,ctf_para_avg.getW());   // 特别注意：astig传给CTF的单位是角度，但过来的是弧度！！

        // evaluation
        int n_eva=0;
        n_eva=get_number_of_rings_fit(psd_avg,box,ctf_para_avg,res2_all,atan_all,x_fft_1d_res);
        float res_eva=sqrt((M_PI*ctf_para_avg.getLambda()*(df)-sqrt(M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df)*(df)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(n_eva*M_PI-ctf_para_avg.getW_phase())))/(M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())));
        cout << "Evaluation of fitting: Fitting to the " << n_eva << "-th Thon ring, with resolution = " << 1/(res_eva*1e-10) << endl;

        res_max_high=sqrt((M_PI*ctf_para_avg.getLambda()*(df_cc_max*1e-10)-sqrt(M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(float(n_eva-1)*M_PI-ctf_para_avg.getW_phase())))/(M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())));
        if(res_max_high>1.0/(pix*1e-10)/2.0)
        {
            res_max_high=1.0/(pix*1e-10)/2.0;
        }
        res_min_high=sqrt((M_PI*ctf_para_avg.getLambda()*(df_cc_max*1e-10)-sqrt(M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(4.0*M_PI-ctf_para_avg.getW_phase())))/(M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())));
        if(n_eva-1<=4)
        {
            res_min_high=sqrt((M_PI*ctf_para_avg.getLambda()*(df_cc_max*1e-10)-sqrt(M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(3.0*M_PI-ctf_para_avg.getW_phase())))/(M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())));
        }
        if(n_eva<=4)
        {
            cout << endl << "The quality of the tilt-series is not good enough. Do not have enough CTF zeros to perform angle refinement! Skip all angle refinement procedures!" << endl << endl;
            skip_axis_refinement=1;
            skip_offset_estimation=1;
            skip_offset_estimation_x=1;
            skip_offset_refinement=1;
            skip_angle_refinement=1;
            skip_offset_refinement_x=1;
        }
        /*
        int N_zeros_min=4;
        while(res_min_high>res_max_high && N_zeros_min>=2)
        {
            N_zeros_min--;
            res_min_high=sqrt((M_PI*ctf_para_avg.getLambda()*(df_cc_max*1e-10)-sqrt(M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(float(N_zeros_min)*M_PI-ctf_para_avg.getW_phase())))/(M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())));
        }
        if(N_zeros_min==1)
        {
            cout << endl << "Do not have enough CTF zeros to perform angle refinement, the following result of tilt angles may be wrong!" << endl << endl;
        }
        */

        // 输出拟合结果
        float *psd_show=new float[box*box];
        // get_psd_fit_show(psd_avg,psd_show,box,ctf_para_avg,res_min,res_max);
        df=(ctf_para_avg.getDefocus1()+ctf_para_avg.getDefocus2()+(ctf_para_avg.getDefocus1()-ctf_para_avg.getDefocus2())*cos(2*(-ctf_para_avg.getAstigmatism())))/2;
        // double chi_min=M_PI*ctf_para_avg.getLambda()*df*(res_min_astig*res_min_astig)-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*double(res_min_astig*res_min_astig)*double(res_min_astig*res_min_astig)+ctf_para_avg.getW_phase();
        // double chi_min_orig=M_PI*ctf_para_avg.getLambda()*df*(res_min_orig*res_min_orig)-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*double(res_min_orig*res_min_orig)*double(res_min_orig*res_min_orig)+ctf_para_avg.getW_phase();
        // double chi_max=M_PI*ctf_para_avg.getLambda()*df*(res_max_astig*res_max_astig)-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*double(res_max_astig*res_max_astig)*double(res_max_astig*res_max_astig)+ctf_para_avg.getW_phase();
        // double chi_max_2=M_PI*ctf_para_avg.getLambda()*df*(res_max_2*res_max_2)-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*double(res_max_2*res_max_2)*double(res_max_2*res_max_2)+ctf_para_avg.getW_phase();
        // double chi_min_2=M_PI*ctf_para_avg.getLambda()*df*(res_min_2*res_min_2)-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*double(res_min_2*res_min_2)*double(res_min_2*res_min_2)+ctf_para_avg.getW_phase();
        chi_min=M_PI*ctf_para_avg.getLambda()*df*(res_min_astig*res_min_astig)-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*double(res_min_astig*res_min_astig)*double(res_min_astig*res_min_astig)+ctf_para_avg.getW_phase();
        chi_min_orig=M_PI*ctf_para_avg.getLambda()*df*(res_min_orig*res_min_orig)-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*double(res_min_orig*res_min_orig)*double(res_min_orig*res_min_orig)+ctf_para_avg.getW_phase();
        chi_max=M_PI*ctf_para_avg.getLambda()*df*(res_max_astig*res_max_astig)-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*double(res_max_astig*res_max_astig)*double(res_max_astig*res_max_astig)+ctf_para_avg.getW_phase();
        // chi_max_2=M_PI*ctf_para_avg.getLambda()*df*(res_max_2*res_max_2)-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*double(res_max_2*res_max_2)*double(res_max_2*res_max_2)+ctf_para_avg.getW_phase();
        chi_max_2=M_PI*ctf_para_avg.getLambda()*df*(res_eva*res_eva)-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*double(res_eva*res_eva)*double(res_eva*res_eva)+ctf_para_avg.getW_phase();
        chi_min_2=M_PI*ctf_para_avg.getLambda()*df*(res_min_2*res_min_2)-M_PI_2*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*double(res_min_2*res_min_2)*double(res_min_2*res_min_2)+ctf_para_avg.getW_phase();
        // get_psd_fit_show_epa(psd_avg,psd_show,box,ctf_para_avg,res_min,res_max,chi_min,chi_max,res2_all,atan_all,x_fft_1d_res);
        // get_psd_fit_show_epa_subtraction(psd_avg,psd_show,box,ctf_para_avg,res_min,res_max,chi_min,chi_max,res_min_orig,chi_min_orig,res_max_2,chi_max_2,res2_all,atan_all,x_fft_1d_res);
        if(!output_raw)
        {
            get_psd_fit_show_epa_subtraction_avg(psd_avg,psd_show,box,ctf_para_avg,res_min,res_max,chi_min,chi_max,res_min_orig,chi_min_orig,res_max_2,chi_max_2,res2_all,atan_all,x_fft_1d_res);
        }
        else
        {
            get_psd_fit_show_epa_raw_avg(psd_avg_raw,psd_show,box,ctf_para_avg,res_min,res_max,chi_min,chi_max,res_min_orig,chi_min_orig,res_max_2,chi_max_2,res2_all,atan_all,x_fft_1d_res);
        }
        // get_psd_fit_show_epa_subtraction_conv(psd_avg,psd_show,box,ctf_para_avg,res_min,res_max,chi_min,chi_max,res_min_orig,chi_min_orig,res_max_2,chi_max_2,res2_all,atan_all,x_fft_1d_res,box_conv);
        MRC stack_psd(average_mrc.c_str(),"w");
        if(!stack_psd.hasFile())
        {
            cerr << "[Error] Cannot open output file " << average_mrc << "!" << endl;
            abort();
        }
        stack_psd.createMRC_empty(box,box,1,2);
        stack_psd.write2DIm(psd_show,0);
        stack_psd.close();
        /*
        string defocus_file_avg=path+"/defocus_file_avg.txt";
        FILE *fdefocus=fopen(defocus_file_avg.c_str(),"w");
        fprintf(fdefocus,"0 %f %f %f 0.000000 %f %f\n",x[0],x[1],x[2]*180.0/M_PI,cc_max,1/(res_eva*1e-10));
        fclose(fdefocus);
        */

        if(log_avail)
        {
            MRC mlog("log_2.mrc","w");
            // mlog.open("log.mrc","w");
            mlog.createMRC_empty(box,box,1,2);
            mlog.write2DIm(psd_avg,0);
            mlog.close();

            // 输出一维平均，用于算法测试
            float *log_psd_radial=new float[box/2-1];
            get_radial_average(psd_avg,log_psd_radial,box,ctf_para_avg,res2_all,atan_all,x_fft_1d_res);
            FILE *flog_radial=fopen("log_radial.txt","w");
            for(int i=0;i<box/2-1;i++)
            {
                fprintf(flog_radial,"%f\n",log_psd_radial[i]);
            }
            fflush(flog_radial);
            fclose(flog_radial);
            delete [] log_psd_radial;

            // 输出SSNR，作为评价指标
            float *psd_avg_tmp=new float[box*box];
            for(int i=0;i<box*box;i++)
            {
                psd_avg_tmp[i]=exp(psd_avg_raw[i])-1;
            }
            float *psd_radial_tmp=new float[box/2-1];
            get_radial_average(psd_avg_tmp,psd_radial_tmp,box,ctf_para_avg,res2_all,atan_all,x_fft_1d_res);
            bool *flag_localmin=new bool[box/2-1];
            memset(flag_localmin,0,sizeof(bool)*(box/2-1));
            for(int i=2;i<box/2-1-2;i++)
            {
                if(psd_radial_tmp[i]<psd_radial_tmp[i-1] && psd_radial_tmp[i]<psd_radial_tmp[i-2] && psd_radial_tmp[i]<psd_radial_tmp[i+1] && psd_radial_tmp[i]<psd_radial_tmp[i+2])
                {
                    flag_localmin[i]=true;
                }
            }
            float *psd_radial_signal=new float[box/2-1];
            float *psd_radial_noise=new float[box/2-1];
            float *psd_radial_ssnr=new float[box/2-1];
            memset(psd_radial_signal,0,sizeof(float)*(box/2-1));
            memset(psd_radial_noise,0,sizeof(float)*(box/2-1));
            memset(psd_radial_ssnr,0,sizeof(float)*(box/2-1));
            for(int i=0;i<box/2-1;i++)
            {
                int t_left=-1;
                for(int t=i;t>=0;t--)
                {
                    if(flag_localmin[t]==true)
                    {
                        t_left=t;
                        break;
                    }
                }
                int t_right=-1;
                for(int t=i;t<box/2-1;t++)
                {
                    if(flag_localmin[t]==true)
                    {
                        t_right=t;
                        break;
                    }
                }
                if(t_left==-1 || t_right==-1)
                {
                    psd_radial_signal[i]=0.0;
                    psd_radial_noise[i]=psd_radial_tmp[i];
                }
                else
                {
                    psd_radial_signal[i]=psd_radial_tmp[i]-(float(t_right-i)/float(t_right-t_left)*psd_radial_tmp[t_left]+float(i-t_left)/float(t_right-t_left)*psd_radial_tmp[t_right]);
                    psd_radial_noise[i]=(float(t_right-i)/float(t_right-t_left)*psd_radial_tmp[t_left]+float(i-t_left)/float(t_right-t_left)*psd_radial_tmp[t_right]);
                }
                psd_radial_ssnr[i]=psd_radial_signal[i]/psd_radial_noise[i];
            }
            FILE *flog_radial_ssnr=fopen("log_radial_ssnr.txt","w");
            for(int i=0;i<box/2-1;i++)
            {
                fprintf(flog_radial_ssnr,"%f\n",psd_radial_ssnr[i]);
            }
            fflush(flog_radial_ssnr);
            fclose(flog_radial_ssnr);
            delete [] psd_radial_signal;
            delete [] psd_radial_noise;
            delete [] flag_localmin;
            delete [] psd_radial_ssnr;
            delete [] psd_radial_tmp;
            delete [] psd_avg_tmp;

            /*
            float *psd_padding_test=new float[box*box];
            get_psd_center_padding(psd_avg,psd_padding_test,box,plan_fft_omp_half[0],plan_ifft_omp[0],fft_buf_in_omp[0],fft_buf_out_omp[0]);
            mlog.open("log_2_padding.mrc","w");
            mlog.createMRC_empty(box,box,1,2);
            mlog.write2DIm(psd_padding_test,0);
            mlog.close();
            delete [] psd_padding_test;
            */

            /*
            float *psd_block_show=new float[box*box];
            get_psd_fit_show_image(psd_block_all[19][0],psd_block_show,box,ctf_para_avg,res_min,res_max,res2_all,atan_all);
            mlog.open("log_3.mrc","w");
            mlog.createMRC_empty(box,box,1,2);
            mlog.write2DIm(psd_block_show,0);
            mlog.close();
            delete [] psd_block_show;
            */
        }

        delete [] psd_show;
        delete [] radial;
        delete [] psd_avg;
        delete [] psd_avg_raw;
    }
    else
    {
        cout << endl << "Skip unified CTF estimation, using 1D-coarse estimation instead: df = " << df_cc_max << endl;
        // ctf_para_avg.setAllCTFPara(df_cc_max,df_cc_max,0,0,ctf_para_avg.getW());   // 特别注意：astig传给CTF的单位是角度，但过来的是弧度！！
    }

    // 每张图分别精搜CTF参数
    time(&current_time);
    strftime(time_buf,1024,"%a %b %d %T %Y",localtime(&current_time));
    cout << endl << "[" << time_buf << "] " << "######## Per-micrograph CTF refinement ########" << endl << endl;
    // cout << endl << "Local optimization for each micrograph: " << endl;
    MRC stack_psd(output_mrc.c_str(),"w");
    if(!stack_psd.hasFile())
    {
        cerr << "[Error] Cannot open output file " << output_mrc << "!" << endl;
        abort();
    }
    // stack_psd.open(output_mrc.c_str(),"w");
    stack_psd.createMRC_empty(box,box,Nz,2);
    // MRC stack_psd_raw(output_mrc_raw.c_str(),"w");
    // stack_psd_raw.createMRC_empty(box,box,Nz,2);
    // MRC stack_psd_conv(output_mrc_conv.c_str(),"w");
    // stack_psd_conv.createMRC_empty(box,box,Nz,2);
    // MRC stack_psd_combine(output_mrc_combine.c_str(),"w");
    // stack_psd_combine.createMRC_empty(box,box,Nz,2);
    // FILE *fdefocus=fopen(defocus_file.c_str(),"w");
    // fdefocus=fopen(defocus_file.c_str(),"w");
    float *psd_image[Nz];
    for(int n=0;n<Nz;n++)
    {
        psd_image[n]=new float[box*box];
    }
    for(int th=0;th<threads;th++)
    {
        for(int i=0;i<box*box;i++)
        {
            psd_avg_omp[th][i]=0.0;
            fft_buf_in_omp[th][2*i]=0.0;
            fft_buf_in_omp[th][2*i+1]=0.0;
            fft_buf_out_omp[th][2*i]=0.0;
            fft_buf_out_omp[th][2*i+1]=0.0;
        }
    }
    float ctf_all[Nz][4];   // {df_1 (A),df_2 (A),astig (degree),cc}
    float ctf_all_prev[Nz][4];
    int ctf_all_eva_n[Nz];
    float ctf_all_eva_res[Nz];
    float *psd_show_all[Nz];
    float *psd_show_all_raw[Nz];
    float *psd_show_all_conv[Nz];
    float *psd_show_all_combine[Nz];
    for(int n=0;n<Nz;n++)
    {
        psd_show_all[n]=new float[box*box];
        psd_show_all_raw[n]=new float[box*box];
        psd_show_all_conv[n]=new float[box*box];
        psd_show_all_combine[n]=new float[box*box];
    }

    CTF ctf_para_all[Nz];
    for(int n=0;n<Nz;n++)
    {
        ctf_para_all[n]=ctf_para_avg;
    }

    float theta_refine[Nz];
    for(int n=0;n<Nz;n++)
    {
        theta_refine[n]=theta[n];
    }
    float theta_refine_opt[Nz];
    for(int n=0;n<Nz;n++)
    {
        theta_refine_opt[n]=theta[n];
    }
    float psi_refine=psi;
    float phi_refine=phi_initial;

    float **psd_block_all_scaling[Nz];
    float *block_z[Nz];
    bool *block_avail[Nz];
    float offset[Nz];
    float normal_vector[Nz][3];
    for(int n=0;n<Nz;n++)
    {
        psd_block_all_scaling[n]=new float*[N_block];
        for(int m=0;m<N_block;m++)
        {
            psd_block_all_scaling[n][m]=new float[box*box];
        }
        block_z[n]=new float[N_block];
        block_avail[n]=new bool[N_block];
    }
    bool image_avail[Nz];
    for(int n=0;n<Nz;n++)
    {
        image_avail[n]=true;
    }
    float *psd_nosubtraction_all[Nz];
    for(int n=0;n<Nz;n++)
    {
        psd_nosubtraction_all[n]=new float[box*box];
    }
    float *psd_nofitting_all[Nz];
    for(int n=0;n<Nz;n++)
    {
        psd_nofitting_all[n]=new float[box*box];
    }

    bool flag_end=false;
    for(int n=0;n<Nz;n++)
    {
        ctf_all[n][0]=ctf_para_all[n].getDefocus1()*1e10;
        ctf_all[n][1]=ctf_para_all[n].getDefocus2()*1e10;
        ctf_all[n][2]=ctf_para_all[n].getAstigmatism()*180.0/M_PI;
        ctf_all[n][3]=0.0;
        ctf_all_prev[n][0]=ctf_all[n][0];
        ctf_all_prev[n][1]=ctf_all[n][1];
        ctf_all_prev[n][2]=ctf_all[n][2];
        ctf_all_prev[n][3]=ctf_all[n][3];
    }
    for(int iter=0;iter<N_it;iter++)
    {
        time(&current_time);
        strftime(time_buf,1024,"%a %b %d %T %Y",localtime(&current_time));
        printf("\n[%s] #### Iteration %2d ####\n\n",time_buf,iter);
        // cout << endl << "it " << iter << ":" << endl << endl;

        // 优化CTF参数
        time(&current_time);
        strftime(time_buf,1024,"%a %b %d %T %Y",localtime(&current_time));
        cout << endl << "[" << time_buf << "] " << "#### Refinement of CTF parameters ####" << endl << endl;;
        #pragma omp parallel for num_threads(threads)
        for(int n=0;n<Nz;n++)
        {
            CTF ctf_para_now=ctf_para_all[n];
            float *psd=new float[box*box];
            float *psd_conv_bak=new float[box*box];
            // psd_image[n]=new float[box*box];

            ctf_all_prev[n][0]=ctf_all[n][0];
            ctf_all_prev[n][1]=ctf_all[n][1];
            ctf_all_prev[n][2]=ctf_all[n][2];
            ctf_all_prev[n][3]=ctf_all[n][3];

            // 切小块估计功率谱
            float df=(ctf_para_now.getDefocus1()+ctf_para_now.getDefocus2()+(ctf_para_now.getDefocus1()-ctf_para_now.getDefocus2())*cos(2*(-ctf_para_now.getAstigmatism())))/2;
            float *psd_now=new float[box*box];
            for(int i=0;i<box*box;i++)
            {
                psd_image[n][i]=0.0;
                psd_now[i]=0.0;
            }
            for(int m=0;m<N_block;m++)
            {
                get_scaled_psd_phi(psd_block_all[n][m],psd_now,block_x[m],block_y[m],box,df*1e10,pix,psi_refine,theta_refine[n],phi_refine,Nx,Ny);
                for(int i=0;i<box*box;i++)
                {
                    psd_image[n][i]+=psd_now[i];
                    // psd_avg_omp[omp_get_thread_num()][i]+=double(psd_now[i]);
                }
            }
            // cout << n << ": Done" << endl;
            delete [] psd_now;

            // if(dose_weighting)
            // {
            //     get_dose_weighted_psd(psd_image[n],box,dose_acc_all[n],x_fft_res,y_fft_res);
            // }

            // 把平均功率谱作为一个先验放着，再加上当前照片的功率谱
            for(int i=0;i<box*box;i++)
            {
                // psd[i]=psd_add[i]+psd_image[n][i];
                // psd[i]=log(psd_add[i]+psd_image[n][i]);
                // psd[i]=log(psd_image[n][i]);
                psd[i]=psd_image[n][i];
            }

            // box convolution
            if(box_conv>0 && box_conv_first)
            {
                get_psd_conv_fft(psd,box,box_conv,plan_fft_omp[omp_get_thread_num()],plan_ifft_omp[omp_get_thread_num()],fft_buf_in_omp[omp_get_thread_num()],fft_buf_out_omp[omp_get_thread_num()]);
            }
            // if(box_conv_first)
            {
                memcpy(psd_conv_bak,psd,sizeof(float)*box*box);
            }

            // 通过优化方法估背景
            float radial_now[box/2-1];
            get_radial_average(psd,radial_now,box,ctf_para_now,res2_all,atan_all,x_fft_1d_res);
            double fit_now[box/2-1];
            background_estimation(fit_now,radial_now,box,N_zeros,ctf_para_now);

            // 回到二维抠背景
            background_subtraction(psd,fit_now,box,res2_all,atan_all,x_fft_1d_res,ctf_para_now);
            // background_subtraction(psd,fit_avg,box,res2_all,atan_all,x_fft_1d_res,ctf_para_now);

            // box convolution
            if(box_conv>0 && !box_conv_first)
            {
                get_psd_conv_fft(psd,box,box_conv,plan_fft_omp[omp_get_thread_num()],plan_ifft_omp[omp_get_thread_num()],fft_buf_in_omp[omp_get_thread_num()],fft_buf_out_omp[omp_get_thread_num()]);
            }
            /*
            if(!box_conv_first)
            {
                memcpy(psd_conv_bak,psd,sizeof(float)*box*box);
            }
            */

            memcpy(psd_nofitting_all[n],psd,sizeof(float)*box*box);

            if(dose_weighting)
            {
                get_dose_weighted_psd(psd,box,dose_acc_all[n],x_fft_res,y_fft_res);
            }

            df=(ctf_para_now.getDefocus1()+ctf_para_now.getDefocus2()+(ctf_para_now.getDefocus1()-ctf_para_now.getDefocus2())*cos(2*(-ctf_para_now.getAstigmatism())))/2;
            Data_opt_epa data_opt_image;
            data_opt_image.box=box;
            data_opt_image.ctf_para=ctf_para_now;
            data_opt_image.psd=psd;
            data_opt_image.res_min=res_min;
            data_opt_image.res_max=res_max;
            data_opt_image.chi_min=M_PI*ctf_para_now.getLambda()*df*(res_min*res_min)-M_PI_2*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*double(res_min*res_min)*double(res_min*res_min)+ctf_para_now.getW_phase();
            data_opt_image.chi_max=M_PI*ctf_para_now.getLambda()*df*(res_max*res_max)-M_PI_2*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*double(res_max*res_max)*double(res_max*res_max)+ctf_para_now.getW_phase();
            data_opt_image.res2_all=res2_all;
            data_opt_image.atan_all=atan_all;
            data_opt_image.x_fft_1d_res=x_fft_1d_res;

            nlopt_opt opt_image;
            opt_image=nlopt_create(NLOPT_LN_NELDERMEAD,3);
            if(optimize_with_avg)
            {
                if(adaptive_range)
                {
                    nlopt_set_max_objective(opt_image,get_correlation_image_epa_zero_mean,&data_opt_image);
                }
                else
                {
                    nlopt_set_max_objective(opt_image,get_correlation_image_epa_zero_mean_circle,&data_opt_image);
                }
            }
            else
            {
                if(adaptive_range)
                {
                    nlopt_set_max_objective(opt_image,get_correlation_image_zero_mean,&data_opt_image);
                }
                else
                {
                    nlopt_set_max_objective(opt_image,get_correlation_image_zero_mean_circle,&data_opt_image);
                }
            }
            nlopt_set_xtol_rel(opt_image,1e-4);
            nlopt_set_ftol_rel(opt_image,1e-4);
            double step[3]={1e2,1e2,1.0*M_PI/180.0};
            if(loose_restriction)
            {
                step[0]=2e3;
                step[1]=2e3;
            }
            nlopt_set_initial_step(opt_image,step);
            double x[3]={ctf_para_now.getDefocus1()*1e10,ctf_para_now.getDefocus2()*1e10,ctf_para_now.getAstigmatism()};
            double cc_max;  // 单位: (A,A,rad)
            if(nlopt_optimize(opt_image,x,&cc_max)<0)
            {
                x[0]=ctf_para_now.getDefocus1()*1e10;
                x[1]=ctf_para_now.getDefocus2()*1e10;
                x[2]=ctf_para_now.getAstigmatism();
                // cout << n << ": nlopt failed! Use tilt-series CTF instead: (" << x[0] << "," << x[1] << "," << x[2]*180.0/M_PI << ")" << endl;
                printf("[Warning] %02d : nlopt failed! Use tilt-series CTF instead: ( %9.3f , %9.3f , %7.2f )\n",x[0],x[1],x[2]*180.0/M_PI);
            }
            else
            {
                if(!hard_restriction)
                {
                    if(!loose_restriction)
                    {
                        if(abs(ctf_para_now.getDefocus1()*1e10-ctf_para_now.getDefocus2()*1e10)>5000.0 && abs((x[2]-ctf_para_now.getAstigmatism())*180.0/M_PI)>5.0)    // 高像散数据才约束像散角
                        {
                            // cout << n << ": Found maximum at (df_1,df_2,astig): (" << x[0] << "," << x[1] << "," << x[2]*180.0/M_PI << "), with cc = " << cc_max << ", the difference of astigmastism is too large, use unified CTF instead :(" << ctf_para_now.getDefocus1()*1e10 << "," << ctf_para_now.getDefocus2()*1e10 << "," << ctf_para_now.getAstigmatism()*180.0/M_PI << ")" << endl;
                            printf("[Warning] %02d : Found maximum at ( df_1 , df_2 , astig ) : ( %9.3f , %9.3f , %7.2f ) , with cc = %9.6f , the difference of astigmastism is too large, use unified CTF instead : ( %9.3f , %9.3f , %7.2f )\n",n,x[0],x[1],x[2]*180.0/M_PI,cc_max,ctf_para_now.getDefocus1()*1e10,ctf_para_now.getDefocus2()*1e10,ctf_para_now.getAstigmatism()*180.0/M_PI);
                            x[0]=ctf_para_now.getDefocus1()*1e10;
                            x[1]=ctf_para_now.getDefocus2()*1e10;
                            x[2]=ctf_para_now.getAstigmatism();
                        }
                        else if(abs(x[0]-ctf_para_now.getDefocus1()*1e10)>10000.0 || abs(x[1]-ctf_para_now.getDefocus2()*1e10)>10000.0)   // 离焦量偏差不能太大
                        {
                            // cout << n << ": Found maximum at (df_1,df_2,astig): (" << x[0] << "," << x[1] << "," << x[2]*180.0/M_PI << "), with cc = " << cc_max << ", the difference of defocus value is too large, use unified CTF instead :(" << ctf_para_now.getDefocus1()*1e10 << "," << ctf_para_now.getDefocus2()*1e10 << "," << ctf_para_now.getAstigmatism()*180.0/M_PI << ")" << endl;
                            printf("[Warning] %02d : Found maximum at ( df_1 , df_2 , astig ) : ( %9.3f , %9.3f , %7.2f ) , with cc = %9.6f , the difference of defocus value is too large, use unified CTF instead : ( %9.3f , %9.3f , %7.2f )\n",n,x[0],x[1],x[2]*180.0/M_PI,cc_max,ctf_para_now.getDefocus1()*1e10,ctf_para_now.getDefocus2()*1e10,ctf_para_now.getAstigmatism()*180.0/M_PI);
                            x[0]=ctf_para_now.getDefocus1()*1e10;
                            x[1]=ctf_para_now.getDefocus2()*1e10;
                            x[2]=ctf_para_now.getAstigmatism();
                        }
                        else
                        {
                            // cout << n << ": Found maximum at (df_1,df_2,astig): (" << x[0] << "," << x[1] << "," << x[2]*180.0/M_PI << "), with cc = " << cc_max << endl;
                            // printf("%02d : Found maximum at ( df_1 , df_2 , astig ) : ( %9.3f , %9.3f , %7.2f ) , with cc = %9.6f\n",n,x[0],x[1],x[2]*180.0/M_PI,cc_max);
                        }
                    }
                    else
                    {
                        
                    }
                }
                else
                {
                    if(abs(ctf_para_now.getDefocus1()*1e10-ctf_para_now.getDefocus2()*1e10)>5000.0 && abs((x[2]-ctf_para_now.getAstigmatism())*180.0/M_PI)>5.0)    // 高像散数据才约束像散角
                    {
                        // cout << n << ": Found maximum at (df_1,df_2,astig): (" << x[0] << "," << x[1] << "," << x[2]*180.0/M_PI << "), with cc = " << cc_max << ", the difference of astigmastism is too large, use unified CTF instead :(" << ctf_para_now.getDefocus1()*1e10 << "," << ctf_para_now.getDefocus2()*1e10 << "," << ctf_para_now.getAstigmatism()*180.0/M_PI << ")" << endl;
                        printf("[Warning] %02d : Found maximum at ( df_1 , df_2 , astig ) : ( %9.3f , %9.3f , %7.2f ) , with cc = %9.6f , the difference of astigmastism is too large, use unified CTF instead : ( %9.3f , %9.3f , %7.2f )\n",n,x[0],x[1],x[2]*180.0/M_PI,cc_max,ctf_para_now.getDefocus1()*1e10,ctf_para_now.getDefocus2()*1e10,ctf_para_now.getAstigmatism()*180.0/M_PI);
                        x[0]=ctf_para_now.getDefocus1()*1e10;
                        x[1]=ctf_para_now.getDefocus2()*1e10;
                        x[2]=ctf_para_now.getAstigmatism();
                    }
                    // else if(abs(x[0]-ctf_para_now.getDefocus1()*1e10)/(ctf_para_now.getDefocus1()*1e10)>0.01 || abs(x[1]-ctf_para_now.getDefocus2()*1e10)/(ctf_para_now.getDefocus2()*1e10)>0.01)   // 离焦量偏差不能太大
                    else if(abs(x[0]-ctf_para_now.getDefocus1()*1e10)>1000.0 || abs(x[1]-ctf_para_now.getDefocus2()*1e10)>1000.0)
                    {
                        // cout << n << ": Found maximum at (df_1,df_2,astig): (" << x[0] << "," << x[1] << "," << x[2]*180.0/M_PI << "), with cc = " << cc_max << ", the difference of defocus value is too large, use unified CTF instead :(" << ctf_para_now.getDefocus1()*1e10 << "," << ctf_para_now.getDefocus2()*1e10 << "," << ctf_para_now.getAstigmatism()*180.0/M_PI << ")" << endl;
                        printf("[Warning] %02d : Found maximum at ( df_1 , df_2 , astig ) : ( %9.3f , %9.3f , %7.2f ) , with cc = %9.6f , the difference of defocus value is too large, use unified CTF instead : ( %9.3f , %9.3f , %7.2f )\n",n,x[0],x[1],x[2]*180.0/M_PI,cc_max,ctf_para_now.getDefocus1()*1e10,ctf_para_now.getDefocus2()*1e10,ctf_para_now.getAstigmatism()*180.0/M_PI);
                        x[0]=ctf_para_now.getDefocus1()*1e10;
                        x[1]=ctf_para_now.getDefocus2()*1e10;
                        x[2]=ctf_para_now.getAstigmatism();
                    }
                    else
                    {
                        // cout << n << ": Found maximum at (df_1,df_2,astig): (" << x[0] << "," << x[1] << "," << x[2]*180.0/M_PI << "), with cc = " << cc_max << endl;
                        // printf("%02d : Found maximum at ( df_1 , df_2 , astig ) : ( %9.3f , %9.3f , %7.2f ) , with cc = %9.6f\n",n,x[0],x[1],x[2]*180.0/M_PI,cc_max);
                    }
                }
            }

            ctf_all[n][0]=x[0];
            ctf_all[n][1]=x[1];
            ctf_all[n][2]=x[2]*180.0/M_PI;
            ctf_all[n][3]=cc_max;
            ctf_para_now.setAllCTFPara(x[0],x[1],x[2]*180.0/M_PI,0,w_cos);
            ctf_para_all[n]=ctf_para_now;

            // evaluation
            int n_eva=0;
            n_eva=get_number_of_rings_fit(psd,box,ctf_para_now,res2_all,atan_all,x_fft_1d_res);
            float df_now=(ctf_para_now.getDefocus1()+ctf_para_now.getDefocus2()+(ctf_para_now.getDefocus1()-ctf_para_now.getDefocus2())*cos(2*(-ctf_para_now.getAstigmatism())))/2;
            float res_eva=sqrt((M_PI*ctf_para_now.getLambda()*(df_now)-sqrt(M_PI*M_PI*ctf_para_now.getLambda()*ctf_para_now.getLambda()*(df_now)*(df_now)-2*M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*(n_eva*M_PI-ctf_para_now.getW_phase())))/(M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())));
            // cout << n << ": Evaluation of fitting: Fitting to the " << n_eva << "-th Thon ring, with resolution = " << 1/(res_eva*1e-10) << endl;
            // printf("%02d : Evaluation of fitting : Fitting to the %2d - th Thon ring, with resolution = %7.4f\n",n,n_eva,1/(res_eva*1e-10));
            ctf_all_eva_n[n]=n_eva;
            ctf_all_eva_res[n]=1/(res_eva*1e-10);
            // if(n_eva>=3)
            if(n_eva>=4)
            {
                image_avail[n]=true;
            }
            else
            {
                image_avail[n]=false;
            }

            // CTF ctf_para_now=ctf_para_avg;
            // ctf_para_now.setAllCTFPara(x[0],x[1],x[2]*180.0/M_PI,0,w_cos);
            // get_psd_fit_show(psd,psd_show_all[n],box,ctf_para_now,res_min,res_max);
            // float res_min_0=sqrt((M_PI*ctf_para_avg.getLambda()*(df_cc_max*1e-10)-sqrt(M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df_cc_max*1e-10)*(df_cc_max*1e-10)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(-ctf_para_avg.getW_phase())))/(M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())));
            // get_psd_fit_show_epa_raw(psd_image[n],psd_show_all_raw[n],box,ctf_para_now,res_min,res_max,chi_min,chi_max,res_min_orig,chi_min_orig,res_max_2,chi_max_2,res2_all,atan_all,x_fft_1d_res);
            // get_psd_fit_show_epa_subtraction(psd,psd_show_all[n],box,ctf_para_now,res_min,res_max,chi_min,chi_max,res_min_orig,chi_min_orig,res_max_2,chi_max_2,res2_all,atan_all,x_fft_1d_res);
            // get_psd_fit_show_epa_subtraction(psd_conv_bak,psd_show_all_conv[n],box,ctf_para_now,res_min,res_max,chi_min,chi_max,res_min_orig,chi_min_orig,res_max_2,chi_max_2,res2_all,atan_all,x_fft_1d_res);

            /*
            MRC stack_log("log.mrc","w");
            stack_log.createMRC_empty(box,box,1,2);
            stack_log.write2DIm(psd_show_now,0);
            stack_log.close();

            stack_log.open("log_psd.mrc","w");
            stack_log.createMRC_empty(box,box,1,2);
            stack_log.write2DIm(psd,0);
            stack_log.close();
            */

            res_eva=sqrt((M_PI*ctf_para_now.getLambda()*(df_now)-sqrt(M_PI*M_PI*ctf_para_now.getLambda()*ctf_para_now.getLambda()*(df_now)*(df_now)-2*M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*(ctf_all_eva_n[n]*M_PI-ctf_para_now.getW_phase())))/(M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())));
            double chi_eva=M_PI*ctf_para_now.getLambda()*df_now*(res_eva*res_eva)-M_PI_2*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*double(res_eva*res_eva)*double(res_eva*res_eva)+ctf_para_now.getW_phase();
            get_psd_fit_show_epa_subtraction(psd_nofitting_all[n],psd_show_all[n],box,ctf_para_now,res_min,res_max,res_min_orig,res_eva,chi_eva,res2_all,atan_all,x_fft_1d_res);
            get_psd_fit_show_epa_raw(psd_image[n],psd_show_all_raw[n],box,ctf_para_now,res_min,res_max,chi_min,chi_max,res_min_orig,chi_min_orig,res_eva,chi_eva,res_min_2,chi_min_2,res2_all,atan_all,x_fft_1d_res);
            get_psd_fit_show_epa_conv(psd_conv_bak,psd_show_all_conv[n],box,ctf_para_now,res_min,res_max,chi_min,chi_max,res_min_orig,chi_min_orig,res_eva,chi_eva,res_min_2,chi_min_2,res2_all,atan_all,x_fft_1d_res);
            // get_psd_fit_show_epa_combine(psd_nofitting_all[n],psd_conv_bak,psd_show_all_combine[n],box,ctf_para_now,res_min,res_max,chi_min,chi_max,res_min_orig,chi_min_orig,res_eva,chi_eva,res_min_2,chi_min_2,res2_all,atan_all,x_fft_1d_res);
            get_psd_fit_show_epa_combine_2(psd_nofitting_all[n],psd_image[n],psd_show_all_combine[n],box,ctf_para_now,res_min,res_max,chi_min,chi_max,res_min_orig,chi_min_orig,res_eva,chi_eva,res_min_2,chi_min_2,res2_all,atan_all,x_fft_1d_res);
            
            memcpy(psd_nosubtraction_all[n],psd_image[n],sizeof(float)*box*box);

            delete [] psd_conv_bak;
            delete [] psd;
        }

        for(int n=0;n<Nz;n++)
        {
            printf("%02d : Found maximum at ( df_1 , df_2 , astig ) : ( %9.3f , %9.3f , %7.2f ) , with cc = %9.6f\n",n,ctf_all[n][0],ctf_all[n][1],ctf_all[n][2],ctf_all[n][3]);
            printf("%02d : Evaluation of fitting : Fitting to the %2d - th Thon ring, with resolution = %7.4f\n",n,ctf_all_eva_n[n],ctf_all_eva_res[n]);
        }

        if(log_avail)
        {
            FILE *flog_eva=fopen("log_eva.txt","w");
            for(int n=0;n<Nz;n++)
            {
                fprintf(flog_eva,"%d %f\n",ctf_all_eva_n[n],ctf_all_eva_res[n]);
            }
            fflush(flog_eva);
            fclose(flog_eva);
        }

        // convergence determination
        bool flag_convergence_ctf=false;
        if(iter>=1)
        {
            flag_convergence_ctf=true;
            for(int n=0;n<Nz;n++)
            {
                if(abs(ctf_all[n][0]-ctf_all_prev[n][0])>convergence_df || abs(ctf_all[n][1]-ctf_all_prev[n][1])>convergence_df || abs(ctf_all[n][2]-ctf_all_prev[n][2])>convergence_astig)
                {
                    flag_convergence_ctf=false;
                    break;
                }
            }
        }

        /*
        cout << endl;
        int N_avail=0;
        for(int n=0;n<Nz;n++)
        {
            if(image_avail[n])
            {
                cout << n << " ";
                N_avail++;
            }
        }
        cout << endl << "Images available for axis estimation: " << N_avail << endl;
        */

        // 第一轮迭代，利用CTF信息估计整体角度偏差
        if(iter==0 && !skip_axis_refinement)
        {
            time(&current_time);
            strftime(time_buf,1024,"%a %b %d %T %Y",localtime(&current_time));
            cout << endl << "[" << time_buf << "] " << "#### Tilt axis angle estimation ####" << endl << endl;
            // cout << "Estimate tilt axis angle:" << endl;
            Data_opt_psi_resolution_all data_opt_psi;
            data_opt_psi.box=box;
            data_opt_psi.N_block=N_block;
            data_opt_psi.block_x=block_x;
            data_opt_psi.block_y=block_y;
            data_opt_psi.Nx=Nx;
            data_opt_psi.Ny=Ny;
            data_opt_psi.Nz=Nz;
            data_opt_psi.ctf_para=ctf_para_all;
            data_opt_psi.ctf_para_avg=ctf_para_avg;
            data_opt_psi.df_ref=(ctf_para_avg.getDefocus1()+ctf_para_avg.getDefocus2()+(ctf_para_avg.getDefocus1()-ctf_para_avg.getDefocus2())*cos(2*(-ctf_para_avg.getAstigmatism())))/2;
            data_opt_psi.res_min=res_min_high;
            data_opt_psi.res_max=res_max_high;
            data_opt_psi.psd_all=psd_block_all;
            data_opt_psi.res2_all=res2_all;
            data_opt_psi.atan_all=atan_all;
            data_opt_psi.x_fft_1d_res=x_fft_1d_res;
            data_opt_psi.psi=psi_refine;
            data_opt_psi.phi=phi_refine;
            data_opt_psi.pix=pix;
            data_opt_psi.box_conv=box_conv;
            data_opt_psi.plan_fft=plan_fft_omp[omp_get_thread_num()];
            data_opt_psi.plan_ifft=plan_ifft_omp[omp_get_thread_num()];
            data_opt_psi.fft_buf_in=fft_buf_in_omp[omp_get_thread_num()];
            data_opt_psi.fft_buf_out=fft_buf_out_omp[omp_get_thread_num()];
            data_opt_psi.N_zeros=N_zeros;
            data_opt_psi.N_block_x=N_block_x;
            data_opt_psi.N_block_y=N_block_y;
            data_opt_psi.theta=theta_refine;
            data_opt_psi.threads=threads;
            data_opt_psi.optimize_with_avg=optimize_with_avg;

            double cc_psi[11];
            #pragma omp parallel for num_threads(threads)
            for(int t=0;t<11;t++)
            {
                double psi_now=double(-5+t+psi_refine);
                Data_opt_psi_resolution_all data_opt_psi_now=data_opt_psi;
                data_opt_psi_now.plan_fft=plan_fft_omp[omp_get_thread_num()];
                data_opt_psi_now.plan_ifft=plan_ifft_omp[omp_get_thread_num()];
                data_opt_psi_now.fft_buf_in=fft_buf_in_omp[omp_get_thread_num()];
                data_opt_psi_now.fft_buf_out=fft_buf_out_omp[omp_get_thread_num()];
                cc_psi[t]=get_psi_resolution_all_omp(1,&psi_now,NULL,&data_opt_psi_now);
            }
            
            for(int t=0;t<11;t++)
            {
                printf("psi = %7.2f : cc = %9.6f\n",float(-5+t+psi_refine),cc_psi[t]);
            }

            if(log_avail)
            {
                FILE *flog_cc=fopen("log_cc_psi.txt","w");
                for(int t=0;t<11;t++)
                {
                    fprintf(flog_cc,"%f\n",cc_psi[t]);
                }
                fflush(flog_cc);
                fclose(flog_cc);
            }

            double cc_psi_max=cc_psi[5];
            double psi_max=0.0;
            for(int t=0;t<11;t++)
            {
                if(cc_psi[t]>cc_psi_max)
                {
                    cc_psi_max=cc_psi[t];
                    psi_max=double(-5+t+psi_refine);
                }
            }
            // cout << "Coarse estimated tilt axis angle : " << psi_max << ", with cc = " << cc_psi_max << endl;
            printf("Coarse estimated tilt axis angle : %7.2f , with cc = %9.6f\n",psi_max,cc_psi_max);

            // 单峰函数，黄金分割搜索
            double left=psi_max-1.0;
            double right=psi_max+1.0;
            double mid_left=left+(right-left)*(3.0-sqrt(5))/2.0;
            double mid_right=left+(right-left)*(sqrt(5)-1.0)/2.0;
            double cc_left=get_psi_resolution_all_omp(1,&mid_left,NULL,&data_opt_psi);
            double cc_right=get_psi_resolution_all_omp(1,&mid_right,NULL,&data_opt_psi);
            while(right-left>1e-2)
            {
                if(cc_left<cc_right)
                {
                    left=mid_left;
                    mid_left=mid_right;
                    mid_right=left+(right-left)*(sqrt(5)-1.0)/2.0;
                    cc_left=cc_right;
                    cc_right=get_psi_resolution_all_omp(1,&mid_right,NULL,&data_opt_psi);
                }
                else
                {
                    right=mid_right;
                    mid_right=mid_left;
                    mid_left=left+(right-left)*(3.0-sqrt(5))/2.0;
                    cc_right=cc_left;
                    cc_left=get_psi_resolution_all_omp(1,&mid_left,NULL,&data_opt_psi);
                }
            }
            // cout << "Fine estimated tilt axis angle: " << left << ", with cc=" << cc_left << endl;
            printf("Fine estimated tilt axis angle: %7.2f , with cc = %9.6f\n",left,cc_left);
            psi_refine=float(left);
        }

        if(iter==0 && !skip_offset_estimation_x)
        {
            time(&current_time);
            strftime(time_buf,1024,"%a %b %d %T %Y",localtime(&current_time));
            cout << endl << "[" << time_buf << "] " << "#### Tilt axis azimuth estimation ####" << endl << endl;
            Data_opt_phi_resolution_all data_opt_phi;
            data_opt_phi.box=box;
            data_opt_phi.N_block=N_block;
            data_opt_phi.block_x=block_x;
            data_opt_phi.block_y=block_y;
            data_opt_phi.Nx=Nx;
            data_opt_phi.Ny=Ny;
            data_opt_phi.Nz=Nz;
            data_opt_phi.ctf_para=ctf_para_all;
            data_opt_phi.ctf_para_avg=ctf_para_avg;
            data_opt_phi.df_ref=(ctf_para_avg.getDefocus1()+ctf_para_avg.getDefocus2()+(ctf_para_avg.getDefocus1()-ctf_para_avg.getDefocus2())*cos(2*(-ctf_para_avg.getAstigmatism())))/2;
            data_opt_phi.res_min=res_min_high;
            data_opt_phi.res_max=res_max_high;
            // data_opt_phi.res_min=res_min;
            // data_opt_phi.res_max=res_max;
            data_opt_phi.psd_all=psd_block_all;
            data_opt_phi.res2_all=res2_all;
            data_opt_phi.atan_all=atan_all;
            data_opt_phi.x_fft_1d_res=x_fft_1d_res;
            data_opt_phi.psi=psi_refine;
            data_opt_phi.phi=phi_refine;
            data_opt_phi.pix=pix;
            data_opt_phi.box_conv=box_conv;
            data_opt_phi.plan_fft=plan_fft_omp[omp_get_thread_num()];
            data_opt_phi.plan_ifft=plan_ifft_omp[omp_get_thread_num()];
            data_opt_phi.fft_buf_in=fft_buf_in_omp[omp_get_thread_num()];
            data_opt_phi.fft_buf_out=fft_buf_out_omp[omp_get_thread_num()];
            data_opt_phi.N_zeros=N_zeros;
            data_opt_phi.N_block_x=N_block_x;
            data_opt_phi.N_block_y=N_block_y;
            data_opt_phi.theta=theta_refine;
            data_opt_phi.threads=threads;
            data_opt_phi.optimize_with_avg=optimize_with_avg;

            double cc_phi[31];
            #pragma omp parallel for num_threads(threads)
            for(int t=0;t<31;t++)
            {
                double phi_now=double(-15+t);
                Data_opt_phi_resolution_all data_opt_phi_now=data_opt_phi;
                data_opt_phi_now.plan_fft=plan_fft_omp[omp_get_thread_num()];
                data_opt_phi_now.plan_ifft=plan_ifft_omp[omp_get_thread_num()];
                data_opt_phi_now.fft_buf_in=fft_buf_in_omp[omp_get_thread_num()];
                data_opt_phi_now.fft_buf_out=fft_buf_out_omp[omp_get_thread_num()];
                cc_phi[t]=get_phi_resolution_all_omp(1,&phi_now,NULL,&data_opt_phi_now);
            }

            for(int t=0;t<31;t++)
            {
                printf("phi = %7.2f : cc = %9.6f\n",float(-15+t),cc_phi[t]);
            }
            
            if(log_avail)
            {
                FILE *flog_cc=fopen("log_cc_phi.txt","w");
                for(int t=0;t<31;t++)
                {
                    fprintf(flog_cc,"%f\n",cc_phi[t]);
                }
                fflush(flog_cc);
                fclose(flog_cc);
            }

            double cc_phi_max=cc_phi[15];
            double phi_max=0.0;
            for(int t=0;t<31;t++)
            {
                if(cc_phi[t]>cc_phi_max)
                {
                    cc_phi_max=cc_phi[t];
                    phi_max=double(-15+t);
                }
            }
            printf("Coarse estimated tilt axis azimuth: %7.2f , with cc = %9.6f\n",phi_max,cc_phi_max);

            // 单峰函数，黄金分割搜索
            double left=phi_max-1.0;
            double right=phi_max+1.0;
            double mid_left=left+(right-left)*(3.0-sqrt(5))/2.0;
            double mid_right=left+(right-left)*(sqrt(5)-1.0)/2.0;
            double cc_left=get_phi_resolution_all_omp(1,&mid_left,NULL,&data_opt_phi);
            double cc_right=get_phi_resolution_all_omp(1,&mid_right,NULL,&data_opt_phi);
            while(right-left>1e-2)
            {
                if(cc_left<cc_right)
                {
                    left=mid_left;
                    mid_left=mid_right;
                    mid_right=left+(right-left)*(sqrt(5)-1.0)/2.0;
                    cc_left=cc_right;
                    cc_right=get_phi_resolution_all_omp(1,&mid_right,NULL,&data_opt_phi);
                }
                else
                {
                    right=mid_right;
                    mid_right=mid_left;
                    mid_left=left+(right-left)*(3.0-sqrt(5))/2.0;
                    cc_right=cc_left;
                    cc_left=get_phi_resolution_all_omp(1,&mid_left,NULL,&data_opt_phi);
                }
            }
            // cout << "Fine estimated tilt axis azimuth: " << left << ", with cc=" << cc_left << endl;
            printf("Fine estimated tilt axis azimuth: %7.2f , with cc = %9.6f\n",left,cc_left);
            phi_refine=float(left);
        }

        if(iter==0 && !skip_offset_estimation)
        // if(iter==-1)
        {
            time(&current_time);
            strftime(time_buf,1024,"%a %b %d %T %Y",localtime(&current_time));
            cout << endl << "[" << time_buf << "] " << "#### Absolute tilt angle offset estimation ####" << endl << endl;
            // cout << "Estimate systematic tilt angle offset:" << endl;
            Data_opt_theta_resolution_all data_opt_theta;
            data_opt_theta.box=box;
            data_opt_theta.N_block=N_block;
            data_opt_theta.block_x=block_x;
            data_opt_theta.block_y=block_y;
            data_opt_theta.Nx=Nx;
            data_opt_theta.Ny=Ny;
            data_opt_theta.Nz=Nz;
            data_opt_theta.ctf_para=ctf_para_all;
            data_opt_theta.ctf_para_avg=ctf_para_avg;
            data_opt_theta.df_ref=(ctf_para_avg.getDefocus1()+ctf_para_avg.getDefocus2()+(ctf_para_avg.getDefocus1()-ctf_para_avg.getDefocus2())*cos(2*(-ctf_para_avg.getAstigmatism())))/2;
            data_opt_theta.res_min=res_min_high;
            data_opt_theta.res_max=res_max_high;
            // data_opt_theta.res_min=res_min;
            // data_opt_theta.res_max=res_max;
            data_opt_theta.psd_all=psd_block_all;
            data_opt_theta.res2_all=res2_all;
            data_opt_theta.atan_all=atan_all;
            data_opt_theta.x_fft_1d_res=x_fft_1d_res;
            data_opt_theta.psi=psi_refine;
            data_opt_theta.phi=phi_refine;
            data_opt_theta.pix=pix;
            data_opt_theta.box_conv=box_conv;
            data_opt_theta.plan_fft=plan_fft_omp[omp_get_thread_num()];
            data_opt_theta.plan_ifft=plan_ifft_omp[omp_get_thread_num()];
            data_opt_theta.fft_buf_in=fft_buf_in_omp[omp_get_thread_num()];
            data_opt_theta.fft_buf_out=fft_buf_out_omp[omp_get_thread_num()];
            data_opt_theta.N_zeros=N_zeros;
            data_opt_theta.N_block_x=N_block_x;
            data_opt_theta.N_block_y=N_block_y;
            data_opt_theta.theta=theta_refine;
            data_opt_theta.threads=threads;
            data_opt_theta.N_ref=N_ref;
            data_opt_theta.N_avg=N_avg;
            data_opt_theta.image_avail=image_avail;
            data_opt_theta.optimize_with_avg=optimize_with_avg;

            // cout << "res_min=" << 1/res_min_high/1e-10 << endl;
            // cout << "res_max=" << 1/res_max_high/1e-10 << endl;

            double cc_offset[61];
            #pragma omp parallel for num_threads(threads)
            for(int t=0;t<61;t++)
            {
                double theta_now=double(-30+t);
                Data_opt_theta_resolution_all data_opt_theta_now=data_opt_theta;
                data_opt_theta_now.plan_fft=plan_fft_omp[omp_get_thread_num()];
                data_opt_theta_now.plan_ifft=plan_ifft_omp[omp_get_thread_num()];
                data_opt_theta_now.fft_buf_in=fft_buf_in_omp[omp_get_thread_num()];
                data_opt_theta_now.fft_buf_out=fft_buf_out_omp[omp_get_thread_num()];
                cc_offset[t]=get_theta_resolution_all_omp(1,&theta_now,NULL,&data_opt_theta_now);
                // cc_offset[t]=get_theta_resolution_all_omp(1,&theta_now,NULL,&data_opt_theta);
            }
            
            for(int t=0;t<61;t++)
            {
                printf("offset = %7.2f : cc = %9.6f\n",float(-30+t),cc_offset[t]);
            }

            if(log_avail)
            {
                FILE *flog_cc=fopen("log_cc.txt","w");
                // flog_cc=fopen("log_cc.txt","w");
                for(int t=0;t<61;t++)
                {
                    fprintf(flog_cc,"%f\n",cc_offset[t]);
                }
                fflush(flog_cc);
                fclose(flog_cc);
            }
            
            double cc_offset_max=cc_offset[30];
            double theta_offset_max=0.0;
            for(int t=0;t<61;t++)
            {
                if(cc_offset[t]>cc_offset_max)
                {
                    cc_offset_max=cc_offset[t];
                    theta_offset_max=double(-30+t);
                }
            }
            // cout << "Coarse estimated systematic tilt offset: " << theta_offset_max << ", with cc=" << cc_offset_max << endl;
            printf("Coarse estimated absolute tilt angle offset : %7.2f , with cc = %9.6f\n",theta_offset_max,cc_offset_max);

            // 单峰函数，黄金分割搜索
            double left=theta_offset_max-1.0;
            double right=theta_offset_max+1.0;
            double mid_left=left+(right-left)*(3.0-sqrt(5))/2.0;
            double mid_right=left+(right-left)*(sqrt(5)-1.0)/2.0;
            double cc_left=get_theta_resolution_all_omp(1,&mid_left,NULL,&data_opt_theta);
            double cc_right=get_theta_resolution_all_omp(1,&mid_right,NULL,&data_opt_theta);
            // left=theta_offset_max-1.0;
            // right=theta_offset_max+1.0;
            // mid_left=left+(right-left)*(3.0-sqrt(5))/2.0;
            // mid_right=left+(right-left)*(sqrt(5)-1.0)/2.0;
            // cc_left=get_theta_resolution_all_omp(1,&mid_left,NULL,&data_opt_theta);
            // cc_right=get_theta_resolution_all_omp(1,&mid_right,NULL,&data_opt_theta);
            while(right-left>1e-2)
            {
                if(cc_left<cc_right)
                {
                    left=mid_left;
                    mid_left=mid_right;
                    mid_right=left+(right-left)*(sqrt(5)-1.0)/2.0;
                    cc_left=cc_right;
                    cc_right=get_theta_resolution_all_omp(1,&mid_right,NULL,&data_opt_theta);
                }
                else
                {
                    right=mid_right;
                    mid_right=mid_left;
                    mid_left=left+(right-left)*(3.0-sqrt(5))/2.0;
                    cc_right=cc_left;
                    cc_left=get_theta_resolution_all_omp(1,&mid_left,NULL,&data_opt_theta);
                }
            }
            // cout << "Fine estimated systematic tilt offset: " << left << ", with cc=" << cc_left << endl;
            printf("Fine estimated absolute tilt angle offset : %7.2f , with cc = %9.6f\n",left,cc_left);

            for(int n=0;n<Nz;n++)
            {
                theta_refine[n]+=float(left);
            }
        }
        /*
        if(iter==0 && !skip_offset_estimation && !skip_axis_refinement)   // 再联合优化一轮
        {
            time(&current_time);
            strftime(time_buf,1024,"%a %b %d %T %Y",localtime(&current_time));
            cout << endl << "[" << time_buf << "] " << "#### Further refinement on systematic tilt angle offset and tilt axis angle ####" << endl << endl;
            // cout << "Further refinement on systematic tilt angle offset and tilt axis angle:" << endl;
            Data_opt_psi_resolution_all data_opt_psi_theta;
            data_opt_psi_theta.box=box;
            data_opt_psi_theta.N_block=N_block;
            data_opt_psi_theta.block_x=block_x;
            data_opt_psi_theta.block_y=block_y;
            data_opt_psi_theta.Nx=Nx;
            data_opt_psi_theta.Ny=Ny;
            data_opt_psi_theta.Nz=Nz;
            data_opt_psi_theta.ctf_para=ctf_para_all;
            data_opt_psi_theta.ctf_para_avg=ctf_para_avg;
            data_opt_psi_theta.df_ref=(ctf_para_avg.getDefocus1()+ctf_para_avg.getDefocus2()+(ctf_para_avg.getDefocus1()-ctf_para_avg.getDefocus2())*cos(2*(-ctf_para_avg.getAstigmatism())))/2;
            data_opt_psi_theta.res_min=res_min_high;
            data_opt_psi_theta.res_max=res_max_high;
            data_opt_psi_theta.psd_all=psd_block_all;
            data_opt_psi_theta.res2_all=res2_all;
            data_opt_psi_theta.atan_all=atan_all;
            data_opt_psi_theta.x_fft_1d_res=x_fft_1d_res;
            data_opt_psi_theta.psi=psi_refine;
            data_opt_psi_theta.phi=phi_refine;
            data_opt_psi_theta.pix=pix;
            data_opt_psi_theta.box_conv=box_conv;
            data_opt_psi_theta.plan_fft=plan_fft_omp[omp_get_thread_num()];
            data_opt_psi_theta.plan_ifft=plan_ifft_omp[omp_get_thread_num()];
            data_opt_psi_theta.fft_buf_in=fft_buf_in_omp[omp_get_thread_num()];
            data_opt_psi_theta.fft_buf_out=fft_buf_out_omp[omp_get_thread_num()];
            data_opt_psi_theta.N_zeros=N_zeros;
            data_opt_psi_theta.N_block_x=N_block_x;
            data_opt_psi_theta.N_block_y=N_block_y;
            data_opt_psi_theta.theta=theta_refine;
            data_opt_psi_theta.threads=threads;

            nlopt_opt opt_psi_theta;
            opt_psi_theta=nlopt_create(NLOPT_LN_NELDERMEAD,2);
            nlopt_set_max_objective(opt_psi_theta,get_psi_theta_resolution_all_omp,&data_opt_psi_theta);
            nlopt_set_xtol_abs1(opt_psi_theta,1e-4);
            double step_psi_theta[2]={0.1,0.1};
            nlopt_set_initial_step(opt_psi_theta,step_psi_theta);
            double x_psi_theta[2]={psi_refine,0.0};
            double loss_min;
            if(nlopt_optimize(opt_psi_theta,x_psi_theta,&loss_min)<0)
            {
                cout << "[Error] nlopt failed!" << endl;
            }
            else
            {
                // cout << "Refined tilt axis angle : " << x_psi_theta[0] << endl;
                // cout << "Refined systematic tilt offset : " << x_psi_theta[1] << endl;
                // cout << "With cc = " << loss_min << endl;
                printf("Refined tilt axis angle : %7.2f\n",x_psi_theta[0]);
                printf("Refined systematic tilt offset : %7.2f\n",x_psi_theta[1]);
                printf("With cc = %9.6f\n",loss_min);
            }
        }
        */

        bool flag_convergence_angle=true;
        if(iter==0)
        {
            flag_convergence_angle=false;
        }
        if(iter>0 && !skip_offset_refinement_x)
        {
            time(&current_time);
            strftime(time_buf,1024,"%a %b %d %T %Y",localtime(&current_time));
            cout << endl << "[" << time_buf << "] " << "#### Tilt offset around x-axis (x-tilt) refinement ####" << endl << endl;
            float phi_prev=phi_refine;
            Data_opt_phi_resolution_all data_opt_phi;
            data_opt_phi.box=box;
            data_opt_phi.N_block=N_block;
            data_opt_phi.block_x=block_x;
            data_opt_phi.block_y=block_y;
            data_opt_phi.Nx=Nx;
            data_opt_phi.Ny=Ny;
            data_opt_phi.Nz=Nz;
            data_opt_phi.ctf_para=ctf_para_all;
            data_opt_phi.ctf_para_avg=ctf_para_avg;
            data_opt_phi.df_ref=(ctf_para_avg.getDefocus1()+ctf_para_avg.getDefocus2()+(ctf_para_avg.getDefocus1()-ctf_para_avg.getDefocus2())*cos(2*(-ctf_para_avg.getAstigmatism())))/2;
            data_opt_phi.res_min=res_min_high;
            data_opt_phi.res_max=res_max_high;
            data_opt_phi.psd_all=psd_block_all;
            data_opt_phi.res2_all=res2_all;
            data_opt_phi.atan_all=atan_all;
            data_opt_phi.x_fft_1d_res=x_fft_1d_res;
            data_opt_phi.psi=psi_refine;
            data_opt_phi.phi=phi_refine;
            data_opt_phi.pix=pix;
            data_opt_phi.box_conv=box_conv;
            data_opt_phi.plan_fft=plan_fft_omp[omp_get_thread_num()];
            data_opt_phi.plan_ifft=plan_ifft_omp[omp_get_thread_num()];
            data_opt_phi.fft_buf_in=fft_buf_in_omp[omp_get_thread_num()];
            data_opt_phi.fft_buf_out=fft_buf_out_omp[omp_get_thread_num()];
            data_opt_phi.N_zeros=N_zeros;
            data_opt_phi.N_block_x=N_block_x;
            data_opt_phi.N_block_y=N_block_y;
            data_opt_phi.theta=theta_refine;
            data_opt_phi.threads=threads;
            data_opt_phi.optimize_with_avg=optimize_with_avg;

            nlopt_opt opt_phi;
            opt_phi=nlopt_create(NLOPT_LN_NELDERMEAD,1);
            nlopt_set_max_objective(opt_phi,get_phi_resolution_all_omp,&data_opt_phi);
            nlopt_set_xtol_abs1(opt_phi,1e-2);
            double step_phi[1]={0.1};
            nlopt_set_initial_step(opt_phi,step_phi);
            double x_phi[1]={phi_refine};
            double loss_min;
            if(nlopt_optimize(opt_phi,x_phi,&loss_min)<0)
            {
                cout << "[Error] nlopt failed!" << endl;
                x_phi[0]=phi_refine;
            }
            else
            {
                printf("Refined tilt offset around x-axis (x-tilt): %7.2f , with cc = %9.6f\n",x_phi[0],loss_min);
            }
            phi_refine=x_phi[0];
            if(abs(phi_refine-phi_prev)>convergence_angle)
            {
                flag_convergence_angle=false;
            }
        }
        if(iter>0 && !skip_offset_refinement)
        {
            time(&current_time);
            strftime(time_buf,1024,"%a %b %d %T %Y",localtime(&current_time));
            cout << endl << "[" << time_buf << "] " << "#### Absolute tilt angle offset refinement ####" << endl << endl;
            Data_opt_theta_resolution_all data_opt_theta;
            data_opt_theta.box=box;
            data_opt_theta.N_block=N_block;
            data_opt_theta.block_x=block_x;
            data_opt_theta.block_y=block_y;
            data_opt_theta.Nx=Nx;
            data_opt_theta.Ny=Ny;
            data_opt_theta.Nz=Nz;
            data_opt_theta.ctf_para=ctf_para_all;
            data_opt_theta.ctf_para_avg=ctf_para_avg;
            data_opt_theta.df_ref=(ctf_para_avg.getDefocus1()+ctf_para_avg.getDefocus2()+(ctf_para_avg.getDefocus1()-ctf_para_avg.getDefocus2())*cos(2*(-ctf_para_avg.getAstigmatism())))/2;
            data_opt_theta.res_min=res_min_high;
            data_opt_theta.res_max=res_max_high;
            // data_opt_theta.res_min=res_min;
            // data_opt_theta.res_max=res_max;
            data_opt_theta.psd_all=psd_block_all;
            data_opt_theta.res2_all=res2_all;
            data_opt_theta.atan_all=atan_all;
            data_opt_theta.x_fft_1d_res=x_fft_1d_res;
            data_opt_theta.psi=psi_refine;
            data_opt_theta.phi=phi_refine;
            data_opt_theta.pix=pix;
            data_opt_theta.box_conv=box_conv;
            data_opt_theta.plan_fft=plan_fft_omp[omp_get_thread_num()];
            data_opt_theta.plan_ifft=plan_ifft_omp[omp_get_thread_num()];
            data_opt_theta.fft_buf_in=fft_buf_in_omp[omp_get_thread_num()];
            data_opt_theta.fft_buf_out=fft_buf_out_omp[omp_get_thread_num()];
            data_opt_theta.N_zeros=N_zeros;
            data_opt_theta.N_block_x=N_block_x;
            data_opt_theta.N_block_y=N_block_y;
            data_opt_theta.theta=theta_refine;
            data_opt_theta.threads=threads;
            data_opt_theta.N_ref=N_ref;
            data_opt_theta.N_avg=N_avg;
            data_opt_theta.image_avail=image_avail;
            data_opt_theta.optimize_with_avg=optimize_with_avg;

            // cout << "res_min=" << 1/res_min_high/1e-10 << endl;
            // cout << "res_max=" << 1/res_max_high/1e-10 << endl;

            // cout << endl << "[TEST] df_1 = " << data_opt_theta.ctf_para[0].getDefocus1() << endl << endl;
            // cout << endl << "[TEST] df_ref = " << data_opt_theta.df_ref << endl << endl;
            // double theta_tmp=0.0;
            // cout << endl << "[TEST] cc = " << get_theta_resolution_all_omp(1,&theta_tmp,NULL,&data_opt_theta) << endl << endl;

            nlopt_opt opt_theta;
            opt_theta=nlopt_create(NLOPT_LN_NELDERMEAD,1);
            nlopt_set_max_objective(opt_theta,get_theta_resolution_all_omp,&data_opt_theta);
            nlopt_set_xtol_abs1(opt_theta,1e-2);
            double step_theta[1]={0.1};
            nlopt_set_initial_step(opt_theta,step_theta);
            double x_theta[1]={0.0};
            double loss_min;
            if(nlopt_optimize(opt_theta,x_theta,&loss_min)<0)
            {
                cout << "[Error] nlopt failed!" << endl;
                x_theta[0]=0.0;
            }
            else
            {
                printf("Refined absolute tilt angle offset : %7.2f , with cc = %9.6f\n",x_theta[0],loss_min);
            }

            for(int n=0;n<Nz;n++)
            {
                theta_refine[n]+=x_theta[0];
            }
            if(abs(x_theta[0])>convergence_angle)
            {
                flag_convergence_angle=false;
            }
        }

        cout << endl;

        // 第二轮迭代开始，反过来利用CTF信息估角度
        // if(iter>0)
        // if(iter==-1)
        if(!skip_angle_refinement)
        {
            time(&current_time);
            strftime(time_buf,1024,"%a %b %d %T %Y",localtime(&current_time));
            cout << endl << "[" << time_buf << "] " << "#### Tilt angle refinement ####" << endl << endl;
            // cout << endl << "Using CTF information to refine tilt angles:" << endl;
            float psi_prev=psi_refine;
            float theta_prev[Nz];
            for(int n=0;n<Nz;n++)
            {
                theta_prev[n]=theta_refine[n];
            }
            #pragma omp parallel for num_threads(threads)
            for(int n=0;n<Nz;n++)
            {
                float df_now=(ctf_all[n][0]+ctf_all[n][1]+(ctf_all[n][0]-ctf_all[n][1])*cos(2*(-ctf_all[n][2]*M_PI/180.0)))/2;
                // float df_ref=(ctf_all[n][0]+ctf_all[n][1]+(ctf_all[n][0]-ctf_all[n][1])*cos(2*(-ctf_all[n][2]*M_PI/180.0)))/2/4.0;
                // float df_ref=(ctf_all[n][0]+ctf_all[n][1]+(ctf_all[n][0]-ctf_all[n][1])*cos(2*(-ctf_all[n][2]*M_PI/180.0)))/2;
                CTF ctf_para_now=ctf_para_all[n];
                // ctf_para_now.setAllCTFPara(ctf_all[n][0],ctf_all[n][1],ctf_all[n][2],0,w_cos);
                for(int m=0;m<N_block;m++)
                {
                    get_scaled_psd_phi(psd_block_all[n][m],psd_block_all_scaling[n][m],block_x[m],block_y[m],box,df_now,pix,psi_refine,theta_refine[n],phi_refine,Nx,Ny);
                }
            }

            for(int n=0;n<Nz;n++)
            {
                offset[n]=0.0;
                for(int t=0;t<N_block;t++)
                {
                    block_z[n][t]=0.0;
                    block_avail[n][t]=0;
                }
                // float df_now=(ctf_all[n][0]+ctf_all[n][1]+(ctf_all[n][0]-ctf_all[n][1])*cos(2*(-ctf_all[n][2]*M_PI/180.0)))/2/4.0;
                float df_now=(ctf_all[n][0]+ctf_all[n][1]+(ctf_all[n][0]-ctf_all[n][1])*cos(2*(-ctf_all[n][2]*M_PI/180.0)))/2;
                CTF ctf_para_now=ctf_para_all[n];
                // ctf_para_now.setAllCTFPara(ctf_all[n][0],ctf_all[n][1],ctf_all[n][2],0,w_cos);
                // CTF ctf_para_scale=ctf_para_all[n];
                // ctf_para_scale.setAllCTFPara(ctf_para_scale.getDefocus1()*1e10/4.0,ctf_para_scale.getDefocus2()*1e10/4.0,ctf_para_scale.getAstigmatism()*180.0/M_PI,ctf_para_scale.getPhaseShift(),ctf_para_scale.getW());
                float res_min_now=sqrt((M_PI*ctf_para_now.getLambda()*(df_now*1e-10)-sqrt(M_PI*M_PI*ctf_para_now.getLambda()*ctf_para_now.getLambda()*(df_now*1e-10)*(df_now*1e-10)-2*M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*(3.0*M_PI-ctf_para_now.getW_phase())))/(M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())));
                if(ctf_all_eva_n[n]-1<=3)
                {
                    res_min_now=sqrt((M_PI*ctf_para_now.getLambda()*(df_now*1e-10)-sqrt(M_PI*M_PI*ctf_para_now.getLambda()*ctf_para_now.getLambda()*(df_now*1e-10)*(df_now*1e-10)-2*M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*(2.0*M_PI-ctf_para_now.getW_phase())))/(M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())));
                }
                // float res_max_now=sqrt((M_PI*ctf_para_now.getLambda()*(df_now*1e-10)-sqrt(M_PI*M_PI*ctf_para_now.getLambda()*ctf_para_now.getLambda()*(df_now*1e-10)*(df_now*1e-10)-2*M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*(ceil(N_zeros/2)*M_PI-ctf_para_now.getW_phase())))/(M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())));
                float res_max_now=sqrt((M_PI*ctf_para_now.getLambda()*(df_now*1e-10)-sqrt(M_PI*M_PI*ctf_para_now.getLambda()*ctf_para_now.getLambda()*(df_now*1e-10)*(df_now*1e-10)-2*M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*(double(ctf_all_eva_n[n]-1)*M_PI-ctf_para_now.getW_phase())))/(M_PI*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())));
                if(res_max_now>1.0/(pix*1e-10)/2.0)
                {
                    res_max_now=1.0/(pix*1e-10)/2.0;
                }
                // float res_min_now=res_min;
                // float res_max_now=res_max;
                double chi_min=M_PI*ctf_para_now.getLambda()*df_now*1e-10*(res_min_now*res_min_now)-M_PI_2*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*double(res_min_now*res_min_now)*double(res_min_now*res_min_now)+ctf_para_now.getW_phase();
                double chi_max=M_PI*ctf_para_now.getLambda()*df_now*1e-10*(res_max_now*res_max_now)-M_PI_2*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*double(res_max_now*res_max_now)*double(res_max_now*res_max_now)+ctf_para_now.getW_phase();

                float *chi_all=new float[box*box];
                for(int j=0;j<box;j++)
                {
                    for(int i=0;i<box;i++)
                    {
                        // float df=(ctf_para_now.getDefocus1()+ctf_para_now.getDefocus2()+(ctf_para_now.getDefocus1()-ctf_para_now.getDefocus2())*cos(2*(atan_all[j*box+i]-ctf_para_now.getAstigmatism())))/2/4.0;
                        float df=(ctf_para_now.getDefocus1()+ctf_para_now.getDefocus2()+(ctf_para_now.getDefocus1()-ctf_para_now.getDefocus2())*cos(2*(atan_all[j*box+i]-ctf_para_now.getAstigmatism())))/2;
                        float chi=M_PI*ctf_para_now.getLambda()*df*res2_all[j*box+i]-M_PI_2*ctf_para_now.getCs()*(ctf_para_now.getLambda()*ctf_para_now.getLambda()*ctf_para_now.getLambda())*double(res2_all[j*box+i])*double(res2_all[j*box+i])+ctf_para_now.getW_phase();
                        chi_all[j*box+i]=chi;
                    }
                }

                float *block_dz=new float[N_block];
                for(int m=0;m<N_block;m++)
                {
                    block_dz[m]=0.0;
                }

                // 切小块分别估计CTF
                #pragma omp parallel for num_threads(threads)
                for(int m=0;m<N_block;m++)
                {
                    float *psd_block_now=new float[box*box];
                    int N_block_now=0;
                    bool flag_now=true;
                    for(int i=0;i<box*box;i++)
                    {
                        psd_block_now[i]=0.0;
                    }
                    for(int ny=-N_avg_block;ny<=N_avg_block;ny++)
                    {
                        for(int nx=-N_avg_block;nx<=N_avg_block;nx++)
                        {
                            if(block_x[m]+nx*(box/2)-box/2>=0 && block_x[m]+nx*(box/2)+box/2<Nx && block_y[m]+ny*(box/2)-box/2>=0 && block_y[m]+ny*(box/2)+box/2<Ny)
                            {
                                N_block_now++;
                                for(int i=0;i<box*box;i++)
                                {
                                    psd_block_now[i]+=psd_block_all_scaling[n][m+nx+ny*N_block_x][i];
                                    // psd_block_now[i]+=psd_block_all[n][m+nx+ny*N_block_x][i];
                                }
                            }
                            else
                            {
                                flag_now=false;
                                break;
                            }
                        }
                    }
                    for(int i=0;i<box*box;i++)
                    {
                        psd_block_now[i]/=float(N_block_now);
                    }
                    /*
                    // 高频部分舍弃，低频部分通过padding加密
                    float *psd_tmp=new float[box*box];
                    get_psd_center_padding(psd_block_now,psd_tmp,box,plan_fft_omp_half[omp_get_thread_num()],plan_ifft_omp[omp_get_thread_num()],fft_buf_in_omp[omp_get_thread_num()],fft_buf_out_omp[omp_get_thread_num()]);
                    memcpy(psd_block_now,psd_tmp,sizeof(float)*box*box);
                    delete [] psd_tmp;
                    */

                    /*
                    if(n==20 && flag_now)
                    {
                        float *psd_block_show=new float[box*box];
                        get_psd_fit_show_epa(psd_block_now,psd_block_show,box,ctf_para_now,res_min,res_max,chi_min,chi_max,res2_all,atan_all,x_fft_1d_res);
                        string log_4_filename="log_4_"+to_string(m)+".mrc";
                        #pragma omp critical
                        {
                            mlog.open(log_4_filename.c_str(),"w");
                            mlog.createMRC_empty(box,box,1,2);
                            mlog.write2DIm(psd_block_show,0);
                            mlog.close();
                        }
                        delete [] psd_block_show;
                    }
                    */
                    
                    if(flag_now)
                    {
                        get_one_block_defocus_epa_omp(psd_block_now,block_x[m],block_y[m],block_z[n],block_avail[n],block_dz,box,pix,psi_refine,theta_refine[n],phi_refine,Nx,Ny,N_zeros,ctf_para_now,res_min_now,res_max_now,chi_min,chi_max,box_conv,m,res2_all,chi_all,atan_all,x_fft_1d_res,plan_fft_omp[omp_get_thread_num()],plan_ifft_omp[omp_get_thread_num()],fft_buf_in_omp[omp_get_thread_num()],fft_buf_out_omp[omp_get_thread_num()]);
                    }
                    else
                    {
                        block_avail[n][m]=0;
                    }
                    delete [] psd_block_now;
                }

                // 统计离焦量变化去野点
                float *block_dz_avail=new float[N_block];
                int N_block_avail=0;
                for(int m=0;m<N_block;m++)
                {
                    block_dz_avail[m]=0.0;
                }
                for(int m=0;m<N_block;m++)
                {
                    if(block_avail[n][m]==1)
                    {
                        block_dz_avail[N_block_avail]=abs(block_dz[m]);
                        N_block_avail++;
                    }
                }
                sort(block_dz_avail,block_dz_avail+N_block_avail);
                // for(int m=0;m<N_block_avail;m++)
                // {
                //     cout << block_dz_avail[m] << endl;
                // }
                // int N_cut=float(N_block_avail)*0.5*0.5;
                // int N_cut=float(N_block_avail)*0.2;  // type-1，功率谱取平均
                int N_cut=float(N_block_avail)*0.95; // type-2，功率谱不取平均
                // int N_cut=float(N_block_avail)-1;
                float dz_cut=block_dz_avail[N_cut];
                // cout << "Cut at " << dz_cut << endl;
                for(int m=0;m<N_block;m++)
                {
                    if(block_avail[n][m]==1)
                    {
                        if(abs(block_dz[m])>dz_cut)
                        {
                            block_avail[n][m]=0;
                        }
                    }
                }

                /*
                // 统计离焦量变化去野点
                double sum_dz=0.0;
                double sum_dz2=0.0;
                float avg_dz=0.0;
                float std_dz=0.0;
                int count_dz=0;
                for(int m=0;m<N_block;m++)
                {
                    if(block_avail[n][m]==1)
                    {
                        sum_dz+=double(block_dz[m]);
                        sum_dz2+=(double(block_dz[m])*double(block_dz[m]));
                        count_dz++;
                    }
                }
                if(count_dz>0)
                {
                    avg_dz=float(sum_dz/double(count_dz));
                    std_dz=float(sqrt(sum_dz2/double(count_dz)-(sum_dz/double(count_dz))*(sum_dz/double(count_dz))));
                }
                cout << "avg: " << avg_dz << ", " << "std: " << std_dz << endl;
                for(int m=0;m<N_block;m++)
                {
                    if(block_dz[m]>avg_dz+1*std_dz || block_dz[m]<avg_dz-1*std_dz)
                    {
                        block_avail[n][m]=0;
                    }
                }
                */

                // 整体拟合平面
                int N_blocks_avail=0;
                for(int t=0;t<N_block;t++)
                {
                    if(block_avail[n][t]==1)
                    {
                        N_blocks_avail++;
                    }
                }
                if(N_blocks_avail<9)
                {
                    // cout << n << ": " << "Not enough available blocks, skip refinement process! Final tilt angle: " << theta_refine[n] << endl;
                    printf("%02d : Not enough available blocks, skip refinement process! Final tilt angle : %7.2f\n",theta_refine[n]);

                    normal_vector[n][0]=0.0;
                    normal_vector[n][1]=0.0;
                    normal_vector[n][2]=1.0;
                }
                else
                {
                    Eigen::MatrixXf A(N_blocks_avail,3);
                    Eigen::VectorXf b(N_blocks_avail);
                    Eigen::Vector3f ww;
                    int k=0;
                    for(int t=0;t<N_block;t++)
                    {
                        if(block_avail[n][t]==1)
                        {
                            A(k,0)=float(block_x[t]-floor(Nx/2))*pix;    // 单位：A
                            A(k,1)=float(block_y[t]-floor(Ny/2))*pix;
                            A(k,2)=1;
                            b(k)=block_z[n][t];
                            k++;
                        }
                    }
                    Eigen::Matrix3f ATA;
                    Eigen::Vector3f ATb;
                    ATA=A.transpose()*A;
                    ATb=A.transpose()*b;
                    ww=ATA.inverse()*ATb;
                    // 旋转phi转回来
                    float nv_x=cos(-psi_refine*M_PI/180.0);
                    float nv_y=-sin(-psi_refine*M_PI/180.0);
                    
                    float ww_n_d=ww(0)*nv_x+ww(1)*nv_y;
                    float ww_n_x=ww_n_d*nv_x;
                    float ww_n_y=ww_n_d*nv_y;
                    
                    float ww_t_x=ww(0)-ww_n_x;
                    float ww_t_y=ww(1)-ww_n_y;
                    float ww_t_z=-1;

                    // float ww_t_x_psi=0.0;
                    // float ww_t_y_psi=Sign(phi_refine)*sqrt(ww_t_x*ww_t_x+ww_t_y*ww_t_y);
                    // float ww_t_x_psi=ww_t_x*cos(-psi_refine*M_PI/180.0)-ww_t_y*sin(-psi_refine*M_PI/180.0);
                    // cout << ww_t_x_psi << endl;
                    float ww_t_x_psi=0.0;
                    float ww_t_y_psi=ww_t_x*sin(-psi_refine*M_PI/180.0)+ww_t_y*cos(-psi_refine*M_PI/180.0);
                    float ww_t_z_psi=ww_t_z;

                    float ww_t_x_psi_phi=ww_t_x_psi;
                    float ww_t_y_psi_phi=ww_t_y_psi*cos(-phi_refine*M_PI/180.0)-ww_t_z_psi*sin(-phi_refine*M_PI/180.0);
                    float ww_t_z_psi_phi=ww_t_y_psi*sin(-phi_refine*M_PI/180.0)+ww_t_z_psi*cos(-phi_refine*M_PI/180.0);

                    float ww_t_x_psi_phi_psi=ww_t_x_psi_phi*cos(psi_refine*M_PI/180.0)-ww_t_y_psi_phi*sin(psi_refine*M_PI/180.0);
                    float ww_t_y_psi_phi_psi=ww_t_x_psi_phi*sin(psi_refine*M_PI/180.0)+ww_t_y_psi_phi*cos(psi_refine*M_PI/180.0);
                    float ww_t_z_psi_phi_psi=ww_t_z_psi_phi;

                    float ww_x_rot=ww_t_x_psi_phi_psi+ww_n_x;
                    float ww_y_rot=ww_t_y_psi_phi_psi+ww_n_y;
                    float ww_z_rot=ww_t_z_psi_phi_psi;
                    float ww_rot_d=sqrt(ww_x_rot*ww_x_rot+ww_y_rot*ww_y_rot+ww_z_rot*ww_z_rot);

                    ww(0)=ww_x_rot/(-ww_z_rot);
                    ww(1)=ww_y_rot/(-ww_z_rot);    // 法向量z坐标归-1，为便于后面求角度，和原直接读平面法向量情况统一

                    psi_refine=atan2(ww(1),ww(0))*180.0/M_PI;
                    if(psi_refine<0.0)
                    {
                        psi_refine+=180.0;
                    }
                    // cout << "Refined tilt axis angle: " << psi_refine << endl;
                    printf("Refined tilt axis angle : %7.2f\n",psi_refine);

                    // float tv_x=sin(-psi_refine*M_PI/180.0);
                    // float tv_y=cos(-psi_refine*M_PI/180.0);
                    // float tv_z=tan(phi_refine*M_PI/180.0);
                    // float tv_d=sqrt(tv_x*tv_x+tv_y*tv_y+tv_z*tv_z);
                    // tv_x/=tv_d;
                    // tv_y/=tv_d;
                    // tv_z/=tv_d;
                    // float ww_t_d=ww(0)*tv_x+ww(1)*tv_y+ww(2)*tv_z;
                    // float ww_t_x=tv_x*ww_t_d;
                    // float ww_t_y=tv_y*ww_t_d;
                    // float ww_t_z=tv_z*ww_t_d;

                    if(psi_refine>=-90.0 && psi_refine<=90.0)
                    {
                        if(psi_refine>=-45 && psi_refine<=45)
                        {
                            if(ww(0)<0)
                            {
                                theta_refine[n]=-atan(sqrt(ww(0)*ww(0)+ww(1)*ww(1)))*180.0/M_PI;
                            }
                            else
                            {
                                theta_refine[n]=atan(sqrt(ww(0)*ww(0)+ww(1)*ww(1)))*180.0/M_PI;
                            }
                        }
                        else
                        {
                            if(ww(1)<0)
                            {
                                theta_refine[n]=-atan(sqrt(ww(0)*ww(0)+ww(1)*ww(1)))*180.0/M_PI;
                            }
                            else
                            {
                                theta_refine[n]=atan(sqrt(ww(0)*ww(0)+ww(1)*ww(1)))*180.0/M_PI;
                            }
                        }
                    }
                    else
                    {
                        if((psi_refine>=90 && psi_refine<=135) || (psi_refine>=-135 && psi_refine<=-90))
                        {
                            if(ww(0)<0)
                            {
                                theta_refine[n]=atan(sqrt(ww(0)*ww(0)+ww(1)*ww(1)))*180.0/M_PI;
                            }
                            else
                            {
                                theta_refine[n]=-atan(sqrt(ww(0)*ww(0)+ww(1)*ww(1)))*180.0/M_PI;
                            }
                        }
                        else
                        {
                            if(ww(1)<0)
                            {
                                theta_refine[n]=atan(sqrt(ww(0)*ww(0)+ww(1)*ww(1)))*180.0/M_PI;
                            }
                            else
                            {
                                theta_refine[n]=-atan(sqrt(ww(0)*ww(0)+ww(1)*ww(1)))*180.0/M_PI;
                            }
                        }
                    }
                    // theta_refine[n]=atan(sqrt(ww(0)*ww(0)+ww(1)*ww(1)))*180.0/M_PI*Sign(theta[n]);;
                    offset[n]=ww(2);
                    // cout << n << ": " << theta_refine[n] << endl;
                    printf("%02d : Refined tilt angle : %7.2f\n",theta_refine[n]);
                    
                    normal_vector[n][0]=-ww(0);
                    normal_vector[n][1]=-ww(1);
                    normal_vector[n][2]=1.0;
                }

                if(n==0)
                {
                    if(log_avail)
                    {
                        FILE *flog=fopen("log_4.txt","w");
                        for(int m=0;m<N_block;m++)
                        {
                            // if(block_avail[n][m])
                            {
                                fprintf(flog,"%d %d %d %f %d\n",m,block_x[m],block_y[m],block_dz[m],block_avail[n][m]);
                            }
                        }
                        fflush(flog);
                        fclose(flog);
                    }
                }

                delete [] block_dz;
                delete [] block_dz_avail;
                delete [] chi_all;
            }

            if(abs(psi_refine-psi_prev)>convergence_angle)
            {
                flag_convergence_angle=false;
            }
            for(int n=0;n<Nz;n++)
            {
                if(abs(theta_prev[n]-theta_refine[n])>convergence_angle)
                {
                    flag_convergence_angle=false;
                    break;
                }
            }
        }

        if(flag_convergence_angle==true && flag_convergence_ctf==true)
        {
            flag_end=true;
        }
        if(flag_end)
        {
            time(&current_time);
            strftime(time_buf,1024,"%a %b %d %T %Y",localtime(&current_time));
            cout << endl << "[" << time_buf << "] " << "#### Iteration converges! End estimation! ####" << endl << endl;
            break;
        }

        cout << endl;
    }

    // Write out final results
    time(&current_time);
    strftime(time_buf,1024,"%a %b %d %T %Y",localtime(&current_time));
    cout << endl << "[" << time_buf << "] " << "################ Final estimation results ################" << endl;
    cout << endl;
    cout << "# Column 1: Frame number@Micrograph name" << endl;
    cout << "# Column 2: Defocus1 (in Angstrom)" << endl;
    cout << "# Column 3: Defocus2 (in Angstrom)" << endl;
    cout << "# Column 4: Astigmatism angle (in degree)" << endl;
    cout << "# Column 5: Phase shift (in radian)" << endl;
    cout << "# Column 6: Tilt axis angle (in degree)" << endl;
    cout << "# Column 7: Tilt angle (in degree)" << endl;
    cout << "# Column 8: Tilt offset around x-axis (x-tilt) (in degree)" << endl;
    cout << "# Column 9: Correlation coefficient" << endl;
    cout << "# Column 10: Well fitted resolution" << endl << endl;
    for(int n=0;n<Nz;n++)
    {
        printf("%02d@%s %9.3f %9.3f %7.2f 0.000000 %7.2f %7.2f %7.2f %9.6f %7.4f\n",micrograph_num_all[n],micrograph_name_all[n].c_str(),ctf_all[n][0],ctf_all[n][1],ctf_all[n][2],psi_refine,theta_refine[n],phi_refine,ctf_all[n][3],ctf_all_eva_res[n]);
    }

    for(int n=0;n<Nz;n++)
    {
        // fprintf(fdefocus,"%d %f %f %f 0.000000 %f %f\n",n,ctf_all[n][0],ctf_all[n][1],ctf_all[n][2],ctf_all[n][3],ctf_all_eva_res[n]);
        if(output_raw)
        {
            stack_psd.write2DIm(psd_show_all_raw[n],n);
        }
        else
        {
            stack_psd.write2DIm(psd_show_all[n],n);
        }
        // stack_psd_raw.write2DIm(psd_show_all_raw[n],n);
        // stack_psd_conv.write2DIm(psd_show_all_conv[n],n);
        // stack_psd_combine.write2DIm(psd_show_all_combine[n],n);
        delete [] psd_show_all[n];
        delete [] psd_show_all_raw[n];
        delete [] psd_show_all_conv[n];
        delete [] psd_show_all_combine[n];
    }
    // fclose(fdefocus);
    stack_psd.close();
    // stack_psd_raw.close();
    // stack_psd_conv.close();

    /*
    FILE *ftlt_out=fopen(output_tlt.c_str(),"w");
    for(int n=0;n<Nz;n++)
    {
        fprintf(ftlt_out,"%f\n",theta_refine[n]);
    }
    fclose(ftlt_out);
    */

    /*
    string fpsi_name=path+"/"+prfx+"_ctf_psi.txt";
    FILE *fpsi=fopen(fpsi_name.c_str(),"w");
    fprintf(fpsi,"%f\n",psi_refine);
    fclose(fpsi);
    */

    // Write equation for calculating defocus of every pixel
    cout << endl;
    cout << "# Equation for calculating defocus values of every pixel:" << endl;
    cout << "# PSI = Tilt axis angle = " << psi_refine << endl;
    cout << "# THETA = Tilt angle = (in output file)" << endl;
    cout << "# PHI = Tilt offset around x-axis (x-tilt) = " << phi_refine << endl;
    cout << "# PIX = Pixel size = " << pix << " (In Angstrom)" << endl;
    cout << "# NX = Micrograph size in X dimension = " << Nx << endl;
    cout << "# NY = Micrograph size in Y dimension = " << Ny << endl;
    cout << endl;
    cout << "# For pixel (X,Y) (start from 0, X stands for the first (fastest changing) dimension):" << endl;
    cout << "# DX(X) = X - X_CENTER = X - " << int(Nx/2) << endl;
    cout << "# DY(Y) = Y - Y_CENTER = Y - " << int(Ny/2) << endl;
    // cout << "# DELTA_Z(X,Y) = (DX(X) * Cos(PSI) + DY(Y) * Sin(PSI)) * Tan(THETA) * PIX" << endl;
    cout << "# DELTA_Z(X,Y) = (DX(X) * (Cos(PHI) * Sin(THETA) * Cos(PSI) - Sin(PHI) * Sin(PSI)) + DY(Y) * (Cos(PHI) * Sin(THETA) * Sin(PSI) + Sin(PHI) * Cos(PSI))) / (Cos(PHI) * Cos(THETA)) * PIX" << endl;
    cout << "# DEFOCUS_1(X,Y) = DEFOCUS_1 + DELTA_Z(X,Y)" << endl;
    cout << "# DEFOCUS_2(X,Y) = DEFOCUS_2 + DELTA_Z(X,Y)" << endl;
    cout << endl;
    if(single_particle)
    {
        cout << "# Input micrograph in " << input_micrograph << endl;
    }
    else
    {
        cout << "# Input micrograph list in " << input_list << endl;
    }
    cout << "# Tilt-series average power spectrum and Thon ring fitting diagnosis in " << average_mrc << endl;
    cout << "# Single micrograph power spectrum and Thon ring fitting diagnosis in " << output_mrc << endl;
    cout << "# Estimation results saved in " << output_file << endl;
    cout << endl;

    // string fall_name=path+"/"+prfx+"_ctf.txt";
    // ofstream fall(fall_name.c_str());
    // ofstream fall(output_file.c_str());
    FILE *fall=fopen(output_file.c_str(),"w");
    if(!fall)
    {
        cerr << "[Error] Cannot open output file!" << endl;
    }
    else
    {
        fprintf(fall,"# Column 1: Frame number@Micrograph name\n");
        fprintf(fall,"# Column 2: Defocus1 (in Angstrom)\n");
        fprintf(fall,"# Column 3: Defocus2 (in Angstrom)\n");
        fprintf(fall,"# Column 4: Astigmatism angle (in degree)\n");
        fprintf(fall,"# Column 5: Phase shift (in radian)\n");
        fprintf(fall,"# Column 6: Tilt axis angle (in degree)\n");
        fprintf(fall,"# Column 7: Tilt angle (in degree)\n");
        fprintf(fall,"# Column 8: Tilt offset around x-axis (x-tilt) (in degree)\n");
        fprintf(fall,"# Column 9: Correlation coefficient\n");
        fprintf(fall,"# Column 10: Well fitted resolution\n\n");

        for(int n=0;n<Nz;n++)
        {
            fprintf(fall,"%02d@%s %9.3f %9.3f %7.2f 0.000000 %7.2f %7.2f %7.2f %9.6f %7.4f\n",micrograph_num_all[n],micrograph_name_all[n].c_str(),ctf_all[n][0],ctf_all[n][1],ctf_all[n][2],psi_refine,theta_refine[n],phi_refine,ctf_all[n][3],ctf_all_eva_res[n]);
        }

        fprintf(fall,"\n");
        fprintf(fall,"# Equation for calculating defocus values of every pixel:\n");
        fprintf(fall,"# PSI = Tilt axis angle = %7.2f\n",psi_refine);
        fprintf(fall,"# THETA = Tilt angle = (in output file)\n");
        fprintf(fall,"# PHI = Tilt offset around x-axis (x-tilt) = %7.2f\n",phi_refine);
        fprintf(fall,"# PIX = Pixel size = %f (In Angstrom)\n",pix);
        fprintf(fall,"# NX = Micrograph size in X dimension = %d\n",Nx);
        fprintf(fall,"# NY = Micrograph size in Y dimension = %d\n",Ny);
        fprintf(fall,"\n");
        fprintf(fall,"# For pixel (X,Y) (start from 0, X stands for the first (fastest changing) dimension):\n");
        fprintf(fall,"# DX(X) = X - X_CENTER = X - %d\n",int(Nx/2));
        fprintf(fall,"# DY(Y) = Y - Y_CENTER = Y - %d\n",int(Ny/2));
        // fprintf(fall,"# DELTA_Z(X,Y) = (DX(X) * Cos(PSI) + DY(Y) * Sin(PSI)) * Tan(THETA) * PIX\n");
        fprintf(fall,"# DELTA_Z(X,Y) = (DX(X) * (Cos(PHI) * Sin(THETA) * Cos(PSI) - Sin(PHI) * Sin(PSI)) + DY(Y) * (Cos(PHI) * Sin(THETA) * Sin(PSI) + Sin(PHI) * Cos(PSI))) / (Cos(PHI) * Cos(THETA)) * PIX\n");
        fprintf(fall,"# DEFOCUS_1(X,Y) = DEFOCUS_1 + DELTA_Z(X,Y)\n");
        fprintf(fall,"# DEFOCUS_2(X,Y) = DEFOCUS_2 + DELTA_Z(X,Y)\n");
        fprintf(fall,"\n");
        if(single_particle)
        {
            fprintf(fall,"# Input micrograph in %s\n",input_micrograph.c_str());
        }
        else
        {
            fprintf(fall,"# Input micrograph list in %s\n",input_list.c_str());
        }
        fprintf(fall,"# Tilt-series average power spectrum and Thon ring fitting diagnosis in %s\n",average_mrc.c_str());
        fprintf(fall,"# Single micrograph power spectrum and Thon ring fitting diagnosis in %s\n",output_mrc.c_str());
        fprintf(fall,"# Estimation results saved in %s\n",output_file.c_str());
        fprintf(fall,"\n");
        
        fflush(fall);
        fclose(fall);
    }

    if(log_avail)
    {
        MRC stack_psd_nofitting("log_5_nofitting.st","w");
        stack_psd_nofitting.createMRC_empty(box,box,Nz,2);
        float *psd_show_nofitting=new float[box*box];
        for(int n=0;n<Nz;n++)
        {   
            get_psd_show_image_2(psd_nofitting_all[n],psd_show_nofitting,box,res_min_orig,res_max_orig,res2_all,atan_all);
            stack_psd_nofitting.write2DIm(psd_show_nofitting,n);
        }
        delete [] psd_show_nofitting;
        stack_psd_nofitting.close();
        MRC stack_psd_nosubtraction("log_5_nosubtraction.st","w");
        stack_psd_nosubtraction.createMRC_empty(box,box,Nz,2);
        float *psd_show_nosubtraction=new float[box*box];
        for(int n=0;n<Nz;n++)
        {   
            get_psd_show_image_2(psd_nosubtraction_all[n],psd_show_nosubtraction,box,res_min_orig,res_max_orig,res2_all,atan_all);
            stack_psd_nosubtraction.write2DIm(psd_show_nosubtraction,n);
            delete [] psd_nosubtraction_all[n];
        }
        delete [] psd_show_nosubtraction;
        stack_psd_nofitting.close();
    
        for(int n=0;n<Nz;n++)
        {
            float norm=sqrt(normal_vector[n][0]*normal_vector[n][0]+normal_vector[n][1]*normal_vector[n][1]+normal_vector[n][2]*normal_vector[n][2]);
            normal_vector[n][0]/=norm;
            normal_vector[n][1]/=norm;
            normal_vector[n][2]/=norm;
        }
        FILE *fnv=fopen("normal_vector.txt","w");
        for(int n=0;n<Nz;n++)
        {
            fprintf(fnv,"%f %f %f\n",normal_vector[n][0],normal_vector[n][1],normal_vector[n][2]);
        }
        fclose(fnv);

        MRC stack_psd_block("log_6_block.st","w");
        // stack_psd_block.createMRC_empty(box,box,N_block+1,2);   // Save the average PSD in the last one
        stack_psd_block.createMRC_empty(box,box,(N_block+1)*Nz,2);   // Save the average PSD in the last one
        MRC stack_psd_block_scaling("log_7_block_scaling.st","w");
        // stack_psd_block_scaling.createMRC_empty(box,box,N_block+1,2);
        stack_psd_block_scaling.createMRC_empty(box,box,(N_block+1)*Nz,2);
        // for(int n=0;n<=0;n++)   // output the block PSD for the highest tilt
        int t_block=0;
        for(int n=0;n<Nz;n++)
        {
            CTF ctf_para_now=ctf_para_all[n];
            float df=(ctf_para_now.getDefocus1()+ctf_para_now.getDefocus2()+(ctf_para_now.getDefocus1()-ctf_para_now.getDefocus2())*cos(2*(-ctf_para_now.getAstigmatism())))/2;
            float *psd_now=new float[box*box];
            float *psd_show=new float[box*box];
            float *psd_avg=new float[box*box];
            float *psd_scaling_avg=new float[box*box];
            for(int i=0;i<box*box;i++)
            {
                psd_now[i]=0.0;
                psd_show[i]=0.0;
                psd_avg[i]=0.0;
                psd_scaling_avg[i]=0.0;
            }
            for(int m=0;m<N_block;m++)
            {
                // int m_x=m%N_block_x;
                // int m_y=m/N_block_x;
                // if(m_x==0 || m_x==N_block_x-1 || m_y==0 || m_y==N_block_y-1)    // neglect the outermost blocks
                // {
                //     continue;
                // }
                get_scaled_psd_phi(psd_block_all[n][m],psd_now,block_x[m],block_y[m],box,df*1e10,pix,psi_refine,theta_refine[n],phi_refine,Nx,Ny);
                if(box_conv>0)
                {
                    // get_psd_conv_fft(psd_now,box,box_conv,plan_fft_omp[0],plan_ifft_omp[0],fft_buf_in_omp[0],fft_buf_out_omp[0]);
                }
                for(int i=0;i<box*box;i++)
                {
                    psd_scaling_avg[i]+=psd_now[i];
                }
                get_psd_fit_show_epa_subtraction_avg_log(psd_now,psd_show,box,res_min,res_max,res2_all);
                // stack_psd_block_scaling.write2DIm(psd_show,m);
                stack_psd_block_scaling.write2DIm(psd_show,t_block);
                memcpy(psd_now,psd_block_all[n][m],sizeof(float)*box*box);
                if(box_conv>0)
                {
                    // get_psd_conv_fft(psd_now,box,box_conv,plan_fft_omp[0],plan_ifft_omp[0],fft_buf_in_omp[0],fft_buf_out_omp[0]);
                }
                get_psd_fit_show_epa_subtraction_avg_log(psd_now,psd_show,box,res_min,res_max,res2_all);
                // stack_psd_block.write2DIm(psd_show,m);
                stack_psd_block.write2DIm(psd_show,t_block);
                t_block++;
                for(int i=0;i<box*box;i++)
                {
                    // psd_avg[i]+=psd_block_all[n][m][i];
                    psd_avg[i]+=psd_now[i];
                }
            }
            // if(box_conv>0)
            // {
            //     get_psd_conv_fft(psd_avg,box,box_conv,plan_fft_omp[0],plan_ifft_omp[0],fft_buf_in_omp[0],fft_buf_out_omp[0]);
            // }
            get_psd_fit_show_epa_subtraction_avg_log(psd_avg,psd_show,box,res_min,res_max,res2_all);
            // stack_psd_block.write2DIm(psd_show,N_block);
            stack_psd_block.write2DIm(psd_show,t_block);
            // if(box_conv>0)
            // {
            //     get_psd_conv_fft(psd_scaling_avg,box,box_conv,plan_fft_omp[0],plan_ifft_omp[0],fft_buf_in_omp[0],fft_buf_out_omp[0]);
            // }
            get_psd_fit_show_epa_subtraction_avg_log(psd_scaling_avg,psd_show,box,res_min,res_max,res2_all);
            // stack_psd_block_scaling.write2DIm(psd_show,N_block);
            stack_psd_block_scaling.write2DIm(psd_show,t_block);
            t_block++;
            delete [] psd_now;
            delete [] psd_show;
        }
        stack_psd_block.close();
        stack_psd_block_scaling.close();

        MRC stack_psd_block_scaling_unified("log_8_block_scaling_unified.mrc","w");
        stack_psd_block_scaling_unified.createMRC_empty(box,box,1,2);
        double *psd_scaling_unified_sum=new double[box*box];
        for(int i=0;i<box*box;i++)
        {
            psd_scaling_unified_sum[i]=0.0;
        }
        for(int n=0;n<Nz;n++)
        {
            CTF ctf_para_now=ctf_para_all[n];
            float df=(ctf_para_now.getDefocus1()+ctf_para_now.getDefocus2()+(ctf_para_now.getDefocus1()-ctf_para_now.getDefocus2())*cos(2*(-ctf_para_now.getAstigmatism())))/2;
            float *psd_now=new float[box*box];
            for(int i=0;i<box*box;i++)
            {
                psd_now[i]=0.0;
            }
            for(int m=0;m<N_block;m++)
            {
                get_scaled_psd_phi(psd_block_all[n][m],psd_now,block_x[m],block_y[m],box,df*1e10,pix,psi_refine,theta_refine[n],phi_refine,Nx,Ny);
                for(int i=0;i<box*box;i++)
                {
                    psd_scaling_unified_sum[i]+=psd_now[i];
                }
            }
            delete [] psd_now;
        }
        float *psd_scaling_unified_avg=new float[box*box];
        for(int i=0;i<box*box;i++)
        {
            psd_scaling_unified_avg[i]=float(psd_scaling_unified_sum[i]/double(N_block*Nz));
        }
        float *psd_show_1=new float[box*box];
        get_psd_fit_show_epa_subtraction_avg_log(psd_scaling_unified_avg,psd_show_1,box,res_min,res_max,res2_all);
        stack_psd_block_scaling_unified.write2DIm(psd_show_1,0);
        stack_psd_block_scaling_unified.close();
        if(box_conv>0)
        {
            float *psd_tmp=new float[box*box];
            memcpy(psd_tmp,psd_scaling_unified_avg,sizeof(float)*box*box);
            get_psd_conv_fft(psd_scaling_unified_avg,box,box_conv,plan_fft_omp[0],plan_ifft_omp[0],fft_buf_in_omp[0],fft_buf_out_omp[0]);
            float df_now=(ctf_para_avg.getDefocus1()+ctf_para_avg.getDefocus2()+(ctf_para_avg.getDefocus1()-ctf_para_avg.getDefocus2())*cos(2*(-ctf_para_avg.getAstigmatism())))/2;
            float res_first_zero=sqrt((M_PI*ctf_para_avg.getLambda()*(df_now)-sqrt(M_PI*M_PI*ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*(df_now)*(df_now)-2*M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())*(M_PI-ctf_para_avg.getW_phase())))/(M_PI*ctf_para_avg.getCs()*(ctf_para_avg.getLambda()*ctf_para_avg.getLambda()*ctf_para_avg.getLambda())));
            for(int i=0;i<box*box;i++)  // recover the PSD inside the first zero
            {
                if(res2_all[i]<(res_first_zero-5.0/float(box)/ctf_para_avg.getPixelSize())*(res_first_zero-5.0/float(box)/ctf_para_avg.getPixelSize()))
                {
                    psd_scaling_unified_avg[i]=psd_tmp[i];
                }
            }
            get_psd_fit_show_epa_subtraction_avg_log(psd_scaling_unified_avg,psd_show_1,box,res_min,res_max,res2_all);
            MRC stack_psd_block_scaling_unified_boxconv("log_8_block_scaling_unified_boxconv.mrc","w");
            stack_psd_block_scaling_unified_boxconv.createMRC_empty(box,box,1,2);
            stack_psd_block_scaling_unified_boxconv.write2DIm(psd_show_1,0);
            delete [] psd_tmp;
        }
        delete [] psd_scaling_unified_sum;
        delete [] psd_scaling_unified_avg;
    }

    /*
    string output_tlt_2=output_tlt+"_2";
    ftlt_out=fopen(output_tlt_2.c_str(),"w");
    for(int n=0;n<Nz;n++)
    {
        fprintf(ftlt_out,"%f\n",theta_refine_opt[n]);
    }
    fclose(ftlt_out);
    */

    for(int th=0;th<threads;th++)
    {
        delete [] psd_avg_omp[th];
        delete [] fft_buf_in_omp[th];
        delete [] fft_buf_out_omp[th];
        fftwf_destroy_plan(plan_fft_omp_half[th]);
        fftwf_destroy_plan(plan_fft_omp[th]);
        fftwf_destroy_plan(plan_ifft_omp[th]);
        fftwf_destroy_plan(plan_ifft_omp_half[th]);
    }
    delete [] res2_all;
    delete [] atan_all;

    for(int n=0;n<Nz;n++)
    {
        delete [] block_z[n];
        delete [] block_avail[n];
    }
    delete [] block_x;
    delete [] block_y;

    for(int n=0;n<Nz;n++)
    {
        for(int m=0;m<N_block;m++)
        {
            delete [] psd_block_all_scaling[n][m];
            delete [] psd_block_all[n][m];
        }
        delete [] psd_block_all_scaling[n];
        delete [] psd_block_all[n];
    }

    for(int n=0;n<Nz;n++)
    {
        delete [] psd_nofitting_all[n];
        delete [] psd_image[n];
        delete [] image_all[n];
    }
    delete [] psd_0;
}
