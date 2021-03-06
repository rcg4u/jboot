#include "heap.h"

void* _malloc(uint32_t size) {
	unsigned int zone;
	unsigned int min_size;
	chunk_header_t** header;
	chunk_header_t* candidate;
	chunk_header_t* next_chunk;

	min_size = get_min_alloc(size);
	zone = get_zone(min_size);
	header = &gHeapHeader[zone];

NEXT:
	if(zone <= 0x1F) {
		for(candidate = *header; ; candidate = candidate->next) {
			if(!candidate) {
				zone++;
				header++;
				goto NEXT;
			}
			if(min_size <= (8 * candidate->size)) {
				break;
			}
		}
		*candidate->head = candidate->next;
		next_chunk = candidate->next;
		if(next_chunk) {
			next_chunk->head = candidate->head;
		}
		link_chunk(candidate, min_size);
		gHeapRemaining -= candidate->size * 8;
	} else {
		return 0;
	}
	return (void*) candidate + 8;
}

void _link_chunk(chunk_header_t* chunk, unsigned int size) {
	chunk_header_t* next;
	chunk_header_t* last;
	chunk_header_t** header;

	if(chunk->size * 8 < size + 16) {
		next = get_next_chunk(chunk);
		chunk->prev_size &= 0xFDu;

	} else {
		next = (void*) chunk + size;
		last = get_next_chunk(chunk);
		next->prev_size = (next->prev_size & 0xFD) | 1;
		next->prev_size = (next->prev_size & 3) | (4 * (size / 8));
		next->size = chunk->size - ((size / 8) & 0x3FFFFFFF);
		chunk->size = size / 8;
		last->prev_size = (last->prev_size & 3) | (4 * next->size);
		header = &gHeapHeader[_get_zone(next->size * 8)];
		next->head = header;
		next->next = *header;
		if(*header) {
			(*header)->head = &next->next;
		}
		*header = next;
	}
	chunk->prev_size &= 0xFEu;
}

