
#pragma once

#define PRINT_INFO(...) do { printf("INFO     ::%30s @ %10ld:", this->name, this->time); printf(__VA_ARGS__);} while(0)
#define ASSERT(cond, ... ) do { if (!cond) { printf("ASSERTION::%30s @ %10ld:", this->name, time); printf(__VA_ARGS__); assert(0);} } while(0)
#define ASSERT_CODE(cond, ... ) do { if (!cond) { printf("ASSERTION from CPP code:"); printf(__VA_ARGS__); assert(0);} } while(0)

#define PRINT_DBG(dbg, level , ...) do { if (dbg < leve) continue; printf("INFO     ::%30s @ %10ld:", this->name, this->time); printf(__VA_ARGS__);} while(0)