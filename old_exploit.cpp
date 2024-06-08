#include <iostream>
#include <unistd.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <mach-o/dyld.h>
#include <mach-o/dyld_images.h>

#include <thread>

static void init_thread() __attribute__((constructor));

uint64_t binja_address = 0x100000000;
uint64_t deserialize_address = 0x100079269;
uint64_t join_address = 0x100079269;
uint64_t print_address = 0x100b3336a;
uint64_t base_address = 0;
uint64_t gettaskscheduler_address = 0x10221b6c6;
uint64_t unk_address = 0x10247aa36;
uint64_t gettop_address = 0x1019e85e7;
uint64_t unk_exit = 0x1019e8551;

typedef void*(* unk_t)(char* arg1);
unk_t unk_void = (unk_t)0x1019e854b;

char create_job_bytes[12] = "\x55\x48\x89\xe5\x41\x57\xe9\x11\x11\x11\x11";
typedef int64_t(* create_job_t)(void** job_ptr, const char* jobname, double* arg3, int32_t arg4);
create_job_t create_job;

typedef int64_t(*deserialize_t)(uint64_t rl, const char* source, const char* arg3, int size, int arg5);
deserialize_t rbx_deserialize;

uint64_t joblocation = 0;
uint64_t last_state = 0;
int64_t create_job_hook(void** job_ptr, const char* jobname, double* arg3, int32_t arg4) {
    if (strcmp(jobname, "WaitingHybridScriptsJob") == 0) {
        joblocation = (uint64_t)job_ptr;
        printf("\nArgument List:\n  - 1: 0x%llX\n   - 2: %s\n   - 3: 0x%llX\n   - 4: 0x%llX\n\n", (uint64_t)job_ptr, jobname, (uint64_t)arg3, (uint64_t)arg4);
    }

    return create_job(job_ptr, jobname, arg3, arg4);
}

template<typename T>
T read(const char* data, size_t size, size_t& offset)
{
    T result;
    memcpy(&result, data + offset, sizeof(T));
    offset += sizeof(T);

    return result;
}

uint32_t readVarInt(const char* arg3, size_t size, size_t& offset) {
    unsigned int result = 0;
    unsigned int shift = 0;

    uint8_t byte;

    do
    {
        byte = read<uint8_t>(arg3, size, offset);
        result |= (byte & 127) << shift;
        shift += 7;
    } while (byte & 128);

    return result;
}

typedef struct Closure
{
    uint8_t white;
    uint8_t tt;
    uint8_t memcat;

    uint8_t isC;
    uint8_t nupvalues;
    uint8_t stacksize;
    uint8_t preload;

    uint64_t gclist;
    uint64_t env;

    union
    {
        struct
        {
            uint64_t f;
            uint64_t cont;
            const char* debugname;
            char upvals[16];
        } c;

        struct
        {
            uint64_t p;
            uint64_t uprefs[1];
        } l;
    };
} Closure;

typedef int64_t(*delay_t)(uint64_t arg1, uint64_t arg2, uint64_t arg3, double delay);
delay_t rbx_delay;

uint64_t aslr_bypass(uint64_t addr) {
    return addr - binja_address + base_address;
}

int64_t delay_hook(uint64_t arg1, uint64_t arg2, uint64_t arg3, double delay) {
    printf("\n[DELAY] Hook Step!\n");
    printf("[DELAY] Supposed Value: %4.2f\n", delay);
    printf("[DELAY] New Value: %4.2f\n\n", 0.01);
    return rbx_delay(arg1, arg2, arg3, 0.01);
}

int64_t deserialize_hook(uint64_t rl, char* name, char* arg3, int size, int arg5) {
    last_state = rl;
    int64_t res = rbx_deserialize(rl, name, arg3, size, arg5);
    printf("[Caught Deserialize] %s on 0x%llX\n", name, rl);
    printf("Version: 0x%X\n\n", *arg3);

    //for (int i = 0; i < size; i++) {
    //    if (i % 15 == 0) printf("\n");
    //    printf("\\x%02X", ((u_char*)arg3)[i]);
    //}

    //printf("\n\n");

    //size_t offset = 1;
    //int stringCount = readVarInt(arg3, size, offset);
    //printf("String Count: %d\n", stringCount);
    //Closure* closure = *(Closure**)((*(uint64_t*)(rl + 0x20)) - 0x10);
    //printf("LClosure Pushed: 0x%llX\n", closure->l.p - (uint64_t)(&closure->l.p));
    //FILE* fp = fopen("/Users/Nexus42/Desktop/test.txt", "w");
    //((int64_t(*)(FILE* arg1, int64_t arg2, uint64_t gco))aslr_bypass(0x101a58000))(fp, 0, closure->l.p - (uint64_t)(&closure->l.p));
    //fclose(fp);
    //printf("Executed!!\n");
    //printf("\n%s\n\n", std::string(arg3, size).c_str());
    return res;
}

