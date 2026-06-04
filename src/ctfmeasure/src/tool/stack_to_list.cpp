#include <cstdio>
#include <iostream>
#include <cstring>
#include <string>
#include "mrc.h"
#include "util.h"
using namespace std;

int main(int argc, char **argv)
{
    if(argc<5)
    {
        printf("Usage: \n%6s%s <input tilt-series stack> <input tilt angle file> <tilt axis angle> <output micrograph list file name>\n\n", "", argv[0]);
        exit(1);
    }

    string input_mrc=argv[1];
    string input_tlt=argv[2];
    float psi=atof(argv[3]);
    string output_file=argv[4];

    MRC stack_input(input_mrc.c_str(),"rb");
    if(!stack_input.hasFile())
    {
        cerr << "[Error] Cannot open input tilt-series stack!" << endl;
        abort();
    }

    FILE *ftlt=fopen(input_tlt.c_str(),"r");
    float theta[stack_input.getNz()];
    for(int n=0;n<stack_input.getNz();n++)
    {
        fscanf(ftlt,"%f",&theta[n]);
    }
    fclose(ftlt);

    FILE *fout=fopen(output_file.c_str(),"w");
    if(!fout)
    {
        cerr << "[Error] Cannot open output micrograph list file!" << endl;
        abort();
    }
    fprintf(fout,"# Column 1: Frame number@Micrograph name\n");
    fprintf(fout,"# Column 2: Tilt angle (in degree)\n");
    fprintf(fout,"# Column 3: Tilt axis angle (in degree)\n\n");
    for(int n=0;n<stack_input.getNz();n++)
    {
        fprintf(fout,"%d@%s %f %f\n",n,input_mrc.c_str(),theta[n],psi);
    }
    fclose(fout);

    cout << "Done" << endl << endl;
    cout << "Micrograph list written in " << output_file << endl << endl;

    return 0;
}
