/*
 * ec2dump - EC2 crash dump tool. 
 *
 * Copyright (C) 2014 Kenta Tada <ktagml@gmail.com>
 * This file is released under the GPL.
 */
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/highmem.h>
#include <linux/pfn.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/string.h>
#include <linux/notifier.h>

#define DUMP_FILE_PATH "/var/dump/ec2dump"

static size_t write_memory_to_fs(void *buf, size_t count);
static int write_memory(struct resource *res);
static int start_ec2dump_on_panic(struct notifier_block *nb, unsigned long e, void *p);
static int setup_for_dump(void);

static struct file *fp = NULL;

static size_t write_memory_to_fs(void *buf, size_t count)
{
	mm_segment_t fs;
	size_t size;

	fs = get_fs();
	set_fs(KERNEL_DS);
	    
	size = vfs_write(fp, buf, count, &fp->f_pos);

	set_fs(fs);

	return size;
}

static int write_memory(struct resource *res)
{
	resource_size_t i, is;

	struct page *pg;
	void *vaddr;
	
	int s;

	for (i = res->start; i <= res->end; i += is) {
		pg = pfn_to_page((i) >> PAGE_SHIFT);
		is = min((size_t) PAGE_SIZE, (size_t) (res->end - i + 1));

		vaddr = kmap(pg);
		s = write_memory_to_fs(vaddr, is);
		kunmap(pg);
	}

	vfs_fsync(fp, 0);
	return 0;
}

static int start_ec2dump_on_panic(struct notifier_block *nb, unsigned long e, void *p)
{
	struct resource *res;
	printk("notified by start_ec2dump_on_panic...\n");

	setup_for_dump();

	for (res = iomem_resource.child; res; res = res->sibling) {
		write_memory(res);
	}

	printk("dumpcore is created...\n");
	return NOTIFY_DONE;
}

static struct notifier_block ec2dump_panic_notifier = {
	.notifier_call = start_ec2dump_on_panic,
	.priority = 1000000,
};

static int setup_for_dump(void)
{
	mm_segment_t fs;
	int fs_err;

	printk("Currently, filesystem is used for dump...\n");
	fs = get_fs();
	set_fs(KERNEL_DS);
	
	local_irq_enable();

	fp = filp_open(DUMP_FILE_PATH, O_WRONLY | O_CREAT | O_LARGEFILE, 0444);
	
	if (!fp || IS_ERR(fp)) {
		printk("error appears when using %ld...\n", PTR_ERR(fp));
		set_fs(fs);
		fs_err = (fp) ? PTR_ERR(fp) : -EIO;
		fp = NULL;
		return fs_err;
	}

	set_fs(fs);
		
	return 0;
}

static int __init ec2dump_init(void)
{	
	printk("ec2dump is started...\n");
	atomic_notifier_chain_register(&panic_notifier_list, &ec2dump_panic_notifier);
	printk("registered panic_notifier_list...\n");

	return 0;
}

static void __exit ec2dump_exit(void)
{
	atomic_notifier_chain_unregister(&panic_notifier_list, &ec2dump_panic_notifier);
	printk("exited ec2dump...\n");
}

module_init(ec2dump_init);
module_exit(ec2dump_exit);

MODULE_LICENSE("GPL");
