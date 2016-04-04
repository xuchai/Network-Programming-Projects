// Header for declaration of Node class and associated functions.
# ifndef __Node_h_
# define __Node_h_

# include <iostream>
# include <string>
using namespace std;

// Node class used to implement the routing table, using data structures binary tries -_-
class Node {
public:
	// Representation
	Node() : left(NULL), right(NULL) {}
	Node(const string& str1, const string& str2, const string& str3) : left(NULL), right(NULL), dest(str1), gateway(str2), interface(str3) {}
	Node* left;
	Node* right;
	// Accessors
	const string& getDest() const {return dest; }
	const string& getGateway() const {return gateway; }
	const string& getInterface() const {return interface; }
	// Modifiers
	void setDest(const string& str) {dest = str; }
	void setGateway(const string& str) {gateway = str; }
	void setInterface(const string& str) {interface = str; }
	
private:
	// Private members
	string dest;
	string gateway;
	string interface;
};
// non-member functions
bool less_than (const string& str1, const string& str2);  
bool matches (const string & str1, const string& str2);
bool insert (const string& str1, const string& str2, const string& str3, Node* &root);
Node* find (const string& str, Node* root);
void destroy_tree(Node* p);


#endif