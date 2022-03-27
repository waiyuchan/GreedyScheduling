#ifndef utils_hpp
#define utils_hpp

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <math.h>
#include <algorithm>

using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::fstream;
using std::ios;
using std::pair;
using std::string;
using std::unordered_map;
using std::vector;
using std::to_string;
using std::count;

bool getQos(const string path, unordered_map<string, vector<int> > &qos);

bool getQosWithEdgeNode(const string path, unordered_map<string, unordered_map<string, int> > &qos_with_edge_node);

bool getQosConfig(const string path, int &qos_threshold);

bool getDemand(const string path, vector<unordered_map<string, int> > &demands);

bool getBandWidth(const string path, unordered_map<string, int> &site_bandWidth);

void writeAnswer(const string path, string &answer);

#endif /* abc_hpp */
