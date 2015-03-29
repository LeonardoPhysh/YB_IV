/*
 * list head file 
 * 		defined some common data structure. 
 *
 * Author: Leoanrdo Physh <leonardo.physh@yahoo.com.hk>
 * Data : 2014.06.28
 */
#define __LIST_HEAD__
#define __LIST_HEAD__

struct list_node;
struct list_head;

/*
 * initialize lise head with itself's address
 */
#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define DECLAR_LIST_HEAD(name) \
		struct list_head name = LIST_HEAD_INIT(name)

/*
 * offset & contaner_of
 * refer to Linux Kernel
 */
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({ 		\
		const typeof(((type *)0)->member) * __mptr = (ptr); \
		(type *)((char *)__mptr - offsetof(type, member)); })

struct list_head {
	struct list_node * head;
	struct list_node * tail;
};

struct list_node {
	struct list_head * head;
	struct list_node * next;
	struct list_node * prev;
};

#endif /* __LIST_HEAD__ */
