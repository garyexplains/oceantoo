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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "oceantoo.h"
#include "sha256.h"

void lfsr128_set_password(lfsr128_t *l, unsigned char *p) {
  BYTE buf[SHA256_BLOCK_SIZE];
  SHA256_CTX ctx;
  uint64_t lfsr_h;
  uint64_t lfsr_l;

  sha256_init(&ctx);
  sha256_update(&ctx, p, strlen((char *) p));
  sha256_final(&ctx, buf);
  memcpy(&lfsr_h, buf, sizeof(uint64_t));
  memcpy(&lfsr_l, buf + sizeof(uint64_t), sizeof(uint64_t));
  lfsr128_init(l, lfsr_h, lfsr_l);
}

void lfsr128x3_set_password(lfsr128x3_t *l, unsigned char *p) {
  lfsr128_t lfsr128_captain;

  lfsr128_set_password(&lfsr128_captain, p);
  l->lfsr[0].lfsr_h = lfsr128_shiftn(&lfsr128_captain, 64);
  l->lfsr[0].lfsr_l = lfsr128_shiftn(&lfsr128_captain, 64);
  l->lfsr[1].lfsr_h = lfsr128_shiftn(&lfsr128_captain, 64);
  l->lfsr[1].lfsr_l = lfsr128_shiftn(&lfsr128_captain, 64);
  l->lfsr[2].lfsr_h = lfsr128_shiftn(&lfsr128_captain, 64);
  l->lfsr[2].lfsr_l = lfsr128_shiftn(&lfsr128_captain, 64);
}

void lfsr128_init(lfsr128_t *l, uint64_t lfsr_h, uint64_t lfsr_l) {
  l->lfsr_h = lfsr_h;
  l->lfsr_l = lfsr_l;
}

uint64_t lfsr128_shift(lfsr128_t *l) {
  uint64_t bit, bit_h, r;
  r = l->lfsr_l & 1;
  bit = ((l->lfsr_l >> 0) ^ (l->lfsr_l >> 1) ^ (l->lfsr_l >> 2) ^
         (l->lfsr_l >> 7)) & 1;
  bit_h = l->lfsr_h & 1;
  l->lfsr_l = (l->lfsr_l >> 1) | (bit_h << 63);
  l->lfsr_h = (l->lfsr_h >> 1) | (bit << 63);

  return r;
}

/* 
 * Return the carry bit, not the shited out bit
 */
uint64_t lfsr128_shift_return_carry(lfsr128_t *l) {
  uint64_t bit, bit_h;
  bit = ((l->lfsr_l >> 0) ^ (l->lfsr_l >> 1) ^ (l->lfsr_l >> 2) ^
         (l->lfsr_l >> 7)) & 1;
  bit_h = l->lfsr_h & 1;
  l->lfsr_l = (l->lfsr_l >> 1) | (bit_h << 63);
  l->lfsr_h = (l->lfsr_h >> 1) | (bit << 63);

  return bit;
}

uint64_t lfsr128_shiftn(lfsr128_t *l, uint8_t n) {
  uint64_t r = 0;
  int i;
  r = lfsr128_shift(l);
  for (i = 0; i < n - 1; i++) {
    r = r << 1;
    r = r | lfsr128_shift(l);
  }

  return r;
}

uint64_t lfsr128_shift_with_mult_dec(lfsr128x3_t *l) {
  uint64_t r0, r1, r2;

  r0 = lfsr128_shift(&l->lfsr[0]);
  r1 = lfsr128_shift(&l->lfsr[1]);
  r2 = lfsr128_shift_return_carry(&l->lfsr[2]);

  if (r2 == 1) {
    /* Decimate r0 by 1 bit, r2 by 2 bits*/
    r0 = lfsr128_shift(&l->lfsr[0]);
    r1 = lfsr128_shift(&l->lfsr[1]);
    r1 = lfsr128_shift(&l->lfsr[1]);
  }

  return r0 ^ r1;
}

uint64_t lfsr128_shiftn_with_mult_dec(lfsr128x3_t *l, uint8_t n) {
  uint64_t r = 0;
  int i;

  r = lfsr128_shift_with_mult_dec(l);
  for (i = 0; i < n - 1; i++) {
    r = r << 1;
    r = r | lfsr128_shift_with_mult_dec(l);
  }

  return r;
}

