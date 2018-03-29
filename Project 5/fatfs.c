#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include <linux/msdos_fs.h>

#define SECTORSIZE 512   //bytes
#define BLOCKSIZE  4096  // bytes - do not change this value
#define MAX_CLUSTER_NUM 4000000

char diskname[48]; 
char p[3];
char todo[20];
char filename[20];
int  disk_fd; 

unsigned char volumesector[SECTORSIZE]; 

long int readBytes( int start,  int offset) {
	char no[2*offset+1];
	char str[SECTORSIZE*2+1];
	strcpy(no, "0x");
	
	for (int i = offset-1; i >= 0; i--) {
		sprintf(str, "%02x", (unsigned char) volumesector[start+i]);
		strcat(no, str);
	}
	char * pEnd;
	return strtol(no, &pEnd, 16);
}

long int readBytesFile( int start,  int offset, char file[]) {
	char no[2*offset+1];
	char str[SECTORSIZE*2+1];
	strcpy(no, "0x");
	
	for (int i = offset-1; i >= 0; i--) {
		sprintf(str, "%02x", (char) file[start+i]);
		strcat(no, str);
	}
	char * pEnd;
	return strtol(no, &pEnd, 16);
}

long int readBytesBigEndian( int start,  int offset) {
    char no[2*offset+1];
    char str[SECTORSIZE*2+1];
    strcpy(no, "0x");
   
    for (int i = 0; i < offset; i++) {
        sprintf(str, "%02x", (unsigned char) volumesector[start+i]);
        strcat(no, str);
    }
    char * pEnd;
    return strtol(no, &pEnd, 16);
}

void printer(long int name) {
    while (name > 0) {
          printf("%c", name & 0xff);
          name >>= 8; // shifts f right side for 8 bits
        }
}

void getClusterChain(long int first_cluster){
	long int next_cluster = first_cluster;
	
	do{
		if((next_cluster != 0) && !((next_cluster & 0x0FFFFFFF) >= 0x0FFFFFF8)){
			printf ("%ld : ", next_cluster); 
			printf("%x\n", next_cluster);
		}

		long int FatSector = next_cluster / 128;
		long int FatOffset = next_cluster % 128;
		get_sector(volumesector, 32 + FatSector);
		next_cluster = readBytes(FatOffset*4, 4);
		//printf ("FatSector: %ld\n", FatSector); 
		//printf ("FatOffset: %ld\n", FatOffset); 
		//printf ("FatSector: %ld\n", FatSector); 

	}while((next_cluster != 0) && !((next_cluster & 0x0FFFFFFF) >= 0x0FFFFFF8));
}

int get_sector (unsigned char *buf, int snum)
{
	off_t offset; 
	int n; 
	offset = snum * SECTORSIZE; 
	lseek (disk_fd, offset, SEEK_SET); 
	n  = read (disk_fd, buf, SECTORSIZE); 
	if (n == SECTORSIZE) 
		return (0); 
	else {
		printf ("sector number %d invalid or read error.\n", snum); 
		exit (1); 
	}
}

void print_sector (unsigned char *s)
{
	int i;

	for (i = 0; i < SECTORSIZE; ++i) {
		printf ("%02x ", (unsigned char) s[i]); 
		if ((i+1) % 16 == 0)
			printf ("\n"); 
	}
	printf ("\n");
}

long int converter(char name[]) {
	int i = 0;
	char file[100];
	char fileName[100] = "";
	while(name[i] != '\0') {
		sprintf(file, "%x", name[i]);
		strcat(fileName, file);
		i++;
	}
	return atol(fileName);
}

