#include <linux/module.h>
#include <linux/proc_fs.h>
#include <asm/io.h>

#define PROC_ENTRY_PATH "physlin"

static struct proc_dir_entry* physlin_entry = NULL;

static ssize_t physlin_read_proc(struct file* filp, char* buf, size_t count, loff_t* offp)
{
	char answer_str[64] = "";
	int len = sizeof(answer_str);
	ssize_t ret = len;

	return ret;
}

static ssize_t physlin_write_proc(struct file* filp, const char* buf,size_t count, loff_t* offp)
{
	return count;
}

struct proc_ops proc_physlin_ops = {
	.proc_read = physlin_read_proc,
	.proc_write = physlin_write_proc
};

static int physlin_proc_init(void)
{
	physlin_entry = proc_create(PROC_ENTRY_PATH, 0, NULL, &proc_physlin_ops);

	if (!physlin_entry)
		return -1;

	return 0;
}

static void physlin_proc_close(void)
{
	if (physlin_entry) {
		physlin_entry = NULL;
		remove_proc_entry(PROC_ENTRY_PATH, NULL);
	}
}

static int __init physlin_init(void)
{
	if (physlin_proc_init() < 0) {
		printk(KERN_ERR "error: can't create /proc/%s", PROC_ENTRY_PATH);
		return -1;
	}

	return 0;
}

static void __exit physlin_cleanup(void)
{
	physlin_proc_close();
}

module_init(physlin_init);
module_exit(physlin_cleanup);

MODULE_AUTHOR("Vladislav Korneev");
MODULE_DESCRIPTION("Read physical addresses");
MODULE_LICENSE("GPL");
