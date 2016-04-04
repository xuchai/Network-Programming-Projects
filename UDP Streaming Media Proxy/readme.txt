Network Programming Project2, UDP Streaming Media Proxy, readme file
Name: Xu Chai

Notes:
This project created a UDP streaming media proxy. I used a dual-stack sockets as mentioned but failed to bind it to both IPv4 and IPv6 addresses, so the proxy now only support IPv6 address. Other part should be fine and tested. Some output in the console was kept to have a better understanding of the back part of media proxy.

Run and Tests:
To run the Proxy, compile with g++ proj2.cc -o proj2.out -pthread -std=c++11
then do:  ./proj2.out
the server is now started

tested with: 
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

References & Collaborators:
www.stackoverflow.com
www.cplusplus.com