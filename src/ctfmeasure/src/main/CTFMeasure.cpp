/*******************************************************************
 *       Filename:  tomo.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  06/15/2020 05:48:48 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:
 *          Email:
 *        Company:
 *
 *******************************************************************/
#include <stdio.h>
#include "util.h"
#include "mrc.h"
#include "CTFAlgo_v3.h"

void getTime(time_t start_time)
{
    time_t end_time;
    time(&end_time);

    double seconds_total=difftime(end_time,start_time);
    int hours=((int)seconds_total)/3600;
    int minutes=(((int)seconds_total)%3600)/60;
    int seconds=(((int)seconds_total)%3600)%60;

    cout << "Time elapsed: ";
    if(hours>0)
    {
        cout << hours << "h ";
    }
    if(minutes > 0 || hours > 0)
    {
        cout << minutes << "m ";
    }
    cout << seconds << "s" << endl << endl;
}

void print_help(char **argv)
{
    // fprintf(stderr, "\n  Usage: \n%6s%s /path/to/parameter/file\n\n", "", argv[0]);
    // printf("\n################ CTFMeasure ################\n");
    printf("\n######## CTF estimation for tilt-series ########\n");
    printf("\n%6s%s --input_micrograph_list <input micrograph list> --pixel_size <pixel size, in Angstrom> --cs <spherical aberration, in mm> --voltage <acceleration volatge, in kV> --w <amplitude contrast> [--params <parameter file>]\n", "", argv[0]);

    printf("\nRequired parameters: \n");
    printf("      --input_micrograph_list      input micrograph list file\n");
    printf("      --pixel_size                 pixel size, in Angstrom\n");
    printf("      --cs                         spherical aberration, in mm\n");
    printf("      --voltage                    acceleration voltage, in kV\n");
    printf("      --w                          amplitude contrast\n");
    printf("\nOptional parameter: \n");
    printf("      --params                     optional parameter file\n");

    printf("\n######## CTF estimation for single-particle micrograph ########\n");
    printf("\n%6s%s --single_particle --input_micrograph <input micrograph file> [--theta <tilt angle, in degree>] [--psi <tilt axis angle, in degree>] --pixel_size <pixel size, in Angstrom> --cs <spherical aberration, in mm> --voltage <acceleration volatge, in kV> --w <amplitude contrast> [--params <parameter file>]\n", "", argv[0]);

    printf("\nRequired parameters: \n");
    printf("      --single_particle            activate single particle CTF estimation\n");
    printf("      --input_micrograph           input micrograph file\n");
    printf("      --pixel_size                 pixel size, in Angstrom\n");
    printf("      --cs                         spherical aberration, in mm\n");
    printf("      --voltage                    acceleration voltage, in kV\n");
    printf("      --w                          amplitude contrast\n");
    printf("\nOptional parameter: \n");
    printf("      --theta                      tilt angle, in degree, default is 0\n");
    printf("      --psi                        tilt axis angle, in degree, default is 0\n");
    printf("      --params                     optional parameter file\n");

    printf("\n######## Format for input micrograph list ########\n\n");
    printf("# Column 1: Frame number@Micrograph name\n");
    printf("# Column 2: Tilt angle (in degree)\n");
    printf("# Column 3: Tilt axis angle (in degree)\n");
    printf("# e.g.\n\n");
    printf("0@tomo_001.ali -60.000000 175.000000\n");
    printf("1@tomo_001.ali -57.000000 175.000000\n");
    printf("...\n");
    printf("40@tomo_001.ali 60.000000 175.000000\n\n");
    
    printf("\n######## Format for optional parameter file ########\n\n");
    printf("# In \"name\"=\"value\" style\n\n");
    printf("For parameters and their meaning in detail, please refer to README\n");
    printf("To write out a parameter file for reference in <example parameter file>, run \"%s --params_example <example parameter file>\"\n\n",argv[0]);
}

