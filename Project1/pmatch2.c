#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <wait.h>
#include <time.h>

#define BUFFER_SIZE 1000000

char* concat( char *s1,  char *s2) {
	char *result = malloc(strlen(s1)+strlen(s2)+1);
	strcpy(result, s1);
	strcat(result, s2);
	return result;
}

int matchKey(char *str1, char *str2) {
	if(strstr(str1, str2) == NULL) 
		return 0;
	int i, k, l;
	k = 1;
	for(i = 0; i < strlen(str2); i++) {
		if(str2[i] != str1[i])
			k = 0;
	}
	
	if(k) {
	
		if(strstr(str1, concat(str2, " "))  != NULL)
			return 1;
		else if(strstr(str1, concat(str2, "\t"))  != NULL)
			return 1;
		else if (strstr(str1, concat(str2, "\n")) != NULL)
			return 1;
	}
	
	l = 1;
	for(i = strlen(str2)-1; i >= 0; i--) {
		if(str2[i] != str1[i + strlen(str1) - strlen(str2)-2])
			l = 0;
	}
	if(l) {
		if(strstr(str1, concat(" ", str2)) != NULL)
			return 1;
		if(strstr(str1, concat("\t", str2)) != NULL)
			return 1;
		if(k)
			return 1;
	}
	
	if(strstr(str1, concat(concat(" ", str2), " ")) != NULL)
		return 1;
	if(strstr(str1, concat(concat(" ", str2), "\t")) != NULL)
		return 1;
	if(strstr(str1, concat(concat(" ", str2), "\n")) != NULL)
		return 1;
	if(strstr(str1, concat(concat("\t", str2), " ")) != NULL)
		return 1;
	if(strstr(str1, concat(concat("\t", str2), "\t")) != NULL)
		return 1;
	if(strstr(str1, concat(concat(" \t", str2), "\n")) != NULL)
		return 1;
	
	return 0;
}

char* concat2( char *s1,  int no, char *s2) {
	char str[10];
	sprintf(str, "%d: ", no);
	char *result = malloc(strlen(s1)+strlen(s2)+strlen(str)+5);
	strcpy(result, s1);
	strcat(result, ", ");
	strcat(result, str);
	strcat(result, s2);
	return result;
}

int searchAndWrite(char *fname, char *str /*, char *oname*/, int i) {
	FILE *fp;
	int line_num = 1;
	int find_result = 0;
	char temp[256];

	if((fp = fopen(fname, "r")) == NULL) {
		return(-1);
	}
	
	char *message;
	message = (char *)malloc(330);
	strcpy(message, "");

	while(fgets(temp, 256, fp) != NULL) {
		if((matchKey(temp, str)) != 0) {
			message = (char *)realloc(message, strlen(message)+330); 
			strcat(message, concat2(fname, line_num, temp));
			find_result++;
		}
		line_num++;
	}
	write(i, message, strlen(message)+1);

	if(fp) {
		fclose(fp);
	}

   	return(0);
}


int main(int argc, char *argv[]) {
	printf ("program started\n"); 
	clock_t begin = clock();
	char read_msg[BUFFER_SIZE];
 	pid_t pid = 0;  

	
	int result, errno;

	char* key = argv[1];
	
	int no = atoi(argv[2]);
	int fd[no*2];
	
	int i;
	for(i = 0; i <no; i++) {
		pipe(&fd[2*i]);
	}
	
	if(argc != no+4) {
		fprintf(stderr, "usage: a.out <value>\n");
		return -1;
	}
	
	if(no<1) {
		fprintf(stderr, "No of input files = %d, must be >= 1\n", no);
		return -1;
	}
	
	char sptr[68];
	int j;
	
	
	for (i = 3; i<no+2; i++) {
		for(j = 3; j<no+2; j++) {
			if(strcmp(argv[j], argv[j+1]) > 0 ) {
				strcpy(sptr, argv[j]);
				strcpy(argv[j], argv[j+1]);
				strcpy(argv[j+1], sptr);
			}
		}
	}
	
	
	for(i = 1; i<=no; i++) {
		pid = fork();
		if(pid<0) {
			fprintf(stderr, "Fork Failed");
			exit(-1);
		}
		else if (pid == 0) {
			result = searchAndWrite(argv[i+2], key, fd[2*i-1]);
			if(result == -1) {
				perror("Error");
				printf("Error number = %d\n", errno);
				exit(1);
			}
			close(fd[2*i-1]);
			
			exit(0);
		}
	}
	
	for(i = 1; i<=no; i++){
		wait(NULL);
	}

	FILE *fp;
	char* out = argv[argc-1];
	fp =fopen(out, "w");
	
	for(i = 0; i<no; i++) {
		read(fd[2*i],read_msg,BUFFER_SIZE);
		fprintf(fp, "%s",read_msg);
		
		close(fd[2*i]);
	}

	fclose(fp);

	clock_t end = clock();
	double time = (double)(end - begin)/CLOCKS_PER_SEC;
	printf("The program takes: %.6f seconds\n", time);


	return 0;
}
