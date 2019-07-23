// SYD_24Kflash.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <sys/stat.h>   
#include <math.h>

#define UTF8_CODE_BASS 0X20
#define UTF8_CODE_END 0X7E
#define UTF8_MASK_ADDR 0X0000      //4E00-9FA5  0X9FA5-0X4E00=0X51A5=20901  20901*2=41802  41802/1024=40.822265625 

#define line_max 100000
#define line_num 100
#define line_num1 500
#define line_ble_num 39
#define line_ble_maxnum 59

struct DISCONNECT_REASON{
	unsigned char time[9];
	unsigned char	reason;
	unsigned char latency_state;
	unsigned char update_latency_mode;
	unsigned char state;
};

struct DISCONNECT{
	unsigned long int count;
	struct DISCONNECT_REASON data[10*1024];
};

struct DISCONNECT disconnect_statistics;

char buff1[line_num1];
char buff2[line_num1];
char value_data[line_max];
char value_data1[line_max];
char file_name_buff[100],*file_name=file_name_buff;
FILE * file;
FILE * targetfile;
char ancs_to_hex(char a){
	if(a< 58)a = a - 48;
    else if( a < 91) a = a - 55;
    else if( a < 123 ) a = a - 87;
	else a=0;
    return a;
}
char str_to_hex(char *a){
	char temp=0;
	temp=(ancs_to_hex(*a) <<4) | ancs_to_hex(*(a+1));
    return temp;
}
unsigned long str_to_dec(char *a,char size){
	unsigned long temp=0;
	char i=0;
	for(i=0;i<size;i++)
	{
		temp +=ancs_to_hex(*(a+size-1-i))*pow(10.0, i);
	}
    return temp;
}
void dbg_hexdump(char *title, unsigned char *buf,unsigned short int sz)
{
	int i = 0;
	
	if (title)
		printf((title));

	for(i = 0; i < sz; i++) 
	{
  		if((i%16) == 0)
			printf("[%04x] ",(unsigned short int)i);

		printf("%02x ",(unsigned short int)buf[i]);

		if(((i+1)%16) == 0)
			printf("\r\n");

	 } 

	if((i%16) != 0)
		printf("\r\n");
}
int main(int argc, char* argv[])
{
	long i=0;
	long write_length;
	long fileLen,targetfileLen;
	struct stat statbuf;
	char * now_point=NULL,* value_point=NULL,* value_point_end=NULL,* start_point=NULL,* save_point,* print_point,* printf_point=NULL;
	long j=0,z=0,have_fill=0,num_value=0;

	long line_valid=0;
	int count=0;
	unsigned long timer_start=0,timer_end=0;

	//system("mode con cols=120");    //cols=show_length*4

	file_name=file_name_buff;

	do
	{
	printf("please input file_name(xxxx.txt):");
    //scanf("%c",file_name_buff);
	//scanf ("%s",file_name_buff);
	scanf ("%[^\n]",file_name_buff);
	getchar();
	}while(strstr(file_name_buff,".txt") == NULL);

	file = fopen(file_name, "r");
	if ( file == NULL )
    {
        printf("open file error \r\n" );
        return -1;
    }

    stat(file_name, &statbuf);
	fileLen=statbuf.st_size;
	printf("Size of file in bytes: %ld \r\n", statbuf.st_size);

	while(!feof(file)){
		fgets(buff1,500,file);
		start_point=strstr(buff1,"Connecting to ");
		printf_point=start_point;
		if(start_point != NULL){
			start_point +=sizeof("Connecting to ")+1;
			break;
		}
	}
	if(start_point == NULL){
		fseek(file,0,0);
		while(!feof(file)){
			fgets(buff1,500,file);
			start_point=strstr(buff1,"nRF Connect,");
			printf_point=start_point;
			if(start_point != NULL){
				start_point +=sizeof("nRF Connect,")+1;
				break;
			}
		}

		if(start_point == NULL){
			printf("not Connected\r\n");
		}else
		{
			printf("%s",printf_point);
			fgets(buff1,500,file);
			printf("%s",buff1);
		}
	}
	else	printf("%s",printf_point);

	j=0;
	memset(&value_data,0,sizeof(value_data));
	save_point=(char *)&value_data;
	timer_start=0;
	timer_end=0;
	while(!feof(file)){
		fgets(buff1,500,file);
		now_point=strstr(buff1,"Notification received from");		
		if(now_point != NULL){
		    if(timer_start){
				timer_end=(str_to_dec(&buff1[2],2)*3600+str_to_dec(&buff1[5],2)*60+str_to_dec(&buff1[8],2))*1000+str_to_dec(&buff1[11],3);
				memcpy(buff2,buff1+2,12);  
			}else
			{
				memcpy(buff2,buff1+2,12);  
				printf("start:%s\r\n",buff2);
				timer_start=(str_to_dec(&buff1[2],2)*3600+str_to_dec(&buff1[5],2)*60+str_to_dec(&buff1[8],2))*1000+str_to_dec(&buff1[11],3);
			}
			
			value_point=strstr(now_point,"\"")+1;
			value_point_end=strstr(value_point,"\"");
			num_value=value_point_end-value_point;
			if(value_point != NULL){
				for(j=0;j<num_value;j++){
				    *(save_point+have_fill)=*(value_point+j);
					have_fill++;
				}
			}
		}
	}
	if(timer_end){
		printf("end:%s -->%ld  ",buff2,timer_end);
		printf("time difference:%ldms\r\n",timer_end-timer_start);
		printf("received num:%ld received rate:%fByte/s  %fByte/400ms\r\n\r\n",have_fill,(float)have_fill/(((float)(timer_end-timer_start))/1000),(float)have_fill/(((float)(timer_end-timer_start))/1000)*0.4);
	}
	value_data[have_fill]='\0';
	if(have_fill)printf("%s",save_point);

	fclose(file);

	//比较文件
	printf("\r\n\r\n......compare start......\r\n");
    file_name=file_name_buff;

	do
	{
	printf("please input file_name(xxxx.txt):");
    //scanf("%c",file_name_buff);
	//scanf ("%s",file_name_buff);
	scanf ("%[^\n]",file_name_buff);
	getchar();
	}while(strstr(file_name_buff,".txt") == NULL);

	file = fopen(file_name, "r");
	if ( file == NULL )
    {
        printf("open file error \r\n" );
        return -1;
    }

    stat(file_name, &statbuf);
	fileLen=statbuf.st_size;
	printf("Size of file in bytes: %ld \r\n", statbuf.st_size);

	memset(&value_data1,0,sizeof(value_data1));
	fread (value_data1,fileLen,1,file);
	count=memcmp(value_data1,value_data,fileLen);
	if(count==0){
		printf("Two file content is equally \r\n");
	}else{
		printf("Two file content is different:%ld\r\n",count);
	}

	fclose(file);

    while(1);
	return 0;
}

