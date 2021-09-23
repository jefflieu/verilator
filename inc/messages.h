
#pragma once

#define PRINT_INFO(...) do { printf("INFO     ::%10s @ %10ld:", this->name, this->time); printf(__VA_ARGS__);} while(0)
#define ASSERT(cond, ... ) do { if (!cond) { printf("ASSERTION::%10s @ %10ld:", this->name, time); printf(__VA_ARGS__); assert(0);} } while(0)
#define ASSERT_CODE(cond, ... ) do { if (!cond) { printf("ASSERTION from CPP code:"); printf(__VA_ARGS__); assert(0);} } while(0)