//NEED TO FIX HEADBUTTING AT RIGHT SIDE OF ROUTE
//NNED TO HAVE FANOUT AT END FOR NETS WITH MANY PINS ON RIGHT


//switch_box.cpp
//Mark Sears
//implement 2-layer switchbox routing
//Greedy switch box algorithm described by W.K. Luk

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

class Switchbox
{
	public:
	
	vector<vector<int>> Hsegs; //horizontal segments in the switchbox
	vector<vector<int>> Vsegs; //veritical segments in the switchbox
	vector<vector<bool>> vias; //coordinates for vias in the switchbox
	vector<int> tracks; //the net on a particular track at a particular column
	
	vector<int> top, bot, left, right; //holds the pins on the perimeter of switchbox
		
	vector<int> split_nets; //list of nets that are split
	vector<vector<int>> locked_nets; //list of nets locked in a column
	
	//constructor
	Switchbox(string benchmark_name)// 5, 7
	{
	
		readInput(benchmark_name);
				
		int height = left.size();
		int width = top.size();
		
		for(int i = 0; i < height; i++)
		{
			vector<int> H_row(width+1, 0);
			Hsegs.push_back(H_row);
		}
		
		for(int i = 0; i < height+1; i++)
		{	
			vector<int> V_row(width, 0);
			Vsegs.push_back(V_row);			
		}
		
		for(int i = 0; i < height; i++)
		{
			vector<bool> via_row(width, false);
			vias.push_back(via_row);
		}
				
		tracks = vector<int>(height, 0);
				
		//initialize split_nets
		for(int i = 0; i < top.size(); i++)
		{
			vector<int> temp(1, 0);
			locked_nets.push_back(temp);
			
			while(top[i] >= split_nets.size() && bot[i] >= split_nets.size())
				split_nets.push_back(false); 
		}
		
		for(int i = 0; i < left.size(); i++)
		{
			while(left[i] >= split_nets.size() && right[i] >= split_nets.size())
				split_nets.push_back(false); 
		}
		
		printSplitNets();
	}
	
	void printSplitNets()
	{
		printf("#####SPLIT NETS: \t");
		for(int i = 0; i < split_nets.size(); i++)
		{
			if(split_nets[i])
			printf("%d ", i);
		}
		printf("\n");
	}
	
	
	void printSwitchbox()
	{
		printf("\n************Switchbox************\n");		
	/*
		printf("Hsegs:\n");
		for(int i = 0; i < Hsegs.size(); i++)
			for(int j = 0; j < Hsegs[i].size(); j++)
				printf("%d ", Hsegs[i][j]);
	*/
		printf("   ");
		for(int i = 0; i < top.size(); i++)
			printf("  %2d", top[i]);
		printf("\n   ");
		
		for(int i = 0; i < top.size()*2+1; i++)
			printf("--");
		printf("\n");
		
		for(int i = 0; i < Vsegs.size()-1; i++)
		{
			printf("  |");
			for(int j = 0; j < Vsegs[i].size(); j++)
				if(Vsegs[i][j] != 0)
					printf("  %2d", Vsegs[i][j]);
				else printf("    ");
			printf("  |\n");
				
			printf("%2d|", left[i]);			
			for(int j = 0; j < Hsegs[i].size(); j++)
			{
				if(Hsegs[i][j] != 0)
					printf("%2d", Hsegs[i][j]);
				else printf("  ");
				if(j != Hsegs[i].size()-1)
					if(vias[i][j])
						printf(" *");
					else printf(" +");
			}
			printf("|%2d\n", right[i]);
		}
		
		//print the last line of Vsegs
		printf("  |");
		int size = Vsegs.size()-1;
		for(int j = 0; j < Vsegs[size].size(); j++)
			if(Vsegs[size][j] != 0)
				printf("  %2d", Vsegs[size][j]);
			else printf("    ");
		printf("  |\n");
		
		printf("   ");
		for(int i = 0; i < 2*bot.size()+1; i++)
			printf("--");
		printf("\n   ");	
		for(int i = 0; i < bot.size(); i++)
			printf("  %2d", bot[i]);
					
		
		printf("\n");
	}
	
