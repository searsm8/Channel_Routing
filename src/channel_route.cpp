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

class Pin;
class Net;

class Pin
{
	public:
	Net* net; //the net that the pin is on
	
	int x, y;
	
	Pin()
	{
		x = 0;
		y = 0;
	}
};

class Net
{
	public:
	string name;
	vector<Pin> pins;
	
	vector<Net*> VCG; //Vertical Constraints Graph
	vector<Net*> HCG; //Horizontal Constraints Graph
	
	int trunk_start; //x-coordinate where the trunk starts for this net
	int trunk_end;
	int track_num; //y-coordinate for the track that the trunk of this net will run
	// 1 < track_num <= num_tracks
	//WHAT ABOUT DOG LEGS???>?>?>?
	
	
	//isRelatedTo() returns true if net1 and net2 are connected in either VCG or HCG
	bool isRelatedTo(Net* net2)
	{
		if(this->isAbove(net2))
			return true;
				
		if(net2->isAbove(this))
			return true;
				
		for(int i = 0; i < this->HCG.size(); i++)
			if(this->HCG[i] == net2)
				return true;
				
		//HCG's are bi-directional, no need to check net2's HCG
		
		return false; //if not found anywhere, the nets are not related
	} //end isRelatedTo()
	
	
	//isAbove() returns true if there is a path down from this net to net2 in the VCG
	bool isAbove(Net* net2)
	{	
		//printf("net %s isAbove(net %s)\n", this->name.c_str(), net2->name.c_str());
						
		vector<Net*> visited;
		
		return isAboveHelper(net2, visited);
		
	}
	
	bool isAboveHelper(Net* net2, vector<Net*> visited)
	{//printf("net %s isAbove(net %s)\n", this->name.c_str(), net2->name.c_str());
		//check if this node has been visited before
		for(int i = 0; i < visited.size(); i++)
		{
			if(this == visited[i])
				return false; //it's a cycle, we've been here before
					//can't find the target!
		}
		
		//add this node to the visited list
		visited.push_back(this);
		
		//check if there is an edge from this to net2 in VCG (direct child)
		for(int i = 0; i < this->VCG.size(); i++)
		{						
			if(this->VCG[i] == net2)
			{
				//printf("YES.\n");
				return true;		
			}
			
			if( this->VCG[i]->isAboveHelper(net2, visited))
			{
				//printf("YES.\n");
				return true;		
			}
		}
		
		//printf("NO.\n");
		return false; //no path found, net1 is not above net2!
	}
	
	
	//returns true if this net is in a cycle
	//if the net is "above" itself, then it is in a cycle
	bool isInCycle()
	{//printf("net %s isInCycle?\n", this->name.c_str());
		vector<Net*> visited;
		return this->isCycleHelper(this, visited); 
	}
	
	bool isCycleHelper(Net* original, vector<Net*> visited)
	{
		//printf("net %s isCycleHelper.\toriginal = %s\n", this->name.c_str(), original->name.c_str());
		//check if there is an edge from this to net2 in VCG (direct child)
		
		//check if this node has been visited before
		for(int i = 0; i < visited.size(); i++)
		{
			if(this == visited[i])
				return false; //it's a cycle, we've been here before
					//can't find the target!
		}
		
		//add this node to the visited list
		visited.push_back(this);
		
		for(int i = 0; i < this->VCG.size(); i++)
		{
			if(this->VCG[i] == original)
			{
				//printf("YES.\n");
				return true;		
			}
			
			if( this->VCG[i]->isCycleHelper(original, visited))
			{
				//printf("YES.\n");
				return true;		
			}
		}
		
		//printf("NO.\n");
		return false; //no path found, net1 is not above net2!
	}
};

