#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <wait.h>
#include <pthread.h>
#include <time.h>

#define MAX_FILE_NO 20

typedef struct ll_node {
	char data[335];
	char key[64];
    	char input[68];
	struct ll_node * next;
} node;

char* concat( char *s1,  char *s2) {
	char *result = malloc(strlen(s1)+strlen(s2)+1);
	strcpy(result, s1);
	strcat(result, s2);
	return result;
}

void add(node *head, char *info) {	
	if(head ==NULL) {
		head = (node*) malloc(sizeof(node));
		strcpy(head->data, info);
		head->next = NULL;
	}
	else {
		node * cur = head;
		while(cur->next != NULL) {
			cur = cur->next;
		}
		cur->next = malloc(sizeof(node));
		strcpy(cur->next->data, info);
		cur->next->next = NULL;
	}
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
	//strcat(result, ": ");
	strcat(result, s2);
	//strcat(result, "\n");
	return result;
}

int searchAndWrite(char *fname, char *str, node * head) {
	FILE *fp;
	int line_num = 1;
	char temp[256];
	char info[335];

	if((fp = fopen(fname, "r")) == NULL) {
		return(-1);
	}
	
	while(fgets(temp, 256, fp) != NULL) {
		if((matchKey(temp, str)) != 0) {
			sprintf(info, "%s, %d: %s", fname, line_num, temp);
			add(head, info);
		}
		line_num++;
	}
	
	if(fp) {
		fclose(fp);
	}
   	return(0);
}

int length(node *head) {
	int l = 0;
	node *cur;
	for(cur=head; cur != NULL; cur = cur->next)
		l++;
	return l;
}

void print(node *head) {
	node * cur = head;
	while(cur != NULL) {
		printf("%s", cur->data);
		cur = cur->next;
	}
}

void *runner(void *param) {	
	node *curr = (node *)param;
	
	char * str = curr->input;
	char * key = curr->key;
	
	int result = searchAndWrite(str, key, curr);
		if(result == -1) {
			perror("Error");
			printf("Error number = %d\n", errno);
			exit(1);
		}
	pthread_exit(0); 
}



int main(int argc, char *argv[]) {	
	printf ("program started\n"); 
	clock_t begin = clock();
	char* key = argv[1];
	
	int no = atoi(argv[2]);
	
	pthread_t tid[no]; /* id of the created thread */
	//pthread_attr_t attr;  /* set of thread attributes */
	
	int i;
	
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
	
	node * heads[no];
	
	for(int i = 0; i < no; i++) {
        	heads[i] = (node*)malloc(sizeof(node));
        	heads[i]->next = NULL;
    	}

	for(i = 0; i<no; i++) {
		strcpy(heads[i]->input,argv[i+3]);
		strcpy(heads[i]->key,key);
		pthread_attr_t attr;
        	pthread_attr_init(&attr);
		pthread_create (&tid[i], &attr, runner, heads[i]); 
		
	}
	
	for(i = 0; i<no; i++) {
		pthread_join (tid[i], NULL); 
	}

	FILE *ptr_file;
	ptr_file =fopen(argv[argc-1], "w");

	if (!ptr_file)
		printf("File cannot opened\n");
		
		
	for(i = 0; i<no; i++) {
		while(heads[i] != NULL) {
			fprintf(ptr_file, "%s", heads[i]->data);
			heads[i] = heads[i]->next;
		}
	}	
	
		
	
	fclose(ptr_file);

	for(int i = 0; i < no; i++) {
		free(heads[i]);
    	}

	clock_t end = clock();
	double time = (double)(end - begin)/CLOCKS_PER_SEC;
	printf("The program takes: %.6f seconds\n", time);

	
	return 0;
}
