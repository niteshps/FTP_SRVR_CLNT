/**@file       : ftp_clnt.c 
 * @brief      : file transfer progress clients send files to server
 *               and calculate the file transfer progress and check it with server. 
 * @author     : Nitesh Agrawal, Akansha Yadav
 */

#include"structure.h"
#include<stdio.h>
#include<netinet/in.h>
#include<netinet/udp.h>
#include<netdb.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<errno.h>
#include<sys/stat.h>
#define CHUNK 256
int chk_file_existence(char *);
data_info dif;

/*
 * @function    : main 
 * @params      : receives command line arguments
 * @return      : returns filename, transfer progress and content/data of file
 * @brief       : file transfer progress client main 
 */

int main(int argc , char *argv[])
{
	char buff[CHUNK];
	int sock_fd,st,i=0,ret=0,f_len=0;
	int t_size=0,prog1=0;
	struct timeval tv;
	tv.tv_sec=30;
	tv.tv_usec=0;
	dif.nol=0;
	dif.thread=1;
	dif.id=0;
	memset(dif.filename,0,sizeof(dif.filename));
	memset(dif.data,0,sizeof(dif.data));
	struct sockaddr_in srvr_addr;
	
	/* check if file exists or not */
	if(chk_file_existence(argv[2]))
	{
		/* open file in read mode specified by client */
		FILE *fp=fopen(argv[2],"r");
		file_prog ob;
		memset(&ob,0,sizeof(ob));
		st=sizeof(srvr_addr);

		/* create file transfer client listener for communication */
		sock_fd=socket(AF_INET,SOCK_DGRAM,0);
		if (sock_fd==-1)
		fprintf(stdout,"socket error\n");

		/* create named socket for accepting connections from UI */
		srvr_addr.sin_family=AF_INET;
		srvr_addr.sin_addr.s_addr=inet_addr(argv[1]);
		srvr_addr.sin_port=htons(9736);

		/* set the timeout value for receiving data from server */
		setsockopt(sock_fd,SOL_SOCKET,SO_RCVTIMEO,(char *)&tv,sizeof(struct timeval));

		/* count total number of lines in file */
		while(fgets(buff,CHUNK,fp)!=NULL)
			{
				i++;
				t_size+=strlen(buff);
			}
		fprintf(stdout,"\ntotal size:%d Bytes\n",t_size);
		fprintf(stdout,"total lines %d\n",i);
		dif.nol=i;
		strcpy(dif.filename,argv[2]);

		/* send filename and nos of lines to server */
		sendto(sock_fd,&dif,sizeof(dif),0,(struct sockaddr*)&srvr_addr,st);
		if((ret=recvfrom(sock_fd,&dif.id,sizeof(dif.id),0,(struct sockaddr*)&srvr_addr,&st))<=0)
			{
				fprintf(stdout,"Failed to receive id:[%s]\n",strerror(errno));
				exit(0);
			}
		fprintf(stdout,"Filename:\t%s\n",argv[2]);	
		fseek(fp,0,SEEK_SET);
		dif.thread=0;

		/* send data of file in chunks to FTP server */
		fprintf(stdout,"*********sending data to server*********\n");
		while(fgets(dif.data,CHUNK,fp)!=NULL)
			{
				prog1+=strlen(dif.data);
				if((sendto(sock_fd,&dif,sizeof(dif),0,(struct sockaddr*)&srvr_addr,st))<0)
					{		
						fprintf(stdout,"\tERROR:[%s]\n",strerror(errno));
						exit(0);
					}
				fprintf(stdout,"bytes of file sent: %d\n",prog1);
				sleep(2);	
			}	

		/* receive filename and progress from the FTP server */
		fprintf(stdout,"Waiting for response from server\n");
		if((ret=recvfrom(sock_fd,&ob,sizeof(ob),0,(struct sockaddr*)&srvr_addr,&st))<=0)
			{
				fprintf(stdout,"Failed to receive filename:[%s]\n",strerror(errno));
				exit(0);
			}
		f_len=strlen(ob.filename_mod);
		ob.filename_mod[f_len]='\0';
		fprintf(stdout,"Filename at servr:\t%s\nProgress at server:\t%d bytes\n",ob.filename_mod,ob.prog);
		close(sock_fd);
		if(ob.prog==prog1)
			{
				fprintf(stdout,"*********file transfer successful*********\n");
			}
		else
			fprintf(stdout,"Sorry, file is not sent completely!!!\n");
      } 
      else
      	{
		fprintf(stdout,"File doesn't exist\n");
 	}
	fclose(fp);
}

/*
 * @function	: chk_file_existence
 * @params	:
 *	@param1	: filename whose existence is to check
 * @return	: file existence status
 * @brief	: this function checks whether the file exists in current directory or not?
 */
int chk_file_existence(char *file)
{	
	int status=0;
	struct stat pstat;
	if(!(lstat(file,&pstat)))
		{
			if(S_ISREG(pstat.st_mode))
				{
					status=1;
				}
		}
	return status;
}
