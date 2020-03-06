//channel_route.cpp
//Mark Sears
//implement 2-layer channel routing

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
#include <ctime>
#include <stack>
#include <numeric>
#include <algorithm> 

using namespace std;

class PIN;
class SEGMENT;
class NET;

class PIN
{
	public:
	NET* net; //the net that the pin is on
	
	int x, y;
	
	PIN()
	{
		x = 0;
		y = 0;
	}
};

// (x1, y1)                   (x2, y2)
//     *-------------------------*
//
class SEGMENT
{
	public:
	int x1, y1, x2, y2;
	
	SEGMENT()
	{
		x1 = 0;
		y1 = 0;
		x2 = 0;
		y2 = 0;
	}
};

class NET
{
	public:
	vector<SEGMENT> path;
	vector<PIN> pins;
	
};

//Each entry indicates the net it belongs to
//Example input file:
//0 1 2 2 1 3 0 4 0
//
//	-channel-
//
//2 0 1 3 0 4 0 0 0
//read the input file and populate the top and bot vectors
int readInputFile(string benchmark_path, vector<int> *top, vector<int> *bot)
{
	FILE *channel_file; 
	channel_file = fopen(benchmark_path.c_str(), "r");	
	if (channel_file == NULL) { fprintf(stderr, "Can't open channel file: %s!\n", benchmark_path.c_str()); exit(1); }
	
	printf("\nReading channel routing benchmarks file: %s\n", benchmark_path.c_str());
	
	int num_nets;
	fscanf(channel_file, "%d\n", &num_nets);//read in number of nets
	
	//read in top row
	for(int i = 0; i < num_nets; i++)
	{
		int next_int;
		fscanf(channel_file,"%d ", &next_int);
		top->push_back(next_int);		
	}
	fscanf(channel_file,"\n");
	
	//read in bot row
	for(int i = 0; i < num_nets; i++)
	{
		int next_int;
		fscanf(channel_file,"%d ", &next_int);
		bot->push_back(next_int);		
	}
	fscanf(channel_file,"\n");
	
	//output to terminal
	printf("top: {");
		for(int j = 0; j < top->size(); j++)
			printf("%d, ", top->at(j));
	printf("}\n");
	
	printf("bot: {");
		for(int j = 0; j < bot->size(); j++)
			printf("%d, ", bot->at(j));
	printf("}\n");
	
	return num_nets;
}

void printGraph(vector< vector<int> > *graph)
{
	printf("Graph: %d x %d\n", graph->size(), graph->at(0).size());
	for(int i = 0; i < graph->size(); i++)
	{
		printf("{");
		for(int j = 0; j < graph->at(i).size(); j++)
			printf("%d, ", graph->at(i)[j]);
		printf("}\n");
	}
}

void printActiveNets(vector< vector<int> > *active_nets)
{
	printf("active_nets:\n");
	for(int i = 0; i < active_nets->at(i).size(); i++)
	{
		printf("{ ");
			for(int k = 0; k < active_nets->size(); k++)
			{
				printf("%d ", active_nets->at(k)[i]);
			}
		printf("}\n");		
	}
}

//returns true if vect contains net
bool vectorContains(vector<int> vect, int net)
{
	for(int i = 0; i < vect.size(); i++)
		if(vect[i] == net) return true;
		
	return false;
}

//returns true if the specified net number has no more connections to make after the given index
bool netIsComplete(int net, int index, vector<int> *top, vector<int> *bot)
{
	for(int i = index; i < top->size(); i++)
	{
		if(top->at(i) == net || bot->at(i) == net)
			return false;
	}
	
	return true;
}

//returns true if set1 is a subset of set2
//i.e. all elements in set1 appear in set2
//useful to determine new zones
bool isSubset(vector<int> set1, vector<int> set2)
{
	//for each int in set1, if it doesn't exist in set2, return false
	for(int i = 0; i < set1.size(); i++)
	{
		bool element_found = false;
		for(int j = 0; j < set2.size(); j++)
		{
			if(set1[i] == set2[j])
			{
				element_found = true;
				break;
			}
		}
		
		if(element_found == false)
			return false; //not a subset!
	}
	
	return true;
}

