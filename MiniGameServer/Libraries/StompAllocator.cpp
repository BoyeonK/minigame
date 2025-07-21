#include "pch.h"
#include "StompAllocator.h"

void* StompAllocator::Alloc(uint32_t size) {
	const uint64_t pageCount = (size + PAGE_SIZE - 1) / PAGE_SIZE;
	const uint64_t offset = pageCount * PAGE_SIZE - size;
	void* pBase = ::VirtualAlloc(NULL, pageCount * PAGE_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	return static_cast<void*>(static_cast<uint8_t*>(pBase) + offset);
}

void StompAllocator::Release(void* ptr) {
	const uint64_t address = reinterpret_cast<uint64_t>(ptr);
	const uint64_t pBase = address - (address % PAGE_SIZE);
	::VirtualFree(reinterpret_cast<void*>(pBase), 0, MEM_RELEASE);
}