//Each entry indicates the net it belongs to
//Example input file:
//0 1 2 2 1 3 0 4 0
//
//	-channel-
//
//2 0 1 3 0 4 0 0 0
//read the input file and populate the top and bot vectors
vector<Net*> readInputFile(string benchmark_path)
{
	FILE *channel_file; 
	channel_file = fopen(benchmark_path.c_str(), "r");	
	if (channel_file == NULL) { fprintf(stderr, "Can't open channel file: %s!\n", benchmark_path.c_str()); exit(1); }
	
	
	vector <Net*> nets;
	
	printf("\nReading channel routing benchmarks file: %s...\n", benchmark_path.c_str());
	
	
	int num_inputs;
	fscanf(channel_file, "%d\n", &num_inputs);//read in number of nets
	
	//read in top row
	for(int i = 0; i < num_inputs; i++)
	{
		//create a pin on the correct net
		char net_name[100];
		fscanf(channel_file,"%s ", &net_name);
		
		Pin new_pin;
		new_pin.x = i; //set x coordinate
		new_pin.y = 1; //top row	

		//if the net already exists, add pin to net
		if(net_name[0] != 48) //does not equal "0"
		for(int j = 0; j <= nets.size(); j++)
		{
			//if reached the end without finding,
			//create a new net and add the pin
			if(j == nets.size())
			{	//printf("CREATING NEW Net: '%s'\n", net_name);
				Net* new_net = new Net;
				new_net->name = net_name;			
				new_pin.net = new_net;
				new_net->pins.push_back(new_pin);				
				nets.push_back(new_net);
				break;
			}
						
			if(nets[j]->name == net_name)
			{
				new_pin.net = nets[j];
				nets[j]->pins.push_back(new_pin);				
				break;
			}
		
			
		}
		
	}
	//fscanf(channel_file,"\n");
	
	//read in bot row
	for(int i = 0; i < num_inputs; i++)
	{
		//create a pin on the correct net
		char net_name[100];
		fscanf(channel_file,"%s ", &net_name);
		
		Pin new_pin;
		new_pin.x = i; //set x coordinate
		new_pin.y = 0; //bot row		

		

		//if the net already exists, add pin to net
		if(net_name[0] != 48)
		{
			for(int j = 0; j <= nets.size(); j++)
			{
				//if reached the end without finding,
				//create a new net and add the pin
				if(j == nets.size())
				{	//printf("CREATING NEW Net: '%s'\n", net_name);
					Net* new_net = new Net;
					new_net->name = net_name;				
					new_pin.net = new_net;
					new_net->pins.push_back(new_pin);				
					nets.push_back(new_net);
					break;
				}
							
				if(nets[j]->name == net_name)
				{
					new_pin.net = nets[j];
					nets[j]->pins.push_back(new_pin);
					
					break;
				}					
			}
			//populate the VCG for this new_pin.
			for(int j = 0; j < nets.size(); j++)
			{
				for(int k = 0; k < nets[j]->pins.size(); k++)
				{
					if(nets[j]->pins[k].x == i && nets[j]->pins[k].y == 1)
					{
						if( nets[j] != new_pin.net) //should not have itself in VCG
						nets[j]->VCG.push_back(new_pin.net);
					}
				}
			}
		}		
	}
	//fscanf(channel_file,"\n");
		printf("Done reading inputs file!\n");
	return nets;
}

//initializes the HCG (Horizontal Constraints Graph) for all nets
void initializeHCG(vector<Net*> nets)
{
	//initialize trunk start and end
	for(int i = 0; i < nets.size(); i++)
	{
		nets[i]->trunk_start = nets[i]->pins[0].x;
		nets[i]->trunk_end = nets[i]->pins[0].x;
		
		for(int j = 1; j < nets[i]->pins.size(); j++)
		{
			if(nets[i]->pins[j].x < nets[i]->trunk_start)
				nets[i]->trunk_start = nets[i]->pins[j].x;
			if(nets[i]->pins[j].x > nets[i]->trunk_end)
				nets[i]->trunk_end = nets[i]->pins[j].x;				
		}
	}

	//initialize HCG based on overlapping trunks
	for(int i = 0; i < nets.size(); i++)
	{
		nets[i]->track_num = i + 1; //initialize trunk tracks
		for(int j = i+1; j < nets.size(); j++)
		{ //if two nets are in the channel at the same place, add to HCG
			if( ( nets[i]->trunk_end >= nets[j]->trunk_start && 
			      nets[i]->trunk_end <= nets[j]->trunk_end ) ||
			    ( nets[j]->trunk_end >= nets[i]->trunk_start && 
			      nets[j]->trunk_end <= nets[i]->trunk_end ) )
			{
				nets[i]->HCG.push_back(nets[j]);
				nets[j]->HCG.push_back(nets[i]);
			}			
		}
	}	
}


