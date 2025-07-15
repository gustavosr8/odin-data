/*!
 * FrameReceiverGenericRxThread.h
 *
 */

#ifndef FRAME_RECEIVER_Generic_RX_THREAD_H
#define FRAME_RECEIVER_Generic_RX_THREAD_H

#include "FrameDecoderGeneric.h"
#include "FrameReceiverRxThread.h"
#include "shm-detector.h"
#include "semaphore.h"
#include <boost/thread.hpp>

using namespace OdinData;

namespace FrameReceiver {
class FrameReceiverGenericRxThread : public FrameReceiverRxThread {
  void run_specific_service() override;
  void cleanup_specific_service() override;

  void semaphore_wait_helper();
  void handle_eventfd_signal(int fd);

  LoggerPtr logger_;
  FrameDecoderGenericPtr frame_decoder_;

  void* shm_addr_;
  size_t shm_size_;
  sem_t* sem_free_;
  sem_t* sem_data_;
  sem_t* sem_start_;
  int efd_;

  bool helper_thread_running_;
  boost::thread helper_thread_;

public:
  FrameReceiverGenericRxThread(FrameReceiverConfig &config,
                           SharedBufferManagerPtr buffer_manager,
                           FrameDecoderPtr frame_decoder,
                           unsigned int tick_period_ms = 100);
  virtual ~FrameReceiverGenericRxThread();
};

} // namespace FrameReceiver
#endif