	//input benchmark format:
	//6
	//1 0 2 0 3 0 //top
	//3 0 1 0 2 0 //bot
	//5
	//3 1 2 0 3 //left
	//0 1 3 0 2 //right
	void readInput(string benchmark_name)
	{
		FILE *input_file; 
		input_file = fopen(benchmark_name.c_str(), "r");	
	if (input_file == NULL) { fprintf(stderr, "Can't open input file: %s!\n", benchmark_name.c_str()); exit(1); }
	
	
	int top_size;
	fscanf(input_file, "%d\n", &top_size);
	
	int next;
	
	for(int i = 0; i < top_size; i++)
	{
		fscanf(input_file, "%d ", &next);
		top.push_back(next);
	}
	fscanf(input_file, "\n");
	
	
	for(int i = 0; i < top_size; i++)
	{
		fscanf(input_file, "%d ", &next);
		bot.push_back(next);
	}
	fscanf(input_file, "\n");
	
	
	int left_size;
	fscanf(input_file, "%d\n", &left_size);
	for(int i = 0; i < left_size; i++)
	{
		fscanf(input_file, "%d ", &next);
		left.push_back(next);
	}
	fscanf(input_file, "\n");
	
	for(int i = 0; i < left_size; i++)
	{
		fscanf(input_file, "%d ", &next);
		right.push_back(next);
	}
	fscanf(input_file, "\n");
	
		printf("Done reading input!\n");				
	
	}
	
//****SWITCHBOX HELPER FUNCTIONS
	
	//returns the target row for the given net at a given col.
	//usually is the row for that net with a right pin,
	//or else is the row currently active
	//or else is an empty  row
	int getTarget(int net, int col)
	{		
		//look for a right pin for this net
		for(int i = 0; i < right.size(); i++)
		{
			if(right[i] == net)
				return i;
		}
		
		//look for an active Hseg for this net
		for(int i = 0; i < Hsegs.size(); i++)
		{
			if(tracks[i] == net)
				return i;
		}
		
		//look for an empty row for this net
		for(int i = 0; i < Hsegs.size(); i++)
		{
			if(tracks[i] == 0)
				return i;
		}
		
		
		return -1; //failed to find a target, may need to create new row
	}
	
	//add a Hsegment, but only if it is allowed
	//return true if a segment was added
	bool addHseg(int net, int row, int col)
	{
		if(col > Hsegs[row].size()-1 || col < 0) return false;
		if(row > Hsegs.size()-1 || row < 0) return false;
		
		//check current spot is vacant
		if(Hsegs[row][col] != 0) return false;
		
		//check if there is empty or same net to the right
		if(col != 0)		
		  if(Hsegs[row][col-1] != 0 && Hsegs[row][col-1] != net)
		    return false;
		    
		//check if there is empty or same net to the left
		if(col != Hsegs[row].size()-1)		
		  if(Hsegs[row][col+1] != 0 && Hsegs[row][col+1] != net)
		    return false;
		
		//if no reason it can't be added, then add it!		
		Hsegs[row][col] = net;
		    
		return true;
	}
	
	//add a Vsegment, but only if it is allowed
	//return true if a segment was added
	bool addVseg(int net, int row, int col)
	{	
		if(col > Vsegs[row].size()-1 || col < 0) return false;
		if(row > Vsegs.size()-1 || row < 0) return false;
		
		//check current spot is vacant
		if(Vsegs[row][col] != 0)  
		{ 
			printf("Cannot add Vseg at (%d, %d) for net %d due to already existing segment: net %d\n", row, col, net, Vsegs[row][col]);
		    return false;
		}
		
		//check if there is empty or same net above
		if(row != 0)		
		  if(Vsegs[row-1][col] != 0 && Vsegs[row-1][col] != net)
		    { printf("Cannot add Vseg at (%d, %d) for net %d due to what's above: net %d\n", row, col, net, Vsegs[row-1][col]);
		    return false;
		}
		    
		//check if there is empty or same net below
		if(row != Vsegs.size()-1)		
		  if(Vsegs[row+1][col] != 0 && Vsegs[row+1][col] != net)
		    { printf("Cannot add Vseg for net %d due to what's below: net %d\n", net, Vsegs[row+1][col]);
		    return false;
		}
		//if no reason it can't be added, then add it!
		Vsegs[row][col] = net;
		printf("Added Vseg on net %d at (%d, %d).\n", net, row, col);
		return true;
	}
	
	bool removeHseg(int row, int col)
	{
		printf("Removing Hseg on net %d at (%d, %d).\n", Hsegs[row][col], row, col);
		Hsegs[row][col] = 0;
		return true;
	}
	
	bool removeVseg(int row, int col)
	{
		printf("Removing Vseg on net %d at (%d, %d).\n", Vsegs[row][col], row, col);
		Vsegs[row][col] = 0;
		return true;
	}
	
