
//switch_box.cpp
//Mark Sears
//implement 2-layer switchbox routing
//Greedy switch box algorithm using conflict resolution
//Step 1: route all nets with minimum wire
//Step 2: resolve conflicts 


#include "switch_box_conflicts.h"

bool printing = false;

//Segment represents a single piece of wire
class Segment
{
	public:
	
	int x1, y1, x2, y2;
	Net* last_move_up; 
	Net* last_move_down;
	Net* last_move_left;
	Net* last_move_right; //help prevent repeating the same move
	
	vector<Net*> nets; //list of all nets trying to occupy this segment
			//when routing is complete, there should only be 1 net per segment
			
	//constructor
	Segment(int a, int b, int c, int d)
	{
		x1 = a;
		y1 = b;
		x2 = c;
		y2 = d;				
	}
	
	Net* lastMoveInDirection(bool even, bool invert)
	{
		if(even)
		{
			if(invert)
				return last_move_up;
			else 	return last_move_down;		
		}
		else
		{
			if(invert)
				return last_move_left;
			else 	return last_move_right;
		}
	}
	
	void addNet(Net* net)
	{ 	if(printing) printf("addNet(net %d) to segment (r%d, c%d) to (r%d, c%d)\n", net->net_id, y1, x1, y2, x2);			
		
		bool a = false;
		bool b = false;	
		//if already have this net, return
		for(int i = 0; i < nets.size(); i++)
		{
			if(nets[i]->net_id == net->net_id)
			{
				if(printing) printf("Net already exists on segment! Returning...\n");
				a = true;
			}
		}
		
		for(int i = 0; i < net->segs.size(); i++)
		{
			if(net->segs[i] == this)
			{
				if(printing) printf("Net already exists on segment! Returning...\n");
				b = true;
			}
		}
		
		if(!a)
		this->nets.push_back(net);
		//nets.insert(nets.begin(), net);
		
		//add this segment to the net
		if(!b)
		net->segs.push_back(this);
		//net->segs.insert(net->segs.begin(), this);
		
		return;
	}
	
	bool removeNet(Net* net)
	{
	//printf("Begin removeNet(%d)\n", net->net_id);
		for(int i = 0; i < nets.size(); i++)
			if(nets[i] == net)
			{
				this->nets.erase(nets.begin()+i);
				if(printing) printf("Removed net %d from seg (r%d, c%d)\n", net->net_id, this->y1, this->x1);
				
				//remove this segment from the net
				for(int j = 0; j < net->segs.size(); j++)
					if(net->segs[j] == this)
					{	net->segs.erase(net->segs.begin()+j);
					if(printing) printf("Removed seg (r%d, c%d) from net %d\n", this->y1, this->x1, net->net_id);
					}
				return true;
			}
									
		return false;
	}
	
	bool hasNet(Net* net)
	{
		for(int i = 0; i < nets.size(); i++)
			if(nets[i] == net)
				return true;
				
		return false;		
	}
	
	bool isVertical()
	{
		return x1 == x2;
	}
	
	//returns true if the segment is occupied by anything except for net
	bool isOccupied(Net* net)
	{
		bool occupied = false;
		
		if(nets.size() > 0)		
			if(nets[0] != net)
				occupied = true;		
		
		return occupied;
	}
	
	
	//returns true if this segment is a nub for the net
	bool isNub(Net* net, Switchbox* sb)
	{
		if(printing) printf("Begin isNub(net %d): (%d, %d)(%d, %d) net->segs.size() = %d\n", net->net_id, x1, y1, x2, y2, net->segs.size());
		bool connected_on_node1 = false;
		bool connected_on_node2 = false;
		
		//if this segment is on the border of the switchbox, cannot be a nub
		if(this->isVertical())
		{
			if(this->y1 == 0 || this->y1 == sb->left.size())
				return false;
		}
		else
		{
			if(this->x1 == 0 || this->x1 == sb->top.size())
				return false;
		}		
		
		//check if this segment is connected to any other segment on it's first node (x1, y1)
		for(int i = 0; i < net->segs.size(); i++)
		{
			Segment* seg2 = net->segs[i];
												
			if(seg2 == this)
				continue;
			
			//if(printing) printf("seg2: (r%d, c%d) to (r%d, c%d)\n", seg2->y1, seg2->x1, seg2->y2, seg2->x2);
			if( ( seg2->x1 == this->x1 && seg2->y1 == this->y1 ) ||
			    ( seg2->x2 == this->x1 && seg2->y2 == this->y1 ) )
				connected_on_node1 = true; 
				
			if( ( seg2->x1 == this->x2 && seg2->y1 == this->y2 ) ||
			    ( seg2->x2 == this->x2 && seg2->y2 == this->y2 ) )
				connected_on_node2 = true; 
		}
		
		//if(printing) printf("connected_on_node1: %d\n", connected_on_node1);
		//if(printing) printf("connected_on_node2: %d\n", connected_on_node2);
		//if connected on both nodes, it is not a nub
		bool isNub = !(connected_on_node1 && connected_on_node2);
		if(isNub)
		{
			this->removeNet(net);
			
			if(printing) printf("Net %d is a nub!\n", net->net_id);
			
			//check for a chain of nubs. Recursively delete until a non-nub is found
			for(int i = 0; i < net->segs.size(); i++)
			{
				Segment* seg2 = net->segs[i];
													
				if(seg2 == this)
					continue;
				
				if( ( seg2->x1 == this->x1 && seg2->y1 == this->y1 ) ||
				    ( seg2->x2 == this->x1 && seg2->y2 == this->y1 ) )
					if(seg2->isNub(net, sb))
						seg2->removeNet(net);
					
				if( ( seg2->x1 == this->x2 && seg2->y1 == this->y2 ) ||
				    ( seg2->x2 == this->x2 && seg2->y2 == this->y2 ) )
					if(seg2->isNub(net, sb))
						seg2->removeNet(net);
			}
		}
		else if(printing) printf("Net %d is NOT a nub!\n", net->net_id);
		
		
		return isNub;
	}
	
	//returns true if this Segment is "touching" to another Segment
	//i.e. they are touching vertices
	bool isTouching(Segment* seg)
	{
		if(this->x1 == seg->x1 && this->y1 == seg->y1)
			return true;
		if(this->x1 == seg->x2 && this->y1 == seg->y2)
			return true;
		if(this->x2 == seg->x1 && this->y2 == seg->y1)
			return true;
		if(this->x2 == seg->x2 && this->y2 == seg->y2)
			return true;
			
		return false;
	}		
	
}; //end class Segment



//constructor
Net :: Net(int id, Switchbox* my_sb)
{
	net_id = id;
	sb = my_sb;
	
	avg_wirelength = 0;
	num_samples = 1;
}

void Net :: addSegment(Segment* seg)
{
	this->segs.push_back(seg);
	
	updateAverage();
}

bool Net :: hasSegment(Segment* seg)
{
	for(int i = 0; i < this->segs.size(); i++)
		if(this->segs[i] == seg)
			return true;
			
	return false;		
}

bool Net :: hasSegmentAt(int a, int b, int c, int d)
{
	for(int i = 0; i < segs.size(); i++)
	     if( (this->segs[i]->x1 == a && this->segs[i]->y1 == b && this->segs[i]->x2 == c && this->segs[i]->y2 == d ) ||
	         (this->segs[i]->x1 == c && this->segs[i]->y1 == d && this->segs[i]->x2 == a && this->segs[i]->y2 == b ) )
			return true;
			
	return false;
}

//returns true if this net has a loop in it
//traverses the entire net. If it ever reaches the same segment twice, a loop exists
bool Net :: hasLoop()
{

	return false;
}

//create new segments to draw from point (a, b) to point (c, d)
bool Net :: drawSegments(int a, int b, int c, int d)
{
	
//printf("drawSegments() from (%d, %d) to (%d, %d)\n", a, b, c, d);
	bool right = true;
	if(a > c) right = false;
	
	bool down = true;
	if(b > d) down = false;
	
	//add Hsegs to this net
	for(int i = (right ? a:c); i != (right ? c:a); i++)
	{
		//segs.push_back(new Segment(i, b, i+1, b));
		//this->segs.push_back(this->sb->Hsegs[b][i]);
		this->sb->Hsegs[b][i]->addNet(this);
		
	}
	
	//add Vsegs to this net
	for(int i = (down ? b:d); i != (down ? d:b); i++)
	{
		//segs.push_back(new Segment(c, i, c, i+1));
		//this->segs.push_back(this->sb->Vsegs[i][c]);
		this->sb->Vsegs[i][c]->addNet(this);
	}

}

//based on the current wirelength, update the average
int Net :: updateAverage()
{
	int curr_wirelength = segs.size(); //each seg has length of 1
	
	float prev_total = avg_wirelength * 10;		
	
	avg_wirelength = ( prev_total + curr_wirelength ) / (1+10);
	
	if(printing)
		printf("Updating net %d avg length to %.3f. Current length = %d\n", this->net_id, this->avg_wirelength, curr_wirelength);
	
	return avg_wirelength;
}

int Net :: getAverage()
{
	return avg_wirelength;
}


//returns the difference between the current wirelength and the average 
//a large diffFromAvg means that the net has been much smaller in the past, 
//and should be moved to hopefully return to that average
int Net :: diffFromAvg()
{
	return segs.size() - avg_wirelength;
}

	
	//constructor
Switchbox :: Switchbox(string benchmark_name)
	{
		readInput(benchmark_name);
		
		//create blank segments to represent the grid
		for(int i = 0; i < left.size()+2; i++)
		{
			vector<Segment*> Hrow;
			for(int j = 0; j < top.size()+1; j++)
				{
					Segment* Hseg = new Segment(j, i, j+1, i);
					
					Hrow.push_back(Hseg);			
				}
			Hsegs.push_back(Hrow);
			
			if(i < left.size()+1)
			{
				vector<Segment*> Vrow;
			
				for(int j = 0; j < top.size()+2; j++)
				{
					Segment* Vseg = new Segment(j, i, j, i+1);
					
					Vrow.push_back(Vseg);			
				}
				
				Vsegs.push_back(Vrow);
			}			
			
		}
		
		orig_width = top.size();
		orig_height = left.size();
	}
	
	


