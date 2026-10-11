// bbport: the fault handlers are written against the Linux signal context (ucontext_t with a
// gregs table). Windows: the vectored handler's EXCEPTION_POINTERS* is converted into the same
// shape (SysV AMD64 register order) on the fly.
#pragma once
#ifdef _WIN32
#include <windows.h>

// windows.h macros collide with names the video core uses (vk::MemoryBarrier, near, far).
#undef MemoryBarrier
#undef near
#undef far


using greg_t = long long;
enum {
    REG_R8 = 0, REG_R9, REG_R10, REG_R11, REG_R12, REG_R13, REG_R14, REG_R15,
    REG_RDI, REG_RSI, REG_RBP, REG_RBX, REG_RDX, REG_RAX, REG_RCX, REG_RSP, REG_RIP, REG_EFL,
};
typedef struct { greg_t gregs[23]; } mcontext_t;
typedef struct { mcontext_t uc_mcontext; } ucontext_t;

inline const ucontext_t* bb_ucontext(const void* context) {
    static thread_local ucontext_t u;
    const CONTEXT* c = static_cast<const EXCEPTION_POINTERS*>(context)->ContextRecord;
    greg_t* g = u.uc_mcontext.gregs;
    g[REG_R8] = c->R8;   g[REG_R9] = c->R9;   g[REG_R10] = c->R10; g[REG_R11] = c->R11;
    g[REG_R12] = c->R12; g[REG_R13] = c->R13; g[REG_R14] = c->R14; g[REG_R15] = c->R15;
    g[REG_RDI] = c->Rdi; g[REG_RSI] = c->Rsi; g[REG_RBP] = c->Rbp; g[REG_RBX] = c->Rbx;
    g[REG_RDX] = c->Rdx; g[REG_RAX] = c->Rax; g[REG_RCX] = c->Rcx; g[REG_RSP] = c->Rsp;
    g[REG_RIP] = c->Rip; g[REG_EFL] = c->EFlags;
    return &u;
}
#define UCONTEXT(c) bb_ucontext(c)
#else
#include <ucontext.h>
#define UCONTEXT(c) static_cast<const ucontext_t*>(c)
#endif
