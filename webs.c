#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>


char webPage[] = 
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<html><head><title>webServer</title></head>\r\n"
"<body><center><h3>Welcome to the server</h3><br>\r\n"
"<img src='test.jpg'/><br>"
"<h4>only the txt file can upload sucessfully</h4><br>"
"<form enctype='multipart/form-data' action='.' method='POST'>"
"<input name='uploadedfile' type='file' /><br>"
"<input type='submit' value='Upload File' />"
"</form>"
"</center></body></html>\r\n";
/*The apperence of web server when browser connnect*/

char imageheader[] = 
"HTTP/1.1 200 Ok\r\n"
"Content-Type: image/jpeg\r\n\r\n";
/*The header that loading image*/

int main(int argc, char *argv[]){

	struct sockaddr_in server_addr, client_addr;
	socklen_t sin_len = sizeof(client_addr);//length of cnt socket
	int fd_server , fd_client;//file descriptor
	char bfr[8192];
	char data[8192];
	int fdimg;//holding fd of file opened
	int on = 1;
	
	fd_server = socket(AF_INET, SOCK_STREAM,0);
	if (fd_server < 0){
		perror("socket");
		exit(1);
	}
	/*means something went wrong*/
	
	setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(8080);
	
	if (bind(fd_server, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){
		perror("bind");
		close(fd_server);
		exit(1);
	}

	if (listen(fd_server,10) == -1){
		perror("Listen");
		close(fd_server);
		exit(1);
	}
	
	while(1){
		fd_client = accept(fd_server, (struct sockaddr *) &client_addr, &sin_len);
		
		if (fd_client == -1){
			perror("Connection failed\n");
			continue;
		}
		
		printf("Got client connection\n");

		if (!fork()){ //child process
			close(fd_server);
			memset(bfr,0,8192);
			read(fd_client, bfr, 8191);
			printf("%s\n", bfr);

			if (!strncmp(bfr,"GET /test.jpg",13)){
				write(fd_client, imageheader, sizeof(imageheader) - 1);
				fdimg = open("test.jpg",O_RDONLY);
				sendfile(fd_client,fdimg,NULL,21000);
				close(fdimg);
			}
			else if (!strncmp(bfr,"POST",4)){
				int i = 0,count = 0,nc = 0;
				memset(data,0,8192);
				char *token;
				char *sp = ":";
				token = strtok(bfr, sp);

				while( token != NULL && count < 19) {
					token = strtok(NULL, sp);
					count++;
				}
				sp = "\n";
				count = 0;
				while(token[i] != '\0'){
					if (token[i] == '\r'){
						nc+= 1;
					}
					if (nc == 2){
						data[count] = token[i];
						count += 1;
					}
					i+=1;
				}
				data[count] = '\0';

				FILE *pFile;
				pFile = fopen("./uploaded.txt","w" );
				fwrite(data,1,count,pFile);
				fclose(pFile);

				write(fd_client,webPage, sizeof (webPage) - 1);

			}
			else{
				write(fd_client,webPage, sizeof (webPage) - 1);
			}
			close(fd_client);
			printf("closing\n");
			exit(0);
		}
		//parent process

		close(fd_client);
	}


	return 0;
}