void Switchbox :: readInput(string benchmark_name)
{
		FILE *input_file; 
		input_file = fopen(benchmark_name.c_str(), "r");	
	if (input_file == NULL) 
	{ fprintf(stderr, "Can't open input file: %s!\n", benchmark_name.c_str()); exit(1); }
		
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
	
	void Switchbox :: printSwitchbox()
	{
		printf("\nprintSwitchbox()\n");
		
		//print graphical representation
		
		printf("        ");
		for(int i = 0; i < top.size(); i++)
			printf("(%2d)", i+1);
		printf("\n");	
		printf("       ");
		for(int i = 0; i < top.size(); i++)
			printf("  %2d", top[i]);
		printf("\n");
		printf("       ");
		
		for(int i = 0; i < top.size()*2+1; i++)
			printf("--");
		printf("\n");
		
		for(int i = 0; i < left.size()+1; )
		{
			//print vertical segs
			printf("      |  ");
			for(int j = 1; j < top.size()+1; j++)
			{
				if(Vsegs[i][j]->nets.size() == 1)
					printf(" %1d  ", Vsegs[i][j]->nets[0]->net_id);
				else if(Vsegs[i][j]->nets.size() > 1)
					{
						//if(Vsegs[i][j]->nets.size() <= 2) printf(" ");
						printf(" %1d%1d ", Vsegs[i][j]->nets[0]->net_id, Vsegs[i][j]->nets[1]->net_id);
						//if(Vsegs[i][j]->nets.size() > 2)
							//printf("%02d", Vsegs[i][j]->nets[2]->net_id);
						
					}
				else printf("    ");
			}
			printf(" |\n");				
			
			i++;
			if(i == left.size()+1) break;
			
			//print horizontal segments						
			printf("(%2d)%2d|", i, left[i-1]);			
			for(int j = 0; j < top.size()+1; j++)
			{
				if(Hsegs[i][j]->nets.size() == 1)
					printf(" %1d ", Hsegs[i][j]->nets[0]->net_id);
				else if(Hsegs[i][j]->nets.size() > 1)
					{
						printf(" %1d%1d", Hsegs[i][j]->nets[0]->net_id, Hsegs[i][j]->nets[1]->net_id);
						//if(Hsegs[i][j]->nets.size() > 2)
							//printf("%02d", Hsegs[i][j]->nets[2]->net_id);
						//else printf("");
					}
				else printf("   ");
				if(j < top.size()) printf("+");
			}
			printf("|%2d\n", right[i-1]);
			
		}
		
		
		printf("       ");
		for(int i = 0; i < 2*bot.size()+1; i++)
			printf("--");
		printf("\n       ");	
		for(int i = 0; i < bot.size(); i++)
			printf("  % 2d", bot[i]);
			
		printf("\n");
	}
	
	void Switchbox :: printAllNets()
	{
		for(int i = 0; i < nets.size(); i++)		
			printNet(nets[i]);		
	}
	
	void Switchbox :: printNet(Net* net)
	{
		printf("   ");
		for(int i = 0; i < top.size(); i++)
			printf("  %2d", top[i]);
		printf("\n   ");
		
		for(int i = 0; i < top.size()*2+1; i++)
			printf("--");
		printf("\n");
		
		for(int i = 0; i <= left.size(); i++)
		{
			//print vertical segs
			printf("  |");
			for(int j = 0; j < top.size(); j++)
				if(net->hasSegmentAt(j+1, i, j+1, i+1))
					printf("  %2d", net->net_id);
				else printf("    ");
			printf("  |\n");				
				
			//print horizontal segments
			if(i < left.size())
			{
			printf("%2d|", left[i]);			
			for(int j = 0; j <= top.size(); j++)
			{
				if(net->hasSegmentAt(j, i+1, j+1, i+1))
					printf("%2d", net->net_id);
				else printf("  ");
				/*if(j != Hsegs[i].size()-1)
					if(vias[i][j])
						printf(" *");
					else*/ 
				if(j < top.size()) printf(" +");
			}
			printf("|%2d\n", right[i]);
			}
		}
		
		
		printf("   ");
		for(int i = 0; i < 2*bot.size()+1; i++)
			printf("--");
		printf("\n   ");	
		for(int i = 0; i < bot.size(); i++)
			printf("  %2d", bot[i]);
					
		
		printf("\nWirelength for net %d: %d\n", net->net_id, net->segs.size());
}
	
	//exports the net to a .csv file
	void Switchbox :: exportNet(Net* net, string output_file_name)
	{
		printf("\n***************EXPORTING NET %d ROUTE TO: %s***************\n", net->net_id,  output_file_name.c_str());
		FILE* out_file = fopen(output_file_name.c_str(), "w");
		fprintf(out_file, "x1,y1,x2,y2,net\n");
	
		for(int i = 0; i < net->segs.size(); i++)
		fprintf(out_file, "%d,%d,%d,%d,%d\n", net->segs[i]->x1, net->segs[i]->y1, net->segs[i]->x2, net->segs[i]->y2, net->net_id);
		
		/*
		//export vias
		for(int i = 0; i < vias.size(); i++)
			for(int j = 0; j < vias[i].size(); j++)
			if(vias[i][j])
			fprintf(out_file, "%d,%d,%d,%d,%d\n", -1, -1, j+1, i+1);
		*/
		
		fclose(out_file);
	}
	
	//count total wirelength for all nets in the switchbox
	int Switchbox :: totalWirelength()
	{
		int wirelength = 0;
		
		for(int i = 0; i < nets.size(); i++)
			wirelength += nets[i]->segs.size();
			
		return wirelength;
	}
	
	//routes this net with minimum wirelength
	//makes no regard for conflicts
	void Switchbox :: routeNet(Net* net)
	{
		printf("\nBEGIN ROUTE FOR NET %d\n", net->net_id);
		//connect all the pins on this net
		
		vector<int> v; //used to record the track of each pin on this net
		
		int min_x = top.size();
		int max_x = 0;
		int min_y = top.size();
		int max_y = 0;
		
		for(int i = 0; i < top.size(); i++)   
			if(top[i] == net->net_id) 
			{
				v.push_back(0);
				
				min_y = 1;				
				if(i+1 < min_x) min_x = i+1;
				if(i+1 > max_x) max_x = i+1;
			}
			
		for(int i = 0; i < bot.size(); i++)   
			if(bot[i] == net->net_id) 
			{
				v.push_back(left.size());
				
				max_y = left.size();
				if(i+1 < min_x) min_x = i+1;
				if(i+1 > max_x) max_x = i+1;
				
			}
			
		for(int i = 0; i < left.size(); i++)  
			if(left[i] == net->net_id) 
			{
				v.push_back(i+1);
				
				min_x = 1;
				if(i+1 < min_y) min_y = i+1;
				if(i+1 > max_y) max_y = i+1;
			}
			
		for(int i = 0; i < right.size(); i++) 
			if(right[i] == net->net_id) 
			{
				v.push_back(i+1);
				
				max_x = top.size();
				if(i+1 < min_y) min_y = i+1;
				if(i+1 > max_y) max_y = i+1;
			}
		
		int sum = 0;
		for(int i = 0; i < v.size(); i++) sum += v[i];
		int trunk = round( (float)sum / v.size() ) ;
		
		if(trunk == 0) trunk = 1;
		if(trunk > left.size()) trunk = left.size();
		
		//printf("trunk for net %d is %d.\n", net->net_id, trunk);
		//printf("min_x: %d \t max_x: %d \t min_y: %d \t max_y: %d \n", min_x, max_x, min_y, max_y);
		
		//place the trunk
		
		net->drawSegments(min_x, trunk, max_x, trunk);
		//printf("Trunk placed on track %d.\n", trunk);
				
		//place the branches
		
		for(int i = 0; i < top.size(); i++)
		{
			if(top[i] == net->net_id)
			{
				net->drawSegments(i+1, 0, i+1, trunk);
				//printf("Connected top pin %d.\n", i);
			}
		}
		
		for(int i = 0; i < bot.size(); i++)
		{
			if(bot[i] == net->net_id)
			{	
				net->drawSegments(i+1, trunk, i+1, left.size()+1);
				//printf("Connected bot pin %d.\n", i);
			}
		}
		
		for(int i = 0; i < left.size(); i++)
		{
			if(left[i] == net->net_id)
			{
				net->drawSegments(0, i+1, 1, trunk);
				//printf("Connected left pin %d.\n", i);
			}
		}
				
		for(int i = 0; i < right.size(); i++)
		{
			if(right[i] == net->net_id)
			{
				net->drawSegments(top.size()+1, i+1, top.size(), trunk);
				//printf("Connected right pin %d.\n", i);
			}
		}
		
		//consolidate branches
		
		
		//remove nubs
		
		
		printNet(net);
	}
	
	bool Switchbox :: netExists(int net_id)
	{
		for(int i = 0; i < nets.size(); i++)
		{
			if(nets[i]->net_id == net_id)
				return true;
		}
		
		return false;
	}
	
	void Switchbox :: routeAllNets()
	{
		//from the pins, get a list of all nets
		for(int i = 0; i < top.size(); i++)
		{
			int net_id = top[i];
			if(net_id != 0)
			if(!netExists(net_id))
				nets.push_back(new Net(net_id, this));
		}
		
		for(int i = 0; i < bot.size(); i++)
		{
			int net_id = bot[i];
			if(net_id != 0)
			if(!netExists(net_id))
				nets.push_back(new Net(net_id, this));
		}
		
		for(int i = 0; i < left.size(); i++)
		{
			int net_id = left[i];
			if(net_id != 0)
			if(!netExists(net_id))
				nets.push_back(new Net(net_id, this));
		}
		
		for(int i = 0; i < right.size(); i++)
		{
			int net_id = right[i];
			if(net_id != 0)
			if(!netExists(net_id))
				nets.push_back(new Net(net_id, this));
		}
		
		//route each net one at a time
		for(int i = 0; i < nets.size(); i++)
		{
			routeNet(nets[i]);
		}
	}


//move through the grid of segments resolving conflicts until no conflicts are seen in an interation of 4 sweeps
//returns true if all conflicts were fixed, or false if some conflicts still remain
bool Switchbox :: resolveAllConflicts()
{
	int sweep_direction = 0; //0=TB 1=RL 2=BT 3=LR
	
	//maximum attempts for sweep iterations
	//constant +500 ensures that very small switchboxes don't increase size too quickly
	int max_attempts = ( 10 * top.size() * left.size() ) + 1000; 
	
	//perform iterations until no change is made
	for(int attempt = 0; attempt < max_attempts; attempt++)
	{	
		if(printing) printf("\n##############BEGIN ITERATION %d##############\n", attempt);
		bool change_made = false;
		
		//needs to do a sweep in all 4 directions
		//but might randomly do some sweep directions more
		bool directions[4] = {false, false, false, false}; 
		
		//unlockAllSegments();
		int count = 0;
		for(int j = 0; j < 4; )
		{								
											
			//try to clean up all nets to be more space effcient
			for(int a = 0; a < nets.size(); a++)			
				//for(int k = 0; k < 2; k++)
					cleanUp(nets[a]);
			
			sweep_direction = rand() % 4;
			
			if(directions[sweep_direction] == false)
				j++;
			
			if(printing) 
				printf("Sweep Count #%d Sweep Direction: %d\n", count++, sweep_direction);
				
			directions[sweep_direction] = true;
						
			if(resolveConflictsSweep(sweep_direction))
				change_made = true;
				
			if(++sweep_direction > 3)
				sweep_direction = 0;					
						
			if(printing) printSwitchbox();	
			
								
		}
	
		//try to clean up all nets to be more space effcient
			for(int a = 0; a < nets.size(); a++)			
				//for(int k = 0; k < 2; k++)
					cleanUp(nets[a]);
			
		//perturb direction so it is different next iteration
		if(++sweep_direction > 3)
			sweep_direction = 0;
		
		printSwitchbox();				
		
		printf("\n##############FINISH ITERATION %d##############\n", attempt);
				
		if(change_made == false)
		{
			printf("\nNo changes made for an entire iteration....\n");
			//return true;
		}
		
		if(!conflictsExist())
		{
			printf("\nNo conflicts exist!....done with routing!\n");
			return true;
		}
		
		
	}
	
	return false;
}

//performs a sweep with a given sweep direction
//returns true if any cfhanges were made
bool Switchbox :: resolveConflictsSweep(int sweep_direction)
{
	if(printing) printf("\n#BEGIN SWEEP: direction = %d\n", sweep_direction);
	bool change_made = false;
	
	bool even = (sweep_direction%2 == 0); //if even, indicates TB or BT
						//if odd, indicates LR or RL
	
	bool invert = (sweep_direction > 1); //if inverted, indicates BT or RL
						//if not, indicates TB or LR
	
	//loop through all nodes (i, j) based on sweep_direction
	
	if(even)
	for(int i = (invert ? left.size():1); i != (invert ? 1:left.size()); i+=(invert ? -1:1))
	{
		for(int j = 1; j < top.size(); j++)
		{
			int num_segs_moved = resolveNodeConflicts(j, i, even, invert);
			
			j += num_segs_moved;
			
			//printf("num_segs_moved: %d\n", num_segs_moved);
				
			if(num_segs_moved > 0)
				change_made = true;
		}
	}
	
	else
	for(int i = (invert ? top.size():1); i != (invert ? 1:top.size()); i+=(invert ? -1:1))
	{
		for(int j = 1; j < left.size(); j++)
		{
			int num_segs_moved = resolveNodeConflicts(i, j, even, invert);
			
			j += num_segs_moved;
			
			//printf("num_segs_moved: %d\n", num_segs_moved);
				
			if(num_segs_moved > 0)
				change_made = true;
		}
	}
	
	return change_made;
}


//resolveSegmentConflicts() attempts to resolve all conflicts 
//on the segment by moving 1 or more nets
//returns true if a change was made.
int Switchbox :: resolveNodeConflicts(int i, int j, bool even, bool invert)
{
	//look at the Hseg and Vseg at node (i, j)
	//if there is a conflict, try to move it.	
	if(printing) printf("Looking for conflict at (r%d, c%d)!\n", j, i);		
	
	int changes_made = false;
	bool segment_conflict = false;
	bool via_conflict = false;
	
	
	
	
		
	if(even) //looking at Hsegs
	{
		Segment* seg = Hsegs[j][i];
	
		if(seg->nets.size() == 0)
			return false;
				
		if(seg->nets.size() > 1)
		{
			segment_conflict = true;
			if(printing) printf("&&Segment conflict detected! line 804\n");
		}
		else
		{		
			if(i > 0) //if there is a seg to left, conflict!
			if(Hsegs[j][i-1]->nets.size() > 0 && Hsegs[j][i-1]->nets[0] != seg->nets[0]) 
				{ via_conflict = true; if(printing) printf("&& Via conflict detected! line 761\n"); }
			
			if(i < Hsegs[j].size()-1) //if there is a seg to right, conflict!
			if(Hsegs[j][i+1]->nets.size() > 0 && Hsegs[j][i+1]->nets[0] != seg->nets[0]) 
				{ via_conflict = true;  if(printing) printf("&& Via conflict detected! line 765\n"); }	
		}
		
		//try to move all but one net from this segment
	if(segment_conflict || via_conflict)
	{ 
		
	Net* net_to_displace = seg->nets[0];
	int most_segs_above = 0;
	int largest_diff = seg->nets[0]->diffFromAvg();
	
	for(int a = 0; a < seg->nets.size(); a++)
	{
		if(seg->lastMoveInDirection(even, invert) == seg->nets[a])
		{
			if(printing) printf("Ignoring net %d because it was moved last time on this seg.\n", seg->nets[a]->net_id);
			continue;
		}
		
		int segs_above = getNumSegsInDirection(j, i, even, invert, seg->nets[a]);
		if(segs_above >= most_segs_above)
		{
			most_segs_above = segs_above;
			net_to_displace = seg->nets[a];
		}
		
		/*
		int diff = seg->nets[a]->diffFromAvg();
		if(diff >= largest_diff)
		{
			largest_diff = diff;
			net_to_displace = seg->nets[a];
		}
		*/
		
	}
		if(printing) printf("net %d was selected with %d segs in the direction.\n", net_to_displace->net_id, most_segs_above);
	
	int free_spaces = getNumFreeSpacesInDirection(j, i, even, invert, net_to_displace);
	if(printing) printf("%d free spaces.\n", free_spaces);	
	
	if(free_spaces == 0)
	{
		if(printing) printf("No free spaces! Canceling moves...");
		//return 0;
	}
	
	
			if(invert) //going up!
			{	
				if(netBenefit(seg, net_to_displace, even, invert, 30))
				changes_made += moveSegUp(seg, net_to_displace, 2);
			}
			else //going down!
			{
				if(netBenefit(seg, net_to_displace, even, invert, 30))
				changes_made += moveSegDown(seg, net_to_displace, 2);
			}
			//printSwitchbox();
		}	
	}
	
	else // if(!even) --> looking at Vsegs
	{
		Segment* seg = Vsegs[j][i];
	
		if(seg->nets.size() == 0)
			return false;						
			
		if(seg->nets.size() > 1)
		{
			segment_conflict = true;
			if(printing) printf("&&Segment conflict detected! line 804\n");
		}
		else
		{		
			if(j > 0) //if there is a seg to top, conflict!
			if(Vsegs[j-1][i]->nets.size() > 0 && Vsegs[j-1][i]->nets[0] != seg->nets[0]) 
				{ via_conflict = true;  if(printing) printf("&& Via conflict detected! line 800\n"); }
			
			if(j < Vsegs.size()-1) //if there is a seg to bot, conflict!	
			if(Vsegs[j+1][i]->nets.size() > 0 && Vsegs[j+1][i]->nets[0] != seg->nets[0]) 	
				{ via_conflict = true;  if(printing) printf("&& Via conflict detected! line 804\n"); }	
		}
		
	//try to move all but one net from this segment
	if(segment_conflict || via_conflict)
	{	
	
	Net* net_to_displace = seg->nets[0];
	int most_segs_above = 0;
	int largest_diff = seg->nets[0]->diffFromAvg();
	
	for(int a = 0; a < seg->nets.size(); a++)
	{
		
		if(seg->lastMoveInDirection(even, invert) == seg->nets[a])
		{
			if(printing) printf("Ignoring net %d because it was moved last time on this seg.\n", seg->nets[a]->net_id);
			continue;
		}
			
		int segs_above = getNumSegsInDirection(j, i, even, invert, seg->nets[a]);
		if(segs_above >= most_segs_above)
		{
			most_segs_above = segs_above;
			net_to_displace = seg->nets[a];
		}
		
		/*
		int diff = seg->nets[a]->diffFromAvg();
		if(diff >= largest_diff)
		{
			largest_diff = diff;
			net_to_displace = seg->nets[a];
		}*/
		
	}
		if(printing) printf("net %d was selected with %d segs in the direction.\n", net_to_displace->net_id, most_segs_above);
	
	int free_spaces = getNumFreeSpacesInDirection(j, i, even, invert, net_to_displace);
	if(printing) printf("%d free spaces.\n", free_spaces);	
	
	if(free_spaces == 0)
	{
		if(printing) printf("No free spaces! Canceling moves...");
		//return 0;
	}
			
			if(invert) //going left!
			{
				if(netBenefit(seg, net_to_displace, even, invert, 30))
				changes_made += moveSegLeft(seg, net_to_displace, 2);
			}
			else //going right!
			{
				if(netBenefit(seg, net_to_displace, even, invert, 30))
				changes_made += moveSegRight(seg, net_to_displace, 2);
			}	
			//printSwitchbox();					
		}
	}
	
	if(printing) if(changes_made > 0) printSwitchbox();
		
	return changes_made;
}

/*
void Switchbox :: unlockAllSegments()
{
	for(int i = 0; i < Hsegs.size(); i++)
		for(int j = 0; j < Hsegs[i].size(); j++)
			Hsegs[i][j]->locked = false;
			
	for(int i = 0; i < Vsegs.size(); i++)
		for(int j = 0; j < Vsegs[i].size(); j++)
			Vsegs[i][j]->locked = false;
}
*/

//attempts to move the segment up to an empty row.
//returns the number of segments moved
int Switchbox :: moveSegUp(Segment* seg, Net* net, int allow_chains)
{	

	//if(!seg->hasNet(net)) return 0;
	
	int num_segs_moved = 0;
	
	//if(seg->locked) return false;
	//if(seg->nets.size() < 1) return num_segs_moved;
	
	//find the target seg
	bool vert = seg->isVertical();
	int x = seg->x1;
	int y = seg->y1;
	
	if(printing) printf("\nmoveSegUp() Hseg at (%d, %d)\t net %d\n", x, y, net->net_id);
	if(y <= 1 || vert) return num_segs_moved;
	
	//if(printing) 
	//if(seg->last_move_up != NULL)
		//printf("last_move_up: %d\n", seg->last_move_up->net_id);
		
	seg->last_move_up = net;
	
	if(printing) printf("last_move_up: %d\n", seg->last_move_up->net_id);
	
	//check if place to move is open
	if(Hsegs[seg->y1-1][seg->x1]->isOccupied(net) && (allow_chains != 2))
	{
		if(printing) printf("Can't perform nmoveSegUp(), something in the way!\n");
		
		if(rand()%10 <= 7)
			return 0;
	}		
	
	//check for other segments that must be added or removed
	if(x > 0)
	if(Hsegs[y][x-1]->hasNet(net) || Vsegs[y][x]->hasNet(net) )
		Vsegs[y-1][x]->addNet(net);
	else
		Vsegs[y-1][x]->removeNet(net);
	
	if(x < top.size())
	if(Hsegs[y][x+1]->hasNet(net) || Vsegs[y][x+1]->hasNet(net) )
		Vsegs[y-1][x+1]->addNet(net);
	else
		Vsegs[y-1][x+1]->removeNet(net);
	
	//add the new segment, but only if it isn't a nub
	if(!Hsegs[y-1][x]->isNub(net, this))
		Hsegs[y-1][x]->addNet(net);
	
	//remove the old segment	
	seg->removeNet(net);
	//seg->locked = true;
	
	//allow_chains:
	//-1: allow chains backwards only
	//0: do not allow chains
	//1: allow chains forwards only
	//2: allow chains either way
	
	//try to move the next segment in line
	if(allow_chains != 0) 
	for(int i = 0; i < net->segs.size(); i++)
	{		
		//check for chain move on Hseg to the right
		if(!net->segs[i]->isVertical() && net->segs[i]->x1 == seg->x2 && net->segs[i]->y1 == seg->y2)
		if(seg->x2 < top.size() && (allow_chains == 2 || allow_chains == -1))
		{
			if(printing) printf("Chain move up!\n");
			
			//if(Hsegs[net->segs[i]->y1][net->segs[i]->x1+1]->nets.size() == 0)
				num_segs_moved += moveSegUp(net->segs[i], net, -1);
			//break;
		}
		
		//check for chain move on Hseg to the left
		//ADDING THIS WAS IMPORTANT!@#!@#!@#
		if(!net->segs[i]->isVertical() && net->segs[i]->x2 == seg->x1 && net->segs[i]->y2 == seg->y1)
		if(net->segs[i]->x1 > 0 && net->segs[i]->x2 > 0 && (allow_chains == 2 || allow_chains == 1))
		{
			if(printing) printf("Chain move up!\n");
			//num_segs_moved += 
			
			//if(Hsegs[net->segs[i]->y1][net->segs[i]->x1-1]->nets.size() == 0)
				moveSegUp(net->segs[i], net, 1);
			//break;
		}
	}
	
	net->updateAverage();
	
	return ++num_segs_moved;
}

int Switchbox :: moveSegDown(Segment* seg, Net* net, int allow_chains)
{	
	//if(!seg->hasNet(net)) return 0;
	
	int num_segs_moved = 0;
	
	//if(seg->locked) return false;
	//if(seg->nets.size() < 1) return num_segs_moved;
		
	//find the target seg
	bool vert = seg->isVertical();
	
	int x = seg->x1;
	int y = seg->y1;
	
	if(printing) printf("\nmoveSegDown() Hseg at (%d, %d)\t net %d\n", x, y, net->net_id);
	if(y >= left.size() || vert) return num_segs_moved;
	
	//if(printing) 
	//if(seg->last_move_down != NULL)
		//printf("last_move_down: %d\n", seg->last_move_down->net_id);
		
	seg->last_move_down = net;
	
	if(printing) printf("last_move: %d\n", seg->last_move_down->net_id);		
	//check if place to move is open
	if(Hsegs[seg->y1+1][seg->x1]->isOccupied(net) && (allow_chains != 2))
	{
		if(printing) printf("Can't perform moveSegDown(), something in the way!\n");
		if(rand()%10 <= 7)
			return 0;
	}
			
	//check for other segments that must be added or removed
	if(x > 0 && y > 0)
	if(Hsegs[y][x-1]->hasNet(net) || Vsegs[y-1][x]->hasNet(net) )
		Vsegs[y][x]->addNet(net);
	else
		Vsegs[y][x]->removeNet(net);
	
	if(x < top.size())
	{ 
	if(Hsegs[y][x+1]->hasNet(net) || Vsegs[y-1][x+1]->hasNet(net) )
		{ Vsegs[y][x+1]->addNet(net); }
	else
		Vsegs[y][x+1]->removeNet(net);
	}
	
	//add the new segment
	if(!Hsegs[y+1][x]->isNub(net, this))
		//Hsegs[y+1][x]->removeNet(net);
	Hsegs[y+1][x]->addNet(net);
	
	//remove the old segment	
	seg->removeNet(net);
	//seg->locked = true;
	
	if(printing) printSwitchbox();
	
	//try to move the next segment in line
	if(allow_chains != 0)
	for(int i = 0; i < net->segs.size(); i++)
	{
		if(!net->segs[i]->isVertical() && net->segs[i]->x1 == seg->x2 && net->segs[i]->y1 == seg->y2)
		if(seg->x2 < top.size() && (allow_chains == 2 || allow_chains == -1))
		{
			if(printing) printf("Chain move down!\n");
			
			//if(Hsegs[net->segs[i]->y1][net->segs[i]->x1+1]->nets.size() == 0)
				num_segs_moved += moveSegDown(net->segs[i], net, -1);
			//break;
		}
		
		if(!net->segs[i]->isVertical() && net->segs[i]->x2 == seg->x1 && net->segs[i]->y2 == seg->y1)
		if(net->segs[i]->x1 > 0 && net->segs[i]->x2 > 0 && (allow_chains == 2 || allow_chains == -1))
		{
			if(printing) printf("Chain move down!\n");
			//num_segs_moved += 
			//if(Hsegs[net->segs[i]->y1][net->segs[i]->x1-1]->nets.size() == 0)
				moveSegDown(net->segs[i], net, 1);
			//break;
		}
	}
	
	net->updateAverage();
	
	return ++num_segs_moved;
}

int Switchbox :: moveSegLeft(Segment* seg, Net* net, int allow_chains)
{
	//if(!seg->hasNet(net)) return 0;
	
	int num_segs_moved = 0;
	//if(seg->locked) return false;
	//if(seg->nets.size() < 1) return num_segs_moved;
	
	//find the target seg
	bool vert = seg->isVertical();
	int x = seg->x1;
	int y = seg->y1;
	if(printing) printf("\nmoveSegLeft()!\n Vseg @ (%d, %d)\t net %d\n", seg->x1, seg->y1, net->net_id);
//return true;
	if(x <= 1 || !vert) return num_segs_moved;		
	
	if(printing) 
	if(seg->last_move_left != NULL)
		printf("last_move_left: %d\n", seg->last_move_left->net_id);
		
	seg->last_move_left = net;
	
	if(printing) printf("last_move: %d\n", seg->last_move_left->net_id);
	
	//check if place to move is open
	if(Vsegs[y][x-1]->isOccupied(net) && (allow_chains != 2))
	{
		if(printing) printf("Can't perform moveSegLeft(), something in the way!\n");
		if(rand()%10 <= 7)
			return 0;
	}		
	
	//check for other segments that must be added or removed
	if(y > 0)
	if(Vsegs[y-1][x]->hasNet(net) || Hsegs[y][x]->hasNet(net) )
		Hsegs[y][x-1]->addNet(net);
	else
		Hsegs[y][x-1]->removeNet(net);

	if(y < left.size())
	{
	if(Vsegs[y+1][x]->hasNet(net) || Hsegs[y+1][x]->hasNet(net) )
		Hsegs[y+1][x-1]->addNet(net);
	else
		Hsegs[y+1][x-1]->removeNet(net);	
	}
	
	//add the new segment, but only if it isn't a nub
	if(!Vsegs[y][x-1]->isNub(net, this))
		Vsegs[y][x-1]->addNet(net);
	
	seg->removeNet(net);
	//seg->locked = true;
	
	//try to move the next segment in line
	if(printing) printf("seg: (%d, %d) to (%d, %d)\n", seg->x1, seg->y1, seg->x2, seg->y2);
	if(allow_chains != 0)
	for(int i = 0; i < net->segs.size(); i++)
	{
		if(net->segs[i]->isVertical() && net->segs[i]->x1 == seg->x2 && net->segs[i]->y1 == seg->y2)
		if(seg->y2 < left.size() && (allow_chains == 2 || allow_chains == -1))
		{
			if(printing) printf("Chain move left!\n");
			//if(Vsegs[net->segs[i]->y1+1][net->segs[i]->x1]->nets.size() == 0)
				num_segs_moved += moveSegLeft(net->segs[i], net, -1);
			//break;
		}
		
		if(net->segs[i]->isVertical() && net->segs[i]->x2 == seg->x1 && net->segs[i]->y2 == seg->y1)
		if(net->segs[i]->y1 > 0 && net->segs[i]->y2 > 0 && (allow_chains == 2 || allow_chains == 1))
		{
			if(printing) printf("Chain move left!\n");
			//num_segs_moved += 
			//if(Vsegs[net->segs[i]->y1-1][net->segs[i]->x1]->nets.size() == 0)
				moveSegLeft(net->segs[i], net, 1);
			//break;
		}
	}
	
	
	net->updateAverage();
	
	return ++num_segs_moved;
}


int Switchbox :: moveSegRight(Segment* seg, Net* net, int allow_chains)
{
	//if(!seg->hasNet(net)) return 0;
	
	int num_segs_moved = 0;
	//if(seg->locked) return false;
	//if(seg->nets.size() < 1) return num_segs_moved;
	
	//find the target seg
	bool vert = seg->isVertical();
	int x = seg->x1;
	int y = seg->y1;
	if(printing) printf("\nmoveSegRight()!\t Vseg @ (%d, %d)\t net %d\n", seg->x1, seg->y1, net->net_id);
	if(printing) printf("seg->nets.size() = %d\n", seg->nets.size());
	
	if(x >= top.size() || !vert) return num_segs_moved;		

	if(printing) 
	if(seg->last_move_right != NULL)
		printf("last_move_right: %d\n", seg->last_move_right->net_id);
		
	seg->last_move_right = net;
	
	if(printing) printf("last_move: %d\n", seg->last_move_right->net_id);
	
	//check if place to move is open
	if(Vsegs[y][x+1]->isOccupied(net) && (allow_chains != 2))
	{
		if(printing) printf("Can't perform moveSegRight(), something in the way!\n");
		if(rand()%10 <= 7)
			return 0;
	}		
	
	//check for other segments that must be added or removed
	if(y > 0 && x > 0)
	if(Vsegs[y-1][x]->hasNet(net) || Hsegs[y][x-1]->hasNet(net) )
		Hsegs[y][x]->addNet(net);
	else
		Hsegs[y][x]->removeNet(net);
	
	if(y < left.size() && x > 0)
	{
	if(Vsegs[y+1][x]->hasNet(net) || Hsegs[y+1][x-1]->hasNet(net) )
		Hsegs[y+1][x]->addNet(net);
	else
		Hsegs[y+1][x]->removeNet(net);
		
	}
	
	//add the new segment
	if(!Vsegs[y][x+1]->isNub(net, this))
	     Vsegs[y][x+1]->addNet(net);
	
	seg->removeNet(net);
	
	if(printing) printSwitchbox();
	
	//try to move the next segment in line
	if(allow_chains != 0)
	{ //printf("line 1069\n");
	for(int i = 0; i < net->segs.size(); i++)
	{//printf("line 1072\t i = %d\n", i); printf("(%d, %d) (%d, %d)\n", net->segs[i]->x1, net->segs[i]->y1, seg->x2, seg->y2);
		if(net->segs[i]->isVertical() && net->segs[i]->x1 == seg->x2 && net->segs[i]->y1 == seg->y2)
		if(seg->y2 < left.size() && (allow_chains == 2 || allow_chains == -1))
		{
			if(printing) printf("Chain move right (down)!\n");
			
			//if(Vsegs[net->segs[i]->y1+1][net->segs[i]->x1]->nets.size() == 0)
				num_segs_moved += moveSegRight(net->segs[i], net, -1);
			//break;
		}
		
		if(net->segs[i]->isVertical() && net->segs[i]->x2 == seg->x1 && net->segs[i]->y2 == seg->y1)
		if(net->segs[i]->y1 > 0 && net->segs[i]->y2 > 0 && (allow_chains == 2 || allow_chains == 1))
		{
			if(printing) printf("Chain move right! (up)\n");
			//num_segs_moved += 
			
			//if(Vsegs[net->segs[i]->y1-1][net->segs[i]->x1]->nets.size() == 0)
				moveSegRight(net->segs[i], net, 1);
			//break;
		}
	}
	}
	
	
	net->updateAverage();
	
	return ++num_segs_moved;
}

//attempts to "clean up" a net by removing cycles and nubs
void Switchbox :: cleanUp(Net* net)
{
	//control flags
	bool two_spaces = true;
	bool three_spaces = true;
	bool four_spaces = false;
	bool five_spaces = false;
	
	if(printing) 
	{ printf("Cleaning up net %d!\nBEFORE CLEAN:\n", net->net_id); printNet(net); }
	
	//cycle through all segments on this net
	for(int i = 0; i < net->segs.size(); i++)
	{		
		if(net->segs[i]->isVertical()) //push two adjacent Vsegs together
		{
			if(net->segs[i]->y1 > 0 && net->segs[i]->y1 < left.size())
			for(int j = 0; j < net->segs.size(); j++)
			{
				//check for segments next to each other, push them together
				if(net->segs[j]->isVertical() && 
				net->segs[i]->y1 == net->segs[j]->y1 &&
				net->segs[i]->x1 == net->segs[j]->x1-1 )
				{	
					if(netBenefit(net->segs[i], net, false, false))
					moveSegRight(net->segs[i], net, 0);
				}
				
				if(net->segs[j]->isVertical() && 
				net->segs[i]->y1 == net->segs[j]->y1 &&
				net->segs[i]->x1 == net->segs[j]->x1+1 )
				{
					if(netBenefit(net->segs[i], net, false, true))
					moveSegLeft(net->segs[i], net, 0);				
				}
				
				
				//check for segements seperated by 1 column, push them together
				if(two_spaces)
				if(net->segs[i]->y1 < left.size()-1)
				if(net->segs[j]->isVertical() && 
				net->segs[i]->y1 == net->segs[j]->y1 &&
				net->segs[i]->x1 == net->segs[j]->x1-2 )
				{
					if(netBenefit(net->segs[i], net, false, false))
					moveSegRight(net->segs[i], net, false);
					
					if(netBenefit(net->segs[j], net, false, true))
					moveSegLeft(net->segs[j], net, false);					
				}
				
				//check for segements seperated by 2 columns, push them together
				if(three_spaces)
				if(net->segs[i]->y1 < left.size()-2)
				if(net->segs[j]->isVertical() && 
				net->segs[i]->y1 == net->segs[j]->y1 &&
				net->segs[i]->x1 == net->segs[j]->x1-3 )
				{
					if(netBenefit(net->segs[i], net, false, false))
					moveSegRight(net->segs[i], net, false);
					
					if(netBenefit(net->segs[j], net, false, true))
					moveSegLeft(net->segs[j], net, false);					
				}
				
				if(four_spaces)
				if(net->segs[i]->y1 < left.size()-3)
				if(net->segs[j]->isVertical() && 
				net->segs[i]->y1 == net->segs[j]->y1 &&
				net->segs[i]->x1 == net->segs[j]->x1-4 )
				{
					if(netBenefit(net->segs[i], net, false, false))
					moveSegRight(net->segs[i], net, false);
					
					if(netBenefit(net->segs[j], net, false, true))
					moveSegLeft(net->segs[j], net, false);					
				}
				
				if(five_spaces)
				if(net->segs[i]->y1 < left.size()-4)
				if(net->segs[j]->isVertical() && 
				net->segs[i]->y1 == net->segs[j]->y1 &&
				net->segs[i]->x1 == net->segs[j]->x1-5 )
				{
					if(netBenefit(net->segs[i], net, false, false))
					moveSegRight(net->segs[i], net, false);
										
					if(netBenefit(net->segs[j], net, false, true))
					moveSegLeft(net->segs[j], net, false);					
				}
					
			}
		}
		else //push two adjacent Hsegs together
		{
			if(net->segs[i]->x1 > 0 && net->segs[i]->x1 < top.size())
			for(int j = 0; j < net->segs.size(); j++)
			{			
				if(!(net->segs[j]->isVertical()) && 
				net->segs[i]->y1 == net->segs[j]->y1-1 &&
				net->segs[i]->x1 == net->segs[j]->x1 )
				{
					if(netBenefit(net->segs[i], net, true, false))
					moveSegDown(net->segs[i], net, 0);					
				}
				
				if(!(net->segs[j]->isVertical()) && 
				net->segs[i]->y1 == net->segs[j]->y1+1 &&
				net->segs[i]->x1 == net->segs[j]->x1 )
				{					
					if(netBenefit(net->segs[i], net, true, false))
					moveSegUp(net->segs[i], net, 0);					
				}
				
				if(two_spaces)
				if( net->segs[i]->x1 < top.size()-1)
				if(!(net->segs[j]->isVertical()) && 
				net->segs[i]->y1 == net->segs[j]->y1-2 &&
				net->segs[i]->x1 == net->segs[j]->x1 )
				{
					if(netBenefit(net->segs[i], net, true, false))
					moveSegDown(net->segs[i], net, false);
					
					if(netBenefit(net->segs[j], net, true, true))
					moveSegUp(net->segs[j], net, false);
				}
				
				if(three_spaces)
				if( net->segs[i]->x1 < top.size()-2)
				if(!(net->segs[j]->isVertical()) && 
				net->segs[i]->y1 == net->segs[j]->y1-3 &&
				net->segs[i]->x1 == net->segs[j]->x1 )
				{
					if(netBenefit(net->segs[i], net, true, false))
					moveSegDown(net->segs[i], net, false);
					
					if(netBenefit(net->segs[j], net, true, true))
					moveSegUp(net->segs[j], net, false);
				}
				
				if(four_spaces)
				if( net->segs[i]->x1 < top.size()-3)
				if(!(net->segs[j]->isVertical()) && 
				net->segs[i]->y1 == net->segs[j]->y1-4 &&
				net->segs[i]->x1 == net->segs[j]->x1 )
				{
					if(netBenefit(net->segs[i], net, true, false))
					moveSegDown(net->segs[i], net, false);
					
					if(netBenefit(net->segs[j], net, true, true))
					moveSegUp(net->segs[j], net, false);
				}
				
				if(five_spaces)
				if( net->segs[i]->x1 < top.size()-4)
				if(!(net->segs[j]->isVertical()) && 
				net->segs[i]->y1 == net->segs[j]->y1-5 &&
				net->segs[i]->x1 == net->segs[j]->x1 )
				{
					if(netBenefit(net->segs[i], net, true, false))
					moveSegDown(net->segs[i], net, false);
					
					if(netBenefit(net->segs[j], net, true, true))
					moveSegUp(net->segs[j], net, false);
				}	
			}
		}
		
	}
	/*
	//clean up "U" shapes in the net
	for(int i = 0; i < net->segs.size(); i++)
	{
		Segment* seg = net->segs[i];
		int col = seg->x1;
		int row = seg->y1;
		
		if(seg->isVertical()) //look for vertical "U" shapes
		{
			row++; 
			if(Vsegs[row][col].size() > 0)
				break;
				
			//traverse across empty gaps
			while(row < Vsegs.size()-1 && Vsegs[row][col].size() == 0)
				row++;
				
			for(int j = 0; j < Vsegs[row][col]->nets.size(); j++)
				if(Vsegs[row][col]->nets[j] == net)
					cleanUpUshape(net, seg);
		}
		else //look for horizontal "U" shapes
		{
			col++;
			if(Hsegs[row][col].size() > 0)
				break;
				
			//traverse across empty gaps
			while(col < Hsegs[row].size()-1 && Hsegs[row][col].size() == 0)
				col++;
				
			for(int j = 0; j < Hsegs[row][col]->nets.size(); j++)
				if(Hsegs[row][col]->nets[j] == net)
					cleanUpUshape(net, seg);
		}
		
	}
	*/
	
	if(printing) 
	{ printf("\nAFTER CLEAN:\n"); printNet(net); }
}

/*
//removes U-shape from the net at the segment
void Switchbox :: cleanUpUshape(Net* net, Segment* seg)
{
return;
	int col = seg->x1;
	int row = seg->y1;
	
	if(seg->isVertical())
	{
		row++;
		while(!Vsegs[row][col].hasNet(net))
		{
			if(col > 0 && Hsegs[row][col-1].hasNet(net))
			{ //look for the Ushape to the left
				while(col > 0 && Hsegs[row][col-1].hasNet(net))
					col--;
			}
			else
			{ //look for the Ushape to the right
				while(col < Hsegs.size()-1 && Hsegs[row][col+1].hasNet(net))
					col++;
			}
		}
	}
	else
	{
	
	}
}
*/


//looks through the newly made column for places where a via should go!
//vias should be added whenever a Vseg and Hseg of the same net touch
int Switchbox :: placeVias()
{
	int num_vias = 0;
	for(int col = 1; col < Hsegs[0].size(); col++)
	{
		for(int row = 1; row < Vsegs.size(); row++)
		{			
			
			for(int k = 0; k < Hsegs[row][col]->nets.size(); k++)
			{
				if(Vsegs[row-1][col]->hasNet(Hsegs[row][col]->nets[k]) ||
				   Vsegs[row][col]->hasNet(Hsegs[row][col]->nets[k])	)
				   {
				   	bool already_exists = false;
					for(int a = 0; a < vias.size(); a++)
						if(vias[a].first == col && vias[a].second == row)
							already_exists = true;
					if(!already_exists)
					{
						vias.push_back(make_pair(col, row));
						num_vias++;
					}
				   }
			}
			
			for(int k = 0; k < Hsegs[row][col-1]->nets.size(); k++)
			{		
				if(Vsegs[row-1][col]->hasNet(Hsegs[row][col-1]->nets[k]) ||
				   Vsegs[row][col]->hasNet(Hsegs[row][col-1]->nets[k])	)
				   {
				   	bool already_exists = false;
					for(int a = 0; a < vias.size(); a++)
						if(vias[a].first == col && vias[a].second == row)
							already_exists = true;
					if(!already_exists)
					{
						vias.push_back(make_pair(col, row));
						num_vias++;
					}
				   }
			}
		}
	}
	return num_vias;
}
	
//exports the switchbox to a .csv file
void Switchbox :: exportSwitchbox(string output_file_name)
{
		printf("\n***************EXPORTING SWITCHBOX ROUTE TO: %s***************\n", output_file_name.c_str());
		FILE* out_file = fopen(output_file_name.c_str(), "w");
		fprintf(out_file, "x1,y1,x2,y2,net\n");
	
		//export Hsegs
		for(int i = 0; i < Hsegs.size(); i++)
			for(int j = 0; j < Hsegs[i].size(); j++)
				if(Hsegs[i][j]->nets.size() > 0) //if a segment is actually there
				fprintf(out_file, "%d,%d,%d,%d,%d\n", j, i, j+1, i, Hsegs[i][j]->nets[0]->net_id);
		
		//export Vsegs
		for(int i = 0; i < Vsegs.size(); i++)
			for(int j = 0; j < Vsegs[i].size(); j++)
				if(Vsegs[i][j]->nets.size() > 0) //if a segment is actually there
				fprintf(out_file, "%d,%d,%d,%d,%d\n", j, i, j, i+1, Vsegs[i][j]->nets[0]->net_id);
		
		//export vias
		for(int i = 0; i < vias.size(); i++)
			fprintf(out_file, "%d,%d,%d,%d,%d\n", -1, -1, vias[i].first, vias[i].second, -1);
		
		fclose(out_file);
}

//returns the number of pins on the given net in the given direction starting at (row, col)
int Switchbox :: getNumPinsInDirection(int row, int col, bool even, bool invert, Net* net)
{
	int num_pins_found = 0;
	int id = net->net_id;
	
	//printf("getNumPinsInDirection(row:%d, col:%d, even:%d, invert%d, net:%d)\n", row, col, even, invert, net->net_id);
	
	if(even)
	{
		if(invert) //even, invert --> UP
		{
			for(int i = row-2; i >=0; i--)
			{
				if(left[i] == id) num_pins_found++;
				if(right[i] == id) num_pins_found++;					
			}
			//printf("num_pins_found: %d line 1286\n", num_pins_found);
			for(int i = 0; i < top.size(); i++)
				if(top[i] == id) num_pins_found++;
			//printf("num_pins_found: %d line 1289\n", num_pins_found);
		}
		else //even, !invert --> DOWN
		{
			for(int i = row; i < left.size(); i++)
			{
				if(left[i] == id) num_pins_found++;
				if(right[i] == id) num_pins_found++;					
			}
			
			for(int i = 0; i < bot.size(); i++)
				if(bot[i] == id) num_pins_found++;
		}
	}
	else
	{
		if(invert) //!even, invert --> LEFT
		{
			for(int i = col-2; i >=0; i--)
			{
				if(top[i] == id) num_pins_found++;
				if(bot[i] == id) num_pins_found++;					
			}
			
			for(int i = 0; i < left.size(); i++)
				if(left[i] == id) num_pins_found++;
		}
		else //!even, !invert --> RIGHT
		{
			for(int i = col; i < top.size(); i++)
			{
				if(top[i] == id) num_pins_found++;
				if(bot[i] == id) num_pins_found++;					
			}
			
			for(int i = 0; i < right.size(); i++)
				if(right[i] == id) num_pins_found++;
		}
	}
	//printf("num_pins_found: %d\n", num_pins_found);
	return num_pins_found;	
}

int Switchbox :: getNumSegsInDirection(int row, int col, bool even, bool invert, Net* net)
{
	int num_segs_found = 0;
	//int id = net->net_id;
	
	if(printing) printf("getNumSegsInDirection(row:%d, col:%d, even:%d, invert%d, net:%d)\n", row, col, even, invert, net->net_id);
	
	if(even)
	{
		if(invert) //even, invert --> UP
		{
			for(int i = row-1; i >= 0; i--)
			{
				//int j = col;
				for(int j = 0; j <= top.size(); j++)
				{
					if(Hsegs[i][j]->hasNet(net)) num_segs_found++;
					if(Vsegs[i][j]->hasNet(net)) num_segs_found++;
				}
			}						
		}
		else //even, !invert --> DOWN
		{
			for(int i = row+1; i <= left.size(); i++)
			{
				//int j = col;
				for(int j = 0; j <= top.size(); j++)
				{
					if(Hsegs[i][j]->hasNet(net)) num_segs_found++;
					if(Vsegs[i][j]->hasNet(net)) num_segs_found++;
				}
			}			
		}
	}
	else
	{
		if(invert) //!even, invert --> LEFT
		{
			for(int i = col-1; i >=0; i--)
			{
				int j = row; //for(int j = 0; j <= left.size(); j++)
				{
					if(Hsegs[j][i]->hasNet(net)) num_segs_found++;
					if(Vsegs[j][i]->hasNet(net)) num_segs_found++;					
				}
			}			
		}
		else //!even, !invert --> RIGHT
		{
			for(int i = col+1; i <= top.size(); i++)
			{
				int j = row; //for(int j = 0; j <= left.size(); j++)
				{
					if(Hsegs[j][i]->hasNet(net)) num_segs_found++;
					if(Vsegs[j][i]->hasNet(net)) num_segs_found++;					
				}					
			}
		}
	}
	
	num_segs_found += getNumPinsInDirection(row, col, even, invert, net);
	if(printing) printf("num_segs_found: %d\n", num_segs_found);
	return num_segs_found;
}

//counts the number of empty rows or columns in the direction specified
int Switchbox :: getNumFreeSpacesInDirection(int row, int col, bool even, bool invert, Net* net)
{

	int num_free_spaces_found = 0;
	
	if(printing) printf("getNumFreeSpacesInDirection(row:%d, col:%d, even:%d, invert%d, net:%d)\n", row, col, even, invert, net->net_id);
	
	if(even)
	{
		if(invert) //even, invert --> UP
		{
			for(int i = row-1; i > 0; i--)
			{				
				if(Hsegs[i][col]->nets.size() == 0) num_free_spaces_found++;
				//if(Vsegs[i][col]->nets.size() == 0) num_free_spaces_found++;			
			}						
		}
		else //even, !invert --> DOWN
		{
			for(int i = row+1; i <= left.size(); i++)
			{				
				if(Hsegs[i][col]->nets.size() == 0) num_free_spaces_found++;
				//if(Vsegs[i][col]->nets.size() == 0) num_free_spaces_found++;			
			}			
		}
	}
	else
	{
		if(invert) //!even, invert --> LEFT
		{
			for(int j = col-1; j > 0; j--)
			{				
				//if(Hsegs[row][j]->nets.size() == 0) num_free_spaces_found++;
				if(Vsegs[row][j]->nets.size() == 0) num_free_spaces_found++;			
			}			
		}
		else //!even, !invert --> RIGHT
		{
			for(int j = col+1; j <= top.size(); j++)
			{				
				//if(Hsegs[row][j]->nets.size() == 0) num_free_spaces_found++;
				if(Vsegs[row][j]->nets.size() == 0) num_free_spaces_found++;			
			}
		}
	}
	if(printing) printf("num_free_spaces_found: %d\n", num_free_spaces_found);
	return num_free_spaces_found;
}

void Switchbox :: increaseSize()
{
	//check if it's a channel
	
	
	//SHOULD RANDOMLY INCREASE ON TOP OR BOT
	//OR LEFT OR RIGHT
	//this minimizes corners with nothing in them
	
	if(getEmptySpaceBelow()*left.size() < getEmptySpaceRight()*top.size())
		addRow();						
	else	
		addCol();
	
	printSwitchbox();
}

int Switchbox :: getEmptySpaceBelow()
{
	int empty_spaces = 0;
	for(int row = orig_height; row < Hsegs.size()-1; row++)	
		for(int col = 0; col < Hsegs[row].size(); col++)		
			if(Hsegs[row][col]->nets.size() == 0)
			{	empty_spaces++;  }
	
	for(int row = orig_height; row < Vsegs.size(); row++)	
		for(int col = 1; col < Vsegs[row].size()-1; col++)		
			if(Vsegs[row][col]->nets.size() == 0)
			{	empty_spaces++;  }
		
	if(printing) printf("Empty spaces below = %d\n", empty_spaces);
	return empty_spaces;
}

int Switchbox :: getEmptySpaceRight()
{
	int empty_spaces = 0;
	for(int col = orig_width; col < Hsegs[0].size(); col++)	
		for(int row = 1; row < Hsegs.size()-1; row++)		
			if(Hsegs[row][col]->nets.size() == 0)
				empty_spaces++;
	for(int col = orig_width; col < Vsegs[0].size()-1; col++)	
		for(int row = 0; row < Vsegs.size(); row++)		
			if(Vsegs[row][col]->nets.size() == 0)
				empty_spaces++;
	
	if(printing) printf("Empty spaces right = %d\n", empty_spaces);
	return empty_spaces;
}

void Switchbox :: addRow()
{
	printf("&&&&&&ADDROW()&&&&&&\n");
	
	//count segs in top half and bot half
	int segs_in_top_half = 0;
	int segs_in_bot_half = 0;
	
	for(int col = 0; col < Hsegs[0].size(); col++)
	{
		for(int row = 0; row < Hsegs.size() / 2; row++)
		{
			segs_in_top_half += Hsegs[row][col]->nets.size();
		}
		
		for(int row = Hsegs.size()/2 + (Hsegs.size() % 2); row < Hsegs.size(); row++)
		{
			segs_in_bot_half += Hsegs[row][col]->nets.size();
		}
	}
	
	for(int col = 0; col < Vsegs[0].size(); col++)
	{
		for(int row = 0; row < Vsegs.size() / 2; row++)
		{
			segs_in_top_half += Vsegs[row][col]->nets.size();
		}
		
		for(int row = Vsegs.size()/2 + (Vsegs.size() % 2); row < Vsegs.size(); row++)
		{
			segs_in_bot_half += Vsegs[row][col]->nets.size();
		}
	}
		
	
	bool add_top = true; //indicates if adding row on the top or bot
	int row = 0;
	
	
	//if more segs in left half, add col left
	if(segs_in_top_half > segs_in_bot_half)
	{
		left.insert(left.begin(), 0);
		right.insert(right.begin(), 0);
		
		row = 0;
		add_top = true;
		//if(printing) 
		printf("Add row top!\n");
	}	
	//else, if more segs in right half, add col right
	else 
	{	
		left.push_back(0);
		right.push_back(0);
		
		row = left.size();
		add_top = false;
		//if(printing) 
		printf("Add row bot!\n");
	}
	
	vector<Segment*> Hrow;
	for(int j = 0; j < Hsegs[0].size(); j++)
	{
		Segment* Hseg = new Segment(j, row+(add_top ? 0:1), j+1, row+(add_top ? 0:1));
		printf("Added Hseg (r%d, c%d)\n", row, j);
		Hrow.push_back(Hseg);			
	}
	
	if(add_top)
		Hsegs.insert(Hsegs.begin(), Hrow);
	else Hsegs.push_back(Hrow);
	
	vector<Segment*> Vrow;
			
	for(int j = 0; j < Vsegs[0].size(); j++)
	{
		Segment* Vseg = new Segment(j, row, j, row+1);
		
		printf("Added Vseg (r%d, c%d)\n", row, j);
		
		//update columns to keep nets connected to pins
		if(j > 0 && j-1 < bot.size() )
		if((add_top ? top[j-1]:bot[j-1]) != 0)
		for(int a = 0; a < nets.size(); a++)
		if(nets[a]->net_id == (add_top ? top[j-1]:bot[j-1]))
		{
			Vseg->addNet(nets[a]);	
			printf("this Vseg is on net %d!\n", nets[a]->net_id);		
			break;
		}
		
		Vrow.push_back(Vseg);			
	}	
	
	if(add_top)
		Vsegs.insert(Vsegs.begin(), Vrow);
	else Vsegs.push_back(Vrow);
	
	
	//IN ORDER TO ADD TOP ROW OR LEFT COL, ALL SEGMENTS MUST BE MOVED!@!@#!@#
	if(add_top)
	{
		for(int col = 0; col < Vsegs[0].size(); col++)
		{
			for(int row = 1; row < Vsegs.size(); row++)
			{
				Vsegs[row][col]->y1++;
				Vsegs[row][col]->y2++;
			}
		}
		
		for(int col = 0; col < Hsegs[0].size(); col++)
		{
			for(int row = 1; row < Hsegs.size(); row++)
			{
				Hsegs[row][col]->y1++;
				Hsegs[row][col]->y2++;
			}
		}
	}
	//if(printing) 
	printSwitchbox();
	
	//exit(1);	
}


void Switchbox :: addCol()
{
	printf("&&&&&&ADDCOL()&&&&&&\n");
	
	//count segs in left half and right half
	int segs_in_left_half = 0;
	int segs_in_right_half = 0;
	
	for(int row = 0; row < Hsegs.size(); row++)
	{
		for(int col = 0; col < Hsegs[row].size() / 2; col++)
		{
			segs_in_left_half += Hsegs[row][col]->nets.size();
		}
		
		for(int col = Hsegs[row].size()/2 + (Hsegs[row].size() % 2); col < Hsegs[row].size(); col++)
		{
			segs_in_right_half += Hsegs[row][col]->nets.size();
		}
	}
	
	for(int row = 0; row < Vsegs.size(); row++)
	{
		for(int col = 0; col < Vsegs[row].size() / 2; col++)
		{
			segs_in_left_half += Vsegs[row][col]->nets.size();
		}
		
		for(int col = Vsegs[row].size()/2 + (Vsegs[row].size() % 2); col < Vsegs[row].size(); col++)
		{
			segs_in_right_half += Vsegs[row][col]->nets.size();
		}
	}
	
	bool add_left = true; //indicates if adding column on the left or right
	int col = 0;
	
	
	//IN ORDER TO ADD TOP ROW OR LEFT COL, ALL SEGMENTS MUST BE MOVED!@!@#!@#
		
	
	//if more segs in left half, add col left
	/*if(segs_in_left_half > segs_in_right_half)
	{
		top.insert(top.begin(), 0);
		bot.insert(bot.begin(), 0);
		
		col = 0;
		add_left = true;
		if(printing) printf("Add col left!\n");
	}	
	//else, if more segs in right half, add col right
	else*/
	{	
		top.push_back(0);
		bot.push_back(0);
		col = top.size();
		add_left = false;
		if(printing) printf("Add col right!\n");
	}
		
	for(int i = 0; i < Hsegs.size(); i++)
	{ 
		Segment* Hseg = new Segment(col, i, col+1, i);
		
		//update rows to keep nets connected to pins
		if(i > 0 && i-1 < left.size())
		if((add_left ? left[i-1] : right[i-1]) != 0)
		for(int a = 0; a < nets.size(); a++)
		if(nets[a]->net_id == (add_left ? left[i-1] : right[i-1]))
		{
			Hseg->addNet(nets[a]);
			break;
		}
		
		if(add_left)
			Hsegs[i].insert(Hsegs[i].begin(), Hseg);
		else Hsegs[i].push_back(Hseg);
	}
	
	for(int i = 0; i < Vsegs.size(); i++)
	{ 
		Segment* Vseg = new Segment(col+(add_left? 0:1), i, col+(add_left? 0:1), i+1);
		
		if(add_left)
			Vsegs[i].insert(Vsegs[i].begin(), Vseg);
		else Vsegs[i].push_back(Vseg);				
	}	
	
	//IN ORDER TO ADD TOP ROW OR LEFT COL, ALL SEGMENTS MUST BE MOVED!@!@#!@#
	if(add_left)
	{
		for(int row = 0; row < Vsegs.size(); row++)
		{
			for(int col = 1; col < Vsegs[row].size(); col++)
			{
				Vsegs[row][col]->x1++;
				Vsegs[row][col]->x2++;
			}
		}
		
		for(int row = 0; row < Hsegs.size(); row++)
		{
			for(int col = 1; col < Hsegs[row].size(); col++)
			{
				Hsegs[row][col]->x1++;
				Hsegs[row][col]->x2++;
			}
		}
	}
	
	//if(printing) 
	printSwitchbox();
	
	//exit(1);
}


//NOT FINISHED
bool Switchbox :: conflictsExist()
{
	bool conflict_exists = false;
	for(int row = 0; row < Hsegs.size(); row++)
	{
		for(int col = 0; col < Hsegs[row].size(); col++)
		{
			if(hasConflicts(Hsegs[row][col]))
			{
				if(printing) printf("Conflict exists on Hseg (r%d, c%d)\n", row, col);
				conflict_exists = true;
			}
		}
	}
	
	for(int row = 0; row < Vsegs.size(); row++)
	{
		for(int col = 0; col < Vsegs[row].size(); col++)
		{
			if(hasConflicts(Vsegs[row][col]))
			{
				if(printing) printf("Conflict exists on Vseg (r%d, c%d)\n", row, col);
				conflict_exists = true;
			}
		}
	}
	
	return conflict_exists;
}

int Switchbox :: hasConflicts(Segment* seg)
{
	int col = seg->x1;
	int row = seg->y1;
	
	if(seg->nets.size() == 0) //if no nets on seg, no conflicts!
		return false;
		
	if(seg->nets.size() > 1) //if more than 1 net, conflicts!
		return true;
		
	if(seg->isVertical())
	{
		//check for via conflict above
		if(row > 0)
		for(int i = 0; i < Vsegs[row-1][col]->nets.size(); i++)
			if(Vsegs[row-1][col]->nets[i] != seg->nets[0])
				return true;
				
		//check for via conflict below
		if(row < Vsegs.size()-1)
		for(int i = 0; i < Vsegs[row+1][col]->nets.size(); i++)
			if(Vsegs[row+1][col]->nets[i] != seg->nets[0])
				return true;
		
	}
	else
	{
		//check for via conflict left
		if(col > 0)
		for(int i = 0; i < Hsegs[row][col-1]->nets.size(); i++)
			if(Hsegs[row][col-1]->nets[i]->net_id != seg->nets[0]->net_id)
				return true;
				
		//check for via conflict right
		if(col < Hsegs[row].size()-1)
		for(int i = 0; i < Hsegs[row][col+1]->nets.size(); i++)
			if(Hsegs[row][col+1]->nets[i]->net_id != seg->nets[0]->net_id)
				return true;
	}
	
	return false;
}

bool Switchbox :: netBenefit(Segment* seg, Net* net, bool even, bool invert)
{
	return netBenefit(seg, net, even, invert, 0); //10% chance if none is specified
}

//returns true if there is a net benefit to a move in the direction proposed
//e.g. a move up will reduce the total number of conflicts in the layout
bool Switchbox :: netBenefit(Segment* seg, Net* net, bool even, bool invert, int percent_chance)
{ 
	if(printing)
		printf("Begin netBenefit() seg (r%d, c%d) \t net %d \t even:%d \t invert:%d \t percent: %d\n", seg->y1, seg->x1, net->net_id, even, invert, percent_chance);
		
	int num_conflicts_removed = 0;
	int num_conflicts_added = 0;
	
	int col = seg->x1;
	int row = seg->y1;
	
	if(row == 0 || col == 0 || row > left.size() || col > top.size())
		return false;
	
	if((even) && col == top.size())
		return false;
	
	if((!even) && row == left.size())
		return false;
		
	if(even) //UP or DOWN
	{
		
			//check if removing a segment conflict
			for(int i = 0; i < Hsegs[row][col]->nets.size(); i++)
				if(Hsegs[row][col]->nets[i] != net)
					num_conflicts_removed++;
				
			//check if removing via conflicts
			if(col > 0)
			for(int i = 0; i < Hsegs[row][col-1]->nets.size(); i++)
				if(Hsegs[row][col-1]->nets[i] != net)
					num_conflicts_removed++;
			if(col < Hsegs.size()-1)
			for(int i = 0; i < Hsegs[row][col+1]->nets.size(); i++)
				if(Hsegs[row][col+1]->nets[i] != net)
					num_conflicts_removed++;
					
			//check if removing a segment conflict			
			for(int i = 0; i < Hsegs[row+(invert? -1:1)][col]->nets.size(); i++)
				if(Hsegs[row+(invert? -1:1)][col]->nets[i] != net)
					num_conflicts_added++;
				
			//check if removing via conflicts
			if(col > 0)
			for(int i = 0; i < Hsegs[row+(invert? -1:1)][col-1]->nets.size(); i++)
				if(Hsegs[row+(invert? -1:1)][col-1]->nets[i] != net)
					num_conflicts_added++;
			if(col < Hsegs.size()-1)
			for(int i = 0; i < Hsegs[row+(invert? -1:1)][col+1]->nets.size(); i++)
				if(Hsegs[row+(invert? -1:1)][col+1]->nets[i] != net)
					num_conflicts_added++;
						
	}
	else //!even LEFT or RIGHT
	{
			//check if removing a segment conflict
			for(int i = 0; i < Vsegs[row][col]->nets.size(); i++)
				if(Vsegs[row][col]->nets[i] != net)
					num_conflicts_removed++;
				
			//check if removing via conflicts
			if(row > 0)
			for(int i = 0; i < Vsegs[row-1][col]->nets.size(); i++)
				if(Vsegs[row-1][col]->nets[i] != net)
					num_conflicts_removed++;
			if(row < Vsegs.size()-1)
			for(int i = 0; i < Vsegs[row+1][col]->nets.size(); i++)
				if(Vsegs[row+1][col]->nets[i] != net)
					num_conflicts_removed++;
					
			//check if removing a segment conflict			
			for(int i = 0; i < Vsegs[row][col+(invert? -1:1)]->nets.size(); i++)
				if(Vsegs[row][col+(invert? -1:1)]->nets[i] != net)
					num_conflicts_added++;
				
			//check if removing via conflicts
			if(row > 0)
			for(int i = 0; i < Vsegs[row-1][col+(invert? -1:1)]->nets.size(); i++)
				if(Vsegs[row-1][col+(invert? -1:1)]->nets[i] != net)
					num_conflicts_added++;
			if(row < Vsegs.size()-1)
			for(int i = 0; i < Vsegs[row+1][col+(invert? -1:1)]->nets.size(); i++)
				if(Vsegs[row+1][col+(invert? -1:1)]->nets[i] != net)
					num_conflicts_added++;
	}
	if(printing)
	printf("net %d benefit of moving %d%d seg at (r%d, c%d): %d\n", net->net_id, even, invert, row, col, num_conflicts_removed - num_conflicts_added);
	
	if( num_conflicts_removed >= num_conflicts_added)
		return true;
	else
	{	//even if this is a "bad" move that causes more conflicts, 30% chance to still take it
		if(rand()%100 < percent_chance)
			return true;
		else
			return false;
	}
	
}

//main driver functions	
bool switchBoxRoute(string benchmark_name)
{
	
	srand(time(0)); //use current time for random seed
	
	Switchbox sb(benchmark_name);
		
	//BEGIN ALGORITHM		
		
	sb.routeAllNets();
	
	
	//printf("Total Wirelength: %d\n", sb.totalWirelength());
				
	
	sb.printSwitchbox();
	
	//sb.resolveAllConflicts();
	
	//until all conflicts are resolved, increase the size of the switchbox
	int num_size_increases = 0;
	while(!sb.resolveAllConflicts())
	{
		sb.increaseSize();
		printf("\nTotal number of size increases: %d\n", ++num_size_increases);
		
	}
	
	
	
	sb.printAllNets();
		
	sb.printSwitchbox();
	
	printf("\nROUTING QUALITY INFORMATION\n");
	printf("Number of rows: %d (%d added)\n", sb.Hsegs.size()-2, sb.Hsegs.size()-2-sb.orig_height);
	printf("Number of cols: %d (%d added)\n", sb.Vsegs[0].size()-2, sb.Vsegs[0].size()-2-sb.orig_width);
	printf("Number of vias: %d\n", sb.placeVias());
	printf("Total wireleng: %d\n", sb.totalWirelength());
	
	
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


