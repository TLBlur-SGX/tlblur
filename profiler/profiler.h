#include <stdint.h>

extern char *ENCLAVE_SO;

void profiler_setup(int eid, int e_size, void *e_start, uint64_t argc,
                    char **argv);
void profiler_run(int eid);
void profiler_destroy(int eid);
