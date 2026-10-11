// bbport: GPU-side assertion failures stop the port with exit code 23.
#ifdef _WIN32
#include <windows.h>
#endif
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include "common/assert.h"
#include "common/logging/log.h"
#include <unistd.h>

// bb-probe (probe.c): set while the port restarts itself through run.sh. The device fd is closed
// before exec; Vulkan calls failing then are not errors: this thread waits for the exec instead.
extern "C" __attribute__((weak)) volatile int runtime_restarting; // absent in the tests

// bbport_write_log.cpp: with BB_WRITE_LOG, the latest guest memory writes (no-op otherwise).
extern "C" void bbgpu_dump_guest_writes(void* ucontext);

void assert_fail_impl() {
    if (&runtime_restarting && runtime_restarting) {
        for (;;) {
#ifdef _WIN32
            Sleep(INFINITE);
#else
            pause();
#endif
        }
    }
    std::fflush(stdout);
    bbgpu_dump_guest_writes(nullptr);
    std::fputs("STOP: GPU library assertion failed (see GPU log above)\n", stderr);
    std::_Exit(23);
}

[[noreturn]] void unreachable_impl() {
    assert_fail_impl();
    throw std::runtime_error("Unreachable code");
}

void assert_fail_debug_msg(const char* msg) {
    LOG_CRITICAL(Debug, "Assertion Failed!\n{}", msg);
    assert_fail_impl();
}
