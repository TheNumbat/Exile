#pragma once

#include "basic.h"
#include <string.h>

// order matters!

#include "alloc_before.inl"
#include "string_before.inl"
#include "reflect.inl"
#include "printf_before.inl"
#include "log.inl"
#include "alloc_after.inl"
#include "string_after.inl"
#include "printf_after.inl"