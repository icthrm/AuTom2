#ifndef OPTS_H__
#define OPTS_H__

#include <iostream>
#include <sstream>
#include <cassert>
extern "C" {
#include <getopt.h>
}
#include "util/exception.h"

struct options{
    char input[255];
    char inputangle[255];
    char xfile[255];
};

inline int GetOpts(int argc, char **argv, options* opts_){

    static struct option longopts[] = {
        { "help",            no_argument,            NULL,              'h' },
        { "input",    	     required_argument,      NULL,              'i' },
	{ "angle",       required_argument,      NULL,              'a' },
	{ "xf",       	required_argument,      NULL,              'x' },
        { NULL,              0,                      NULL,               0  }
    };

    if((argc != 7) && argc >= 3 || (argc == 2 && argv[1][0] != '-' && argv[1][1] != 'h') || argc == 1) {
        EX_TRACE("[-i INPUT FILENAME][-a ANGLE FILENAME][-x XF FILENAME]\n");
        return -1;
    }

    int ch;
    while((ch = getopt_long(argc, argv, "hi:a:x:", longopts, NULL))!= -1) {
        switch(ch) {

        case '?':
            EX_TRACE("Invalid option '%s'.", argv[optind-1]);
            return -1;

        case ':':
            EX_TRACE("Missing option argument for '%s'.", argv[optind-1]);
            return -1;

        case 'h':
			EX_TRACE("[-i INPUT FILENAME][-a ANGLE FILENAME][-x XF FILENAME]\n");
			return 0;

        case 'i':
        {
            std::istringstream iss(optarg);
            iss >> opts_->input;
            if(iss.fail())
                EX_TRACE("Invalid argument '%s'.", optarg);
        }
        break;
	
	case 'a': 
        {
            std::istringstream iss(optarg);
            iss >> opts_->inputangle;
            if (iss.fail())
                EX_TRACE("Invalid argument '%s'.", optarg);
        }
        break;
	
	case 'x':
        {
            std::istringstream iss(optarg);
            iss >> opts_->xfile;
            if(iss.fail())
                EX_TRACE("Invalid argument '%s'.", optarg);
        }
        break;

        default:
            assert(false);
        }
    }
    return 1;
}

#endif