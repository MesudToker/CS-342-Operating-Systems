#include <linux/init.h>
#include <linux/stat.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/fdtable.h>
#include <linux/path.h>
#include <linux/dcache.h>
#include <linux/slab.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("ERKAN ÖNAL and ÖMER MESUD TOKER");
MODULE_DESCRIPTION("Process Monitoring Module");

int a = 0;

//Sub-method to print tree structure
char * space(int a){
	if (a == 0)
		return "";
	char b[2000];
	int n = 0;
	while((n/5)<a){
		b[n] = ' ';
		b[n+1] = ' ';
		b[n+2] = ' ';
		b[n+3] = ' ';
		b[n+4] = ' ';
		n = n+5;
	}
	
	n = 0;
	
	while((n/5)< a) {
		printk("%c", b[n++]);
	}
	return "";
}

//Recursively printing the structure of the tree
void print_tree(struct task_struct *root){
	struct list_head *theList;
	struct task_struct *new;
	struct task_struct *new_copy;
	a = 0;
	list_for_each(theList, &root->children){
		new = list_entry(theList, struct task_struct, sibling);
		new_copy = new;
		a = 0;
		while(new_copy->parent != &init_task){
			new_copy = new_copy->parent;
			a++;
		}  
		    
		printk("%s | [%d], [%s], [%d]\n",space(a), new->pid, new->comm, new->parent->pid /*, head_of_children*/);

		if((&new->children)->next != &new->children)
			print_tree(new);
	}
}
//Printing VM region informations
void print_vm_info_process(struct task_struct *my_current,int id, int *flag) { 
	struct task_struct *next_process = NULL; 
	struct list_head *head_of_children_list;
  
	if(my_current->pid == id){
		*flag = 1;
		printk(KERN_INFO "Process is found: %d\n", id); 
		struct vm_area_struct *list_of_vms = NULL;
		struct vm_area_struct *list_of_vms_temp = NULL;
		if( my_current->mm != NULL){
			list_of_vms = my_current->mm->mmap;
			list_of_vms_temp = list_of_vms;
		}
		if(list_of_vms == NULL){
			printk(KERN_ALERT "The process' virtual memory size is 0\n");
			return;
		}

		int VM_counter = 0;
		long vm_size_total = 0;
		long vm_size = 0;
		while(list_of_vms_temp != NULL){
			long vm_start_address = list_of_vms_temp->vm_start;
			long vm_end_address = list_of_vms_temp->vm_end;
			vm_size = (vm_end_address - vm_start_address);
			printk(KERN_INFO "VM REGION %d START ADDRESS: %lu\n",VM_counter, vm_start_address);
			printk(KERN_INFO "VM REGION %d END ADDRESS: %lu\n",VM_counter, vm_end_address);
			printk(KERN_INFO "VM REGION %d ADDRESS SIZE: %lu bytes = %lu KB \n",VM_counter, vm_size, vm_size/1024);
			vm_size_total = vm_size_total + vm_size;
			list_of_vms_temp = list_of_vms_temp->vm_next;
			VM_counter++;
		}
		if(list_of_vms != NULL){
			printk(KERN_INFO "TOTAL SIZE OF THE VM REGION USED BY PROCESS %d IS: %lu bytes = %lu KB\n",id, vm_size_total,   vm_size_total/1024);
		}
		return;
	}
	//recursively call this function on each child of the current process  	
	head_of_children_list = &(my_current->children); 
	list_for_each_entry(next_process, head_of_children_list, sibling) {
		print_vm_info_process(next_process, id, flag);
	} 
}
//Printing file information of a file
void print_data_for_process(struct task_struct *my_current, int id, int *flag) { 
	struct task_struct *next_process = NULL; 
	struct list_head *head_of_children_list;
  
	if(my_current->pid == id){
		*flag = 1;
		printk("Process is found: %d\n", id); 
		//struct file *list_of_files[64];
		//memcpy(list_of_files, my_current->files->fd_array, sizeof list_of_files);
		
		//My test cases
		struct files_struct *current_files; 
		struct fdtable *files_table;
		unsigned int *fds;
		int i=0;
		struct path files_path;
		char *cwd;
		char *buf = (char *)kmalloc(GFP_KERNEL,100*sizeof(char));
	
		current_files = my_current->files;
		files_table = files_fdtable(current_files);
		int entered = 1;
		while(files_table->fd[i] != NULL) { 
			struct file *myfile = files_table->fd[i];
			unsigned long *blocks = myfile->f_inode->i_blocks;
		      unsigned long *ino = myfile->f_inode->i_ino;
			char *file_name = myfile->f_path.dentry->d_name.name;
			long size = myfile->f_inode->i_size;
			printk(KERN_ALERT "File %d size is: %lu bytes = %lu KB\n", i, size, size/1024);	
			printk(KERN_ALERT "File %d name is: %s  \n", i, file_name);
			printk(KERN_ALERT "File %d fd is: %d  \n", i, i);
			printk(KERN_ALERT "File %d inode number is: %lu \n", i, ino);
		      printk(KERN_ALERT "File %d no of blocks is: %lu\n\n", i, blocks);
			i++;
			entered = 0;
		}
		if(entered == 1)
			printk(KERN_ALERT "Process %d has currently no open files\n", my_current->pid);
		return;			
	}
	//recursively calling this function on each child of the current process  	
	head_of_children_list = &(my_current->children); 
	list_for_each_entry(next_process, head_of_children_list, sibling) {
		print_data_for_process(next_process, id, flag);
	} 
} 	

//Parameter to pass while inserting the module
static int PID = 1;
module_param(PID, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

static int __init init_func(void){
	printk(KERN_INFO "Initialization started\n");
	struct task_struct *my_current;
	my_current = current; 
	
	while (my_current->pid != 0){ 
		my_current = my_current->parent; 
	} 
	
	printk("Process information...\n");

	printk(" | [PID], [name], [parent PID]\n"); 
  	print_tree(my_current);
	int flag1 = 0;	
	int flag2 = 0;	
  	print_vm_info_process(my_current, PID, &flag1); 
	if(flag1 == 0)
		printk(KERN_ALERT "There is no process whose PID is: %d\n", PID);

	print_data_for_process(my_current, PID, &flag2);
	if(flag2 == 0)
		printk(KERN_ALERT "There is no process whose PID is: %d\n", PID);
	

	printk(KERN_ALERT "MY PID is: %d", PID); 
 	
	return 0;
}

static void __exit cleanup_func(void){
	printk(KERN_INFO "MODULE REMOVED\n");
}

module_init(init_func);
module_exit(cleanup_func);