	//when routing can't be completed, may need to increase switchbox size
	void addCol()
	{
	
		printf("&&&&&&ADDCOL()&&&&&&\n");
		Vsegs[0].push_back(0);
		for(int i = 0; i < Hsegs.size(); i++)
		{
			Hsegs[i].push_back(0);
			Vsegs[i+1].push_back(0);
			vias[i].push_back(false);
		}
		
		top.push_back(0);
		bot.push_back(0);
		
		vector<int> temp(1, 0);
		locked_nets.push_back(temp);
	}
	
	void addRowBot(int stop_col)
	{
		printf("&&&&&&ADDROWBOT()&&&&&&\n");
		vector<int> H_row(Hsegs[0].size(), 0);
		Hsegs.push_back(H_row);
		
		vector<int> V_row(Vsegs[0].size(), 0);		
		Vsegs.push_back(V_row);
		
		vector<bool> via_row(vias[0].size(), false);
		vias.push_back(via_row);
		
		left.push_back(0);
		right.push_back(0);
		tracks.push_back(0);
		
		//update previous columns to keep nets connected to pins
		for(int i = 0; i <= stop_col; i++)
			Vsegs[Vsegs.size()-1][i] = bot[i];		
	}
	
	void addRowTop(int stop_col)
	{
		printf("&&&&&&ADDROWTOP()&&&&&&\n");
		vector<int> H_row(Hsegs[0].size(), 0);
		Hsegs.insert(Hsegs.begin(), H_row);
		
		vector<int> V_row(Vsegs[0].size(), 0);		
		Vsegs.insert(Vsegs.begin(), V_row);
		
		vector<bool> via_row(vias[0].size(), false);
		vias.insert(vias.begin(), via_row);
		
		left.insert(left.begin(), 0);
		right.insert(right.begin(), 0);
		tracks.insert(tracks.begin(), 0);
		
		//update previous columns to keep nets connected to pins
		for(int i = 0; i <= stop_col; i++)
			Vsegs[0][i] = top[i];
	}
	
	//returns true if the net is done routing
	//false if there is more to route
	bool netIsComplete(int net, int col)
	{
		//if there are more than one track of this net, it is not complete
		int track_count = 0;
		for(int i = 0; i < tracks.size(); i++)
			if(tracks[i] == net)
				track_count++;
		if(track_count > 1)	
			return false;
				
		//if there is a right pin, the net is not complete
		for(int i = 0; i < right.size(); i++)
			if(right[i] == net)
				return false;
		
		//if there is another pin on top or bot, net is not complete
		for(int i = col; i < top.size(); i++)
		{
			if(top[i] == net)
				return false;
				
			if(bot[i] == net)
				return false;
		}
		
		
		//else net IS complete, remove it from tracks
		
		for(int i = 0; i < tracks.size(); i++)
			if(tracks[i] == net)
				tracks[i] = 0;
		
		return true;
	}
	
	//returns true if the net is done routing other than the right side
	//false if there are more pins to come in
	bool netIsCompleteExceptRight(int net, int col)
	{
		//if there are more than one track of this net, it is not complete
		if(split_nets[net])
			return false;
				
		
		//if there is another pin on top or bot, net is not complete
		for(int i = col+1; i < top.size(); i++)
		{
			if(top[i] == net)
				return false;
				
			if(bot[i] == net)
				return false;
		}
				
		return true;
	}
	
	bool canMoveToTrack(int start, int end, int col)
	{
		if(start == end) return true; //no move to be made!
		
		int net = tracks[start];
				
		if(Hsegs[end][col] != 0 && Hsegs[end][col] != net)
			return false; //some Hseg is blocking
				
		bool can_move = true;
						
		bool down = true; //going down!
		if(end < start)
			down = false; //going up!
		
		
		
		//check for Vsegs blocking the path			
		for(int j = start + (down ? 1 : 0); j != end + (down ? 1 : -1); j += (down ? 1 : -1) )
		{
			if(Vsegs[j][col] != 0 && Vsegs[j][col] != net)
				can_move = false;
		}
		
		return can_move;
		
	}
	
