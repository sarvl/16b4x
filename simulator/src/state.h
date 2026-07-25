#pragma once

#include <cstdint>

#include "cr.h"
#include "privilege.h"


extern bool interrupt_enabled;
extern bool halt;

extern uint16_t regs[8];
extern uint16_t ip;
extern uint16_t lc;
extern uint16_t ui;
extern uint16_t sp;
extern uint16_t ar;

extern uint16_t fl_s, fl_o, fl_c, fl_z, fl_d;


extern bool interrupt_enabled;
extern bool io_enabled       ;
extern bool ui_modified      ;

extern bool interrupt_pgf;
extern bool interrupt_prv;
extern bool interrupt_pev;
extern bool interrupt_ins;
extern bool interrupt_ina;
extern bool interrupt_tmr;
extern bool interrupt_div;
extern bool interrupt_io ;
extern bool interrupt_sw ;

extern int  interrupt_sw_id;


constexpr uint8_t  interrupt_pgf_id                      =  8;
constexpr uint8_t  interrupt_prv_id                      =  9;
constexpr uint8_t  interrupt_pev_id                      = 10;
constexpr uint8_t  interrupt_ins_id                      = 11;
constexpr uint8_t  interrupt_ina_id                      = 12;
constexpr uint8_t  interrupt_tmr_id                      = 32;
constexpr uint8_t  interrupt_div_id                      = 33;

constexpr uint16_t state_update_disable_int              = 0x0001;
constexpr uint16_t state_update_enable_int               = 0x0002;
constexpr uint16_t state_update_iht_reload               = 0x0004;
constexpr uint16_t state_update_flush_tlb                = 0x0008;

constexpr uint16_t paf_0_fast_boot                       = 0x0001;

constexpr uint16_t uaf_0_divide                          = 0x0001;
constexpr uint16_t uaf_0_random                          = 0x0002;
