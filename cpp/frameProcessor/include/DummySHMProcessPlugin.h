#ifndef DUMMYSHMPROCESSPLUGIN_H
#define DUMMYSHMPROCESSPLUGIN_H

#include "ClassLoader.h"
#include "FrameProcessorPlugin.h"
#include "DataBlock.h"
#include "shm-detector.h"
#include "version.h"
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>

namespace FrameProcessor {
class DummySHMProcessPlugin : public FrameProcessorPlugin {
public:
  DummySHMProcessPlugin();
  virtual ~DummySHMProcessPlugin();

  // Version information
  int get_version_major();
  int get_version_minor();
  int get_version_patch();
  std::string get_version_short();
  std::string get_version_long();

  // Configuration methods
  void configure(OdinData::IpcMessage &config, OdinData::IpcMessage &reply);
  void requestConfiguration(OdinData::IpcMessage &reply);
  std::vector<std::string> requestCommands();
  void execute(const std::string &command, OdinData::IpcMessage &reply);

  // Status and processing
  void status(OdinData::IpcMessage &status);
  bool reset_statistics();
  void process_frame(boost::shared_ptr<Frame> frame);

private:
  /** Pointer to logger */
  LoggerPtr logger_;

  // Configuration parameters
  size_t image_width_;
  size_t image_height_;
  bool copy_frame_;

  // Statistics
  size_t frames_processed_;
  size_t frames_dropped_;

  // Constants from shm-detector
  static const size_t frame_size_ = TOTAL_HALF_FRAME_SIZE;
  static const size_t header_size_ = sizeof(struct half_frame_header);

  // Configuration parameter names
  static const std::string CONFIG_IMAGE_WIDTH;
  static const std::string CONFIG_IMAGE_HEIGHT;
  static const std::string CONFIG_COPY_FRAME;

  // Commands
  static const std::string EXECUTE_PRINT;
};
} /* namespace FrameProcessor */

#endif /* DUMMYSHMPROCESSPLUGIN_H */
