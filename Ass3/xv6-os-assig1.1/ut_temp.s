	.text

/* Switch from current_thread to next_thread. Make next_thread
 * the current_thread, and set next_thread to 0.
 * Use eax as a temporary register, which should be caller saved.
 */
	.globl thread_switch
thread_switch:

	movl current_thread,%eax
	movl %esp,(%eax)

	movl %eax, %esp
	addl $(8192),%esp 
	pushal
	movl next_thread,%eax
	movl %eax,current_thread
	 movl current_thread,%esp 
	addl $8160,%esp  
/*	 (4+8192-4-32)  */
	popal

	movl current_thread,%eax
	movl (%eax),%esp

    movl current_thread,%eax 
	addl $8196,%eax
    push %eax

	movl $0, next_thread
	ret				/* pop return address from stack */
