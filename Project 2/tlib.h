#ifndef _TLIB_H_
#define _TLIB_H_
#include <ucontext.h>

#define TLIB_MAX_THREADS 1024
#define TLIB_MIN_STACK  32768

int tcbNo;
int tcbCount;

/* You will define your TCB structure here */
typedef struct tcb {
    	void *stackptr;
        int id;
        int state;
        ucontext_t *ucp; 
        struct tcb * next;  
} tcb;

tcb *tcbRoot;
tcb *tcbCurrent;
//tcb *tcbList;
tcb *tcbTail;
//tcb *tcb3;

/* Some definitions below */

static const int TLIB_ANY = -1;
static const int TLIB_SELF = -2;
static const int TLIB_INVALID = -3;
static const int TLIB_NONE = -4;
static const int TLIB_NOMORE = -5;
static const int TLIB_NOMEMORY = -6;
static const int TLIB_FAILED = -7;
static const int TLIB_SUCCESS = -8;
static const int TLIB_ERROR = -9;


int tlib_init(void);
int tlib_create_thread (void (*func)(void *), void *param);
int tlib_yield (int tid);
int tlib_delete_thread (int tid);

#endif