int main(int argc, char **argv)
{
    printf("\n################ CTFMeasure v1.4.0 ################\n");

    if(argc==3)
    {
        if(strcmp(argv[1],"--params_example")==0)
        {
            printf("Usage: \n%6s%s --input_micrograph_list <input micrograph list> --pixel_size <pixel size, in Angstrom> --cs <spherical aberration, in mm> --voltage <acceleration volatge, in kV> --w <amplitude contrast> [--params <parameter file>]\n", "", argv[0]);
            printf("%6s%s --single_particle --input_micrograph <input micrograph file> [--theta <tilt angle, in degree>] [--psi <tilt axis angle, in degree>] --pixel_size <pixel size, in Angstrom> --cs <spherical aberration, in mm> --voltage <acceleration volatge, in kV> --w <amplitude contrast> [--params <parameter file>]\n", "", argv[0]);

            printf("\nA parameter file for reference is written in %s\n\n",argv[2]);
            
            FILE *fpara=fopen(argv[2],"w");
            if(!fpara)
            {
                cerr << "[Error] Cannot open example parameter file " << argv[2] << "!" << endl;
                abort();
            }

            fprintf(fpara,"# This is an example parameter file.\n");
            fprintf(fpara,"# All values showed here are default values.\n");
            fprintf(fpara,"# To change a specific parameter value, please uncomment the corresponding line.\n\n");

            fprintf(fpara,"#### Basic parameters ####\n");
            fprintf(fpara,"workpath=./\n");
            fprintf(fpara,"# prfx={file basename of \"--input_micrograph_list\"}\n");
            fprintf(fpara,"# average_mrc={prfx}_avg.mrc\n");
            fprintf(fpara,"# output_mrc={prfx}_diag.st\n");
            fprintf(fpara,"# output_file={prfx}_ctf.txt\n");
            fprintf(fpara,"j=10\n\n");
            
            fprintf(fpara,"#### Parameters for CTF estimation ####\n");
            fprintf(fpara,"# box=512\n");
            fprintf(fpara,"# N_zeros=10\n");
            fprintf(fpara,"# defocus_min=10000.0\n");
            fprintf(fpara,"# defocus_max=100000.0\n");
            fprintf(fpara,"# defocus_step=100.0\n");
            fprintf(fpara,"# resolution_max=4*{pixel_size}\n");
            fprintf(fpara,"# resolution_min=30.0\n");
            fprintf(fpara,"# resolution_adaptive=1\n");
            fprintf(fpara,"# adaptive_range=1\n");
            fprintf(fpara,"# it=3\n\n");
            
            fprintf(fpara,"#### Advanced parameters ####\n");
            fprintf(fpara,"# search_phase_shift=0\n");
            fprintf(fpara,"# box_conv={box}/20\n");
            fprintf(fpara,"# N_avg=0\n");
            fprintf(fpara,"# N_ref={Micrograph with minimum tilt angle}\n");
            fprintf(fpara,"# box_conv_first=1\n");
            fprintf(fpara,"# skip_box_conv_coarse=0\n");
            fprintf(fpara,"# hard_restriction=0\n");
            fprintf(fpara,"# skip_unified=0\n");
            fprintf(fpara,"# optimize_with_average=0\n");
            fprintf(fpara,"# per_tilt_estimation=0\n");
            fprintf(fpara,"# tight_blocks=0\n");
            fprintf(fpara,"# astigmatism_angstrom=2000\n");
            fprintf(fpara,"# dose_weighting=0\n");
            fprintf(fpara,"# dose_acc_file=<accumulated dose file name>\n");
            fprintf(fpara,"# convergence_defocus=100\n");
            fprintf(fpara,"# convergence_astigmatism=1\n");
            fprintf(fpara,"# convergence_angle=0.1\n");
            fprintf(fpara,"# output_raw=0\n\n");

            fprintf(fpara,"#### Parameters for angle estimation ####\n");
            fprintf(fpara,"# tlt_offset=0.0\n");
            fprintf(fpara,"# xtilt=0.0\n");
            fprintf(fpara,"# pre_offset_estimation=0\n");
            fprintf(fpara,"# dose_file=<dose file name>\n");
            fprintf(fpara,"# skip_offset_estimation=1\n");
            fprintf(fpara,"# skip_offset_refinement=1\n");
            fprintf(fpara,"# skip_offset_estimation_x=1\n");
            fprintf(fpara,"# skip_offset_refinement_x=1\n");
            fprintf(fpara,"# skip_axis_refinement=1\n");
            fprintf(fpara,"# skip_angle_refinement=1\n\n");
            fprintf(fpara,"# N_avg_block=2\n");
            
            fflush(fpara);
            fclose(fpara);
            exit(1);
        }
    }

    if(argc==1)
    {
        print_help(argv);
        exit(1);
    }

    time_t start_time;
    time(&start_time);

    map<string, string> inputPara;
    map<string, string> outputPara;
    char *paraFileName;
    
    time_t current_time;
    char time_buf[1024];
    time(&current_time);
    cout << asctime(localtime(&current_time)) << endl;

    bool has_input_micrograph_list=false;
    bool has_pix=false;
    bool has_cs=false;
    bool has_voltage=false;
    bool has_w=false;
    
    bool has_single_particle=false;
    bool has_input_micrograph=false;

    for(int t=1;t<argc;t++)
    {
        if(strcmp(argv[t],"--input_micrograph_list")==0)
        {
            addOnePara(inputPara,"input_micrograph_list",argv[t+1]);
            // getOnePara(inputPara,"input_micrograph_list");
            t++;
            has_input_micrograph_list=true;
        }
        else if(strcmp(argv[t],"--pixel_size")==0)
        {
            addOnePara(inputPara,"pixel_size",argv[t+1]);
            // getOnePara(inputPara,"pixel_size");
            t++;
            has_pix=true;
        }
        else if(strcmp(argv[t],"--cs")==0)
        {
            addOnePara(inputPara,"Cs",argv[t+1]);
            // getOnePara(inputPara,"Cs");
            t++;
            has_cs=true;
        }
        else if(strcmp(argv[t],"--voltage")==0)
        {
            addOnePara(inputPara,"voltage",argv[t+1]);
            // getOnePara(inputPara,"voltage");
            t++;
            has_voltage=true;
        }
        else if(strcmp(argv[t],"--w")==0)
        {
            addOnePara(inputPara,"w",argv[t+1]);
            // getOnePara(inputPara,"w");
            t++;
            has_w=true;
        }
        else if(strcmp(argv[t],"--params")==0)
        {
            paraFileName=argv[t+1];
            readParaFile(inputPara, paraFileName);
            t++;
        }
        else if(strcmp(argv[t],"--input_micrograph")==0)
        {
            addOnePara(inputPara,"input_micrograph",argv[t+1]);
            t++;
            has_input_micrograph=true;
        }
        else if(strcmp(argv[t],"--theta")==0)
        {
            addOnePara(inputPara,"theta",argv[t+1]);
            t++;
        }
        else if(strcmp(argv[t],"--psi")==0)
        {
            addOnePara(inputPara,"psi",argv[t+1]);
            t++;
        }
        else if(strcmp(argv[t],"--single_particle")==0)
        {
            addOnePara(inputPara,"single_particle","1");
            has_single_particle=true;
        }
    }
    /*
    time(&current_time);
    strftime(time_buf,1024,"%a %b %d %T %Y",localtime(&current_time));
    cout << endl << "[" << time_buf << "] " << "################ All parameters ################" << endl;
    getAllParas(inputPara);
    */

    if(!has_cs || !has_pix || !has_w || !has_voltage || (has_single_particle && !has_input_micrograph) || (!has_single_particle && !has_input_micrograph_list))
    {
        cout << "[Error] Missing required parameters for running CTFMeasure!" << endl;
        print_help(argv);
        abort();
    }

    time(&current_time);
    strftime(time_buf,1024,"%a %b %d %T %Y",localtime(&current_time));
    cout << endl << "[" << time_buf << "] " << "################ Run CTFMeasure! ################" << endl << endl;
    CTFBase *cb = new CTFAlgo_v3();
    cb->doCTF(inputPara, outputPara);
    delete cb;

    time(&current_time);
    strftime(time_buf,1024,"%a %b %d %T %Y",localtime(&current_time));
    cout << endl << "[" << time_buf << "] " << "################ Finish! ################" << endl;
    getTime(start_time);

    return 0;
}
