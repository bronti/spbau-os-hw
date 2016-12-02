#include <serial.h>
#include <memory.h>
#include <balloc.h>
#include <paging.h>
#include <debug.h>
#include <alloc.h>
#include <print.h>
#include <ints.h>
#include <time.h>
#include "spinlock.h"
#include "thread.h"

static void qemu_gdb_hang(void)
{
#ifdef DEBUG
	static volatile int wait = 1;

	while (wait);
#endif
}

// static void test_kmap(void)
// {
// 	const size_t count = 1024;
// 	struct page **pages = mem_alloc(sizeof(*pages) * count);
// 	size_t i;

// 	BUG_ON(!pages);
// 	for (i = 0; i != count; ++i) {
// 		pages[i] = __page_alloc(0);
// 		if (!pages[i])
// 			break;
// 	}

// 	char *ptr = kmap(pages, i);

// 	BUG_ON(!ptr);
// 	BUG_ON((uintptr_t)ptr < HIGHER_BASE);

// 	for (size_t j = 0; j != i * PAGE_SIZE; ++j)
// 		ptr[i] = 13;

// 	for (size_t j = 0; j != i * PAGE_SIZE; ++j)
// 		BUG_ON(ptr[i] != 13);

// 	kunmap(ptr);
// 	mem_free(pages);
// }

static void test_alloc(void)
{
	struct list_head head;
	unsigned long count = 0;

	list_init(&head);
	while (1) {
		struct list_head *node = mem_alloc(sizeof(*node));

		if (!node)
			break;
		BUG_ON((uintptr_t)node < HIGHER_BASE);
		++count;
		list_add(node, &head);
	}

	printf("Allocated %lu bytes\n", count * sizeof(head));

	while (!list_empty(&head)) {
		struct list_head *node = head.next;

		BUG_ON((uintptr_t)node < HIGHER_BASE);
		list_del(node);
		mem_free(node);
	}

	mem_alloc_shrink();
}

static void test_slab(void)
{
	struct list_head head;
	struct mem_cache cache;
	unsigned long count = 0;

	list_init(&head);
	mem_cache_setup(&cache, sizeof(head), sizeof(head));
	while (1) {
		struct list_head *node = mem_cache_alloc(&cache);

		if (!node)
			break;
		BUG_ON((uintptr_t)node < HIGHER_BASE);
		++count;
		list_add(node, &head);
	}

	printf("Allocated %lu bytes\n", count * sizeof(head));

	while (!list_empty(&head)) {
		struct list_head *node = head.next;

		BUG_ON((uintptr_t)node < HIGHER_BASE);
		list_del(node);
		mem_cache_free(&cache, node);
	}

	mem_cache_release(&cache);
}

static void test_buddy(void)
{
	struct list_head head;
	unsigned long count = 0;

	list_init(&head);
	while (1) {
		struct page *page = __page_alloc(0);

		if (!page)
			break;
		++count;
		list_add(&page->ll, &head);
	}

	printf("Allocated %lu pages\n", count);

	while (!list_empty(&head)) {
		struct list_head *node = head.next;
		struct page *page = CONTAINER_OF(node, struct page, ll);

		list_del(&page->ll);
		__page_free(page, 0);
	}
}

static void test_buddy_(void * par)
{
	(void)par;
	test_buddy();
}

static void test_slab_(void * par)
{
	(void)par;
	test_slab();
}

static void test_alloc_(void * par)
{
	(void)par;
	test_alloc();
}

// static void test_kmap_(void * par)
// {
// 	(void)par;
// 	test_kmap();
// }

void * fic;

volatile struct spinlock lck = { UNLOCKED };


void add(void * x_)
{
	for (int i = 0; i < 100; ++i)
	{
		int * x = (int *) x_;
		printf("inc x\n");
		lock(&lck);
		*x += 1;
		unlock(&lck);
		for(uint64_t i = 0; i < (1ul << 19); ++i);
	}	
}

void print(void * x_)
{
	// while (*((int *)x_) == 0);
	for (int i = 0; i < 100; ++i)
	{
		int * x = (int *) x_;
		lock(&lck);
		printf("x: %d\n", *x);
		unlock(&lck);
		for(uint64_t i = 0; i < (1ul << 19); ++i);
	}	
}

void main(void *bootstrap_info)
{
	qemu_gdb_hang();

	serial_setup();
	ints_setup();
	time_setup();
	balloc_setup(bootstrap_info);
	paging_setup();
	page_alloc_setup();
	mem_alloc_setup();
	kmap_setup();
	thread_setup();
	enable_ints();

	(void)test_buddy_;
	(void)test_alloc_;
	(void)test_slab_;

	// printf("Tests Begin\n");
	// thread_t * bud_th = thread_create(&test_buddy_, fic);
	// thread_t * slab_th = thread_create(&test_slab_, fic);
	// thread_t * alloc_th = thread_create(&test_alloc_, fic);
	// thread_t * kmap_th = thread_create(&test_kmap_, fic);
	// thread_wait(bud_th);
	// thread_wait(slab_th);
	// thread_wait(alloc_th);
	// thread_wait(kmap_th);
	// test_alloc();
	// printf("Tests Finished\n");

	// struct spinlock locker;
	// lock(&locker);
	// printf("Locked\n");
	// // lock(&locker);
	// // printf("Locked\n");
	// unlock(&locker);
	// printf("Unlocked\n");


	printf("Thread test begin.\n");


	int * x = mem_alloc(sizeof(int));
	*x = 0;
	thread_t * printer = thread_create(&print, x);
	thread_t * adder = thread_create(&add, x);
	(void) adder;
	(void) printer;

	// for(uint64_t i = 0; i < (1ul << 26); ++i);
	// printf("Killing adder.\n");
	// thread_kill(adder);
	// for(uint64_t i = 0; i < (1ul << 21); ++i);
	// printf("Killing printer.\n");
	// thread_kill(printer);
	thread_wait(adder);
	thread_wait(printer);

	mem_free(x);
	printf("Thread test end.\n");

	while(1);
}
