#ifndef DUMMYSHMFRAMEDECODER_H
#define DUMMYSHMFRAMEDECODER_H

#include "FrameDecoder.h"
#include "shm-detector.h"
#include "version.h"
#include <boost/thread.hpp>
#include <semaphore.h>

namespace FrameReceiver {
class DummySHMFrameDecoder : public FrameDecoder {
  // Shared memory state
  void *shm_addr_;
  size_t shm_size_;
  struct shm_header *header_;

  // Processing state
  uint64_t next_buffer_index_;
  uint64_t frames_received_;
  uint64_t frames_dropped_;

  // Configuration
  uint64_t num_buffers_;
  uint64_t frames_per_buffer_;
  uint64_t endpoint_frames_;

  // Constants from shm-detector
  static const size_t frame_size_ = TOTAL_HALF_FRAME_SIZE;
  static const size_t header_size_ = sizeof(struct half_frame_header);

public:
  DummySHMFrameDecoder();
  virtual ~DummySHMFrameDecoder();

  // Version information methods
  int get_version_major() override;
  int get_version_minor() override;
  int get_version_patch() override;
  std::string get_version_short() override;
  std::string get_version_long() override;

  // Overridden from FrameDecoder
  void init(OdinData::IpcMessage &config_msg) override;
  void request_configuration(const std::string param_prefix,
                             OdinData::IpcMessage &config_reply) override;
  const size_t get_frame_buffer_size(void) const override;
  const size_t get_frame_header_size(void) const override;
  void monitor_buffers(void) override;
  void get_status(const std::string param_prefix,
                  OdinData::IpcMessage &status_msg) override;
  void reset_statistics(void) override;

  // SHM-specific methods
  void init_shm(void *shm_addr, size_t shm_size, uint64_t num_buffers,
                uint64_t frames_per_buffer);
  void process_semaphore_signal();
};
} // namespace FrameReceiver

#endif // DUMMYSHMFRAMEDECODER_H
