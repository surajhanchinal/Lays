#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
using namespace std;
int sockfd, portno, n;
struct sockaddr_in serv_addr;
struct hostent *server;
float theta = 0;

void setup(int argc, char *argv[]){
    if (argc < 4) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        printf("ERROR opening socket\n");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    int stat = connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if(stat < 0){
        cout<<"Error connecting\n";
    }
}

/* void print_matrix(char* lala,Eigen::Matrix4f ieg){
for(int i=0;i<4;i++){
    for(int j=0;j<4;j++){
        lala += sprintf(lala,"%f ",ieg(i,j));
    }
}
} */

void print_matrix(char* lala,Eigen::Matrix4f ieg){
/* for(int i=0;i<4;i++){
    for(int j=0;j<4;j++){
        lala += sprintf(lala,"%f ",ieg(i,j));
    }
} */
  for(int i=0;i<3;i++){
    lala += sprintf(lala,"%f , ",ieg(i,3));
  }
  lala += sprintf(lala,"%f , ",theta);
  lala += sprintf(lala,"%s  ,","ABCD");
  lala += sprintf(lala,"%s  ,  %s","1");
}


int main(int argc, char *argv[])
{
    
    Eigen::Matrix4f mat = Eigen::Matrix4f::Identity();
    mat.block<3,1>(0,3) = Eigen::Vector3f(50,6,0);
    char buffer[1024];
    
    setup(argc,argv);
    bzero(buffer,1024);
    int player = 0;
    player = atoi(argv[3]);
    while(true){
    theta += 1*(M_PI/180);
    theta = fmod(theta,2*M_PI);
    //mat.block<3,3>(0,0) = Eigen::AngleAxis<float>(theta,Eigen::Vector3f(0,1,0)).toRotationMatrix();
    //mat(0,3) = 50 + theta;
    char* lala = buffer;
    lala += sprintf(lala,"p%d:",player);
    //cout<<mat<<endl;
    print_matrix(lala,mat);
    n = write(sockfd, buffer, strlen(buffer));
    if(n < 0)
        cout<<"Error writing\n";
    bzero(buffer,1024);
    n = read( sockfd , buffer, 1024);
    if(n < 0)
        cout<<"Error reading\n";
    printf("%s\n",buffer);
    bzero(buffer,1024);
    usleep(5000);
    }
    
    bzero(buffer,256);

    close(sockfd);
    return 0;
}