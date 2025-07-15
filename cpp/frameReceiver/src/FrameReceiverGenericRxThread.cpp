/*!
 * FrameReceiverGenericRxThread.cpp
 *
 */

#include "FrameReceiverGenericRxThread.h"
#include "shm-detector.h"
#include "shm-detector-impl.h"
#include <cstddef>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/eventfd.h>
#include <boost/bind.hpp>

using namespace FrameReceiver;

FrameReceiverGenericRxThread::FrameReceiverGenericRxThread(
    FrameReceiverConfig &config, SharedBufferManagerPtr buffer_manager,
    FrameDecoderPtr frame_decoder, unsigned int tick_period_ms)
    : FrameReceiverRxThread(config, buffer_manager, frame_decoder,
                            tick_period_ms),
      logger_(log4cxx::Logger::getLogger("FR.GenericRxThread")),
      shm_addr_(nullptr),
      shm_size_(0),
      sem_free_(SEM_FAILED),
      sem_data_(SEM_FAILED),
      sem_start_(SEM_FAILED),
      efd_(-1),
      helper_thread_running_(false)
{
  LOG4CXX_DEBUG_LEVEL(1, logger_, "FrameReceiverGenericRxThread constructor entered");
  frame_decoder_ = boost::dynamic_pointer_cast<FrameDecoderGeneric>(frame_decoder);
}

FrameReceiverGenericRxThread::~FrameReceiverGenericRxThread() {
  LOG4CXX_DEBUG_LEVEL(1, logger_, "Destroying FrameReceiverGenericRxThread");
}

void FrameReceiverGenericRxThread::run_specific_service() {
  LOG4CXX_DEBUG_LEVEL(1, logger_, "Running Generic RX thread service");

  //Setup
  struct shm_names names;
  if (set_shm_names(&names, config_.rx_address_.c_str())) {
    std::stringstream ss;
    ss << "Invalid shared memory name: " << config_.rx_address_;
    set_thread_init_error(ss.str());
    return;
  }

  int shm_fd = shm_open(names.shm_name, O_RDONLY, 0);
  if (shm_fd < 0) {
    std::stringstream ss;
    ss << "Failed to open shared memory: " << strerror(errno);
    set_thread_init_error(ss.str());
    return;
  }

  struct stat st;
  if (fstat(shm_fd, &st) < 0) {
    std::stringstream ss;
    ss << "Failed to stat shared memory: " << strerror(errno);
    set_thread_init_error(ss.str());
    close(shm_fd);
    return;
  }

  shm_addr_ = mmap(0, st.st_size, PROT_READ, MAP_SHARED | MAP_POPULATE, shm_fd, 0);
  close(shm_fd);
  if (shm_addr_ == MAP_FAILED) {
    std::stringstream ss;
    ss << "Failed to mmap shared memory: " << strerror(errno);
    set_thread_init_error(ss.str());
    return;
  }
  shm_size_ = st.st_size;

  sem_free_ = sem_open(names.sem_free_name, 0);
  sem_data_ = sem_open(names.sem_data_name, 0);
  sem_start_ = sem_open(names.sem_start_name, 0);
  if (sem_free_ == SEM_FAILED || sem_data_ == SEM_FAILED ||
      sem_start_ == SEM_FAILED) {
    std::stringstream ss;
    ss << "Failed to open semaphores: " << strerror(errno);
    set_thread_init_error(ss.str());
    return;
  }

  struct shm_header *header = static_cast<struct shm_header*>(shm_addr_);
  frame_decoder_->init_shm(shm_addr_, shm_size_);

  efd_ = eventfd(0, EFD_NONBLOCK);
  if (efd_ < 0) {
    std::stringstream ss;
    ss << "Failed to create eventfd: " << strerror(errno);
    set_thread_init_error(ss.str());
    return;
  }

  helper_thread_running_ = true;
  helper_thread_ = boost::thread(
      boost::bind(&FrameReceiverGenericRxThread::semaphore_wait_helper, this));

  sem_post(sem_start_);

  reactor_.register_socket(efd_, boost::bind(&FrameReceiverGenericRxThread::handle_eventfd_signal, this, efd_));
}

void FrameReceiverGenericRxThread::cleanup_specific_service() {
  LOG4CXX_DEBUG_LEVEL(1, logger_, "Cleaning up Generic RX thread service");

  if (helper_thread_running_) {
    helper_thread_running_ = false;
    sem_post(sem_data_); // Unblock helper thread
    helper_thread_.join();
  }

  if (efd_ >= 0) {
    reactor_.remove_socket(efd_);
    close(efd_);
  }

  if (sem_free_ != SEM_FAILED)
    sem_close(sem_free_);
  if (sem_data_ != SEM_FAILED)
    sem_close(sem_data_);
  if (sem_start_ != SEM_FAILED)
    sem_close(sem_start_);

  if (shm_addr_ != nullptr && shm_addr_ != MAP_FAILED) {
    munmap(shm_addr_, shm_size_);
  }
}

void FrameReceiverGenericRxThread::semaphore_wait_helper() {
  while (helper_thread_running_) {
    if (sem_wait(sem_data_) == 0) {
      uint64_t count = 1;
      if (write(efd_, &count, sizeof(count)) < 0 && errno != EAGAIN) {
        LOG4CXX_ERROR(logger_, "Error writing to eventfd: " << strerror(errno));
      }
    }
  }
}

void FrameReceiverGenericRxThread::handle_eventfd_signal(int fd) {
  uint64_t event_count;
  if (read(fd, &event_count, sizeof(event_count)) > 0) {
    frame_decoder_->process_semaphore_signal();

    sem_post(sem_free_);
  }
}