	//moves a track from start to end, or at least as far as possible.
	//return true if able to move to the target track
	//false if it failed
	bool moveTrack(int start, int end, int col)
	{	
		int net = tracks[start];
		
		printf("moveTrack(): tracks[start] = %d \t right[start] = %d\n", tracks[start], right[start]);
		if(right[start] == net && right[start] != right[end]) return false;
		if(start == end) return true; //no move to be made!
		
		if(col > 0)
		for(int i = 1; i < locked_nets[col-1].size(); i++)
			if(locked_nets[col-1][i] == net)
			{
			printf("LOCKED NET %d. Canceling move....\n", net);
				return false;
			}
		
		
		
		bool successful_move = true;
		
		
		
		bool down = true; //going down!
		if(end < start)
			down = false; //going up!
		
	printf("*****Performing moveTrack()! net %d from track %d to %d.\n", net, start, end);
								
		for(int j = start + (down ? 1 : 0); j != end + (down ? 1 : -1); j += (down ? 1 : -1) )
		{		
			if(!addVseg(net, j, col)) 
			{ //if can't add, end early				
				//go backwards until find an unoccupied track
				successful_move = false;
				while(tracks[j - (down ? 1 : 0)] != 0 && tracks[j - (down ? 1 : 0)] != net)
				{printf("BACKING UP in moveTrack()\n");					
					j += (down ? -1 : 1);
					removeVseg(j, col);					
				}
				
				tracks[start] = 0;
				tracks[j - (down ? 1 : 0)] = net;
			printf("SET TRACK: net %d from track %d to track %d.\n", net, start, j-1);
				break;
			}
			
			//if encounter another track of the same net in the middle, remove that from active tracks
			if(tracks[j] == net)
				tracks[j] = 0;
			
			if( (down && j == end) || (!down && j == end+1) )
			{
				tracks[start] = 0;
				while(tracks[j + (down ? 0 : -1)] != 0 && tracks[j + (down ? 0 : -1)] != net)
				{printf("BACKING UP in moveTrack()\n");					
					successful_move = false;
					removeVseg(j, col);
					j += (down ? -1 : 1);					
				}
				setTrack(net, getTarget(net, col), col);
				break;
			}
		}
		
		printf("*****End of moveTrack()! net %d from track %d to %d.\n", net, start, end);
		
		return successful_move;
	}
	
	void printTracks()
	{
	
		//print tracks info
		printf("\n");
		for(int i = 0; i < tracks.size(); i++)
			printf("tracks[%d] = %d\n", i, tracks[i]);
		printf("\n");
	}
	
	//returns the number of tracks that are currently running the net
	int getNumActiveTracks(int net)
	{
		int num = 0;
		
		for(int i = 0; i < tracks.size(); i++)
		{
			if(tracks[i] == net)
				num++;
		}
		
		return num;	
	}
	
//****SWITCHBOX ALGORITHM FUNCTIONS
	
	// (SW0) determine scan diretion
	//start the routing scan on the left. All left pins must enter with an Hseg
	void enterLeft()
	{	
		//enter from left	
		for(int i = 0; i < left.size(); i++)
		{
			addHseg(left[i], i, 0);//Hsegs[i][0] = left[i];
			tracks[i] = left[i]; //ensure tracks is correct
			
			//update split nets based on left pins
			for(int j = 0; j < i; j++)
				if(left[i] != 0)
				if(left[i] == left[j])
				{ split_nets[left[i]] = true; printf("enterLeft split net: %d\n", left[i]); }
		}
		
		/*	
		//enter from right	
		for(int i = 0; i < right.size(); i++)
			addHseg(right[i], i, Hsegs[i].size()-1);//Hsegs[i][Hsegs[i].size()-1] = right[i];
			
		//enter from top	
		for(int i = 0; i < top.size(); i++)
			addVseg(top[i], 0, i);//Vsegs[0][i] = top[i];
			
		//enter from bot	
		for(int i = 0; i < bot.size(); i++)
			addVseg(bot[i], Vsegs.size()-1, i);//Vsegs[Vsegs.size()-1][i] = bot[i];
			*/
	}
	
	
	// (SW1) if empty track exists (or matching net, bring in top[i] and bot[i] to empty rows
	void bringInPins(int col)
	{ 		
	printf("\n***************Start: bringInPins(%d)\n", col);
	
	int top_target = -1;
	int bot_target = -1;
	
	int net = top[col];
		//try to bring in pin from top 
		if(net != 0)
		{
			
			
			//try to match to existing track or empty track
			for(int i = 0; i < Hsegs.size(); i++)
				if(Hsegs[i][col] == net || Hsegs[i][col] == 0)
				{
					top_target = i;					
					break;
				}
						
			//if still no target track found, cannot bring the pin in!...must add new row
			if(top_target == -1)
			{
				printf("CANNOT BRING IN PIN: col = %d\tAdding new row on top...\n", col);
				addRowTop(col);
				top_target = 0;
				
			}
			
			tracks[top_target] = net;
			
			if(Hsegs[top_target][col] != net && getNumActiveTracks(net) > 1)
				split_nets[net] = true;
			
			//draw Vsegs down to try to get to target
			for(int i = 0; i <= top_target; i++)
			{ //can only add Vseg if it doesn't touch another net				
				addVseg(net, i, col);					
			}
		}
		
		int bot_net = bot[col];
		
		//try to bring in pin from bot
		if(bot_net != 0)
		{			
			
			//try to match to existing track or empty track
			for(int i = Hsegs.size()-1; i >= 0; i--)
				if(Hsegs[i][col] == bot_net || Hsegs[i][col] == 0)
				//if(i != top_target)
				{
					bot_target = i;				
					break;
				}
			//if still no target track found, cannot bring the pin in!...must add new row
			if(bot_target == -1 || bot_target == top_target) 
			//if(bot_target == -1)
			{
				printf("CANNOT BRING IN PIN: col = %d\tAdding new row on bot...\n", col);
				addRowBot(col);
				bot_target = Hsegs.size()-1;				
			}
			
			//check if there are no other vertical tracks in the way.
			for(int i = Hsegs.size()-1; i > bot_target; i--)
			{
				//if there are, add another row
				if(Vsegs[i][col] != 0) 
				{
					printf("CANNOT BRING IN PIN (vertical segment in the way!): col = %d\tAdding new row on bot...\n", col);
					addRowBot(col);
					bot_target = Hsegs.size()-1;					
				}
			}
			
			tracks[bot_target] = bot_net;
			
			if(Hsegs[bot_target][col] != bot_net && getNumActiveTracks(bot_net) > 1)
				split_nets[bot_net] = true;
				
			//draw Vsegs up try to get to target
			for(int i = Vsegs.size()-1; i > bot_target; i--)
			{ //can only add Vseg if it doesn't touch another net					
				addVseg(bot[col], i, col);				
			}
		}
		
		printf("***************End: bringInPins(%d)\n", col);
	}
	
