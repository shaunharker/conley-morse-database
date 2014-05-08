#ifndef NETWORK_H
#define NETWORK_H

#include <vector>

// A class representing a local piece of a regulatory network
class Network {
public:
	int numVariables;
	// An array storing which nodes feed-in information
	std::vector<std::vector<int> > inputs;
	// An array storing which feed-in nodes are up regulators (true)
	// and which are down-regulators (false)
	std::vector< std::vector<bool> > regulation;
	// An array storing which nodes use information
	std::vector<std::vector<int> > outputs;
	// An array storing how many thresholds there are per variable 
	// (which implies there will be one more interval, however:
	//   (0, t_1) (t_1,t_2) ... (t_{n-1}, t_n) (t_n, \infty) )
	std::vector< int > threshold_count;
private:

};


#endif