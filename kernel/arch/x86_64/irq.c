/*----------------------------------------------------------------------
 * Copyright (C) 2016, 2017 Pedro Falcato
 *
 * This file is part of Onyx, and is made available under
 * the terms of the GNU General Public License version 2.
 *
 * You can redistribute it and/or modify it under the terms of the GNU
 * General Public License version 2 as published by the Free Software
 * Foundation.
 *----------------------------------------------------------------------*/
/**************************************************************************
 *
 *
 * File: irq.c
 *
 * Description: Contains irq installation functions
 *
 * Date: 1/2/2016
 *
 *
 **************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <kernel/task_switching.h>
#include <kernel/panic.h>
#include <kernel/registers.h>
#include <kernel/irq.h>

volatile _Bool is_in_irq = false;
irq_list_t *irq_routines[24]  =
{
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};
_Bool isirq()
{
	return is_in_irq;
}
void irq_install_handler(int irq, irq_t handler)
{
	irq_list_t *lst = irq_routines[irq];
	if(!lst)
	{
		lst = (irq_list_t*) malloc(sizeof(irq_list_t));
		if(!lst)
		{
			errno = ENOMEM;
			return; /* TODO: Return a value indicating an error */
		}
		memset(lst, 0, sizeof(irq_list_t));
		lst->handler = handler;
		irq_routines[irq] = lst;
		return;
	}
	while(lst->next != NULL)
		lst = lst->next;
	lst->next = (irq_list_t*) malloc(sizeof(irq_list_t));
	if(!lst->next)
	{
		errno = ENOMEM;
		return; /* See the above TODO */
	}
	lst->next->handler = handler;
	lst->next->next = NULL;
}
void irq_uninstall_handler(int irq, irq_t handler)
{
	irq_list_t *list = irq_routines[irq];
	if(list->handler == handler)
	{
		free(list);
		irq_routines[irq] = NULL;
		return;
	}
	irq_list_t *prev = NULL;
	while(list->handler != handler)
	{
		prev = list;
		list = list->next;
	}
	free(list);
	prev->next = list->next;
}
uintptr_t irq_handler(uint64_t irqn, registers_t *regs)
{
	if(irqn > 23)
	{
		return (uintptr_t) regs;
	}
	uintptr_t ret = (uintptr_t) regs;
	irq_list_t *handlers = irq_routines[irqn];
	if(!handlers)
		printf("Unhandled interrupt at IRQ %u\n", irqn);
	is_in_irq = true;
	for(irq_list_t *i = handlers; i != NULL;i = i->next)
	{
		irq_t handler = i->handler;
		uintptr_t p = handler(regs);
		if(p != 0)
		{
			ret = p;
		}
	}
	is_in_irq = false;
	return ret;
}
static struct irq_work *queue = NULL;
int irq_schedule_work(void (*callback)(void *, size_t), size_t payload_size, void *payload)
{
	struct irq_work *q = queue;
	while(q->callback)
	{
		q = (struct irq_work *) (char*) &q->payload + q->payload_size;
	}
	uintptr_t remaining_size = ((uintptr_t) queue + IRQ_WORK_QUEUE_SIZE) - (uintptr_t) q;
	if(sizeof(struct irq_work) + payload_size > remaining_size)
		return 1;
	q->callback = callback;
	q->payload_size = payload_size;
	memcpy(&q->payload, payload, payload_size);
	return 0;
}
int irq_get_work(struct irq_work *strct)
{
	if(!queue->callback)
		return -1;
	memcpy(strct, queue, sizeof(struct irq_work) + queue->payload_size);
	struct irq_work *next_work = (struct irq_work*) (char*) queue + sizeof(struct irq_work) + strct->payload_size;
	memmove(queue, next_work, IRQ_WORK_QUEUE_SIZE - sizeof(struct irq_work) - strct->payload_size);

	return 0;
}
struct irq_work *worker_buffer = NULL;
void irq_worker(void *ctx)
{
	while(1)
	{
		/* Do any work needed */
		if(irq_get_work(worker_buffer) < 0)
		{
			sched_yield();
			continue;
		}
		worker_buffer->callback(&worker_buffer->payload, worker_buffer->payload_size);
	}
}
void irq_init(void)
{
	if(!sched_create_thread(irq_worker, 1, NULL))
		panic("irq_init: Could not create the worker thread!\n");
	queue = malloc(IRQ_WORK_QUEUE_SIZE);
	if(!queue)
		panic("irq_init: failed to allocate queue!\n");
	memset(queue, 0, IRQ_WORK_QUEUE_SIZE);
	worker_buffer = malloc(IRQ_WORK_QUEUE_SIZE);
	if(!worker_buffer)
		panic("irq_init: failed to allocate buffer!\n");
	memset(worker_buffer, 0, IRQ_WORK_QUEUE_SIZE);
}