	void joinSplitNets(int col)
	{
	printf("***************Start: joinSplitNets(%d)\n", col);
		//count how many of each net are currently active
		vector<int> active_nets;
		vector<int> net_count;
					
		
		for(int i = 0; i < tracks.size(); i++)
		{
			bool found = false;
			for(int j = 0; j < active_nets.size(); j++)
			{
				if(tracks[i] == active_nets[j]) //already exists in the list
				{
					net_count[j]++;
					found = true;	
				}
			}
			
			//not already in list, then
			if(!found && tracks[i] != 0)
			{
				active_nets.push_back(tracks[i]);				
				net_count.push_back(1);
			}
		}
		
		printf("\nActive nets at col %d:\n", col);
		for(int j = 0; j < active_nets.size(); j++)
		{
			printf("net %d, count: %d\n", active_nets[j], net_count[j]);
		}
		
		int max_active_tracks = 0; //net number with the most active tracks at this column		
		
		//determine priority for joining nets
		
		//join the nets as best as possible
		
		for(int i = 0; i < active_nets.size(); i++)
		{
			if(net_count[i] > 1)
				joinNet(active_nets[i], col);
		}
	printf("***************End: joinSplitNets(%d)\n", col);
	}
	
	//attempt to set the net in the target track.
	//if target_track is occupied, place it in closest valid track
	bool setTrack(int net,  int new_track, int col)
	{		
				
		printf("SET TRACK() -- net %d to track %d\n", net, new_track);
		int modifier = -1;
		bool edge_hit = false; //true if the search has hit either top or bottom
		
		//try to find the closest available track to the target new_track
		while(1)
		{		
			if( ( tracks[new_track] == 0 || tracks[new_track] ==  net ) && 
			( Hsegs[new_track][col] == net || (Vsegs[new_track][col] == net || Vsegs[new_track+1][col] == net) ) )
			{
				tracks[new_track] = net;
				printf("Set net %d to track %d\n", net, new_track);
				return true;
			}
			new_track += modifier;
			if(!edge_hit)
			{				
				if(new_track >= 0 && new_track < Hsegs.size())
				{
					if(modifier < 0)
						modifier -= 1;
					else modifier += 1;			
				
					modifier *= -1;
				}
				
				if(new_track < 0)
				{
					new_track = 0;
					modifier = 1;
					edge_hit = true;
				}
				
				if(new_track >= Hsegs.size())
				{
					new_track = Hsegs.size()-1;
					modifier = -1;
					edge_hit = true;
				}
				
				
			}			
		}
		printf("!!!!!!!!!!!!!!!!Cannot set net %d to track %d. No empty tracks!\n", net, new_track);
				
		return false;
	}
	
	
	int getClosestEmptyTrack(int net,  int target_track, int col)
	{		
				
		printf("Begin getClosestEmptyTrack() net: %d\t target_track: %d\n", net, target_track);
		int modifier = -1;
		bool edge_hit = false; //true if the search has hit either top or bottom
		int new_track = target_track;
		//try to find the closest available track to the target new_track
		int attempts = 0;
		while(attempts < tracks.size())
		{		
			if( ( tracks[new_track] == 0 || tracks[new_track] == net ) &&
			( (Hsegs[new_track][col] == net || Hsegs[new_track][col] == 0) ||
			  (Vsegs[new_track][col] == net || Vsegs[new_track+1][col] == net) ) )
			{				
				//printf("Set net %d to track %d\n", net, new_track);
				if(tracks[new_track] != net)
				return new_track;
			}
			new_track += modifier;
			if(!edge_hit)
			{				
				if(new_track >= 0 && new_track < Hsegs.size())
				{
					if(modifier < 0)
						modifier -= 1;
					else modifier += 1;			
				
					modifier *= -1;
				}
				
				if(new_track < 0)
				{
					new_track = 0;
					modifier = 1;
					edge_hit = true;
				}
				
				if(new_track >= Hsegs.size())
				{
					new_track = Hsegs.size()-1;
					modifier = -1;
					edge_hit = true;
				}
				
				
			}
			attempts++;			
		}	
		
		//no other row available...make a new one!
		//if in upper half
		if(target_track < tracks.size()/2)
		{
			addRowTop(col);
			return 0;
		}
		else
		{
			addRowBot(col);
			return tracks.size()-1;
		}
	}
	
