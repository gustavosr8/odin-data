#include "DummySHMProcessPlugin.h"
#include "DataBlockFrame.h"

namespace FrameProcessor {
// Configuration parameter name definitions
const std::string DummySHMProcessPlugin::CONFIG_IMAGE_WIDTH = "width";
const std::string DummySHMProcessPlugin::CONFIG_IMAGE_HEIGHT = "height";
const std::string DummySHMProcessPlugin::CONFIG_COPY_FRAME = "copy_frame";
// Command and parameters
const std::string DummySHMProcessPlugin::EXECUTE_PRINT = "print";

/**
 * The constructor sets up default configuration parameters and logging used
 * within the class.
 */
DummySHMProcessPlugin::DummySHMProcessPlugin()
    : image_width_(448), // Default from PIXELS calculation: 448 * 256 = 114688
      image_height_(256), copy_frame_(true), frames_processed_(0) {
  // Setup logging for the class
  logger_ = Logger::getLogger("FP.DummySHMProcessPlugin");
  LOG4CXX_INFO(logger_, "DummySHMProcessPlugin version "
                            << this->get_version_long() << " loaded");
}

/**
 * Destructor.
 */
DummySHMProcessPlugin::~DummySHMProcessPlugin() {
  LOG4CXX_TRACE(logger_, "DummySHMProcessPlugin destructor.");
}

int DummySHMProcessPlugin::get_version_major() {
  return ODIN_DATA_VERSION_MAJOR;
}

int DummySHMProcessPlugin::get_version_minor() {
  return ODIN_DATA_VERSION_MINOR;
}

int DummySHMProcessPlugin::get_version_patch() {
  return ODIN_DATA_VERSION_PATCH;
}

std::string DummySHMProcessPlugin::get_version_short() {
  return ODIN_DATA_VERSION_STR_SHORT;
}

std::string DummySHMProcessPlugin::get_version_long() {
  return ODIN_DATA_VERSION_STR;
}

/**
 * Configure the plugin.  This receives an IpcMessage which should be processed
 * to configure the plugin, and any response can be added to the reply
 * IpcMessage.
 *
 * \param[in] config - Reference to the configuration IpcMessage object.
 * \param[out] reply - Reference to the reply IpcMessage object.
 */
void DummySHMProcessPlugin::configure(OdinData::IpcMessage &config,
                                      OdinData::IpcMessage &reply) {
  // Check for and set image width parameter
  if (config.has_param(CONFIG_IMAGE_WIDTH)) {
    image_width_ = config.get_param<size_t>(CONFIG_IMAGE_WIDTH);
    LOG4CXX_DEBUG(logger_, "Setting image width to " << image_width_);
  }

  // Check for and set image height parameter
  if (config.has_param(CONFIG_IMAGE_HEIGHT)) {
    image_height_ = config.get_param<size_t>(CONFIG_IMAGE_HEIGHT);
    LOG4CXX_DEBUG(logger_, "Setting image height to " << image_height_);
  }

  // Check for and set copy frame mode
  if (config.has_param(CONFIG_COPY_FRAME)) {
    copy_frame_ = config.get_param<bool>(CONFIG_COPY_FRAME);
    LOG4CXX_DEBUG(logger_, "Setting copy frame mode to "
                               << (copy_frame_ ? "true" : "false"));
  }
}

/**
 * Respond to configuration requests from clients.
 *
 * This method responds to configuration requests from client, populating the
 * supplied IpcMessage reply with configured parameters.
 *
 * \param[in,out] reply - IpcMessage response to client, to be populated with
 * plugin parameters
 */
void DummySHMProcessPlugin::requestConfiguration(OdinData::IpcMessage &reply) {
  std::string base_str = get_name() + "/";
  reply.set_param(base_str + CONFIG_IMAGE_WIDTH, image_width_);
  reply.set_param(base_str + CONFIG_IMAGE_HEIGHT, image_height_);
  reply.set_param(base_str + CONFIG_COPY_FRAME, copy_frame_);
}

/**
 * Execute a command on the plugin.  This receives an IpcMessage which should be
 * processed to execute a command within the plugin, and any response can be
 * added to the reply IpcMessage. The dummy plugin implements a single command
 * "print" that prints the value of the parameter named.
 *
 * \param[in] config - String containing the command to execute.
 * \param[out] reply - Reference to the reply IpcMessage object.
 */
void DummySHMProcessPlugin::execute(const std::string &command,
                                    OdinData::IpcMessage &reply) {
  if (command == EXECUTE_PRINT) {
    LOG4CXX_INFO(logger_, "Image width is " << image_width_);
    LOG4CXX_INFO(logger_, "Image height is " << image_height_);
    LOG4CXX_INFO(logger_, "Copy frame is " << (copy_frame_ ? "true" : "false"));
    LOG4CXX_INFO(logger_, "Frames processed: " << frames_processed_);
  } else {
    std::stringstream ss;
    ss << "Unsupported command: " << command;
    LOG4CXX_ERROR(logger_, ss.str());
    throw std::runtime_error(ss.str());
  }
}

/**
 * Respond to command execution requests from clients.
 *
 * This method responds to command executions requests from client, populating
 * the supplied IpcMessage reply with the commands and command parameters
 * supported by this plugin.
 *
 * \return - Vector containing supported command strings.
 */
std::vector<std::string> DummySHMProcessPlugin::requestCommands() {
  std::vector<std::string> commands;
  commands.push_back(EXECUTE_PRINT);
  return commands;
}

/**
 * Collate status information for the plugin.  The status is added to the status
 * IpcMessage object.
 *
 * \param[out] status - Reference to an IpcMessage value to store the status.
 */
void DummySHMProcessPlugin::status(OdinData::IpcMessage &status) {
  std::string base_str = get_name() + "/";
  status.set_param(base_str + "frames_processed", frames_processed_);
}

bool DummySHMProcessPlugin::reset_statistics() {
  frames_processed_ = 0;
  return true;
}

void DummySHMProcessPlugin::process_frame(boost::shared_ptr<Frame> frame) {
  LOG4CXX_TRACE(logger_, "Processing SHM frame");
  frames_processed_++;

  // Get pointer to frame header
  struct half_frame_header *hdr_ptr =
      static_cast<struct half_frame_header *>(frame->get_data_ptr());

  LOG4CXX_DEBUG(logger_, "Frame sequence: " << hdr_ptr->sequence_number);
  LOG4CXX_DEBUG(logger_, "Frame identity: " << hdr_ptr->identity);

  // Create frame metadata
  FrameMetaData frame_meta;
  frame_meta.set_dataset_name("dummy");
  frame_meta.set_data_type(raw_16bit);
  frame_meta.set_frame_number(static_cast<long long>(hdr_ptr->sequence_number));

  // Set dimensions (height x width)
  dimensions_t dims(2);
  dims[0] = image_height_;
  dims[1] = image_width_;
  frame_meta.set_dimensions(dims);
  frame_meta.set_compression_type(no_compression);

  // Calculate data size (pixels * sizeof(uint16_t))
  const size_t data_size = image_width_ * image_height_ * sizeof(uint16_t);

  if (copy_frame_) {
    LOG4CXX_DEBUG(logger_,
                  "Copying data for frame " << hdr_ptr->sequence_number);

    // Create new frame with just the image data
    boost::shared_ptr<Frame> output_frame(
        new DataBlockFrame(frame_meta, data_size));

    // Copy data from the frame (skip header)
    void *output_ptr = output_frame->get_data_ptr();
    const void *data_ptr =
        static_cast<const char *>(frame->get_data_ptr()) + header_size_;
    std::memcpy(output_ptr, data_ptr, data_size);

    // Push the new frame
    this->push(output_frame);
  } else {
    LOG4CXX_DEBUG(logger_, "Using existing frame " << hdr_ptr->sequence_number);

    // Set metadata on existing frame
    frame->set_meta_data(frame_meta);

    // Set image offset to skip the header
    frame->set_image_offset(header_size_);

    // Set image size to just the pixel data
    frame->set_image_size(data_size);

    // Push the existing frame
    this->push(frame);
  }
}
} /* namespace FrameProcessor */
