#include <linux/module.h>
#include <linux/proc_fs.h>
#include <asm/io.h>
#include <linux/slab.h>

#define PROC_ENTRY_PATH "physlin"
#define ADDR_AND_VAL_SEPARATOR 'w'

static struct proc_dir_entry* physlin_entry = NULL;
static u32 reg_value = 0;

static int str_containt_addr_and_val(const char* str)
{
	int i = 0;
	while(str[i] != '\0') {
		if (str[i] == ADDR_AND_VAL_SEPARATOR)
			return 1;
		i++;
	}

	return 0;
}

static int get_addr_and_val_from_str(const char* str, u32* addr, u32* val)
{
	char* addr_str = kmalloc(16, GFP_KERNEL);
	char* val_str  = kmalloc(16, GFP_KERNEL);
	int j = 0, i = 0;

	for (; str[i+1] != ADDR_AND_VAL_SEPARATOR && str[i] != '\0'; i++)
		addr_str[i] = str[i];
	addr_str[i+1] = '\0';

	i += 3;
	for (; str[i] != '\0'; i++, j++)
		val_str[j] = str[i];
	val_str[j+1] = '\0';

	if (kstrtou32(addr_str, 0, addr)) {
		printk(KERN_ERR "<%s> can't convert str to u32", __func__);
		kfree(addr_str);
		kfree(val_str);
		return -EINVAL;
	}

	if (kstrtou32(val_str, 0, val)) {
		printk(KERN_ERR "<%s> can't convert str to u32", __func__);
		kfree(addr_str);
		kfree(val_str);
		return -EINVAL;
	}

	kfree(addr_str);
	kfree(val_str);
	return 0;
}

static int write_val_in_reg_by_str(const char* str)
{
	void __iomem *reg_addr_virtyal;
	u32 physical_addr, reg_val;

	if (get_addr_and_val_from_str(str, &physical_addr, &reg_val)) {
		printk(KERN_ERR "<%s> can't parse addr and val from str", __func__);
		return -EINVAL;
	}

	reg_addr_virtyal = ioremap(physical_addr, PAGE_SIZE);

	if (!reg_addr_virtyal) {
		printk(KERN_ERR "<%s> can't ioremap physical addr %u", __func__, physical_addr);
		return -1;
	}

	iowrite32(reg_val, reg_addr_virtyal);
	reg_value = ioread32(reg_addr_virtyal);
	iounmap(reg_addr_virtyal);

	return 0;
}

static ssize_t physlin_read_proc(struct file* filp, char* buf, size_t count, loff_t* offp)
{
	char answer_str[64] = "";
	int len = sizeof(answer_str);
	ssize_t ret = len;

	snprintf(answer_str, sizeof(answer_str), "0x%X\n", reg_value);

	if (*offp >= len || copy_to_user(buf, answer_str, len)) {
		ret = 0;
		return ret;
	}

	*offp += len;

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

static int read_reg_by_str(char* str, u32* reg_val)
{
	u32 physical_addr;

	if (kstrtou32(str, 0, &physical_addr)) {
		printk(KERN_ERR "<%s> can't convert str to u32", __func__);
		return -EINVAL;
	}

	if (read_reg(physical_addr, reg_val)) {
		printk(KERN_ERR "<%s> can't read register\n", __func__);
		return -EINVAL;
	}

	return 0;
}


static ssize_t physlin_write_proc(struct file* filp, const char* buf,size_t count, loff_t* offp)
{
	char data_from_user[64] = "";
	size_t size;
	u32 reg_val;
	int ret;

	size = sizeof(data_from_user);
	if (count > size)
		size = count;

	if (copy_from_user(data_from_user, buf, size)) {
		printk(KERN_ERR "<%s> can't copy str from user", __func__);
		return -EFAULT;
	}

	data_from_user[count] = '\0';

	if (str_containt_addr_and_val(data_from_user)) {
		if (write_val_in_reg_by_str(data_from_user)) {
			printk(KERN_ERR "<%s> can't write val in reg by str", __func__);
			return -EFAULT;
		}
		return count;
	}

	ret = read_reg_by_str(data_from_user, &reg_val);
	if (ret) {
		printk(KERN_ERR "<%s> can't read reg by str", __func__);
		return ret;
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