typedef uint64_t(* newthread_t)(uint64_t rl);
uint64_t newthread_address = 0x101a4db89;

typedef char*(*unk_function_t)(const char* arg1);
unk_function_t unk_function;

typedef uint64_t(*gettaskscheduler_t)();
gettaskscheduler_t rbx_gettaskscheduler;

typedef int64_t(*print_t)(int32_t type, const char* str, ...);
print_t rbx_print;

typedef void(* setfield_t)(uint64_t rl, int idx, const char* key);
setfield_t rbx_setfield = (setfield_t)0x101a4f369;

typedef void(* pushcclosure_t)(uint64_t rl, void* function, const char* debugname, int nup, uint64_t cont);
pushcclosure_t rbx_pushcclosure = (pushcclosure_t)0x101a4eb8d;

typedef void(* pushlstring_t)(uint64_t rl, const char* string, int64_t len);
pushlstring_t rbx_pushlstring = (pushlstring_t)0x101a4e97d;

uint64_t deserialize_body = 0x10007926f;
uint64_t join_body = 0x10007926f;
void rbx_pushstring(uint64_t rl, std::string string) {
    rbx_pushlstring(rl, string.c_str(), string.size());
}

void tramp_hook() {
    int tramp_len = sizeof(create_job_bytes) - 1;
    mach_vm_address_t tramp_addr;
    mach_vm_allocate(mach_task_self(), &tramp_addr, tramp_len, VM_FLAGS_ANYWHERE);
    mach_vm_protect(mach_task_self(), tramp_addr, tramp_len, FALSE, VM_PROT_ALL);
    memcpy((void*)tramp_addr, create_job_bytes, tramp_len);

    printf("[/] Tramp Location: 0x%llX\n", tramp_addr);
    printf("[/] Tramp Length: %d\n", tramp_len);

    int rel_addr = aslr_bypass(join_body) - (tramp_addr + tramp_len);
    *(int*)(tramp_addr + tramp_len - 4) = rel_addr;
    rbx_delay = (delay_t)tramp_addr;
    mach_vm_protect(mach_task_self(), tramp_addr, tramp_len, FALSE, VM_PROT_EXECUTE | VM_PROT_READ);

    uint64_t join_addr = aslr_bypass(join_address);
    mach_vm_protect(mach_task_self(), join_addr, 5, FALSE, VM_PROT_ALL);
    int hook_rel_addr = (uint64_t)delay_hook - (join_addr + 5);
    uint8_t jmp_bytes[5] = { 0xE9, 0x0, 0x0, 0x0, 0x0 };
    printf("[/] rbx_delay() Location: 0x%llX\n", join_addr);

    memcpy((void*)join_addr, jmp_bytes, 5);
    *(int*)(join_addr + 1) = hook_rel_addr;
    mach_vm_protect(mach_task_self(), join_addr, 5, FALSE, VM_PROT_EXECUTE | VM_PROT_READ);
}

int httpget(uint64_t rl) {
    rbx_pushstring(rl, "Hello World!");
    return 1;
}

typedef uint64_t(* getthread_t)(int32_t* arg1, int32_t arg2);
getthread_t lua_getthread = (getthread_t)0x101a61cc8;

uint64_t rbx_getstate(uint64_t sc, int type) {
    int32_t type_1;
    printf("Retrieved Type Information: %lld\n", *(int64_t*)(sc + 0xe8));
    sleep(120);
    ((int64_t(*)(int32_t*, int32_t, int64_t, int64_t))aslr_bypass(0x101a61cb8))(&type_1, type, 0, 0);
    uint64_t thread_num = lua_getthread(&type_1, 5) * 0xe8;
    int32_t thread_1 = *(int32_t*)((sc + thread_num) + 0x188);
    int32_t thread_2 = *(int32_t*)((sc + thread_num) + 0x18c);
    return ((((uint64_t)(thread_2 - ((sc + thread_num) + 0x188))) << 0x20) | ((uint64_t)(thread_1 - ((sc + thread_num) + 0x188))));
}

