/* Copyright (c) 2021, Gary Sims
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
*/

#ifndef LFSR128_H
#define LFSR128_H

/*************************** HEADER FILES ***************************/
#include <stdint.h>

/*************************** DEFINES ***************************/
#define BUFFER_SZ 4096

/**************************** DATA TYPES ****************************/
typedef struct lfsr128_t {
  uint64_t lfsr_h;
  uint64_t lfsr_l;
} lfsr128_t;

typedef struct lfsr128x3_t {
  lfsr128_t lfsr[3];
} lfsr128x3_t;

/*********************** FUNCTION DECLARATIONS **********************/

void lfsr128_set_password(lfsr128_t *l, unsigned char *p);
void lfsr128_init(lfsr128_t *l, uint64_t lfsr_h, uint64_t lfsr_l);
uint64_t lfsr128_shift(lfsr128_t *l);
uint64_t lfsr128_shiftn(lfsr128_t *l, uint8_t n);
uint64_t lfsr128_shift_with_mult_dec(lfsr128x3_t *l);
uint64_t lfsr128_shiftn_with_mult_dec(lfsr128x3_t *l, uint8_t n);
uint64_t lfsr128_shift_return_carry(lfsr128_t *l);
void lfsr128x3_set_password(lfsr128x3_t *l, unsigned char *p);
uint64_t lfsr128_shift_with_mult_dec(lfsr128x3_t *l);
void do_print_random_numbers(lfsr128x3_t *l, int sz);
void code_buffer(uint8_t *b, lfsr128x3_t *l, int sz);

void usage();

#endif   // LFSR128_H
