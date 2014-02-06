#include "api/mcl/mcl_api.h"
#include <iostream>

using namespace mclMA;
using namespace std;
using namespace observables;

const string name = "Jack";

void init_mcl(){
	initializeMCL(name, 0);
	//cout << startMCL(name) << "\n";
	configureMCL(name, "basic", "default");
	
	declare_observable_self(name, "blue_loc");
	setSensorProp(name, "blue_loc", PROP_SCLASS, SC_SPATIAL);
	
	declare_observable_self(name, "blue_held");
	setSensorProp(name, "blue_held", PROP_SCLASS, SC_STATE);
	
	declare_observable_self(name, "success");
	setSensorProp(name, "success", PROP_SCLASS, SC_SPATIAL);
	
}

void start_search(){
	declareExpectationGroup(name, 0);
	declareSelfExpectation(name, 0, "blue_loc", EC_STAYOVER, 0.5);
}

void pick_up(){
	expectationGroupComplete(name, 0);
	declareExpectationGroup(name, 1);
	declareSelfExpectation(name, 1, "blue_held", EC_STAYOVER, 0.5);
}

void drop(){
	//expectationGroupComplete(name, 1);
	declareExpectationGroup(name, 2);
	declareSelfExpectation(name, 2, "success", EC_STAYOVER, 0.5);
}

void restart(){
	expectationGroupComplete(name, 2);
}

void my_monitor(double blue_located, double blue_held, double success){
	update the_update;
	the_update.set_update("blue_loc",blue_located);
	the_update.set_update("blue_held",blue_held);
	the_update.set_update("success",success);
	monitor(name, the_update);
	
	/*
	update_observable(name,"blue_loc",blue_located);
	update_observable(name,"blue_held",blue_held);
	update_observable(name,"success",success);
	
	*/
	
	/*
	float * sensorV = malloc(sizeof(float) * 3);
	sensorV[0] = blue_located;
	sensorV[0] = blue_held;
	sensorV[0] = success;
	*/
}

int main(int argc, char **argv)
{
	init_mcl();
	//look for blue thing
	//start_search();

	my_monitor(0, 0, 0);
	//use mcl response to locate blue thing. Once found:
	my_monitor(1, 0, 0);
	//move to blue thing
	//pick_up();
	my_monitor(1, 0, 0); //try again, pickup failed
	my_monitor(1, 1, 0); //success on second try
	//move to target
	drop();
	my_monitor(1, 0, 0); //huh? why not success? Reevaluate or something.
	
	restart();
	//repeat
	
	
	return 0;
	
}