	//after selecting which net to join, this function joinNet()
	//is called to actually perform the join (if possible)
	//returns true if the join was made, else false
	bool joinNet(int net, int col)
	{
		if(net == 0) return false;
		
		//if the net is not split, just end
		if(!split_nets[net]) { printf("CANCEL joinNet(net %d) because already joined!\n", net); return false; }
		
		int start_track = -1, end_track = -1;
		
		//look for the net at the column
		for(int i = 0; i < Hsegs.size(); i++)
		{
			if(tracks[i] == net) //found a track to join!
			{
				start_track = i;
				break;
			}
		}
		
		for(int i = Hsegs.size()-1; i >= 0; i--)
		{
			if(tracks[i] == net) //found a track to join!
			{
				end_track = i;
				break;
			}
		}
		
		
		printf("Performing joinNet(net: %d, col: %d) start_track = %d\tend_track = %d\n", net, col,  start_track, end_track);
		
		
		//check to see how far the net can be joined
		
		bool join_successful = moveTrack(start_track, end_track, col);
		printf("join_successful: %d\n", join_successful);
		if(join_successful)
			split_nets[net] = false;
		else	{ split_nets[net] = true;	printf("Join failed! Net %d is still split!\n", net); }
		
		return join_successful;
	}
	
	//for all the tracks at col, attempt to move nets closer to their final target track
	void jogToTargets(int col)
	{
		printf("***************Start jogToTargets()\n");
		
		//look for jogs that can be made
		for(int i = 0; i < tracks.size(); i++)
		{
			int net = tracks[i];
			if(net != 0)
			if(net != right[i])
			{
			
				int target = getTarget(net, col);
								
				if(conflictCycleExists(i, col))
				{printf("!!!?!?!?!? CONFLICT CYCLE EXISTS! net: %d track: %d\n", tracks[i], i);
					target = getClosestEmptyTrack(net,  getTarget(net, col), col);
					if(moveTrack(i, target, col))
					{
						setTrack(net, target, col);
						tracks[i] = 0;
						locked_nets[col].push_back(net);
					}
				}
				else if(split_nets[net] || tracks[target] != net)
					moveTrack(i, target, col);						
			}
		}		
		printf("***************End jogToTargets()\n");
	}
	
	
	//returns true if a conflict cycle exists
	//e.g. there is a group of tracks that are all in each other's way
	bool conflictCycleExists(int original_track, int col)
	{
		int current_track = original_track;
		
		int attempts = 0;
		printf("BEGIN conflictCycleExists(): original_track: %d original_net: %d\n", original_track, tracks[original_track]);
		while(attempts < tracks.size())
		{
			int next_track = getTarget(tracks[current_track], col);
			printf("next_track: %d (net %d)\n", next_track, tracks[next_track]);
			
			//if found an empty track, not a conflict cycle
			if(tracks[next_track] == 0 || next_track == current_track) 
				return false;
				
				
			//if(tracks[next_track] == tracks[original_track]) //if found the original, it's a conflict cycle!
			if(next_track == original_track)
				return true;
				
			
			current_track = next_track;
			attempts++;
		}	
		printf("conflictCycleExists() FAILED!\n");
		//exit(1);
		return false;
	}
	
	
	//check if it is time to start fanning out to multiple pins on the right side
	//if #remaining columns < #nets to fannout
	//then start fanning out
	//if fanout too late, unneeded columns will be added for no reason
	//if fanout too soon, unneeded tracks will be occupied
	bool checkForFanout(int col)
	{
	printf("CHECK FOR FANOUT: \n");
		int columns_left = Vsegs.size()-col;
		int num_nets_to_fannout = 0;

		for(int i = 0; i < right.size(); i++)
		{
			if(right[i] != 0)
			for(int j = i+1; j < right.size(); j++)
			{				
				if(right[i] == right[j])
					num_nets_to_fannout++;				
			}
		}
		
		if(columns_left < num_nets_to_fannout)
		{			
			fanoutNets(col);
			return true;
		}
		
		printf("Don't fanout yet.\n");
		return false;
	}
	
	
	//for nets with more than 1 right pin, attempt to fan them out
	void fanoutNets(int col)
	{
	printf("BEGIN: fanoutNets()\n");
		//setTrack(int net,  int new_track, int col)		
		
		for(int i = 0; i < right.size(); i++)
		{
			if(right[i] != 0)
			for(int j = i+1; j < right.size(); j++)
			{
				if(right[i] == right[j] && tracks[i] != tracks[j])
				{printf("Need to fanout net %d at col %d\t tracks[%d]: %d\t tracks[%d]: %d\n", right[i], col, i, tracks[i], j, tracks[j]);
					if(netIsCompleteExceptRight(right[i], col))
					{ printf("net %d is complete except for right pins.\n", right[i]);
						int start = i;
						int end = j;
						if(tracks[i] != right[i])
						{
							start = j;
							end = i;
						}
						
						if(canMoveToTrack(start, end, col))					
						{
							printf("Can move to track: %d to %d.\n", start, end);
							moveTrack(start, end, col);					
						} else printf("Can NOT move to track: %d to %d.\n", start, end);
					}
				}
			}
		}
		
		//try to set all tracks to the right pin if possible
		
		for(int i = 0; i < right.size(); i++)
		{
			if(right[i] != 0)
			if(Hsegs[i][col] == 0 || Hsegs[i][col] == right[i])
			if(right[i] == Vsegs[i][col] ||right[i] == Vsegs[i+1][col])
				{ tracks[i] = right[i]; printf("$$$$Keeping net %d on track %d.\n", right[i], i); }
		}
	}
	
