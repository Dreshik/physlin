#include <linux/module.h>
#include <linux/proc_fs.h>
#include <asm/io.h>

#define PROC_ENTRY_PATH "physlin"

static struct proc_dir_entry* physlin_entry = NULL;
static u32 reg_value = 0;

static ssize_t physlin_read_proc(struct file* filp, char* buf, size_t count, loff_t* offp)
{
	char answer_str[64] = "";
	int len = sizeof(answer_str);
	ssize_t ret = len;

	return ret;
}

static int read_reg(u32 physical_addr, u32* reg)
{
	void __iomem *reg_addr_virtyal = ioremap(physical_addr, PAGE_SIZE);

	if (!reg_addr_virtyal) {
		printk(KERN_ERR "<%s> can't ioremap physical addr %u", __func__, physical_addr);
		return -1;
	}

	*reg = ioread32(reg_addr_virtyal);
	iounmap(reg_addr_virtyal);

	return 0;
}

static ssize_t physlin_write_proc(struct file* filp, const char* buf,size_t count, loff_t* offp)
{
	char register_str[64] = "";
	size_t size;
	u32 physical_addr;
	u32 reg_val;

	size = sizeof(register_str);
	if (count > size)
		size = count;

	if (copy_from_user(register_str, buf, size)) {
		printk(KERN_ERR "<%s> can't copy str from user", __func__);
		return -EFAULT;
	}

	if (kstrtou32(register_str, 0, &physical_addr)) {
		printk(KERN_ERR "<%s> can't convert str to u32", __func__);
		return -EINVAL;
	}

	if (read_reg(physical_addr, &reg_val)) {
		printk(KERN_ERR "<%s> can't read register\n", __func__);
		return -EINVAL;
	}

	reg_value = reg_val;

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
