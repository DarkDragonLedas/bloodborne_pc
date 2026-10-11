// bbport: Windows stand-ins for the POSIX facilities the GPU-side diagnostics use: dladdr
// (module name and offset instead of a symbol), anonymous mmap (VirtualAlloc) and mprotect
// (VirtualProtect) of the port's own host blocks, in the Linux PROT_* spelling.
#pragma once
#ifdef _WIN32
#include <windows.h>

// windows.h macros collide with names the video core uses (vk::MemoryBarrier, near, far).
#undef MemoryBarrier
#undef near
#undef far


struct Dl_info {
    const char* dli_fname;
    void* dli_fbase;
    const char* dli_sname;
    void* dli_saddr;
};
inline int dladdr(const void* addr, Dl_info* info) {
    HMODULE m = nullptr;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            static_cast<LPCWSTR>(addr), &m)) {
        return 0;
    }
    static thread_local wchar_t wpath[MAX_PATH];
    static thread_local char path[MAX_PATH];
    GetModuleFileNameW(m, wpath, MAX_PATH);
    WideCharToMultiByte(CP_UTF8, 0, wpath, -1, path, MAX_PATH, nullptr, nullptr);
    info->dli_fname = path;
    info->dli_fbase = m;
    info->dli_sname = nullptr;
    info->dli_saddr = nullptr;
    return 1;
}

#define PROT_NONE 0x0
#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define MAP_PRIVATE 0x02
#define MAP_ANONYMOUS 0x20
#define MAP_NORESERVE 0x4000
#define MAP_FAILED reinterpret_cast<void*>(-1)

inline void* mmap(void*, size_t length, int, int, int, long long) {
    return VirtualAlloc(nullptr, length, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}
inline int munmap(void* mem, size_t) {
    VirtualFree(mem, 0, MEM_RELEASE);
    return 0;
}
inline unsigned gettid() {
    return GetCurrentThreadId();
}
// glibc's execinfo backtrace: a plain capture without symbol names (dbghelp would need the
// debug directories); diagnostics only.
inline int backtrace(void** buffer, int size) {
    return CaptureStackBackTrace(1, size, buffer, nullptr);
}
inline void backtrace_symbols_fd(void* const*, int, int) {}

inline int mprotect(void* addr, size_t len, int prot) {
    DWORD win = PAGE_NOACCESS;
    if (prot & PROT_WRITE) {
        win = PAGE_READWRITE;
    } else if (prot & PROT_READ) {
        win = PAGE_READONLY;
    }
    DWORD old = 0;
    return VirtualProtect(addr, len, win, &old) ? 0 : -1;
}
#endif
