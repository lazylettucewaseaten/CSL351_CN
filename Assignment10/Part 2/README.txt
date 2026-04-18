Author: Ashish Ranjan
Student ID: 12340370
Course: CSL351

====================================================
Question 1.

Files : q1server.c , q1client.c

COMPILATION : 
gcc q1server.c -o q1server
gcc q1client.c -o q1client 

RUNNING : 
./q1server 9000
./q1client 127.0.0.1 9000 10 500 64

====================================================


====================================================
Question 2.

Files : q2server.c , q2client.c ,q2plot.py

COMPILATION : 
gcc q2server.c -o q2server   
gcc  q2client.c -o q2client       

RUNNING : 
./q2server 9000
./q2client 127.0.0.1 9000 10 100 1024 > results.csv

python3 q2plot.py results.csv

====================================================


====================================================
Question 3.

Files : q3traceroute.c

COMPILATION : 
gcc q3traceroute.c -o q3traceroute                                                               

RUNNING : 
sudo ./q3traceroute google.com

====================================================
