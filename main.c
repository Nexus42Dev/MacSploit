#include <stdio.h>
#include <stdlib.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <libproc.h>
#include <pthread.h>

#define STACK_SIZE 65536
char shellcode[19] = "\x55\x48\x89\xE5\x48\x8D\x3D\xBA\xE3\xFF\x01\x31\xC0\xE8\xBD\x92\xD8\x01\xC3";


uint64_t binja_address = 0x100000000;
uint64_t print_address = 0x102474B61;
uint64_t base_address = 0;

uint64_t aslr_bypass(uint64_t addr) {
    return print_address - binja_address + base_address;
}

int find_roblox_pid() {
    pid_t pid_list[512 * sizeof(pid_t)];
    struct proc_bsdinfo pinfo;
    int byte_count = proc_listpids(PROC_ALL_PIDS, 0, pid_list, 2048);
    int proc_count = byte_count / 4;
    for (int i = 0; i < proc_count; i++) {
        int read = proc_pidinfo(pid_list[i], PROC_PIDTBSDINFO, 0, &pinfo, PROC_PIDTBSDINFO_SIZE);
        if (read == PROC_PIDTBSDINFO_SIZE) {
            if (strcmp(pinfo.pbi_name, "RobloxPlayer") == 0) {
                return pid_list[i];
            }
        }
    }

    return 0;
}

int main() {
    //Process Attach
    char print_string[128];
    mach_port_t task, object_name;
    kern_return_t task_retval, region_retval, alloc_retval, write_retval;
    vm_info_region_t region_info;
    mach_vm_size_t region_size;
    mach_vm_address_t address = 0;
    mach_msg_type_number_t count = VM_REGION_BASIC_INFO_COUNT_64;
    mach_vm_address_t alloc_address;

    //Stack + Thread
    kern_return_t alloc_shellcode_retval, stack_retval, thread_retval;
    mach_vm_address_t alloc_shellcode_address, stack_address;
    x86_thread_state64_t remote_thread_state;
    memset(&remote_thread_state, '\0', sizeof(remote_thread_state));
    thread_act_t remote_thread;

    //Find Roblox Process
    int roblox_pid = find_roblox_pid();
    if (!roblox_pid) {
        printf("Roblox must be open for this to work.\n");
        return 1;
    }

    //Setup Print Script
    //printf("What would you like to print?\n");
    //scanf("%s", print_string);
    strcpy(print_string, "Hello World!");

    //Open Roblox Process
    printf("Roblox Process Found: %d\n", roblox_pid);
    task_retval = task_for_pid(mach_task_self(), roblox_pid, &task);
    if (task_retval != KERN_SUCCESS) {
        printf("Failed to task_for_pid(): System Error %d.\n", task_retval);
        return 1;
    }

    //Retrieve Module Base Address
    printf("Roblox Process Hooked!\n");
    region_retval = mach_vm_region(task, &address, &region_size, VM_REGION_BASIC_INFO, (vm_region_info_t)&region_info, &count, &object_name);
    if (region_retval != KERN_SUCCESS) {
        printf("Failed to produce region info: System Error %d.\n", region_retval);
        return 1;
    }

    base_address = address;
    printf("Found Roblox Print Address: 0x%llX\n", aslr_bypass(print_address));

    //Write String to Memory
    alloc_retval = mach_vm_allocate(task, &alloc_address, strlen(print_string) + 1, VM_FLAGS_ANYWHERE);
    if (alloc_retval != KERN_SUCCESS) {
        printf("Failed to allocate memory inside process: %d.\n", alloc_retval);
        return alloc_retval;
    }
    
    vm_protect(task, alloc_address, strlen(print_string), FALSE, VM_PROT_WRITE | VM_PROT_READ);
    write_retval = mach_vm_write(task, alloc_address, (uint64_t)print_string, strlen(print_string) + 1);
    if (write_retval != KERN_SUCCESS) {
        printf("Failed to write memory to proccess: %d.\n", write_retval);
        return write_retval;
    }

    printf("Wrote string to memory: 0x%llX\n", alloc_address);

    alloc_shellcode_retval = mach_vm_allocate(task, &alloc_shellcode_address, sizeof(shellcode), VM_FLAGS_ANYWHERE);
    if (alloc_shellcode_retval != KERN_SUCCESS) {
        printf("Failed to allocate shellcode memory: %d.\n", alloc_shellcode_retval);
        return alloc_shellcode_retval;
    }

    *(int*)((uint64_t)shellcode + 6) = (alloc_address - (alloc_shellcode_address + 11));
    *(int*)((uint64_t)shellcode + 14) = (aslr_bypass(print_address) - (alloc_shellcode_address + 18));
    printf("Created Shellcode Successfully.\n");

    kern_return_t write_shellcode_retval;
    vm_protect(task, alloc_shellcode_address, sizeof(shellcode), FALSE, VM_PROT_WRITE | VM_PROT_READ);
    write_shellcode_retval = mach_vm_write(task, alloc_shellcode_address, (uint64_t)shellcode, sizeof(shellcode));
    if (write_shellcode_retval != KERN_SUCCESS) {
        printf("Failed to write shellcode to memory: %d.\n", write_shellcode_retval);
        return write_shellcode_retval;
    }
    
    printf("Successfully Injected Shellcode into Process: 0x%llX\n", alloc_shellcode_address);
    stack_retval = mach_vm_allocate(task, &stack_address, STACK_SIZE, VM_FLAGS_ANYWHERE);
    if (stack_retval != KERN_SUCCESS) {
        printf("Failed to allocate stack: %d.\n", stack_retval);
        return stack_retval;
    }

    vm_protect(task, alloc_shellcode_address, sizeof(shellcode), FALSE, VM_PROT_EXECUTE | VM_PROT_READ);
    vm_protect(task, stack_address, STACK_SIZE, TRUE, VM_PROT_WRITE | VM_PROT_READ);
    printf("Allocated Remote Stack: 0x%llX\n", stack_address);

    //Stack Init
    stack_address += (STACK_SIZE / 2);
    remote_thread_state.__rip = alloc_shellcode_address;
    remote_thread_state.__rsp = stack_address;
    remote_thread_state.__rbp = stack_address;

    //Create Thread
    thread_retval = thread_create_running(task, x86_THREAD_STATE64, (thread_state_t)&remote_thread_state, x86_THREAD_STATE64_COUNT, &remote_thread);
    if (thread_retval != KERN_SUCCESS) {
        if (thread_retval == KERN_INVALID_ARGUMENT) {
            printf("One of the arguments to thread_create_running are incorrect.\n");
            return thread_retval;
        }

        printf("Failed to create process thread: %d.\n", thread_retval);
        return thread_retval;
    }

    printf("Print executed.\n");
    //Cleanup...
    return 0;
}
