Ömer Mesud TOKER	21302479
Nihad AZIMLI		21402907

Project 2 - Verison 2

Changes 

Our first code has only a problem in delete so that we got 50. 
However, by changing only 1 line, our code works well. 
What we did is in tlib_delete_thread function, at line 189 and 290 we had
	if(yielding)
                tlib_yield(TLIB_ANY); 
code. We changed it as 
	if(yielding)
            	setcontext(tcbCurrent->ucp);
                //tlib_yield(TLIB_ANY);
Yielding in delete causes exiting of the process since after returning from yield, it returns end of the delete function so end of the stub. And it causes process to exit. 
By changing it into setcontext, we eliminate that problem. So now our code works well. 


int tlib_init

We initialize the main thread (id = 0) and its tcb (tcbRoot) here. 
If successful returns TLIB_SUCCESS.


int tlib_create_thread(void (*func)(void *), void *param)

We create a thread with calling this function. 
We add created tcb to the our tcb linked list. (Head of the linked list is tcbRoot)
By getcontext, we get the context and put in the thread's ucp. 
Then we allocate stack for the thread, and change its REG_EIP to stub, REG_ESP to sp, which is the stack pointer.
By sp[1] = func, sp[2] = param instructions we put the func and param into the thread's stack.
When this thread is yielded it starts its execution from stub function. 
If successful returns the thread's id.


int tlib_yield(int wantTid)

It yields the CPU to the thread whose id is wantTid.
Caller thread's context is saved to start from where it is left when it is yielded again.
Context is setted to the called thread's context to start its execution. 
If successful returns the called or yielded thread's id, which is wantTid.


int tlib_delete_thread(int tid)

It deletes the thread whose id is tid.
After deleteing it yileds any thread.
If successful returns TLIB_SUCCESS.






