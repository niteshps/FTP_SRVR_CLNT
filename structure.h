/**
 * @file       : structure.h 
 * @brief      : structure header file
 * @author     : Nitesh Agrawal, Akansha Yadav 
 */

/*
 * @structure   : file_prog 
 * @brief       : structure holding filename and progress information 
 * @members     :
 *      @mem1   : hold the file transfer progress
 *      @mem2	: contains filename
 */

typedef struct {
	int prog;
	char filename_mod[32];
}file_prog;

/*
 * @structure   : data_info 
 * @brief       : structure holding thread creation, data information of file 
 * @members     :
 *      @mem1   : server generated id foe each client
 *      @mem2	: contains number of lines in a file in a thread for each client
 *      @mem3	: thread creation information
 *      @mem4	: contains filename in a thread for each client
 *	@mem5	: contains data of file in chunks for each client
 */

typedef struct{
	int id;
	int nol;
	int thread;
	char filename[32];
	char data[1024];
}data_info;	
