#include "DaemonManager.h"

#include <juce_core/juce_core.h>

// Socket headers still needed for findFreePort()
#if JUCE_MAC || JUCE_LINUX
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#elif JUCE_WINDOWS
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#endif

DaemonManager::DaemonManager() : juce::Thread("DaemonOutputReader"), process(std::make_unique<juce::ChildProcess>()) {}

void DaemonManager::setListener(Listener* l) { listener = l; }

DaemonManager::~DaemonManager() {
    stop();
    stopThread(2000);
}

int DaemonManager::findFreePort() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        return 0;
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;  // Let OS assign a free port

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
#if JUCE_MAC || JUCE_LINUX
        close(sock);
#elif JUCE_WINDOWS
        closesocket(sock);
#endif
        return 0;
    }

    socklen_t addrLen = sizeof(addr);
    if (getsockname(sock, (struct sockaddr*)&addr, &addrLen) < 0) {
#if JUCE_MAC || JUCE_LINUX
        close(sock);
#elif JUCE_WINDOWS
        closesocket(sock);
#endif
        return 0;
    }

    int port = ntohs(addr.sin_port);

#if JUCE_MAC || JUCE_LINUX
    close(sock);
#elif JUCE_WINDOWS
    closesocket(sock);
#endif

    return port;
}

juce::File DaemonManager::getProjectRoot() {
    // In debug builds, __FILE__ is in vst/, so go up one level
    return juce::File(__FILE__).getParentDirectory().getParentDirectory();
}

std::pair<juce::String, juce::StringArray> DaemonManager::getLaunchCommand() const {
    // Use launcher script to redirect stdout/stderr to log file
    // This avoids pipe buffer blocking issues with ChildProcess
    juce::StringArray args;
    auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory).getFullPathName();

#if JUCE_DEBUG
    // Development: use uv run from project root
    auto projectRoot = getProjectRoot();
    auto launcherScript = projectRoot.getChildFile("vst/Resources/launch_daemon.sh");

    args.add(tempDir);
    args.add(juce::String(oscPort));
    args.add("uv");
    args.add("run");
    args.add("python");
    args.add("-m");
    args.add("daemomnify");
    args.add("--osc-port");
    args.add(juce::String(oscPort));
    args.add("--temp-dir");
    args.add(tempDir);

    return {launcherScript.getFullPathName(), args};
#else
    // Release: use bundled executable
    auto pluginFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile);
    auto resourcesDir = pluginFile.getParentDirectory().getParentDirectory().getChildFile("Resources");
    auto launcherScript = resourcesDir.getChildFile("launch_daemon.sh");
    auto daemonExe = resourcesDir.getChildFile("daemomnify");

    args.add(tempDir);
    args.add(juce::String(oscPort));
    args.add(daemonExe.getFullPathName());
    args.add("--osc-port");
    args.add(juce::String(oscPort));
    args.add("--temp-dir");
    args.add(tempDir);

    return {launcherScript.getFullPathName(), args};
#endif
}

bool DaemonManager::start() {
    if (isRunning()) {
        return true;
    }

    // Find a free port for OSC
    oscPort = findFreePort();
    if (oscPort == 0) {
        DBG("DaemonManager: Failed to find free port");
        return false;
    }

    DBG("DaemonManager: Starting daemon on OSC port " << oscPort);

    // Delete any stale ready file before starting
    getReadyFile().deleteFile();
    readyFired = false;

    auto [command, args] = getLaunchCommand();

    // Build full command string for ChildProcess
    juce::String fullCommand = command;
    for (const auto& arg : args) {
        fullCommand += " " + arg;
    }

    DBG("DaemonManager: Running: " << fullCommand);

    // Don't capture stdout/stderr - launcher script redirects to log file
    if (!process->start(fullCommand, 0)) {
        DBG("DaemonManager: Failed to start process");
        return false;
    }

    // Start the output reader thread
    startThread();

    // Connect OSC sender to the daemon
    if (!oscSender.connect("127.0.0.1", oscPort)) {
        DBG("DaemonManager: Failed to connect OSC sender");
    }

    DBG("DaemonManager: Daemon started successfully");
    return true;
}

void DaemonManager::run() {
    // Poll for ready file to trigger daemonReady callback
    // Daemon stdout/stderr is redirected to log file by launch_daemon.sh
    auto readyFile = getReadyFile();

    while (!threadShouldExit() && process && process->isRunning()) {
        // Check for ready file if we haven't fired yet
        if (!readyFired && listener && readyFile.existsAsFile()) {
            readyFired = true;
            DBG("DaemonManager: Ready file detected");
            // Call listener on message thread to avoid threading issues
            juce::MessageManager::callAsync([this]() {
                if (listener) {
                    listener->daemonReady();
                }
            });
        }

        // Sleep briefly to avoid spinning CPU
        Thread::sleep(50);
    }
}

void DaemonManager::sendOscQuit() {
    oscSender.send("/quit");
}

juce::File DaemonManager::getReadyFile() const {
    return juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getChildFile("daemomnify-" + juce::String(oscPort) + ".ready");
}

void DaemonManager::stop() {
    if (!process || !process->isRunning()) {
        return;
    }

    DBG("DaemonManager: Stopping daemon");

    // Send OSC /quit for graceful shutdown
    sendOscQuit();

    // Wait up to 2 seconds for graceful exit
    if (process->waitForProcessToFinish(2000)) {
        DBG("DaemonManager: Daemon exited gracefully");
        return;
    }

    // Force kill if graceful shutdown didn't work
    DBG("DaemonManager: Forcing kill");
    process->kill();
    process->waitForProcessToFinish(1000);
}

bool DaemonManager::isRunning() const { return process && process->isRunning(); }