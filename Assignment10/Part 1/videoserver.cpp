#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include <arpa/inet.h>
#include <opencv2/opencv.hpp>

#define MAXLINE 65535  
void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]){
    int PORT=8790;
    PORT=atoi(argv[1]);
  int socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
  if (socketDescriptor < 0) error("ERROR opening socket");
  
  struct sockaddr_in  serverAddress,clientAddress;
  socklen_t addressLength;
  char message[MAXLINE];
  	
  bzero(&serverAddress, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr=INADDR_ANY;
  serverAddress.sin_port=htons(PORT);

  if (bind(socketDescriptor, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
        error("ERROR on binding");
        
struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 200000;
    setsockopt(socketDescriptor, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    
cv::VideoCapture capture(0);
    if (!capture.isOpened()) {
        error("ERROR opening webcam");
    }
    
    printf("video server started on port %d\n", PORT);

    std::vector<int> encode_params = {cv::IMWRITE_JPEG_QUALITY, 40};
    
  while(1){	
  
  addressLength = sizeof(clientAddress);
        bzero(message, MAXLINE);
        
	int n= recvfrom(socketDescriptor,message,MAXLINE,0,(struct sockaddr*)&clientAddress,&addressLength);

if (n<0){
            continue; 
        }
     
cv::Mat frame;
capture >> frame;
if (frame.empty()) continue;   

std::vector<uchar> encoded_img;
cv::imencode(".jpg", frame, encoded_img, encode_params);

if (!encoded_img.empty()) {
            sendto(socketDescriptor, encoded_img.data(), encoded_img.size(), 0, (struct sockaddr*)&clientAddress, addressLength);
        }   
}
  
  capture.release();
  return 0;
}
