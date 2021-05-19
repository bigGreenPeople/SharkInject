//
// Created by Administrator on 2021/5/3.
//
#pragma once

#ifndef ANDROIDINJECT_PTRACE_UTILS_H
#define ANDROIDINJECT_PTRACE_UTILS_H

#define CPSR_T_MASK     ( 1u << 5 )
#define REMOTE_ADDR(addr, local_base, remote_base) ( (uint32_t)(addr) + (uint32_t)(remote_base) - (uint32_t)(local_base) )

#if defined(__aarch64__)
#define pt_regs         user_pt_regs
#define uregs    regs
#define ARM_pc    pc
#define ARM_sp    sp
#define ARM_cpsr    pstate
#define ARM_lr        regs[30]
#define ARM_r0        regs[0]
#define PTRACE_GETREGS PTRACE_GETREGSET
#define PTRACE_SETREGS PTRACE_SETREGSET
#elif defined(__x86_64__)
#define pt_regs user_regs_struct
#define eax     rax
#define esp     rsp
#define eip     rip
#endif

#if defined(__aarch64__)
#define pt_regs         user_pt_regs
#elif defined(__x86_64__)
#define pt_regs user_regs_struct
#endif

int ptrace_attach(pid_t pid);

int ptrace_detach(pid_t pid);

int ptrace_continue(pid_t pid);

int ptrace_getregs(pid_t pid, struct pt_regs *regs);

int ptrace_setregs(pid_t pid, struct pt_regs *regs);

long ptrace_getret(struct pt_regs *regs);

long ptrace_getpc(struct pt_regs *regs);

int ptrace_readdata(pid_t pid, uint8_t *pSrcBuf, uint8_t *pDestBuf, size_t size);

int ptrace_writedata(pid_t pid, uint8_t *pWriteAddr, uint8_t *pWriteData,
                     size_t size);

int ptrace_call(pid_t pid, uintptr_t ExecuteAddr, long *parameters, long num_params,
                struct pt_regs *regs);

void dump_registers(struct pt_regs *regs);

void *get_mmap_address(pid_t pid);

void *get_dlopen_address(pid_t pid);

void *get_dlclose_address(pid_t pid);

void *get_dlsym_address(pid_t pid);

void *get_dlerror_address(pid_t pid);


#endif //ANDROIDINJECT_PTRACE_UTILS_H
