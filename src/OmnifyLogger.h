#pragma once

#include <juce_core/juce_core.h>

/**
 * Shared logging and temp directory for Omnify plugin.
 * Creates a unique session directory under the system temp dir.
 *
 * Use via juce::SharedResourcePointer<OmnifyLogger> to ensure proper cleanup.
 */
class OmnifyLogger {
   public:
    OmnifyLogger();

    juce::File getTempDir() const { return sessionTempDir; }
    void log(const juce::String& message);

   private:
    juce::File sessionTempDir;
    std::unique_ptr<juce::FileLogger> logger;

    static juce::String getSystemTempDir();
};
