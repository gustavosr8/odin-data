#include "DummySHMFrameDecoder.h"
#include "SharedBufferManager.h"
#include <cstring>
#include <log4cxx/logger.h>
#include <stdexcept>

using namespace FrameReceiver;

namespace DummyShmFrameDecoderDefaults {
const uint64_t frame_number = 0;
const uint64_t buffer_id = 0;
} // namespace DummyShmFrameDecoderDefaults

DummySHMFrameDecoder::DummySHMFrameDecoder()
    : FrameDecoder(), shm_addr_(nullptr), shm_size_(0), header_(nullptr),
      next_buffer_index_(DummyShmFrameDecoderDefaults::frame_number),
      frames_received_(0), frames_dropped_(0), num_buffers_(0),
      frames_per_buffer_(0), endpoint_frames_(0) {
  this->logger_ = Logger::getLogger("FR.DummySHMFrameDecoder");
  LOG4CXX_INFO(logger_, "DummySHMFrameDecoder version "
                            << this->get_version_long() << " loaded");
}

DummySHMFrameDecoder::~DummySHMFrameDecoder() {}

void DummySHMFrameDecoder::init(OdinData::IpcMessage &config_msg) {
  FrameDecoder::init(config_msg);
}

void DummySHMFrameDecoder::reset_statistics(void) {
  FrameDecoder::reset_statistics();
  frames_received_ = 0;
  next_buffer_index_ = 0;
}

void DummySHMFrameDecoder::request_configuration(
    const std::string param_prefix, OdinData::IpcMessage &config_reply) {
  FrameDecoder::request_configuration(param_prefix, config_reply);

  config_reply.set_param(param_prefix + "num_buffers", num_buffers_);
  config_reply.set_param(param_prefix + "frames_per_buffer",
                         frames_per_buffer_);
  config_reply.set_param(param_prefix + "endpoint_frames", endpoint_frames_);
  config_reply.set_param(param_prefix + "frames_received", frames_received_);
}

void DummySHMFrameDecoder::get_status(const std::string param_prefix,
                                      OdinData::IpcMessage &status_msg) {

  status_msg.set_param(param_prefix + "num_buffers", num_buffers_);
  status_msg.set_param(param_prefix + "frames_per_buffer", frames_per_buffer_);
  status_msg.set_param(param_prefix + "endpoint_frames", endpoint_frames_);
  status_msg.set_param(param_prefix + "frames_received", frames_received_);
  status_msg.set_param(param_prefix + "next_buffer_index", next_buffer_index_);
}

const size_t DummySHMFrameDecoder::get_frame_buffer_size(void) const {
  return frame_size_;
}

const size_t DummySHMFrameDecoder::get_frame_header_size(void) const {
  return sizeof(struct half_frame_header);
}

void DummySHMFrameDecoder::monitor_buffers(void) {
  // Check for buffer timeouts - not implemented for SHM
}

void DummySHMFrameDecoder::process_semaphore_signal() {
  uint64_t buffer_index = next_buffer_index_ % num_buffers_;
  next_buffer_index_++;

  uint8_t *buffer_ptr =
      reinterpret_cast<uint8_t *>(shm_addr_) + SHM_HEADER_SIZE;
  buffer_ptr += buffer_index * frames_per_buffer_ * frame_size_;

  for (uint64_t frame_idx = 0; frame_idx < frames_per_buffer_; frame_idx++) {
    if (empty_buffer_queue_.empty()) {
      LOG4CXX_WARN(logger_, "No empty buffers available, dropping frame");
      frames_dropped_++;
      continue;
    }

    int buffer_id = empty_buffer_queue_.front();
    empty_buffer_queue_.pop();

    void *frame_buffer = buffer_manager_->get_buffer_address(buffer_id);
    uint8_t *frame_ptr = buffer_ptr + (frame_idx * frame_size_);

    std::memcpy(frame_buffer, frame_ptr, frame_size_);

    struct half_frame_header *header =
        reinterpret_cast<struct half_frame_header *>(frame_buffer);

    frame_buffer_map_[header->sequence_number] = buffer_id;

    ready_callback_(buffer_id, header->sequence_number);
    frames_received_++;
  }
}

int FrameReceiver::DummySHMFrameDecoder::get_version_major() {
  return ODIN_DATA_VERSION_MAJOR;
}

int FrameReceiver::DummySHMFrameDecoder::get_version_minor() {
  return ODIN_DATA_VERSION_MINOR;
}

int FrameReceiver::DummySHMFrameDecoder::get_version_patch() {
  return ODIN_DATA_VERSION_PATCH;
}

std::string FrameReceiver::DummySHMFrameDecoder::get_version_short() {
  return ODIN_DATA_VERSION_STR_SHORT;
}

std::string FrameReceiver::DummySHMFrameDecoder::get_version_long() {
  return ODIN_DATA_VERSION_STR;
}
