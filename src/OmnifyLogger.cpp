#include "OmnifyLogger.h"

#if JUCE_MAC || JUCE_LINUX
#include <unistd.h>  // for confstr
#endif

OmnifyLogger::OmnifyLogger() {
    auto systemTempDir = getSystemTempDir();

    // Create a unique session directory: omnify-<uuid>/
    auto uuid = juce::Uuid().toString();
    sessionTempDir = juce::File(systemTempDir).getChildFile("omnify-" + uuid);
    sessionTempDir.createDirectory();

    // Create logger in the session directory
    auto logFile = sessionTempDir.getChildFile("omnify.log");
    logger = std::make_unique<juce::FileLogger>(logFile, "Omnify Debug Log");

    // Set as current logger so Logger::writeToLog works
    juce::Logger::setCurrentLogger(logger.get());
}

OmnifyLogger::~OmnifyLogger() {
    juce::Logger::setCurrentLogger(nullptr);
}

void OmnifyLogger::log(const juce::String& message) {
    if (logger) {
        logger->logMessage(message);
    }
}

juce::String OmnifyLogger::getSystemTempDir() {
#if JUCE_MAC
    // Use confstr to get _CS_DARWIN_USER_TEMP_DIR (/var/folders/.../T)
    // This matches Python's tempfile.gettempdir()
    size_t len = confstr(_CS_DARWIN_USER_TEMP_DIR, nullptr, 0);
    if (len > 0) {
        std::vector<char> buf(len);
        confstr(_CS_DARWIN_USER_TEMP_DIR, buf.data(), len);
        return juce::String(buf.data());
    }
#endif
    // Fallback to JUCE's temp directory
    return juce::File::getSpecialLocation(juce::File::tempDirectory).getFullPathName();
}
