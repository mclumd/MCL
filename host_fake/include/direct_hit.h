#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <list>
#include "mcl/mcl_multiagent_api.h"

using namespace std;

#define DHK "directHit"
#define SSN "status"
#define OBJT "object_t"
#define OBJN "it"
#define OBJS "state"

enum biff_eg { EGK_BLS = 0xff,
	       EGK_BLO,
	       EGK_BHS,
	       EGK_BHO,
	       EGK_ILS,
	       EGK_ILO
};

extern list<biff_eg> open_egks;

void biff_bl_self(string oname,double* sv);
void biff_bl_obj(string obj,string obs,double* sv);
void biff_bh_self(string oname,double* sv);
void biff_bh_obj(string obj,string obs,double* sv);
void biff_illegal_self(string oname,double* sv);
void biff_illegal_obj(string obj,string obs,double* sv);

void open(biff_eg begk);
void complete(biff_eg begk,mclMA::observables::update uo);
void abort(biff_eg begk);
void dump_egks();
