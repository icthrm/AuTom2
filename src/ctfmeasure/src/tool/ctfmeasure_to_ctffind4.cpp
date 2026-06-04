#include <cstdio>
#include <iostream>
#include <cstring>
#include <string>
#include "mrc.h"
#include "util.h"
using namespace std;

int main(int argc, char **argv)
{
    if(argc<4)
    {
        printf("Usage: \n%6s%s <input defocus file (in CTFMeasure style)> <output defocus file (in CTFFIND4 style)> <output tilt angle file>\n\n", "", argv[0]);
        exit(1);
    }

    string input_defocus_file=argv[1];
    string output_defocus_file=argv[2];
    string output_tilt_file=argv[3];

    ifstream fdefocus_in(input_defocus_file.c_str(),ios::in);
    if(!fdefocus_in)
    {
        cerr << "[Error] Cannot open input defocus file!" << endl;
        abort();
    }

    FILE *fdefocus_out=fopen(output_defocus_file.c_str(),"w");
    if(!fdefocus_out)
    {
        cerr << "[Error] Cannot open output defocus file!" << endl;
        abort();
    }
    
    FILE *ftlt=fopen(output_tilt_file.c_str(),"w");
    if(!ftlt)
    {
        cerr << "[Error] Cannot open output defocus file!" << endl;
        abort();
    }
    
    string s;
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
                n=atoi(num_now.c_str());
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

        fprintf(fdefocus_out,"%02d %9.3f %9.3f %7.2f %9.6f %9.6f %7.4f\n",n,df_1,df_2,astig,phase_shift,cc,res);
        fprintf(ftlt,"%7.2f\n",theta);
    }

    fdefocus_in.close();
    fclose(fdefocus_out);
    fclose(ftlt);

    cout << "Done" << endl << endl;
    cout << "Output defocus file (in CTFFIND4 style) written in " << output_defocus_file << endl;
    cout << "Output tilt angle file written in " << output_tilt_file << endl << endl;

    return 0;
}