	//looks through the newly made column for places where a via should go!
	//vias should be added whenever a Vseg and Hseg of the same net touch
	int placeVias()
	{
		int num_vias = 0;
		for(int col = 0; col < Hsegs[0].size()-1; col++)
		{
			for(int row = 0; row < Vsegs.size()-1; row++)
			{
				if(Vsegs[row][col] == 0 && Hsegs[row][col] == 0 &&
				   Vsegs[row+1][col] == 0 && Hsegs[row][col+1] == 0 )
					continue;
					
				if(Vsegs[row][col] == Hsegs[row][col] || Vsegs[row][col] == Hsegs[row][col+1] || 					Vsegs[row+1][col] == Hsegs[row][col] || Vsegs[row+1][col] == Hsegs[row][col+1])
				{
					vias[row][col] = true;
					num_vias++;
				}
			}
		}
		return num_vias;
	}
	
	
	//after joining, move the trunks forward to continue through the switchbox
	void moveTrunksForward(int col)
	{
		
		for(int i = 0; i < tracks.size(); i++)
		{
			if(!netIsComplete(tracks[i], col+1))
			   addHseg(tracks[i], i, col+1);
		}
	}
		
	//exports the switchbox to a .csv file
	void exportSwitchbox(string output_file_name)
	{
		printf("\n***************EXPORTING SWITCHBOX ROUTE TO: %s***************\n", output_file_name.c_str());
		FILE* out_file = fopen(output_file_name.c_str(), "w");
		fprintf(out_file, "x1,y1,x2,y2,net\n");
	
		//export Hsegs
		for(int i = 0; i < Hsegs.size(); i++)
			for(int j = 0; j < Hsegs[i].size(); j++)
				if(Hsegs[i][j] != 0) //if a segment is actually there
				fprintf(out_file, "%d,%d,%d,%d,%d\n", j, i+1, j+1, i+1, Hsegs[i][j]);
		
		//export Vsegs
		for(int i = 0; i < Vsegs.size(); i++)
			for(int j = 0; j < Vsegs[i].size(); j++)
				if(Vsegs[i][j] != 0) //if a segment is actually there
				fprintf(out_file, "%d,%d,%d,%d,%d\n", j+1, i, j+1, i+1, Vsegs[i][j]);
		
		//export vias
		for(int i = 0; i < vias.size(); i++)
			for(int j = 0; j < vias[i].size(); j++)
			if(vias[i][j])
			fprintf(out_file, "%d,%d,%d,%d,%d\n", -1, -1, j+1, i+1);
		
		fclose(out_file);
	}
	
