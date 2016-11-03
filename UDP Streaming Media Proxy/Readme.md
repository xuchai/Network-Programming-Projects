#UDP Streaming Media Proxy

This project creates a UDP streaming media proxy server, similar to a SIP proxy used for VoIP.
## Description
Please check and read the UDP Streaming Media Proxy.pdf
## Structures
* proj2.cc: The main program that handles registration (keeping track of  users' IP address), call setup (aceepting calls and notifying proxy users when they have an incoming call), and media passthrough (passing media packets between users who are engaged in a call).
* sample_client.js:Test client for the project
## Deployment
Download the zip file and place it in a path you want
## Compile and Run
* To Compile:
```
proj2.cc -o proj2.out -pthread -std=c++11
```
* To Run:
```
./proj2.out
```
## Sample Test and Output

input: node p2_sample_client.js localhost Amy
return: Received ACK_REGISTER 

input: node p2_sample_client.js localhost Amy Tom
return: Call failed!

intput: node p2_sample_client.js localhost Tom
return: Received ACK_REGISTER

intput: node p2_sample_client.js localhost Tom Amy
return: Call accepted!

intput: node p2_sample_client.js localhost Amy Tom
return: Call accepted!
        media from Amy: Hi there, this is Amy
## Built With
C++, Dual-stack sockets, POSIX Threads
## Contributer
Chelsey Chai