int main(int argc, char *argv[])
{	
	if (argc < 4) {
		printf ("wrong usage\n"); 
		exit (1); 
	}
	
	long int file_name_int;

	strcpy (diskname, argv[1]); 
	strcpy (p, argv[2]); 
	strcpy (todo, argv[3]); 
	if(argc == 5) {
 		strcpy (filename, argv[4]);
 		
		char file_name_arr[8];
		int count = 0;
		while (filename[count] != '\0') {
			file_name_arr[count] = filename[count];
			count++;
		}
	
		for(; count < 8; count++)
			file_name_arr[count] = ' ';

		file_name_int = readBytesFile(0, 8, file_name_arr);
 	}
		
      disk_fd = open (diskname, O_RDWR); 
	if (disk_fd < 0) {
		printf ("could not open the disk image\n"); 
		exit (1); 
	}

	get_sector(volumesector, 48); 
	//print_sector(volumesector); 

	get_sector(volumesector, 0); 
	
	//
	long int sectors_per_fat = readBytes(36,4);
	long int fat_begin_lba = readBytes(28,4) + readBytes(14,2);
	long int cluster_begin_lba = readBytes(28,4) + readBytes(14,2) + (readBytes(16, 1) * readBytes(36, 4));
	long int sectors_per_cluster = readBytes(13,1);
	long int root_dir_first_cluster = readBytes(44,4);
	long int lba_addr = cluster_begin_lba + (root_dir_first_cluster - 2) * sectors_per_cluster;
	//

	/*printf ("fat_begin_lba: %ld\n", fat_begin_lba);
	printf ("cluster_begin_lba: %ld\n", cluster_begin_lba);
	printf ("sectors_per_cluster: %ld\n", sectors_per_cluster);
	printf ("root_dir_first_cluster: %ld\n", root_dir_first_cluster);
	printf ("lba_addr: %ld\n", lba_addr);*/

	get_sector(volumesector, lba_addr); 
	long int root_name = readBytes(0,8);

	printer(root_name);
	printf("\n"); 
		
	
	if(!strcmp(todo, "volumeinfo")) {
		printf ("Number of sectors that FAT occupies: %ld\n", sectors_per_fat);
	}

	int start_read;

	for(int j = 0; j < sectors_per_cluster; j++){
		get_sector(volumesector, lba_addr + j); 
		if(j == 0)
			start_read = 32;
		else start_read = 0;
		
		for (int i = start_read; i < SECTORSIZE; i++){
						
			if ((i) % 32 == 0){
				if(readBytes(i,1) != 0){
					long int file_name = readBytes(i,8);
					//printf("file_name = %ld", file_name);
					//printf("\n");
					
					//long int fileInt = readBytesFile(0, );
					long int file_extension = readBytes(i+8,3);
					long int file_size = readBytes(i+28,4);
					int isFileSizeZero = 0;
					if(file_size == 0)
						isFileSizeZero = 1;

					if(!strcmp(todo, "rootdir") ){
						if(file_size != 4294967295) {
							printer(file_name);
							printf("   ");
							printer(file_extension);
							printf("\n");
						}							
					}
			
					unsigned char cluster_char_1[50];
					strcpy(cluster_char_1, "0x");					
					unsigned char high_16_1 = volumesector[i+20];
					unsigned char high_16_2 = volumesector[i+21];
					unsigned char low_16_1 = volumesector[i+26];
					unsigned char low_16_2= volumesector[i+27];
					char str[50];
					sprintf(str, "%02x", (unsigned char) high_16_2);
					strcat(cluster_char_1, str);
					sprintf(str, "%02x", (unsigned char) high_16_1);
					strcat(cluster_char_1, str);
					sprintf(str, "%02x", (unsigned char) low_16_2);
					strcat(cluster_char_1, str);
					sprintf(str, "%02x", (unsigned char) low_16_1);
					strcat(cluster_char_1, str);
					char *cluster_char_2;
				
					//first cluster number of current directory entry is found here
   					long int first_cluster_number = strtol(cluster_char_1, &cluster_char_2, 16);
					//printf ("My num: %ld\n", first_cluster_number);	
		
					//high and low orders of first cluster of current directory entry	
					/*printf ("high_16_1: %02x\n", high_16_1);
					printf ("high_16_2: %02x\n", high_16_2);
					printf ("low_16_1: %02x\n", low_16_1);
					printf ("low_16_2: %02x\n", low_16_2);*/

					//File name and file extension
					//printf("file name: %ld\n", file_name);
					//printf("file ext: %ld\n", file_extension);
					if(!strcmp(todo, "blocks") && !(isFileSizeZero) && file_name_int == file_name){
						printer(file_name);
						printf("   ");
						printer(file_extension);
						printf("\n");
						getClusterChain(first_cluster_number);
					}			
				}
				else{
					break;
				}
				//getClusterChaing methodu sürekli sectoru değiştireceği için burda tekrar eski halini alıyoruz sectorun
				get_sector(volumesector, lba_addr + j); 	
			}
		}
	}	

	close (disk_fd); 
	return 0; 	
}
