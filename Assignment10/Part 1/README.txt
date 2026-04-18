Author: Ashish Ranjan
Student ID: 12340370
Course: CSL351

================================================================
IMPORTANT : (In case opencv library is not installed)

sudo apt update
sudo apt install libopencv-dev pkg-config
================================================================

==============================================
DNS Server TCP Based
File Name : Server.c

COMPILATION : 
gcc Server.c -o server

RUNNING : 
./server 8080

==============================================


==============================================
Video Server UDP Based
File Name : videoserver.cpp

COMPILATION : 
g++ videoserver.cpp -o videoserver `pkg-config --cflags --libs opencv4`

RUNNING : 
./videoserver 8190

==============================================


==============================================
Client TCP UDP Based
File Name : client.cpp

COMPILATION : 
g++ client.cpp -o client `pkg-config --cflags --libs opencv4`         

RUNNING : 
./client 127.0.0.1 8080 8190 video.server.com

================================================
