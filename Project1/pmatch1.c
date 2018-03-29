#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <wait.h>
#include <time.h>

char* concat(const char *s1, const char *s2) {
	char *result = malloc(strlen(s1)+strlen(s2)+1);
	strcpy(result, s1);
	strcat(result, s2);
	return result;
}

int indexOfS( char * source, char *key) {
	int i, j, k;
	for(i = strlen(key) -1 ; i < strlen(source); i++) {
		k = 1;
		for(j = i; j > i- strlen(key); j--) {
			if(source[j] != key[j])
				k = 0;
		}
	}
	if (k == 1)
		return j;
	return -1;
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

int searchAndWrite(char *fname, char *str, char *oname) {
	FILE *fp;
	int line_num = 1;
	int find_result = 0;
	char temp[256];

	if((fp = fopen(fname, "r")) == NULL) {
		return(-1);
	}
	
	FILE *ptr_file;
	char* ostr = oname;
	ptr_file =fopen(ostr, "w");

	if (!ptr_file)
		return 1;

	while(fgets(temp, 256, fp) != NULL) {
		if((matchKey(temp, str)) != 0) {
			fprintf(ptr_file, "%s, %d: %s", fname, line_num, temp);
			find_result++;
		}
		line_num++;
	}
	
	fclose(ptr_file);
	
	if(fp) {
		fclose(fp);
	}
	free(ostr);
   	return(0);
}

int main(int argc, char *argv[]) {
	printf ("program started\n"); 
	clock_t begin = clock();
	pid_t n = 0;
	
	int result, errno;

	char* key = argv[1];
	
	int no = atoi(argv[2]);
	
	if(argc != no+4) {
		fprintf(stderr, "usage: a.out <value>\n");
		return -1;
	}
	
	if(no<1) {
		fprintf(stderr, "No of input files = %d, must be >= 1\n", no);
		return -1;
	}
	
	int i;
	for(i = 1; i<=no; i++) {
		n = fork();
		if(n<0) {
			fprintf(stderr, "Fork Failed");
			exit(-1);
		}
		else if (n == 0) {
			result = searchAndWrite(argv[i+2], key, concat("o", argv[i+2]));
			if(result == -1) {
				perror("Error");
				printf("Error number = %d\n", errno);
				exit(1);
			}
			
			exit(0);
		}
	}
	
	for(i = 1; i<=no; i++){
		wait(NULL);
	}
	
	FILE *ptr_file;
	char* out = argv[argc-1];
	ptr_file =fopen(out, "w");

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
	
	for(i = 0; i<no; i++) {
		FILE *fp;
	
		char tmp[350];
		
		char *filename = concat("o", argv[i+3]);

		if((fp = fopen(filename, "r")) == NULL) {
			printf ("error while opening intermediate files\n");
			return(-1);
		}

		while(fgets(tmp, 350, fp) != NULL) {
			fprintf(ptr_file, "%s",tmp);
		}
	

		if(fp) {
			fclose(fp);
			remove(filename);
		}
	
	}
	
	fclose(ptr_file);
	
	clock_t end = clock();
	double time = (double)(end - begin)/CLOCKS_PER_SEC;
	printf("The program takes: %.6f seconds\n", time);

	return 0;	
	
}
