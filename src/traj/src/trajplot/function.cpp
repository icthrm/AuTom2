#include "function.h"
#include "gnuplot_i.hpp"
#include <string.h>

using namespace std;

void wait_for_key ()
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__)  // every keypress registered, also arrow keys
    cout << endl << "Press any key to continue..." << endl;

    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
    _getch();
#elif defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__)
    cout << endl << "Press ENTER to continue..." << endl;

    std::cin.clear();
    std::cin.ignore(std::cin.rdbuf()->in_avail());
    std::cin.get();
#endif
    return;
}

void PlotPointListXY(const char* filename)
{
    Gnuplot g1("lines");
    string s;

    string first_str;
    int l;
    char ch;
    cout<<"here"<<endl;
    ifstream fin(filename);

    double x,y;
    int z;
    int nx , ny ,nz;
    while(getline(fin ,s)) {
        stringstream ss;
        ss<<s;
        ss>>first_str;
        if(first_str == "max") {
            ss>>nx>>ny>>nz;
            cout<<nx<<" "<<ny<<" "<<nz<<endl;
            break;
        }
    }
    g1.set_xrange(0,nx);
    g1.set_yrange(0,ny);

    while(getline(fin ,s)) {
        stringstream ss;
        ss<<s;
        ss>>first_str;
        if(first_str == "contour") {
            cout<<s<<endl;
            ss>>l>>ch>>l;
            vector<double> X, Y;
            for(int i=0; i<l; i++) {
                getline(fin ,s);
                ss.clear();
                ss<<s;
                ss>>x>>y>>z;
                X.push_back(x);
                Y.push_back(y);
            }
            g1.set_style("lines").plot_xy(X,Y, "" , "1");
        }
    }

    // ================== 修改开始 ==================
    
    // 1. 保存 SVG (必须在 savetops 之前)
    g1.cmd("set terminal svg size 800,600"); // 建议加上 size
    if(strstr(filename, "init.fid.txt") != NULL) {
        g1.cmd("set output 'init_trajxy.svg'");
    } else {
        g1.cmd("set output 'fin_trajxy.svg'");
    }
    g1.replot();          // 关键：强制将之前的绘图指令输出到当前的 SVG 文件
    g1.cmd("set output"); // 关键：关闭 SVG 文件，确保数据写入磁盘

    // 2. 保存 PostScript (原有的功能)
    // savetops 会内部切换 terminal 为 postscript，所以必须放在 SVG 之后
    g1.savetops("trajxy"); 
    
    // ================== 修改结束 ==================
}

void PlotPointListYZ(const char* filename)
{
    string s;
    string first_str;
    int l;
    char ch;
    cout<<"here"<<endl;
    ifstream fin(filename);

    double x,y;
    int z;
    int nx , ny ,nz;
    while(getline(fin ,s)) {
        stringstream ss;
        ss<<s;
        ss>>first_str;
        if(first_str == "max") {
            ss>>nx>>ny>>nz;
            cout<<nx<<" "<<ny<<" "<<nz<<endl;
            break;
        }
    }
    Gnuplot g1("lines");
    g1.set_yrange(0,nz);
    g1.set_xrange(0,nx);
    g1.set_xlabel("Locations of Markers(y)").set_ylabel("Image Index(z)");
    g1.set_style("linespoints");
    g1.set_pointsize(0.3);

    while(getline(fin ,s)) {
        stringstream ss;
        ss<<s;
        ss>>first_str;
        if(first_str == "contour") {
            cout<<s<<endl;
            ss>>l>>ch>>l;
            vector<double> X, Z,Y,N;
            for(int i=0; i<l; i++) {
                getline(fin ,s);
                ss.clear();
                ss<<s;
                ss>>x>>y>>z;
                X.push_back(x);
                Y.push_back(y);
                Z.push_back(z);
            }
            g1.plot_xy(Y,Z, "" , "1");
        }
    }
    
    // ================== 修改开始 ==================
    
    // 1. 保存 SVG
    g1.cmd("set terminal svg size 800,600");
    if(strstr(filename, "init.fid.txt") != NULL) {
        g1.cmd("set output 'init_trajyz.svg'");
    } else {
        g1.cmd("set output 'fin_trajyz.svg'");
    }
    g1.replot();          
    g1.cmd("set output"); 

    // 2. 保存 PS
    g1.savetops("trajyz");
    
    // ================== 修改结束 ==================
}


