/*
 * FrameDecoderGeneric.h
 *
 */

#ifndef FRAME_DECODER_GENERIC_H
#define FRAME_DECODER_GENERIC_H

#include "FrameDecoder.h"

namespace FrameReceiver {
class FrameDecoderGeneric : public FrameDecoder {
public:
  virtual ~FrameDecoderGeneric() = 0;
  virtual int get_semaphore_fd() const = 0;
  virtual void process_semaphore_signal() = 0;
  virtual void init_shm(void *shm_addr, size_t shm_size) = 0;
};

inline FrameDecoderGeneric::~FrameDecoderGeneric() {}

typedef boost::shared_ptr<FrameDecoderGeneric> FrameDecoderGenericPtr;

} // namespace FrameReceiver
#endif
