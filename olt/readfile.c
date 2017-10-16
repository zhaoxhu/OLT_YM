
#include "vxWorks.h"
#include "sysLib.h"
#include "string.h"
#include "ftplib.h"
#include "stdio.h"
#include "fioLib.h"
#include "ioLib.h"

#define FILE_BUFFER_SIZE 1000

unsigned char DBAImage[20000];
int DBAImageLength;

int CompDBAImage()
{
	int ret;
	int counter;
	unsigned char*Ptr1, *Ptr2;

	counter = DBAImageLength;
	Ptr1 = DBAImage;
	Ptr2 = (unsigned char *)0xfe780000;
	
	while( counter > 0 ){
		if( counter > FILE_BUFFER_SIZE ){
			ret = memcmp( Ptr1, Ptr2 , FILE_BUFFER_SIZE );
			counter -= FILE_BUFFER_SIZE;
			Ptr1 +=  FILE_BUFFER_SIZE;
			Ptr2 +=  FILE_BUFFER_SIZE;
			}
		else{
			ret = memcmp( Ptr1, Ptr2, counter );
			counter = 0;
			}
		if( ret != 0 ){
			printf(" the data is not equal \n");
			break;
			}
		}
	return( ret );
	
}


int read_DBA_Image()
{
	int Ctrl_FD;
	int Data_FD;
	unsigned int nbytes;
	unsigned char *Temp_buffer;

	Ctrl_FD = 0;
	Data_FD = 0;
	nbytes = 0;
	DBAImageLength = 0;
	memset( DBAImage, 0, sizeof( DBAImage));
	Temp_buffer = DBAImage;
	
	if (ftpXfer ( "192.168.7.139", "target", "123", "", "RETR %s", "", "Plato21.dll", &Ctrl_FD, &Data_FD)==ERROR){
		printf(" open DBA iamge err \n");
		return (0);
	}

	while (( nbytes = read( Data_FD, Temp_buffer, FILE_BUFFER_SIZE )) > 0 ){
		printf(" read data len %d \n", nbytes );
		DBAImageLength += nbytes;
		Temp_buffer += nbytes;
		}

	close( Data_FD );

	if ( 0 ){

		}
	else  if( DBAImageLength > 0 ){
		if( nbytes < 0 ) DBAImageLength = 0;
		else if (ftpReplyGet (Ctrl_FD, TRUE) != FTP_COMPLETE) DBAImageLength = 0;
		}
	
	if (ftpCommand (Ctrl_FD, "QUIT", 0, 0, 0, 0, 0, 0) != FTP_COMPLETE)  DBAImageLength = 0;
	close( Ctrl_FD );

	return( DBAImageLength );	

}


 int  read_DBA_file()
 {
	int  fd_host_read;
	unsigned int  nbytes;
	unsigned char *ptr_temp_buffer;
 
	memset(DBAImage, 0, sizeof( DBAImage ) );
	ptr_temp_buffer = DBAImage;
	DBAImageLength = 0;
	/*** read file *****/
    
	fd_host_read = open ("host:Plato21.dll",0x02,777);
	if( fd_host_read == -1 ){ 
		printf(" open file err \n");
		return (-1);
		}
	
	printf(" open file success ! \n" );

	DBAImageLength = 0;
	while ((nbytes = fioRead ( fd_host_read, ptr_temp_buffer, FILE_BUFFER_SIZE )) > 0)
		{
		ptr_temp_buffer += nbytes;
		DBAImageLength += nbytes;
 	
		if (nbytes != FILE_BUFFER_SIZE )
			break;
		if (DBAImageLength > (17* FILE_BUFFER_SIZE -1))
			{
			DBAImageLength = 17* FILE_BUFFER_SIZE;
			break;	
			}
		}
	if(nbytes <= 0)printf(" Error read file after %d bytes \n", DBAImageLength);
	else printf(" the file total bytes is %d \n", DBAImageLength );

	close ( fd_host_read );
	return( DBAImageLength);
 }


 void ftpRead()
{
	char host[] = "192.168.7.35";
	char user[]="target";
	char passwd[] = "target";
	char cmd[] ="RETR %s";
	char filename[]= "setenv.bat";
	char DataPtr[1000]= {0};

	int errFd;
	int Fd;

	int  nBytes=0;
	int  totalBytes = 0;
	
	

	if( ftpXfer(host, user, passwd, "",cmd,"", filename, &errFd, &Fd ) == ERROR ) 
		{
		printf(" ftp open file err \r\n");
		return;
		}
	
	while(( nBytes = read( Fd,  &DataPtr[totalBytes], 100 )) > 0)
		{
		totalBytes += nBytes;
		}
	
	close( Fd );
	close(errFd);

}
