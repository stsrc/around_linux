#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/timekeeping.h>
#include <asm/barrier.h>
#include <linux/cdev.h>
#include <asm/page.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <asm/io.h>
#include <asm/pgtable.h>
#include <linux/slab.h>
#include <linux/delay.h>

static void *test_page;
static struct cdev *cdev;
static dev_t devt;
static struct device *device;
static struct class *class;
static struct page *test_page_page;

#define debug_print() printk("%s():%d\n", __FUNCTION__, __LINE__)

static int test_open(struct inode *inodep, struct file *filep)
{
	printk("%s():%d opened\n", __FUNCTION__, __LINE__);
	return 0;
}

static int test_release(struct inode *inodep, struct file *filep)
{
	printk("%s():%d closed\n", __FUNCTION__, __LINE__);
	return 0;
}

static int test_vma_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	struct page *page;

	page = virt_to_page(test_page);
	get_page(page);
	vmf->page = page;

	return 0;
}

struct vm_operations_struct test_vma_ops = {
	.fault = test_vma_fault
};


static int test_mmap(struct file *filep, struct vm_area_struct *vma)
{
	debug_print();
	vma->vm_ops = &test_vma_ops;
	return 0;
}

static struct file_operations fops =
{
	.open = test_open,
	.mmap = test_mmap,
	.release = test_release,
};

int __init test_start(void)
{
	struct timespec start, end;
	int i, j;
	long int result_in_ns;
	const long int max_count = 250;
	const int repeat_count = 20;
	int rt;
	const char *driver_name = "fast_test";

	rt = alloc_chrdev_region(&devt, 0, 1, driver_name);
	if (rt)
		return -EINVAL;
	class = class_create(THIS_MODULE, driver_name);
	if (IS_ERR(class)) {
		rt = PTR_ERR(class);
		return rt;
	}
	cdev = cdev_alloc();
	if (!cdev) {
		return -ENOMEM;
	}
	cdev_init(cdev, &fops);
	cdev_add(cdev, devt, 1);

	device = device_create(class, NULL, devt, NULL, driver_name);

	test_page_page = alloc_page(GFP_KERNEL);
	if (!test_page_page) {
		printk("Failed to allocate mem 0!\n");
		return -EAGAIN;
	}

	test_page = page_to_virt(test_page_page);
	if (!test_page) {
		printk("Failed to allocate mem!\n");
		return -EAGAIN;
	}

	msleep(5000);

	for (j = 0; j < repeat_count; j++) {
		memset(&start, 0, sizeof(struct timespec));
		memset(&end, 0, sizeof(struct timespec));

		getnstimeofday(&start);
		for (i = 0; i < max_count; i++) {
			*(int *)(test_page) = i;
			mb();
		}
		getnstimeofday(&end);

		result_in_ns = (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);

//		printk("%s():%d. WITH mb(): result = %ld [ns]\n", __FUNCTION__, __LINE__, result_in_ns);
		printk("%s():%d. WITH mb(): result divided by %ld = %ld [ns]\n", __FUNCTION__, __LINE__, max_count, result_in_ns / max_count);

		/*memset(&start, 0, sizeof(struct timespec));
		memset(&end, 0, sizeof(struct timespec));

		getnstimeofday(&start);
		for (i = 0; i < max_count; i++) {
			*(int *)(test_page) = i;
		}
		getnstimeofday(&end);

		result_in_ns = (end.tv_sec - start.tv_sec) * 1000000000 + (end.tv_nsec - start.tv_nsec);

		printk("%s():%d. WITHOUT mb(): result = %ld [ns]\n", __FUNCTION__, __LINE__, result_in_ns);
		printk("%s():%d. WITHOUT mb(): result divided by %ld = %ld [ns]\n", __FUNCTION__, __LINE__, max_count, result_in_ns / max_count);*/

		msleep(2);
	}

	return 0;
}

void __exit test_end(void)
{
	device_destroy(class, devt);
	cdev_del(cdev);
	class_destroy(class);
	unregister_chrdev_region(devt, 1);
}

module_init(test_start);
module_exit(test_end);

MODULE_LICENSE("GPL");