void initializeActiveNets(int num_nets, vector<int> *top, vector<int> *bot, vector< vector<int> > *active_nets)
{	
	if(top->size() < 1) exit(2);
	 	
	for(int i = 0; i < top->size(); i++)
	{
		vector<int> vect; //list for the next line					
			
		//copy over all from previous vect
		if(i != 0)
		for(int j = 0; j < active_nets->at(i-1).size(); j++)
		{
			if(active_nets->at(i-1)[j] != 0)
			if(!netIsComplete(active_nets->at(i-1)[j], i, top, bot))
			{ //add to active_nets
				vect.push_back(active_nets->at(i-1)[j]);
			}
		}		
		
		
		//if vect does not already contain the top and bot, add them
		if(!vectorContains(vect, top->at(i)) && top->at(i) != 0)
			vect.push_back(top->at(i));
		if(!vectorContains(vect, bot->at(i)) && bot->at(i) != 0)
			vect.push_back(bot->at(i));
			
		//pad the rest with 0's (empty tracks) until num_tracks = num_nets
		//will add more tracks later if necessary
		for(int k = vect.size(); k < num_nets; k++)		
			vect.push_back(0);			
		
		sort(vect.begin(), vect.end());	
		
		active_nets->push_back(vect);
		
	}
}


//returns the number of unique ints in the vector
//doesn't count 0, which means no connection
int countNumNets(vector<int> *top, vector<int> *bot)
{
	int net_count = 0;
	
	vector<int> vect;
	for(int i = 0; i < top->size(); i++)
	{
		vect.push_back(top->at(i));
		vect.push_back(bot->at(i));
	}
		
	sort(vect.begin(), vect.end());
	
	for(int i = 0; i < vect.size(); i++)	
		if(vect[i] != 0 && vect[i] != vect[i+1])
			net_count++;		
			
	printf("net_count: %d\n", net_count);
	return net_count;
}


//vertical constraint graph
void initializeVCG(int num_nets, vector<int> *top, vector<int> *bot, vector< vector<int> > *active_nets, vector<vector<int>> *VCG) 
{
	//fill vector with all 0's
	for(int i = 0; i <= num_nets; i++)
	{		
		vector<int> row;
		for(int j = 0; j <= num_nets; j++)
		{
			 row.push_back(0);			
		}		
		VCG->push_back(row);		
	}
	
	//set 1's for the directional edges
	for(int i = 0; i < top->size(); i++)
	{	
		if(top->at(i) != 0 && bot->at(i) != 0) //if either has no net, skip
		{					
			VCG->at(top->at(i))[bot->at(i)] = 1;
		}
	}
}


//horizontal constraint graph
void initializeHCG(int num_nets, vector<int> *top, vector<int> *bot, vector< vector<int> > *active_nets, vector<vector<int>> *HCG)
{
	//fill vector with all 0's
	for(int i = 0; i <= num_nets; i++)
	{		
		vector<int> row;
		for(int j = 0; j <= num_nets; j++)
		{
			 row.push_back(0);			
		}		
		HCG->push_back(row);		
	}
	
	//set 1's for the non-directional edges
	for(int i = 0; i < active_nets->size(); i++)
	{
		vector<int> curr = active_nets->at(i);
		
		for(int j = 0; j < curr.size(); j++)
			for(int k = 0; k < curr.size(); k++)
			{
				HCG->at(curr[j])[curr[k]] = 1;
				HCG->at(curr[k])[curr[j]] = 1;
			}
	}
}

//adds an empty "track" by adding a 0 to each vector in active_nets
void addTrack(vector< vector<int> > *active_nets)
{
	for(int i = 0; i < active_nets->size(); i++)	
		active_nets->at(i).push_back(0);	
}


//returns true if net1 is above net2 in the VCG
//called recursively on immediate neighbors in VCG
bool isAboveVCG(int net1, int net2, vector<vector<int>> *VCG)
{
//DOES NOT HANDLE CYCLES WELL!!!! WILL JUST RETURN TRUE~

	//check if there is an edge from net1 to net2 in VCG (direct child)
	if(VCG->at(net1)[net2] == 1) return true;					
	
	//recursively check down the tree for all children
	for(int i = 1; i < VCG->at(net1).size(); i++)
	{ 
		if(VCG->at(net1)[i] == 1 && isAboveVCG(i, net2, VCG)) 
			return true;					
	}
	
	return false; //no path found, net1 is not above net2!
}

