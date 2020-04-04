#include <stdio.h>
#include <stdlib.h>  
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <string>
#include <list>
#include <map> 
#include <vector> 
#include <iterator>
#include <iostream>
#include <fstream>
#include <ctime>
#include <stack>
#include <numeric>
#include <algorithm>

using namespace std;

class Segment;
class Net;
class Switchbox;

class Net
{
	public:
	int net_id;	
	Switchbox* sb;	
	vector<Segment*> segs; //all the segments on this net
	
	float avg_wirelength; //used to determine if the current net is longer than its average shape
	int num_samples; //number of wirelengths measured for this net
	
	Net(int id, Switchbox* my_sb);
	bool hasLoop();
	void addSegment(Segment* seg);
	bool hasSegment(Segment* seg);
	bool hasSegmentAt(int a, int b, int c, int d);
	bool drawSegments(int a, int b, int c, int d);
	int updateAverage();
	int getAverage();
	int diffFromAvg();
	
};

class Switchbox
{
	public:	
	vector<Net*> nets;	
	vector< vector<Segment*> > Hsegs;
	vector< vector<Segment*> > Vsegs;
	vector<pair<int, int>> vias;	
	vector<int> top, bot, left, right; //holds the pins on the perimeter of switchbox
	int orig_width, orig_height;
	
	Switchbox(string benchmark_name);
	void readInput(string benchmark_name);	
	void printSwitchbox();
	void printAllNets();
	void printNet(Net* net);
	void exportNet(Net* net, string output_file_name);
	int totalWirelength();
	void routeNet(Net* net);
	bool netExists(int net_id);
	void routeAllNets();
	bool resolveAllConflicts();
	bool resolveConflictsSweep(int sweep_direction);
	int resolveNodeConflicts(int i, int j, bool even, bool invert);
	void unlockAllSegments();
	int moveSegUp(Segment* seg, Net* net, int allow_chains);
	int moveSegDown(Segment* seg, Net* net, int allow_chains);
	int moveSegLeft(Segment* seg, Net* net, int allow_chains);
	int moveSegRight(Segment* seg, Net* net, int allow_chains);
	void cleanUp(Net* net);
	void cleanUpUshape(Net* net, Segment* seg);
	int placeVias();
	void exportSwitchbox(string output_file_name);
	int getNumPinsInDirection(int row, int col, bool even, bool invert, Net* net);
	int getNumSegsInDirection(int row, int col, bool even, bool invert, Net* net);
	int getNumFreeSpacesInDirection(int row, int col, bool even, bool invert, Net* net);
	void increaseSize();
	int getEmptySpaceBelow();
	int getEmptySpaceRight();
	void addRow();
	void addCol();
	bool conflictsExist();
	int hasConflicts(Segment* seg);
	bool netBenefit(Segment* seg, Net* net, bool even, bool invert);
	bool netBenefit(Segment* seg, Net* net, bool even, bool invert, int percent_chance);
};


