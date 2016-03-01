/**
 * @file       : ftp_srvr.c 
 * @brief      : file transfer progress server saves all the files recieved from clients
 *               and sends the file transfer progress status to respective clients. 
 * @author     : Nitesh Agrawal, Akansha Yadav
 */

#include<pthread.h>
#include"structure.h"
#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netinet/udp.h>
#include<netdb.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<errno.h>
#define CHUNK 1024

void *ssrvr_thread(void *arg);
void data_copy();

/* 
 * @structure	: func_data
 * @brief	: structure holding arguments for sending information to client
 * @members	:
 *	@mem1	: pointer to sever socket file descriptor
 *	@mem2	: client address structure
 	@mem3	: size of client address
 */
typedef struct 
{
	int *send_fd;
	struct sockaddr_in client_addr;
	int cli_len;
}func_data;

file_prog ob1[5];
data_info th_cr_info;
func_data fd_addr;
int sock_fd;
char *ptr[5];
int id=0,line[5],cnt=0;
int counter=0;

/*
 * @function	: main
 * @params	: none
 * @return	: none
 * @brief	: FTP server creates separate thread for each client
 */
int main()
{
	struct sockaddr_in srvr1_addr;
	int res,i,optval=1,fdmax,rret=0;

	/* read file descriptor list */
	fd_set *readfds=NULL;
	pthread_t srvr_thread[5];
	fd_addr.cli_len=sizeof(fd_addr.client_addr);
	readfds=malloc(sizeof(fd_set));
	FD_ZERO(readfds);
	for(i=0;i<5;i++){
		memset(&ob1[i],0,sizeof(ob1[i]));
	}
	memset(&th_cr_info,0,sizeof(th_cr_info));
	fprintf(stdout,"\n\n\t*********FTP_SERVER*********\n\n");

	/* create server socket descriptor*/
	if((sock_fd=socket(AF_INET,SOCK_DGRAM,0))==-1)
	{
		fprintf(stdout,"Failed to create socket\n");
	}

	/* initialise the server address */
	srvr1_addr.sin_family=AF_INET;
	srvr1_addr.sin_addr.s_addr=INADDR_ANY;
	srvr1_addr.sin_port=htons(9736);

	/* bind the FTP server socket descriptor with server address */
	bind(sock_fd,(struct sockaddr*)&srvr1_addr,sizeof(srvr1_addr));

	/* check if socket is already in use? */
	setsockopt(sock_fd,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(int));
	
	/* add sock_fd to the readfds set */
	FD_SET(sock_fd,readfds);
	fdmax=sock_fd;

	fd_addr.send_fd=malloc(sizeof(int));

	if (fd_addr.send_fd == NULL) 
	{
		fprintf(stdout,"No more memory");	
		exit(0);
	}

	
	*(fd_addr.send_fd)=sock_fd;
	
	while(1)
	{
		/* wait till socket descriptor is ready for operation */
		if(select(fdmax+1,readfds,NULL,NULL,NULL)==-1)
		{
			fprintf(stdout,"Failed to perform select() on socket ERROR[%s]\n",strerror(errno));
			break;
		}

		/* receive data from  client */
		if((rret=recvfrom(sock_fd,&th_cr_info,sizeof(th_cr_info),0,(struct sockaddr*)&(fd_addr.client_addr),&(fd_addr.cli_len)))<0)
		{
			fprintf(stdout,"ERROR:[%s]\n",strerror(errno));
			exit(0);
		}

		/* check if the client is new? */
		if(th_cr_info.thread==1)
		{       
			/* if new client, create separate thread for it */
			res=pthread_create(&srvr_thread[cnt],NULL,ssrvr_thread,&th_cr_info.nol);
			cnt++;
			if(res)
			{
				fprintf(stdout,"Thread creation failed:ERROR[%s]\n",strerror(errno));			
			}
		}
		else 
		{
			/* if existing client, write data into the file */
			data_copy();
		}
	}
	free(readfds);
	for(i=0;i<5;i++)
	  free(ptr[i]);
	close(sock_fd);
	return 0;
}

/*
 * @function	: ssrvr_thread
 * @params	:
 *	@param1	: total number of lines in the file of client
 * @return	: none
 * @brief	: this thread appends the id with the filename of the client
 *		  and sends id to the client
 */
