Network Programming Project1, readme file
Name: Xu Chai

Notes:
This project implemented a routing table as a binary tree in Node.h and Node.cc file.
 
In part 1.cc, the program read routes, arp, and pdus file, and generated the results by checking routing table.
The expected command line is:   ./part1.out routes.txt arp.txt pdus.txt

In part2.cc, based on the result of part1, it also read nat file and translated the source address. By parsing data to nat_test.rb file, the nat test results were generated and the program received the response.
The expected command line is: ruby nat_test.rb "./part2.out routes.txt arp.txt pdus.txt nat.txt"


References & Collaborators:
www. stackoverflow.com
www.cplusplus.com