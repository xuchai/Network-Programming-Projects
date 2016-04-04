// Network Programming project1, part 2
# include <iostream>
# include <fstream>
# include <string>
# include <map>
# include <set>
# include <vector>
# include <cstdlib>
# include "Node.h"
using namespace std;

// read the data from routing table and implemented it using binary tries
void ReadRoutes(istream & istr, Node* & root) {
	string dest, gateway, interface;
	while (istr >> dest >> gateway >> interface) {  
		 insert(dest, gateway, interface, root);
	}
}

// read the data from ARP file
void ReadARP(istream & istr, map<string, string> &arp) {
	string ip_address, eth_address;
	while (istr >> ip_address >> eth_address) {
		arp.insert(make_pair(ip_address, eth_address));
	}
}

// read the data from PDU header file
// each pdu header has 7 columns: 0, interface on which pdu arrived; 1, source IPv4 address
// 2, destination IPv4 address;  3, IPv4 protocol number; 4, Time to live; 
// 5, Source port number; 6, Destination port number
void ReadPdus(istream & istr, vector<vector<string> > & pdus) {
	string tmp;
	vector<string> tmp_row;
	int counter = 0; 
	while (istr >> tmp) {
		tmp_row.push_back(tmp);
		++counter;
		if (counter > 6) {
			pdus.push_back(tmp_row);
			tmp_row.clear();
			counter= 0;
		}
	}
}

// read each pdu string, check routing table, arp file and nat, determine each pdu routes and output results
void CheckRoute(vector<string>& pdus, Node* root, const map<string, string>& arp, const string& nat_address, 
map<string, vector<string> > & trans_table, vector<vector<string> > & response_string, bool nat) {
	string source_ip = pdus[1];
	string dest_ip = pdus[2];
	string source_port = pdus[5];
	string dest_port = pdus[6];
	string gateway, interface, hardware_add;
	bool discarded = false;
	bool direct_connect = false;
	bool point_to_point = false;
	int ttl = stoi(pdus[4]);
	--ttl;
	if (ttl == 0) discarded = true;   // check ttl to see if discarded
	if (!discarded) {
		Node* result = find(dest_ip, root);  // check routing table to find pdu addresses
		// if address not found, check the default address in rout table
		if (!result)  {
			result = find("0.0.0.0", root); 
			point_to_point = true;
		}
		gateway = result->getGateway();
		interface = result->getInterface();
		// if gateway 0.0.0.0 then directly connected
		if (gateway == "0.0.0.0") {
			direct_connect = true;
			hardware_add = arp.find(dest_ip)->second;
		}
		// if point to point, need to change here, nat_interface.....
		// if nat, then translate
		else if (point_to_point) {
			// store the original source and port
			vector<string> tmp;
			tmp.push_back(source_ip);
			tmp.push_back(source_port);
			tmp.push_back(dest_ip);
			tmp.push_back(dest_port);
			tmp.push_back(to_string(ttl));
			// check if already has one in translated table
			if (trans_table.insert(make_pair(source_port, tmp)).second == false) {
				int new_port = stoi(source_port) + 10000;
				source_port  = to_string(new_port);  
				trans_table.insert(make_pair(source_port, tmp));
			}
			source_ip = nat_address;
		}
		else hardware_add = arp.find(gateway)->second;
	}
	if(nat) {
		// output in the format: source:src_port­ > dest:dest_port via gateway (iface­l2address) ttl x
		cout << source_ip << ":" << source_port << "->" << dest_ip << ":" << dest_port;
		if (discarded) {
			cout << " discarded (TTL expired)" << endl;
		}
		else if (direct_connect) {
			cout << " directly connected (" << interface << "-" << hardware_add << ") ttl " << ttl << endl;
		}
		else if ( point_to_point) {
			cout << " via " << gateway << "(" << interface << ") ttl " << ttl << endl;
		}
		else {
			cout << " via " << gateway << "(" << interface << "-" << hardware_add << ") ttl " << ttl << endl;
		}		
	}
	// read the response string
	vector<string> tmp_line;
	string tmp;
	for (int i = 0; i < 7; ++i) {
		cin >> tmp;
		tmp_line.push_back(tmp);
	}
	response_string.push_back(tmp_line);
}


// main function, reading and writing files
int main(int argc, char* argv[]) {
	// process command line arguments
	if (argc != 5) {
		cerr << "ERROR: incorrect number of command line arguments" << std::endl;
		cerr << "USAGE:   " << argv[0] << " <input_file> <output_file>" << std::endl;
		exit(1);
	}
	
  // read the routes file
	ifstream istr1(argv[1]);
    if (!istr1) {
      cerr << "ERROR: could not open input file " << argv[1] << std::endl;
      exit(1);
    }
	Node * root = NULL;
	ReadRoutes(istr1,root);
	
	// read the arp file
	ifstream istr2(argv[2]);   
    if (!istr2) {
       cerr << "ERROR: could not open input file " << argv[2] << std::endl;
       exit(1);
   }
	map<string, string> arp;
	ReadARP(istr2, arp);

	// read the pdu header file
	ifstream istr3(argv[3]);
    if (!istr3) {
      cerr << "ERROR: could not open input file " << argv[3] << std::endl;
       exit(1);
    }
	vector<vector<string> > pdus;
	ReadPdus(istr3, pdus);
	
	// read the nat file
	ifstream istr4(argv[4]);
   if (!istr4) {
      cerr << "ERROR: could not open input file " << argv[4] << std::endl;
       exit(1);
    }
	string nat_interface, nat_address;
	istr4 >> nat_interface >>  nat_address;  
	
	map<string, vector<string> > trans_table; // translation table, use translated port as key in a map
	vector<vector<string> > response_string; // get the response string from ruby progress
	
	// parse the data from pdu file, checking the routing table and ARPfile and output
	for (int i = 0; i < pdus.size(); ++i) {
		CheckRoute(pdus[i], root, arp, nat_address, trans_table, response_string, true);
	}
	
	// new map with nat port as key to store the translated pdus
	map<string, vector<string> > trans_pdus;
	
	// reverse the translate table
	for (map<string, vector<string> >::iterator itr = trans_table.begin(); itr != trans_table.end(); ++itr) {
		vector<string> tmp(7);
		tmp[1] = itr->second[2];  // reverse dest ip to source ip
		tmp[2] = itr->second[0]; // reverse source ip to dest ip
		tmp[5] = itr->second[3];  // reverse dest port to source port
		tmp[6] = itr->second[1];   // reverse source port to dest port
		tmp[4] = itr->second[4];
		trans_pdus.insert(make_pair(itr->first, tmp));
		CheckRoute(tmp, root, arp, nat_address, trans_table, response_string, false); 
	}
	
	// read the response string from ruby file and output recevied package back
	for (map<string, vector<string> >::iterator itr = trans_pdus.begin(); itr != trans_pdus.end(); ++itr) {
		for (int i = 0; i < response_string.size(); ++i) {
			if (response_string[i][6] == itr->first) {
				itr->second[4] = response_string[i][4];
				break;
			}
		}
		CheckRoute(itr->second, root, arp, nat_address, trans_table, response_string, true); 
	}
	// delete routing table binary tree in case of memory leak
	destroy_tree(root);
}