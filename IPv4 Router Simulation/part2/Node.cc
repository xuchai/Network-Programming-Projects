// Implementation of the Node class
# include <iostream>
# include <string>
# include <vector>
# include "Node.h"

using namespace std;

// convert a string address into a vector of int for further comparisions
// i.e. 192.168.2.0/24, return vector(192, 168, 2, 0), regardless of prefix here (used later in matches function)
vector<int> convert_to_vec(const string& str) {   
	vector<int> vec;
	string tmp = " ";  // store each character in 4 bytes
	int counter  = 0;   // count the index in each vector
	for (int i = 0; i < str.size(); ++i) {
		if (str[i] >= '0' && str[i] <= '9') tmp += str[i];
		if (str[i] == '.') {
			vec.push_back(stoi(tmp));
			++counter;
			tmp = " ";
		}
		if (counter == 3) break;  
	}
	return vec;
}

// compare two destination strings
// i.e. str1 = 10.3.0.0/16, str2 = 192.168.1.0/24, then return true.
bool less_than (const string& str1, const string& str2) {
	vector<int> vec1 = convert_to_vec(str1);
	vector<int> vec2 = convert_to_vec(str2);
	// compare two arrays from each column
	for (int i = 0; i < 4; ++i) {
		if (vec1[i] < vec2[i]) return true;
		else if (vec1[i] > vec2[i]) return false;
		else continue;
	}
	return false;   // defaulted
}

// define if str2(IP address from PDU) belongs to str1(routing table dest address)
// i.e. str1 = 10.3.0.0/16, str2 = 10.3.0.1, then return true.
bool matches (const string & str1, const string& str2) {
	vector<int> vec1 = convert_to_vec(str1);
	vector<int> vec2 = convert_to_vec(str2);
	// find the prefix in str1 (routing table)
	string tmp = " ";   
	bool found = false;
	for (int i = 0; i < str1.size(); ++i) {
		if (str1[i] == '/') { 
			found = true;
			continue;
		}
		if (found) tmp += str1[i];
	}
	int prefix = stoi(tmp)/8;   // get the longest bytes of each routing table address
	for (int i = 0; i < prefix; ++i) {
		if(vec1[i] != vec2[i]) return false;
	}
	return true;
}

// insert a new Node into binary tree
bool insert (const string& str1, const string& str2, const string& str3, Node* &root) {
	if ( !root) {
		root  = new Node(str1, str2, str3);
		return true;
	}
	else if (less_than(str1, root->getDest())) {
		return insert(str1, str2, str3, root->left);
	} 
	else {
		return insert(str1, str2, str3, root->right);
	}
	return false; 
}

// find a Node from binary tree
Node* find (const string& str, Node* root) {
	if ( !root) return NULL;
	if (matches(root->getDest(), str)) return root;
	if (less_than(root->getDest(), str)) return find(str, root->right);
	else return find(str, root->left);
}

// destory binary tree in case of memory leak
void destroy_tree(Node* p) {
	if (!p) return;
	destroy_tree(p->left);
	destroy_tree(p->right);	
	delete p;
}





