//sort trunks based on VCG
//NEEDS TO HANDLE CYCLES
void rearrangeTrunks(vector<Net*> nets)
{
printf("Start rearrangeTrunks.\n");
	bool more_to_do = true;
	int loop_count = 0;
	while(more_to_do && loop_count++ < nets.size())
	{ //printf("loop_count: %d\n", loop_count++);
		more_to_do = false;
		for(int i = 0; i < nets.size(); i++)
		{			
			for(int k = 0; k < nets[i]->VCG.size(); k++)
			{				
				if(nets[i]->track_num < nets[i]->VCG[k]->track_num)
				{ //swap tracks
					int temp_track = nets[i]->track_num;
					nets[i]->track_num = nets[i]->VCG[k]->track_num;
					nets[i]->VCG[k]->track_num = temp_track;
					more_to_do = true;
					//should exit on cycles
				}
			}			
		}
	}
printf("End rearrangeTrunks.\n");
}

//decrements the track_num for all nets above the target
void removeEmptyTrack(int target, vector<Net*> nets)
{
	//cancel if there are any nets still in the track
	for(int i = 0; i < nets.size(); i++)
	{
		if(nets[i]->track_num == target)
			return;
	}
	
	for(int i = 0; i < nets.size(); i++)
	{
		if(nets[i]->track_num > target)
			nets[i]->track_num--;
	}
	
}

//returns true if there is any net currently sitting in track between start and end
bool isConflict(Net net_to_move,vector<Net*> nets, int track, int start, int end)
{
printf("isConflict on track %d between %d and %d?\n", track, start, end);
	
	for(int i = 0; i < nets.size(); i++)
	{
		if(nets[i]->track_num == track)
			printf("Net %s on track %d from %d to %d.\n", nets[i]->name.c_str(), track, nets[i]->trunk_start, nets[i]->trunk_end);
	}
	
	//check that this move will not violate VCG
	//NOT IMPLEMENTED
	
	for(int i = 0; i < nets.size(); i++)
	{
		if(nets[i]->track_num == track)
		if( (nets[i]->trunk_start <= end && nets[i]->trunk_start >= start) ||
		( nets[i]->trunk_end <= end   && nets[i]->trunk_end >= start) ||
		(nets[i]->trunk_start <= start && nets[i]->trunk_end >= end))	 
		{
			printf("Conflict detected for net %s on track %d.\n", nets[i]->name.c_str(), track);
			return true;			
		}
	}
	printf("No.\n");
	return false; //no conflict was found
}

//when there are unrelated trunks, attempt to collapse them into the same track
void collapseUnrelatedTrunks(vector<Net*> nets)
{
	for(int i = 0; i < nets.size(); i++)
	{
		for(int j = i+1; j < nets.size(); j++)
		{
			if(!nets[i]->isRelatedTo(nets[j]) && 
			!isConflict(nets[i], nets, nets[j]->track_num, nets[i]->trunk_start, nets[i]->trunk_end))
			{
			//if not related, try to make the move
			//only make the move if no VCG is violated
			//only make the move if no HCG is violated
				printf("%s is NOT related to %s.\n", nets[i]->name.c_str(), nets[j]->name.c_str());
				printf("Moving net %s to track: %d\nRemoving track: %d\n", nets[i]->name.c_str(), nets[j]->track_num, nets[i]->track_num);		
				int old_track = nets[i]->track_num;		
				nets[i]->track_num = nets[j]->track_num;
				removeEmptyTrack(old_track, nets);
			}	
		}
	}
}

