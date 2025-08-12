#ifndef SHM_DETECTOR_RECV_H
#define SHM_DETECTOR_RECV_H

#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>

#include "shm-detector.h"

struct dr_handle {
	const struct shm_header *header;
	const struct half_frame *half_frames;
	struct sem_handles sems;
	uint64_t buffers_to_receive;

	struct {
		void *shm_addr;
		size_t shm_size;
	} internal;
};

/* if callback returns true, we sem_post(sem_free) in dr_run ourselves; otherwise it's left to the user of the function */
typedef int dr_run_cb(struct dr_handle *handle, unsigned buffer_index, unsigned iteration_index, void *user_data);

int dr_handle_init(struct dr_handle *, const char *);
void dr_handle_free(struct dr_handle *);

static inline void dr_run(struct dr_handle *handle, dr_run_cb callback, void *user_data, int verbose)
{
	sem_post(handle->sems.sem_start);

	for (unsigned i = 0; i < handle->buffers_to_receive; i++) {
		if (verbose)
			printf("receiving buffer %u/%llu\n", i, (long long unsigned)handle->buffers_to_receive);

		sem_wait(handle->sems.sem_data);

		unsigned buffer_index = i % handle->header->num_buffers;
		if (callback(handle, buffer_index, i, user_data))
			sem_post(handle->sems.sem_free);
	}
}

#endif
