#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <assert.h>
#include <stdint.h>
#define __USE_GNU

#include <ucontext.h>
#include "tlib.h"



int tlib_init (void)
{
    tcbRoot = (tcb*)malloc(sizeof(tcb));
    tcbCount++;
    
    
    if (tcbRoot == NULL)
	return (TLIB_NOMEMORY);
	
    tcbRoot->id =tcbNo;
    tcbRoot->state = 1;

	
    tcbRoot->ucp = malloc(sizeof(ucontext_t)); // TODO
    
    //tcbRoot->ucp->uc_stack.ss_sp = (char*) malloc(TLIB_MIN_STACK);
    //tcbRoot->ucp->uc_stack.ss_size = TLIB_MIN_STACK;
    
    
    tcbNo++;

	tcbRoot->next = NULL;
	
	getcontext(tcbRoot->ucp);
	
	if(tcbRoot->ucp == NULL) 
		return (TLIB_ERROR);
	
	tcbCurrent = tcbRoot;
	
	printf("Main thread is created\n");
    return (TLIB_SUCCESS);
}

/* implementation of stub is already given below */
void stub (void (*tstartf)(void *), void *arg)
{
    printf("stub basladi\n");
    fflush(stdout);
    	tcbCurrent->state = 1;
    
	tstartf (arg); /* calling thread start function to execute */
    printf("\nstub tstratf bitti\n");
    
    /* 
        We are done with executing the application specified thread start
        function. Hence we can now terminate the thread
    */
	tlib_delete_thread(TLIB_SELF);
    printf("\nstub b\n");
    exit(0);
    /*
    stub (void (*tstartf) (void*), void *param)
    {
    // thread starts here
    int ret;
    tstartf (param); // thread start function specified by the application
    ret = tlib_delete_thread (TLIB_SELF)
    exit(0)
    }*/
}


int tlib_create_thread(void (*func)(void *), void *param)
{
   if(tcbCount == TLIB_MAX_THREADS)
   	return(TLIB_NOMORE);
   	
        tcb *tcbTemp = tcbCurrent;
        while(tcbTemp->next != NULL)
            tcbTemp = tcbTemp->next;
        
        /*
        tcb *tcbTemp = tcbRoot;
        int i;
        for(i = 1; i < tcbCount; i++)
        	tcbTemp = tcbTemp->next;  
        */	  
            
        tcb *tcbNew;
        tcbNew = (tcb*)malloc(sizeof(tcb));
        if(tcbNew == NULL) 
		return (TLIB_FAILED);
        
        tcbNew->id = tcbNo++;
        tcbCount++;
        tcbNew->state = 0;
        
        tcbNew->ucp = malloc(sizeof(ucontext_t));
        
        if(tcbNew->ucp == NULL) 
		return (TLIB_ERROR);
        
        //tcbNew->next = NULL;
                
        tcbTemp->next = tcbNew;
        
        tcbTail=tcbNew;
        //tcbNew->ucp->uc_stack.ss_sp = (char*) malloc(TLIB_MIN_STACK);
        
        
        //greg_t *sp = (greg_t*)((uint32_t) tcbNew->ucp->uc_stack.ss_sp
	//+ tcbNew->ucp->uc_stack.ss_size);
	
	getcontext(tcbNew->ucp);
	
	tcbNew->ucp->uc_stack.ss_sp = malloc(TLIB_MIN_STACK);
	tcbNew->ucp->uc_stack.ss_size = TLIB_MIN_STACK;
	
	if(tcbNew->ucp->uc_stack.ss_sp == NULL)
		return(TLIB_NOMEMORY);
	
	//uint32_t *sp = malloc(tcbNew->ucp->uc_stack.ss_sp+tcbNew->ucp->uc_stack.ss_size);
	//tcbNew->ucp->uc_stack.ss_sp = sp;
	
        uint32_t *sp = malloc(TLIB_MIN_STACK*2);
        printf("parametre for this thread = %d\n", param);
        //printf("id of this thread = %d\n", tcbNew->id);
             
        tcbNew->ucp->uc_mcontext.gregs[REG_EIP] = &stub;
        tcbNew->ucp->uc_mcontext.gregs[REG_ESP] = sp;
        
        sp[0] = NULL;
        sp[1] = func;
        sp[2] = param;
                 
        //sp = sp+2;
        
       
                 
    return tcbNo-1;
}


