// Network Programming project1, part 1
# include <iostream>
# include <fstream>
# include <string>
# include <map>
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

// main function, reading and writing files
int main(int argc, char* argv[]) {
	
	// process command line arguments
	if (argc != 4) {
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

	// parse the data from pdu file, checking the routing table and ARPfile and output
	for (int i = 0; i < pdus.size(); ++i) {
		string gateway, interface, hardware_add;
		bool discarded = false;
		bool direct_connect = false;
		bool point_to_point = false;
		int ttl = stoi(pdus[i][4]);
		--ttl;
		if (ttl == 0) discarded = true;   // check ttl to see if discarded
		if (!discarded) {
			Node* result = find(pdus[i][2], root);  // check routing table to find pdu addresses
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
				hardware_add = arp.find(pdus[i][2])->second;
			}
			else hardware_add = arp.find(gateway)->second;
		}
		// output in the format: 
		// source:src_port­ > dest:dest_port via gateway (iface­l2address) ttl x
       //source:src_port­ > dest:dest_port via gateway (pppinterface) ttl x
		cout << pdus[i][1] << ":" << pdus[i][5] << "->" << pdus[i][2] << ":" << pdus[i][6];
		if (discarded) {
			cout << " discarded(TTL expired)" << endl;
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
	destroy_tree(root);
}