	//returns true if done routing, i.e. all tracks have the same net as the right pin, and no nets are split
	bool doneRouting(int col)
	{	
		//if not on the last column, not done routing
		if(col != Vsegs[0].size() - 1)
			return false;
		//if not all tracks match the right pins, not done routing
		for(int i = 0; i < tracks.size(); i++)
		{
			if(tracks[i] != right[i])
				return false;						
		}
		
		//if any nets are still split, not done routing
		/*
		for(int i = 0; i < split_nets.size(); i++)
		{
			if(split_nets[i] == true)
				return false;
		}
		*/
		
		return true;
	}
	
	//return the total number of wire segments in the switchbox
	int getTotalWirelength()
	{
		int length = 0;
		
		for(int i = 0; i < Hsegs.size(); i++)
			for(int j = 0; j < Hsegs[i].size(); j++)
				if(Hsegs[i][j] != 0)
					length++;
					
		for(int i = 0; i < Vsegs.size(); i++)
			for(int j = 0; j < Vsegs[i].size(); j++)
				if(Vsegs[i][j] != 0)
					length++;
					
		return length;
	}
	
}; //end class Switchbox







bool switchBoxRoute(string benchmark_name)
{
	//int height = 5, width = 7;
	
	Switchbox sb(benchmark_name);
		
	//sb.readInput(benchmark_name);
	//sb.top 	= { 1, 0, 2, 0, 3, 0, 0};
	//sb.bot 	= { 3, 0, 1, 0, 2, 0, 0};
	//sb.left = { 3, 1, 2, 0, 3};
	//sb.right= { 0, 1, 3, 1, 2};
	
	
	//BEGIN ALGORITHM
		
	// (SW0) determine scan diretion
	sb.enterLeft();
	sb.printSwitchbox();
	for(int col = 0; col < sb.Vsegs[0].size(); col++)
	//for(int col = 0; col < 1; col++)
	{
		printf("\n********BEGIN ROUTING COL %d********\n", col);
		sb.printSplitNets();
		// (SW1) if empty track exists, bring in top[i] and bot[i] to empty rows
		
		sb.bringInPins(col);
		sb.printSwitchbox();
	
		// (SW2) join split nets as much as possible
		sb.joinSplitNets(col);
		
		// (SW3a) for nets with no right terminals, bring split nets closer by jogging 
		
		// (SW3b) for nets with right terminals, do SWJOG
		sb.printTracks();
		sb.jogToTargets(col);
		
		// (SW4) when close to right edge, fanout to target right terminals
		sb.checkForFanout(col);
				
		// (SW5) if SW1 failed, increase number of rows and repeat SW1
		
		
		sb.moveTrunksForward(col);
		sb.printSwitchbox();
		sb.printTracks();
		if(col == sb.Vsegs[0].size() - 1)
		if(!sb.doneRouting(col))
		{
		printf("Split nets still exist! Adding column...");
			sb.addCol();
		}
	}
	
		// (SW6) while split nets still exist, increase number of cols and join split nets
	
		
	int num_vias = sb.placeVias();		
	
	sb.printSwitchbox();
	
	printf("\nROUTING QUALITY INFORMATION\n");
	printf("Number of rows: %d\n", sb.Hsegs.size());
	printf("Number of cols: %d\n", sb.Vsegs[0].size());
	printf("Number of vias: %d\n", num_vias);
	printf("Total wireleng: %d\n", sb.getTotalWirelength());
	
	string output_file_name = "./output/switchbox.sb";
			
	sb.exportSwitchbox(output_file_name);
			
		
	return true; //route successful
}

int main(int argc, char *argv[])
{
	string benchmark_name = argv[1];
	switchBoxRoute(benchmark_name);	
	exit(1);
}