int tlib_yield(int wantTid)
{                   
	tcbCurrent->state = 0; 
	int none = 0;
	//int invalid = 0;
	
	if(tcbCount == 1) {
		printf("There is only 1 thread, so return to itself\n");
		wantTid = TLIB_SELF;
		none = 1;
	}
	
	if(wantTid == TLIB_SELF) {
		/*
		if(tcbCount == 1) {
			printf("There is only 1 thread, so return to itself\n");
			none = 1;
		}*/
		tcbCurrent->state = 1;
		wantTid = tcbCurrent->id;
		printf("Yielding to TLIB_SELF, id = %d\n", wantTid);
	}
        
	else if(wantTid == TLIB_ANY) {
		/*
		tcb *tcbTemp=tcbCurrent;
		tcbCurrent=tcbCurrent->next;
		tcbTail->next=tcbTemp;
		//tcbTemp->next=NULL;
		tcbTail=tcbTemp;
		*/
		if(tcbCurrent->next != NULL)	
			wantTid = tcbCurrent->next->id;
			
		else wantTid = 0;	
		printf("Yielding to ANY which is next, id = %d\n", wantTid);
        }
        
        //else{
		tcb *tcbTemp = tcbRoot;		
		for (int i = 0; i < tcbCount ; i++) { 
			if(tcbTemp->id == wantTid) {
				int run = 0;	
        			getcontext(tcbCurrent->ucp);
        			
        			if(!run) {
        				run = 1;
        				tcbCurrent = tcbTemp;
        				tcbCurrent->state = 1;	
        				printf("Yielding to thread, id = %d\n", wantTid);
        				setcontext(tcbCurrent->ucp);
        			}
        			printf ("returning from yield\n");
        			if(none)
        				return (TLIB_NONE);
        			return wantTid;
			}
			if(i == tcbCount-1)
				break;
			tcbTemp = tcbTemp->next;
		}
		printf ("returning from yield, no vaild id\n");
        	return (TLIB_INVALID);
        //}
	   
        //makecontext(tcbCurrent->ucp, &stub, 2,tcbCurrent->func,tcbCurrent->parameter);//TODO

        //return (TLIB_ERROR);
}


int tlib_delete_thread(int tid)
{
	int yielding=0;
    int inv=0;
   

    if(tcbCount==1){
        tcbRoot->ucp->uc_stack.ss_size=0;
        tcbRoot->ucp->uc_link=0;
        printf("\n\n7\n\n");
        free(tcbRoot->ucp->uc_stack.ss_sp);
        printf("\n\n8\n\n");
        tcbRoot->next=NULL;
        free(tcbRoot);
        return (TLIB_NONE);
    }


     if(tid == TLIB_SELF || tid == tcbCurrent->id) {
        tid = tcbCurrent->id;
        yielding=1;
       
       
        printf("Deleting to TLIB_SELF, id = %d\n", tid);
    }
      
    else if(tid == TLIB_ANY) {
        if(tcbCurrent->next != NULL)  
            tid = tcbCurrent->next->id;
        else tid = tcbRoot->next->id;
       
        printf("Deleting to ANY which is next, id = %d\n", tid);
    }
   

    tcb *tcbPrev;
    tcb *tcbTemp = tcbRoot;
   
    printf ("\ndelete is STARTed\n");
    for (int i=0; i<tcbCount-1; i++){
        if(tcbTemp->next->id==tid){
            tcb* tcbDelete=tcbTemp->next;
            if(tcbCurrent == tcbDelete) {
            	if(tcbCurrent->next != NULL)
            		tcbCurrent = tcbCurrent->next;
            	else tcbCurrent = tcbRoot;
            }
            	
            if(tcbDelete->next!=NULL)
                tcbTemp->next=tcbDelete->next;
            else
                tcbTemp->next=tcbRoot;
                
            //tcbDelete->next=NULL;
            tcbDelete->ucp->uc_stack.ss_size=0;
            printf ("\n1\n");
            tcbDelete->ucp->uc_link=0;
            printf ("\n1\n");
            //free(tcbDelete->ucp->uc_stack.ss_sp);
            printf ("\n1\n");
            free(tcbDelete);
            printf ("\n1\n");
            tcbCount--;
            
            /*
            if(tcbCurrent->next!=NULL)
              tcbCurrent=tcbCurrent->next;
            else
              tcbCurrent=tcbRoot;*/
              
            if(yielding)
            	setcontext(tcbCurrent->ucp);
                //tlib_yield(TLIB_ANY);
            break;

        }
        if(i == tcbCount-1){
            inv=1;
            break;
        }
        printf("webzuliyaaa\n");
        tcbPrev=tcbTemp;
        tcbTemp=tcbTemp->next;
    }
    if(inv){
        printf("returning from delete no valid ID found");
        return (TLIB_INVALID);
    }
    //tlib_yield(TLIB_ANY);
    return (TLIB_SUCCESS);
	printf ("\nthread %d deleted\n", tid);
    //return (TLIB_ERROR);
}

