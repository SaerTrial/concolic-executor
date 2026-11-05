/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Concolic side of the hook port: input bytes become symbolic where they land. */
#include "qemu/osdep.h"
#include "qemu/log.h"
#include "cpu.h"
#include "hooks/hook_engine.h"

#define SymExpr void*
#include "RuntimeCommon.h"

extern uint32_t fuzz_cursor;
bool getFuzz(uint8_t *buf, uint32_t size);
void arm_sym_clear_reg(CPUARMState *env, unsigned reg);    /* target/arm/tcg/translate.c */

uint64_t hook_port_take_input(uint8_t *buf, size_t len)
{
    uint64_t offset = fuzz_cursor;

    getFuzz(buf, len);          /* ends the run when input runs out */
    return offset;
}

void hook_port_mem_input(void *host, size_t len, uint64_t offset)
{
    _sym_make_symbolic(host, len, offset);      /* host[i] is input byte offset+i */
}

void hook_port_mem_concrete(void *host, size_t len)
{
    _sym_write_memory(host, len, NULL, true);
}

void hook_port_reg_concrete(CPUState *cs, unsigned reg)
{
    arm_sym_clear_reg(cpu_env(cs), reg);
}

void hook_port_log(const char *msg)
{
    qemu_log_mask(LOG_CONCOLIC_REPLAY, "%s", msg);
}