//prints relevant information about the nets
void printNets(vector<Net*> nets)
{
	printf("\n%d nets:\n", nets.size());
	for(int i = 0; i < nets.size(); i++)
	{
		printf("****Net %s****", nets[i]->name.c_str());
		printf("\n%d pins:\t", nets[i]->pins.size());
		for(int j = 0; j < nets[i]->pins.size(); j++)
			printf("(%d, %d)\t", nets[i]->pins[j].x, nets[i]->pins[j].y);
			
		printf("\nVCG:\t");
		for(int j = 0; j < nets[i]->VCG.size(); j++)
			printf("Net %s\t", nets[i]->VCG[j]->name.c_str());
		
		printf("\nHCG:\t");
		for(int j = 0; j < nets[i]->HCG.size(); j++)
			printf("Net %s\t", nets[i]->HCG[j]->name.c_str());
		
			
		printf("\nTrunk in track %d -- from %d to %d", nets[i]->track_num, nets[i]->trunk_start, nets[i]->trunk_end);
		printf("\n");
	}
}

//performs clean up functions
void finalize(vector<Net*> nets)
{
//after completing the route, set all the top row pins cooridinates based on number of tracks used
	int num_tracks = 0;
	for(int i = 0; i < nets.size(); i++)
	{
		if(nets[i]->track_num > num_tracks) num_tracks = nets[i]->track_num;
	}
	
	for(int i = 0; i < nets.size(); i++)
	{
		for(int j = 0; j < nets[i]->pins.size(); j++)
		{
			if(nets[i]->pins[j].y == 1)
				nets[i]->pins[j].y = num_tracks + 1;
		}
	}

}


void exportNet(Net* net, FILE* output_file)
{
	//fprintf(output_file, "%d\n", net->pins.size() + 1);
	
	//print trunk
	fprintf(output_file, "%d\t%d\t%d\t%d\t%s\n", net->trunk_start, net->track_num, net->trunk_end, net->track_num, net->name.c_str());
	//print branches
	for(int i = 0; i < net->pins.size(); i++)
		fprintf(output_file, "%d\t%d\t%d\t%d\t%s\n", net->pins[i].x, net->pins[i].y, net->pins[i].x, net->track_num, net->name.c_str());
}

	
void exportChannelNets(vector<Net*> nets)
{
	//open export file
	FILE *output_file;
	string path = "./output/channel.route";

	output_file = fopen(path.c_str(), "w");	
	if (output_file == NULL) { fprintf(stderr, "Can't open output_layout file!");	exit(1); }
	
	printf("\nExporting channel route file.\n\n");
	
	//fprintf(output_file, "%d\n", nets.size());
	fprintf(output_file, "x1\ty1\tx2\ty2\tnet\n");
	//print the segements for each net to file
	for(int i = 0; i < nets.size(); i++)
	{
		exportNet(nets[i], output_file);
	}
	
}

void channelRoute(string benchmark_name)
{		
	printf("\n\n****CHANNEL ROUTING****\n\n");		
	
	string benchmark_path = "./channel_benchmarks/" + benchmark_name;
	
	vector<Net*> nets = readInputFile(benchmark_path);
	
	initializeHCG(nets);

	rearrangeTrunks(nets);
	collapseUnrelatedTrunks(nets);
	printNets(nets);				
	
	//if cycles exist, add doglegs
		
	finalize(nets);
	
	exportChannelNets(nets);

	
/*
	for(int i = 0; i < nets.size(); i++)
	{
		for(int j = 0; j < nets.size(); j++)
		{
			if(nets[i]->isAbove(nets[j]))
				printf("net %s is above %s.\n", nets[i]->name.c_str(), nets[j]->name.c_str());
			else printf("net %s is NOT above %s.\n", nets[i]->name.c_str(), nets[j]->name.c_str());
		}
		
	}
*/
	
}

int main(int argc, char *argv[])
{
	string benchmark_name = argv[1];
	channelRoute(benchmark_name);	
	exit(1);
}
