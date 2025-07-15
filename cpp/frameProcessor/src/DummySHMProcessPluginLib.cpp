#include "ClassLoader.h"
#include "DummySHMProcessPlugin.h"

namespace FrameProcessor {
/**
 * Registration of this plugin through the ClassLoader.  This macro
 * registers the class without needing to worry about name mangling
 */
REGISTER(FrameProcessorPlugin, DummySHMProcessPlugin, "DummySHMProcessPlugin");

} // namespace FrameProcessor