void PlotPointListXZ(const char* filename)
{
    string s;
    string first_str;
    int l;
    char ch;
    cout<<"here"<<endl;
    ifstream fin(filename);

    double x,y;
    int z;
    int nx , ny ,nz;
    while(getline(fin ,s)) {
        stringstream ss;
        ss<<s;
        ss>>first_str;
        if(first_str == "max") {
            ss>>nx>>ny>>nz;
            cout<<nx<<" "<<ny<<" "<<nz<<endl;
            break;
        }
    }
    Gnuplot g1("lines");
    g1.set_yrange(0,nz);
    g1.set_xrange(0,nx);
    g1.set_xlabel("Locations of Markers(y)").set_ylabel("Image Index(z)");
    g1.set_style("linespoints");
    g1.set_pointsize(0.3);

    while(getline(fin ,s)) {
        stringstream ss;
        ss<<s;
        ss>>first_str;
        if(first_str == "contour") {
            cout<<s<<endl;
            ss>>l>>ch>>l;
            vector<double> X, Z,Y,N;
            for(int i=0; i<l; i++) {
                getline(fin ,s);
                ss.clear();
                ss<<s;
                ss>>x>>y>>z;
                X.push_back(x);
                Y.push_back(y);
                Z.push_back(z);
            }
            g1.plot_xy(X,Z, "" , "1");
        }
    }
    
    // ================== 修改开始 ==================
    
    // 1. 保存 SVG
    g1.cmd("set terminal svg size 800,600");
    if(strstr(filename, "init.fid.txt") != NULL) {
        g1.cmd("set output 'init_trajxz.svg'");
    } else {
        g1.cmd("set output 'fin_trajxz.svg'");
    }
    g1.replot();          
    g1.cmd("set output"); 

    // 2. 保存 PS
    g1.savetops("trajxz");
    
    // ================== 修改结束 ==================
}


void Plot_Tragitorys(const char* filename)
{
    Gnuplot g1("lines");
    string s;

    string first_str;
    int l,n;
    char ch;
    cout<<"here"<<endl;
    ifstream fin(filename);

    double x,y;
    int z;
    int nx , ny ,nz , nt;
    while(getline(fin ,s)) {
        stringstream ss;
        ss<<s;
        ss>>first_str;
        if(first_str == "max") {
            ss>>nx>>ny>>nz;
            cout<<nx<<" "<<ny<<" "<<nz<<endl;
        }
        if(first_str == "object") {
            ss>>ch>>nt>>ch;
            break;
        }
    }
    g1.set_xrange(0,nz);
    g1.set_yrange(0,nt);
    std::ostringstream cmdstr;
    cmdstr<<"xtics 0,1,"<<nz;
    g1.cmd(cmdstr.str());
    cmdstr.clear();
    cmdstr<<"ytics 0,1,"<<nt;
    g1.cmd(cmdstr.str());
    while(getline(fin ,s)) {
        stringstream ss;
        ss<<s;
        ss>>first_str;
        if(first_str == "contour") {
            cout<<s<<endl;
            ss>>n>>ch>>l;
// 	    cout<<"length : "<<l<<endl;
            vector<double> Z, T;
            for(int i=0; i<l; i++) {
                getline(fin ,s);
                ss.clear();
                ss<<s;
                ss>>x>>y>>z;
                Z.push_back(z);
                T.push_back(n);
                g1.set_style("points").plot_xy(Z,T, "" , "1");

            }
        }
    }
    g1.savetops("Tragitorys");
    g1.replot();
    wait_for_key();
}

void PlotTDistribute(const char* filename)
{
    Gnuplot g1("lines");
    string s;

    string first_str;
    int l;
    char ch;
    cout<<filename<<endl;
    ifstream fin(filename);

    double x,y;
    int z;
    int nx , ny ,nz;
    while(getline(fin ,s)) {
        stringstream ss;
        ss<<s;
        ss>>first_str;
//         cout<<ss<<endl;
        if(first_str == "max") {
            ss>>nx>>ny>>nz;
            cout<<nx<<" "<<ny<<" "<<nz<<endl;
            break;
        }
    }
    g1.set_xrange(0,nz);
    g1.set_xlabel("Image number");
    g1.set_ylabel("Image number");

    vector<int> X;
    for(int i=0 ; i<nz; i++) {
        X.push_back(0);
    }
    while(getline(fin ,s)) {
        stringstream ss;
        ss<<s;
        ss>>first_str;
        if(first_str == "contour") {
            cout<<s<<endl;
            ss>>l>>ch>>l;
            X[l]++;
        }
    }
    g1.savetops("distribute");
    g1.set_grid();
    g1.cmd("set style fill solid 1.00");
    g1.cmd("set boxwidth 0.9 absolute");
    g1.set_style("histograms").plot_x(X);
// 	g1.remove_tmpfiles();
//     wait_for_key();
}
