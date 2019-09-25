
#pragma once

#include "basic.h"

u8* galloc(usize sz);
void gfree(void* mem);

void mem_validate();
