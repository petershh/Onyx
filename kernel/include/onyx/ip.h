/*
* Copyright (c) 2016, 2017 Pedro Falcato
* This file is part of Onyx, and is released under the terms of the MIT License
* check LICENSE at the root directory for more information
*/
#ifndef _KERNEL_IP_H
#define _KERNEL_IP_H

#include <stdint.h>

#include <onyx/ethernet.h>

#define IPV4_ICMP 1
#define IPV4_IGMP 2
#define IPV4_TCP 6
#define IPV4_UDP 17
#define IPV4_ENCAP 41
#define IPV4_OSPF 89
#define IPV4_SCTP 132

struct sock;

typedef struct
{
	unsigned int ihl : 4;
	unsigned int version : 4;
	unsigned int dscp : 6;
	unsigned int ecn : 2;
	uint16_t total_len;
	uint16_t identification;
	uint16_t frag_off__flags;
	uint8_t ttl;
	uint8_t proto;
	uint16_t header_checksum;
	uint32_t source_ip;
	uint32_t dest_ip;
	char payload[0];
	char options[0];
} __attribute__((packed)) ip_header_t;

static inline uint16_t __ipsum_unfolded(void *addr, size_t bytes, uint16_t init_count)
{
	uint32_t sum = init_count;
	uint32_t ret = 0;
	uint16_t *ptr = (uint16_t*) addr;
	size_t words = bytes / 2;
	for(size_t i = 0; i < words; i++)
	{
		sum += ptr[i];
	}

	ret = sum & 0xFFFF;
	uint32_t carry = sum - ret;
	while(carry)
	{
		ret += carry;
		carry = ret >> 16;
		ret &= 0xFFFF;
	}
	return ~ret;
}

static inline uint16_t ipsum_unfolded(void *addr, size_t bytes)
{
	return __ipsum_unfolded(addr, bytes, 0);
}

static inline uint16_t ipsum_fold(uint16_t s)
{
	return ~s;
}

static inline uint16_t ipsum(void *addr, size_t bytes)
{
	return ipsum_fold(ipsum_unfolded(addr, bytes));
}

int ipv4_send_packet(uint32_t senderip, uint32_t destip, unsigned int type,
		     char *payload, size_t payload_size, struct netif *netif);
struct socket *ipv4_create_socket(int type, int protocol);
void ipv4_handle_packet(ip_header_t *header, size_t size, struct netif *netif);

#endif
