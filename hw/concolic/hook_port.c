/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Concolic side of the hook port: input bytes become symbolic where they land. */
#include "qemu/osdep.h"
#include "qemu/log.h"
#include "cpu.h"
#include "tcg/tcg.h"
#include "hooks/hook_engine.h"

#define SymExpr void*
#include "RuntimeCommon.h"

extern uint32_t fuzz_cursor;
bool getFuzz(uint8_t *buf, uint32_t size);

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

/*
 * A register's symbolic value sits in the expression slot of the TCG global
 * stored at `reg`. tcg.c allocates that slot as the temp right after the
 * global, at mem_offset from env. Every guest register is such a global.
 */
void hook_port_reg_concrete(CPUState *cs, void *reg)
{
    char *env = (char *)cpu_env(cs);
    TCGTemp *env_temp = tcgv_ptr_temp(tcg_env);

    for (int i = 0; i < tcg_ctx->nb_globals; i++) {
        TCGTemp *ts = &tcg_ctx->temps[i];

        if (ts->mem_base == env_temp && !ts->symbolic_expression &&
            env + ts->mem_offset == (char *)reg) {
            g_assert(ts[1].symbolic_expression);
            *(void **)(env + ts[1].mem_offset) = NULL;
            return;
        }
    }
    g_assert_not_reached();
}

void hook_port_log(const char *msg)
{
    qemu_log_mask(LOG_CONCOLIC_REPLAY, "%s", msg);
}