void code_buffer(uint8_t *b, lfsr128x3_t *l, int sz) {
  for(int i=0;i<sz;i++) {
    b[i] = b[i] ^ (uint8_t)lfsr128_shiftn_with_mult_dec(l, 8);
  }
}

void do_print_random_numbers(lfsr128x3_t *l, int sz) {
  uint64_t ln = 0;

  do {
    ln++;
    printf("%03d\n", (uint8_t)lfsr128_shiftn_with_mult_dec(l, 8));
  } while (ln < sz);
}

void usage() {
  printf(
      "Oceantoo - Copyright (c) 2021, Gary Sims\n"
      "Usage: oceantoo [-p password] [-n offset] <source> <output>\n"
      "Options: \n"
      "\t-p - Password (i.e. key)\n"
      "\t-n - Random number seq offset\n"
      "\t-r - Print random numbers (doesn't encode/decode the file)\n"
      "Arguments: \n"
      "\tsource - Input filename \n"
      "\toutput - Output filename \n");
}

int main(int argc, char *argv[]) {

  int opt;
  int offset = -1;
  int verbose = 0;
  int print_random_numbers = 0;
  unsigned char *password = NULL;
  char *input_fn = NULL;
  char *output_fn = NULL;
  lfsr128x3_t lfsr;
  int how_many_rand_nums = 1000000;

  while ((opt = getopt(argc, argv, "rl:n:p:hv?")) != -1) {
    switch (opt) {
    case 'v':
      verbose = 1;
      break;
    case 'r':
      print_random_numbers = 1;
      break;
    case 'l':
      how_many_rand_nums = atoi(optarg);
      break;
    case 'n':
      offset = atoi(optarg);
      break;
    case 'p':
      password = malloc(strlen(optarg) + 1);
      strcpy((char *) password, optarg);
      break;
    case 'h':
      usage();
      exit(EXIT_FAILURE);
      break;
    default: /* '?' */
      usage();
      exit(EXIT_FAILURE);
    }
  }

  if (password == NULL) {
    fprintf(stderr, "Password needed.\n\n");
    usage();
    exit(EXIT_FAILURE);
  }

  if (print_random_numbers == 0) {
    if (optind >= argc) {
      fprintf(stderr, "Expected two arguments after options.\n\n");
      usage();
      exit(EXIT_FAILURE);
    }

    if (optind + 1 >= argc) {
      fprintf(stderr, "Expected another argument.\n\n");
      usage();
      exit(EXIT_FAILURE);
    }
    input_fn = malloc(strlen(argv[optind]) + 1);
    strcpy(input_fn, argv[optind]);
    output_fn = malloc(strlen(argv[optind + 1]) + 1);
    strcpy(output_fn, argv[optind + 1]);
  }

  if (verbose) {
    if (offset > 0) {
      printf("Offset (-n): %d\n", offset);
    }

    if (password != NULL) {
      printf("Password: %s\n", password);
    }
    if (print_random_numbers) {
      printf("Printing random numbers...\n");
    } else {
      printf("Input filename: %s\n", input_fn);
      printf("Output filename: %s\n", output_fn);
    }
  }

  lfsr128x3_set_password(&lfsr, password);

  if(offset > 0) {
    for(int i=0;i<offset;i++) {
      lfsr128_shiftn_with_mult_dec(&lfsr, 8);
    }
  }

  if (print_random_numbers) {
    do_print_random_numbers(&lfsr, how_many_rand_nums);
    return EXIT_SUCCESS;
  }

  FILE *fp_in = fopen(input_fn, "rb");
  if (!fp_in) {
    perror("Couldn't open input");
    return EXIT_FAILURE;
  }

  FILE *fp_out = fopen(output_fn, "wb");
  if (!fp_out) {
    perror("Could't write to ouput");
    return EXIT_FAILURE;
  }

  uint8_t buffer[BUFFER_SZ];
  size_t n;

  while ((n = fread(buffer, 1, BUFFER_SZ, fp_in)) != 0) {
    code_buffer(buffer, &lfsr, n);
    fwrite(buffer, 1, n, fp_out);
  }
  if (n > 0) {
    code_buffer(buffer, &lfsr, n);
    fwrite(buffer, 1, n, fp_out);
  }

  fclose(fp_in);
  fclose(fp_out);

  return EXIT_SUCCESS;
}
