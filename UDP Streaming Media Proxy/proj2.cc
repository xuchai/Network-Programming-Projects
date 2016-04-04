/* 
 * project2 file
 * UDP Streaming Media Proxy
 * To compile run as
 * g++ proj2.cc -o proj2.out -pthread -std=c++11
 */

#include <iostream>  
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <map>
#include <vector>
#include <cstdlib>
#include <pthread.h>

#define BUF_SIZE 64

typedef std::map<std::string, std::string > myMap;
myMap addr_received; // recording the addresses that already have recived packets


// struct of passing arguments when calling pthread function
struct arg_struct {
	int sockfd;
	std::string peer_addr;
	std::string dest_addr;
	std::string peer_user;
	std::string dest_user;
};

// function that send a socket
void send_socket (int sockfd, std::string msg, const char* host, const char* svc) {
	int ret;
	size_t msg_len = msg.size();

	struct addrinfo ai_hints;
	struct addrinfo *ai_results;

	memset(&ai_hints, 0, sizeof(ai_hints));
	ai_hints.ai_family = AF_INET6;
	ai_hints.ai_socktype = SOCK_DGRAM;
	ai_hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG;

	ret = getaddrinfo(host, svc, &ai_hints, &ai_results);

	std::cout << "respond msg: " << msg << std::endl;
	ret = sendto(sockfd, msg.c_str(), msg_len, 0, ai_results->ai_addr, ai_results->ai_addrlen);
	if (ret == -1) {
		perror("sendto");
		return;
	}
}

// pthread function, set media path between two clients
void *set_media_path(void *arguments) {
	std::cout << "inside set_media_path thread" << std::endl;
	struct arg_struct *args = (struct arg_struct *)arguments;

	int ret;
	struct sockaddr_in6 src;
	socklen_t srclen;
	ssize_t size;
	char host[80], svc[80];
	char buf[BUF_SIZE];
	bool peer_known = false, dest_known = false; // set to true when found in addr_recevied map
	std::string dest_port;
	std::vector<std::string> peer_bufs;

	// keep listening to the server
	while(1) {
		srclen = sizeof(src);
		size = recvfrom(args->sockfd, buf, BUF_SIZE, 0, (struct sockaddr*)&src, &srclen);
		if (size == -1) {
			perror("recvfrom");
			return NULL;
		}
		ret = getnameinfo((struct sockaddr*)&src, sizeof(src), host, sizeof(host), svc, sizeof(svc), 0);
		if (ret != 0) {
			fprintf(
				stderr, "getnameinfo() failed: %s\n", 
				gai_strerror(ret)
			);
		}
		else {
			printf("inside thread received from %s (%s):", host, svc);
			std::cout << buf << std::endl;
			peer_bufs.push_back(std::string(buf));

			// check if peer_user's port is known or not
			myMap::iterator itr = addr_received.find(args->peer_user);
			if (itr == addr_received.end()) {
				// check if port number have been used or not
				bool port_used = false;
				for (itr = addr_received.begin(); itr != addr_received.end(); ++itr) {
					if (itr->second == std::string(svc)) {
						port_used = true;
					}
				}
				if (!port_used) {
					std::cout << "inserted peers here: " << args->peer_user << " " << svc << std::endl;
					addr_received.insert(std::make_pair(args->peer_user, std::string(svc)));
				}				
			}
			else {
				peer_known = true;
			}

			// check if dest_user's port is known or not
			itr = addr_received.find(args->dest_user);
			if (itr != addr_received.end()) {
				dest_known = true;
				dest_port = itr->second;
			}

			// only send packets when both sides' port are known
			if (peer_known && dest_known) {
				std::cout << "both peers known" << std::endl;
				// send all the packets of peer_bufs
				for (int i = 0; i < peer_bufs.size(); ++i) {
					std::cout << "send packets to " << dest_port << " : " << peer_bufs[i] << std::endl;
					send_socket(args->sockfd, peer_bufs[i], args->dest_addr.c_str(), dest_port.c_str());
				}
			
				// clear vector of bufs when finish sending packets
				peer_bufs.clear();
			}
			else {
				std::cout << "one side unknown." << std::endl;
			}
		}
	}

}

