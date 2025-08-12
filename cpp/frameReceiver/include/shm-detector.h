#ifndef SHM_DETECTOR_H
#define SHM_DETECTOR_H

#include <assert.h>
#include <semaphore.h>
#include <stdint.h>

#define NS_IN_S 1000000000

#define PIXELS (448ULL * 512ULL / 2)
#define BYTE_DEPTH 2ULL
#define TOTAL_BYTES (PIXELS * BYTE_DEPTH)
#define EXPECTED_PAGE_SIZE 4096
#define SHM_HEADER_SIZE EXPECTED_PAGE_SIZE
#define TOTAL_HALF_FRAME_SIZE (TOTAL_BYTES + EXPECTED_PAGE_SIZE)

struct half_frame_header {
	uint64_t sequence_number;
	uint32_t identity;
	uint8_t padding[EXPECTED_PAGE_SIZE - 12];
};
static_assert(sizeof(struct half_frame_header) == EXPECTED_PAGE_SIZE);

struct half_frame {
	struct half_frame_header header;
	uint16_t data[PIXELS];
};
static_assert(sizeof(struct half_frame) == TOTAL_HALF_FRAME_SIZE);

struct shm_header {
	uint64_t num_frames;
	uint64_t frames_per_buffer;
	uint64_t num_buffers;
	uint64_t endpoint_frames;
};

struct sem_handles {
	sem_t *sem_free;
	sem_t *sem_data;
	sem_t *sem_start;
};

#endif