//rearranges active_nets such that all track are in target_track
void moveTrack(int track, int target_track, vector<vector<int>> *active_nets)
{
	printf("Moving track %d to target %d.\n", track, target_track);
	for(int i = 0; i < active_nets->size(); i++)
	{
		for(int j = 0; j < active_nets->at(i).size(); j++)
		{
			if(active_nets->at(i)[j] == track)
			{
				//active_nets->at(i).erase(active_nets->at(i).begin() + j);
				//active_nets->at(i).insert(active_nets->at(i).begin() + target_track, track);
				
				if(active_nets->at(i)[target_track] != 0)
					active_nets->at(i)[j] = active_nets->at(i)[target_track];
				else active_nets->at(i)[j] = 0;
				active_nets->at(i)[target_track] = track;
			}
		}
	}

	printActiveNets(active_nets);
}

//performs the routing procedure to place wires in the channel
//rearrange the active_nets so that it represents the horizontal tracks in the channel
void sortActiveNets(vector<int> *top, vector<int> *bot, vector<vector<int>> *active_nets, vector<vector<int>> *VCG, vector<vector<int>> *HCG)
{
	moveTrack(1, 5, active_nets);
	moveTrack(2, 4, active_nets);
	moveTrack(3, 3, active_nets);
	moveTrack(4, 2, active_nets);
	moveTrack(5, 1, active_nets);
}


void exportNet(NET n, FILE* output_file)
{
	fprintf(output_file, "%d\n", n.path.size());
}

//prints to file a set of coordinates representing wiring nets
//OUTPUT FORMAT:
//3 --num nets
//4 --segments on net
//x1 y1 x2 y2 --segment
//x1 y1 x2 y2
//x1 y1 x2 y2
//x1 y1 x2 y2
//5 --start of next net

void exportChannelNets(vector<NET> nets)
{
	//open export file
	FILE *output_file;
	string path = "./channel_routes/channel.route";

	output_file = fopen(path.c_str(), "w");	
	if (output_file == NULL) { fprintf(stderr, "Can't open output_layout file!");	exit(1); }
	
	printf("\nExporting channel route file.\n\n");
	
	fprintf(output_file, "%d\n", nets.size());

	//print the segements for each net to file
	for(int i = 0; i < nets.size(); i++)
	{
		exportNet(nets[i], output_file);
	}
		
}

void channelRoute()
{
	vector<int> *top = new vector<int>; //the nodes on top of the channel
	vector<int> *bot = new vector<int>; //the nodes on bottom of the channel
	
	vector<vector<int>> *active_nets = new vector<vector<int>>; 
		//list of nets that are active in the channel for this zone
	
	vector<vector<int>> *VCG = new vector<vector<int>>; //vertical constraints graph
	vector<vector<int>> *HCG = new vector<vector<int>>; //horizontal constraints graph
	
	printf("\n\n****CHANNEL ROUTING****\n\n");
	
	string benchmark_name = "channel2";
	
	string benchmark_path = "./channel_benchmarks/" + benchmark_name + ".ch";
	
	readInputFile(benchmark_path, top, bot);
	
	int num_nets = countNumNets(top, bot);
	
	initializeActiveNets(num_nets, top, bot, active_nets);
	
	printActiveNets(active_nets);	
		
	initializeVCG(num_nets, top, bot, active_nets, VCG); //vertical constraint graph
	
	initializeHCG(num_nets, top, bot, active_nets, HCG); //horizontal constraint graph

/*	
printf("\nVCG ");
printGraph(VCG);
printf("\nHCG ");
printGraph(HCG);
*/
	
	sortActiveNets(top, bot, active_nets, VCG, HCG);
	
	vector<NET> nets;
	
	for(int net_id = 1; net_id <= num_nets; net_id++)
	{
		NET new_net;
		
		int track_num = 0;
		
		
		
		nets.push_back(new_net);
	}

	exportChannelNets(nets);
	//printActiveNets(active_nets);
	
}

int main(int argc, char *argv[])
{
	channelRoute();	
	exit(1);
}