void *ssrvr_thread(void *arg)
{      
	int i=0,ret=0,nol1=0,k=0,dot=0,l=0,lock_res;
	int *recv_nol=(int *)arg;
	file_prog ob;
	char filename[32],ar1[32],ar2[10];
	pthread_mutex_t lock;

	/* mutex initialisation */
	lock_res=pthread_mutex_init(&lock,NULL);
	if(lock_res!=0)
	{
		fprintf(stdout,"Mutex Initialisation failed\n");
		exit(0);
	}
	nol1=*recv_nol;
	memset(&ob,0,sizeof(ob));
	memset(filename,0,sizeof(filename));	
	memset(ar1,0,sizeof(ar1));	
	memset(ar2,0,sizeof(ar2));	
	strcpy(filename,th_cr_info.filename);
	ret=strlen(filename);
	filename[ret]='\0';
	fprintf(stdout,"%s\n",filename);

	/* separate the filename and its extension */ 
	for(k=0;filename[k]!='\0';k++)
	{
		if(filename[k]=='.')
		{
			dot=k;
			break;
		}
	}
	for(k=0;filename[k]!='\0';k++)
	{
		if(k<dot)
		{
			ar1[k]=filename[k];
		}
		if(k>dot)
		{
			ar2[l]=filename[k];
			l++;
		}
	}
	ar2[l]='\0';
	fprintf(stdout,"%s\t%s\n",ar1,ar2);

	/* lock the id parameter for performing operation */
	pthread_mutex_lock(&lock);
	id++;

	/* append the id with the filename of client */ 
	sprintf(ob.filename_mod,"%s%d.%s",ar1,id,ar2);
	fprintf(stdout,"%s\n",ob.filename_mod);

	/* send id to the client */
	if((sendto(*(fd_addr.send_fd),&id,sizeof(id),0,(struct sockaddr*)&(fd_addr.client_addr),fd_addr.cli_len))<0)
	{
		fprintf(stdout,"Failed to send client id:[%s]\n",strerror(errno));
		exit(0);
	}

	/* unlock the mutex */
	pthread_mutex_unlock(&lock);
	ptr[id]=ob.filename_mod;
	
	/* mutex destroy */ 
	pthread_mutex_destroy(&lock);

	/* wait till the data is completely copied in the file */
	while(1)

	{
		if(line[th_cr_info.id]==nol1)	
		{	

			pthread_exit(0);
		
		}

	}
}

/* 
 * @function	: data_copy
 * @params	: none
 * @brief	: this function is to  copy data into the file
 *		  and send file transfer progress to the client
 */
void data_copy()
{

	FILE *fp1=NULL;
	char buff1[1024];

	/* open the file in append mode */
	fp1=fopen(ptr[th_cr_info.id],"a+");
	memset(buff1,0,sizeof(buff1));
	strcpy(buff1,th_cr_info.data);	

	/* increment the no. of lines received of corresponding client */ 
	line[th_cr_info.id]++;
	ob1[th_cr_info.id].prog+=strlen(buff1);					
	fprintf(stdout,"%d Bytes received from client %d\n",ob1[th_cr_info.id].prog,th_cr_info.id);
	fprintf(stdout,"Total no. of lines to receive:%d lines received:%d\n",th_cr_info.nol,line[th_cr_info.id]);	
	sleep(.5);
	
	/* write the data into the file */
	fputs(buff1,fp1);

	/* check if complete file is received? */
	if(line[th_cr_info.id]==th_cr_info.nol)	
	{
		fprintf(stdout,"received\n");
		fprintf(stdout,"copied in file...\n");
		strcpy(ob1[th_cr_info.id].filename_mod,ptr[th_cr_info.id]);

		/* send filename and file transfer progress to the client */
		if((sendto(*(fd_addr.send_fd),&(ob1[th_cr_info.id]),sizeof(ob1[th_cr_info.id]),0,(struct sockaddr*)&(fd_addr.client_addr),fd_addr.cli_len))<0)
		{
			fprintf(stdout,"filename and progress sending failed:[%s]\n",strerror(errno));
			exit(0);
		}
		fprintf(stdout,"filename and progress sent\n");
		fprintf(stdout,"Filename: %s\tProgress: %d bytes\n",ob1[th_cr_info.id].filename_mod,ob1[th_cr_info.id].prog);

	}
	fclose(fp1);
}