uint64_t main_state = 0;
void exploit_thread() {
    rbx_print = (print_t)aslr_bypass(print_address);
    rbx_gettaskscheduler = (gettaskscheduler_t)aslr_bypass(gettaskscheduler_address);
    lua_getthread = (getthread_t)aslr_bypass((uint64_t)lua_getthread);
    
    while (true) {
        std::string print_script;
        std::getline(std::cin, print_script);
        
        if (print_script == "inject") {
            //rbx_pushcclosure = (pushcclosure_t)aslr_bypass((uint64_t)rbx_pushcclosure);
            //rbx_setfield = (setfield_t)aslr_bypass((uint64_t)rbx_setfield);
            //rbx_pushlstring = (pushlstring_t)aslr_bypass((uint64_t)rbx_pushlstring);
            //newthread_t rbx_newthread = (newthread_t)aslr_bypass(newthread_address);
            main_state = last_state;
            //((int64_t(*)(uint64_t, int))aslr_bypass(0x101a4dc3e))(last_state, 0);
            //printf("Thread On Stack: 0x%llX\n", *(*(uint64_t**)(last_state + 0x20) - 2));
            rbx_print(0, "Successful: %s", "Hello World");
            //printf("Result: %d\n", *((int32_t*(*)())aslr_bypass(0x101ac7930))());

            //init functions
            //rbx_pushcclosure(main_state, (void*)httpget, "httpget", 0, 0);
            //rbx_setfield(main_state, -10002, "httpget");
            printf("Added Functions!\n");

            //int* iden = ((int*(*)())aslr_bypass(0x101ac7930))();
            //printf("Identity?: 0x%X\n", *(int*)(main_state + 2504));
            printf("Abyss Injected Into State 0x%llX!\n", main_state);
            continue;
        }

        if (print_script == "execute") {
            //*(uint64_t*)(main_state + 0x20) = *(uint64_t*)(main_state + 0x30);
            //printf("Stack Size: 0x%llX\n", ((uint64_t(*)(uint64_t))aslr_bypass(gettop_address))(last_state));
            ((int64_t(*)(uint64_t, int))aslr_bypass(0x1019e85f9))(main_state, 0); //settop
            //printf("Stack Size: 0x%llX\n", ((uint64_t(*)(uint64_t))aslr_bypass(gettop_address))(last_state));
            const char* bytecode = "\x03\x06\x04\x67\x61\x6D\x65\x07\x50\x6C\x61\x79\x65\x72\x73\x0B\x4C\x6F\x63\x61\x6C\x50\x6C\x61\x79\x65\x72\x09\x43\x68\x61\x72\x61\x63\x74\x65\x72\x08\x48\x75\x6D\x61\x6E\x6F\x69\x64\x09\x57\x61\x6C\x6B\x53\x70\x65\x65\x64\x01\x03\x00\x00\x01\x0B\xA3\x00\x00\x00\xA4\x02\x03\x00\x02\x04\x00\xC0\x4D\x01\x02\x69\x04\x00\x00\x00\x4D\x00\x01\x01\x05\x00\x00\x00\x8C\x01\xF4\x01\x30\x01\x00\x8E\x06\x00\x00\x00\x82\x00\x01\x00\x07\x03\x01\x03\x02\x03\x03\x04\x02\x04\x00\xC0\x03\x04\x03\x05\x03\x06\x00\x01\x00\x01\x18\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00";
            size_t len = 146;
            printf("Deserializing... %lu\n", len);
            deserialize_t aby_load = (deserialize_t)aslr_bypass(deserialize_address);
            if (aby_load(main_state, "@Abyss", bytecode, len, 0) != 0) {
                printf("ERROR\n");
                continue;
            }

            ((int64_t(*)(uint64_t, int))aslr_bypass(0x1019e85f9))(main_state, -1); //settop
            printf("Stack Size: 0x%llX\n", ((uint64_t(*)(uint64_t))aslr_bypass(gettop_address))(last_state));
            printf("DESERIALIZED!\n");
            ((int64_t(*)(uint64_t))aslr_bypass(0x1006d9db2))(main_state);
            continue;
        }

        if (print_script == "scandown") {
            uint64_t taskscheduler = rbx_gettaskscheduler();
            printf("\nInitializing Scan on 8000 bytes...\n");
            printf("Taskscheduler: 0x%llX\n", taskscheduler);
            printf("Job: 0x%llX\n", joblocation);

            bool found = false;
            for (int i = 0; i < 8000; i++) {
                if (*(short*)(taskscheduler + i) == 0x007f) {
                    uint64_t ptr = *(uint64_t*)(taskscheduler + i - 5);
                    for (int j = 0; j < 4000; j++) {
                        uint64_t ptr_2 = *(uint64_t*)(ptr + j);
                        if (ptr_2 == joblocation) {
                            found = true;
                            printf("JOB FOUND!\nOffset: %d\nJob: WaitingHybridScriptsJob\nJob Location: 0x%llX\n", i - 5, joblocation);
                        }
                    }
                }
            }

            if (!found) {
                printf("failed to find the job in task scheduler :c\n");
            }

            continue;
        }

        if (print_script == "scanup") {
            uint64_t taskscheduler = rbx_gettaskscheduler();
            printf("\nInitializing Scan on 8000 bytes...\n");
            printf("Taskscheduler: 0x%llX\n", taskscheduler);
            printf("Job: 0x%llX\n", joblocation);

            bool found = false;
            for (int i = 0; i < 8000; i++) {
                if (*(short*)(taskscheduler + i) == 0x007f) {
                    uint64_t ptr = *(uint64_t*)(taskscheduler + i - 5) - 8000;
                    if (ptr > 0x7FF000000000) {
                        printf("BAD: 0x%llX\n", ptr);
                        continue;
                    }
                    if (ptr < 0x7F0000000000) {
                        printf("BAD: 0x%llX\n", ptr);
                        continue;
                    }
                    printf("0x%llX\n", ptr);
                    for (int j = 0; j < 8000; j++) {
                        uint64_t ptr_2 = *(uint64_t*)(ptr + j);
                        if (ptr_2 == joblocation) {
                            found = true;
                            printf("JOB FOUND!\nOffset: %d\nJob: WaitingHybridScriptsJob\nJob Location: 0x%llX\n", i - 5, joblocation);
                        }
                    }
                }
            }

            if (!found) {
                printf("failed to find the job in task scheduler :c\n");
            }

            continue;
        }

        rbx_print(0, print_script.c_str(), 0);

        uint64_t scheduler = rbx_gettaskscheduler();
        uint64_t start = *(uint64_t*)(scheduler + 0x158);
        uint64_t end = *(uint64_t*)(scheduler + 0x160);
        while (start < end) {
            uint64_t job = *(uint64_t*)start;
            void** job_ptr = (void**)job;
            std::string jobname = (*(std::string*)(job + 0x18));
            printf("[Scheduler] %s\n", jobname.c_str());
            if (strcmp(jobname.c_str(), "WaitingHybridScriptsJob") == 0) {
                printf("[Scheduler] Analysing Job...\n");
                printf("Value2: 0x%llX\n", (uint64_t)job_ptr[0x2b]);

                //int64_t ptr_2;
                //uint64_t ptr_1 = ((uint64_t(*)(void*, int*, int64_t*))aslr_bypass(0x1006d7806))(job_ptr[0x2a], &type, &ptr_2);
                //printf("0x%llX\n", ptr_1);

                uint64_t state = rbx_getstate((uint64_t)job_ptr[0x2b], 4);
                printf("[Scheduler] Pointer: 0x%llX\n", state);
                uint64_t unk_ptr = (uint64_t)job_ptr[0x2a];

                last_state = state;
                printf("GETTOP: 0x%llX\n", ((uint64_t(*)(uint64_t))aslr_bypass(gettop_address))(state));
                //newthread_t rbx_newthread = (newthread_t)aslr_bypass(newthread_address);
                //last_state = rbx_newthread(state);

                //int* addr = (int*)(((int64_t(*)(uint64_t rl))aslr_bypass(0x101a4f77c))(last_state) + 0x30);
                //*addr = 7;
                //printf("Sus: %d\n", *addr);
            }

            start += sizeof(uint64_t) * 2;
        }
    }
}

void exploit_thread2() {
    uint64_t waittime_addr = aslr_bypass(0x10335d038);
    printf("[DEBUG PRINT] THREAD STARTED!\n");
    float old_data = 0.0f;
    while (true) {
        float data = *(float*)(waittime_addr);
        printf("[DEBUG PRINT] DELAY: %4.2f\n", data);
        old_data = data;
    }
}

void init_thread() {
    long slide = _dyld_get_image_vmaddr_slide(0);
    base_address = binja_address + slide;
    if (*(char*)aslr_bypass(print_address) != 0x55) return;
    std::cout << "\nAbyss MacOS | Attached to Process\n\n";
    std::cout << "[/] Initializing Roblox...\n";
    tramp_hook();
    std::cout << "[/] Attempted Hook!\n";
    //std::thread(exploit_thread2).detach();
}
