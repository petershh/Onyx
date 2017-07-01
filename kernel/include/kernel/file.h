/*
* Copyright (c) 2017 Pedro Falcato
* This file is part of Onyx, and is released under the terms of the MIT License
* check LICENSE at the root directory for more information
*/
#ifndef _KERNEL_FILE_H
#define _KERNEL_FILE_H
#include <kernel/ioctx.h>

void file_do_cloexec(ioctx_t *ctx);
#endif