int main() {
	std::cout <<"project2 server start..." << std::endl;
	int sockfd, sockfd_media, ret;
	struct sockaddr_in6 src, bindaddr;
	socklen_t srclen;
	ssize_t size;
	char host[80], svc[80], peer_user[20], dest_user[20];
	char buf[BUF_SIZE];
	std::string re_msg, peer_addr, dest_addr;
	myMap peer_map;
	bool peer_known, dest_known;  // set to true when found in peer_map

	// create a UDP socket
	sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		perror("socket");
		return 1;
	}

	// create an IPv6 address to receive sockets
	// tried to use dual stack, but failed in bind() function
	// so this project only support IPv6 addresses
	memset(&bindaddr, 0, sizeof(bindaddr));
	bindaddr.sin6_family = AF_INET6;
	bindaddr.sin6_port = htons(34567);
	memcpy(&bindaddr.sin6_addr, &in6addr_any, sizeof(in6addr_any));

	// bind the socket to addr port
	if (bind(sockfd, (struct sockaddr *)&bindaddr, sizeof(bindaddr)) != 0) {
		perror("bind");
		return 1;
	}

	// keep track of users' IP addresses
	while(1) {
		// listen to the server and receive register msgs
		srclen = sizeof(src);
		size = recvfrom(sockfd, buf, BUF_SIZE, 0, (struct sockaddr*)&src, &srclen);
		if (size == -1) {
			perror("recvfrom");
			return 1;
		}

		// get the addr name info
		ret = getnameinfo((struct sockaddr*)&src, sizeof(src), host, sizeof(host), svc, sizeof(svc), 0);
		if (ret != 0) {
			fprintf(stderr, "getnameinfo() failed: %s\n", gai_strerror(ret));
		}
		else {
			printf("received from %s (%s):", host, svc);
			std::cout << buf << std::endl;
			// =========== part1: registration =======================
			if (sscanf(buf, "REGISTER %s", peer_user)) {
				std::cout << "success part1! " << peer_user << std::endl;

				// record the peer addr in a table
				peer_map.insert(std::make_pair(std::string(peer_user), std::string(host)));
				re_msg = "ACK_REGISTER " + std::string(peer_user);

				// send response msg about part1
				send_socket(sockfd, re_msg, host, svc);
			}
			// =========== part2: call setup =======================
			if (sscanf(buf, "CALL FROM: %s %s", peer_user, dest_user)&& sscanf(dest_user, "TO: %s", dest_user)) {
				std::cout << "success for part 2! user1: " << peer_user << " user2: " << dest_user << std::endl;
				peer_known = false; // set to true when found in table
				dest_known = false;

				// check peer table to see if the addr is known or not
				for (myMap::iterator itr = peer_map.begin(); itr != peer_map.end(); ++itr) {
					if (itr->first == std::string(peer_user)) {
						peer_known = true;
						peer_addr = std::string(host);
					}
					if (itr->first == std::string(dest_user)) {
						dest_known = true;
						dest_addr = itr->second;
					}
				}

				// set response messages
				if (peer_known && dest_known) {
					re_msg = "ACK_CALL FROM:" + std::string(peer_user) + " TO:" + std::string(dest_user);
				}
				else {
					re_msg = "CALL_FAILED FROM:" + std::string(peer_user) + " TO:" + std::string(dest_user);
				}

				// send response message about part2
				send_socket(sockfd, re_msg, host, svc);

				// =========== part3: media path setup =======================
				if (peer_known && dest_known) {
					// create a new socket
					sockfd_media = socket(AF_INET6, SOCK_DGRAM, 0);
					if (sockfd_media == -1) {
						perror("socket");
						return 1;
					}	

					// randomly generate a port number in range 5000 to 5999
					int media_port = rand() % 1000 + 5000;
					std::cout << "media_port: " << media_port << std::endl;

					// bind the socket to the media port
					bindaddr.sin6_port = htons(media_port);
					if (bind(sockfd_media,(struct sockaddr *) &bindaddr,sizeof(bindaddr)) != 0) {
						perror("bind");
						return 1;
					}

					// send response message about part3
					re_msg = "MEDIA_PORT FROM:" + std::string(peer_user) + " TO:" 
							+ std::string(dest_user) + " " + std::to_string(media_port);

					send_socket(sockfd, re_msg, host, svc);

					// finally, start a thread
					pthread_t thread;
					struct arg_struct args;
					args.sockfd = sockfd_media;
					args.peer_addr = peer_addr;
					args.dest_addr = dest_addr;
					args.peer_user = peer_user;
					args.dest_user = dest_user;

					// create a pthread and passing arguments in
					if (pthread_create(&thread, NULL, &set_media_path, (void *)&args) != 0) {
						fprintf(stderr, "pthread_create failed\n");
						return 1;
					}
				}
			}
		}
	}
}