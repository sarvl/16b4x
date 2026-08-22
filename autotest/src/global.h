#pragma once

extern bool print_help;
extern bool print_summary;
extern bool print_summary_pretty;
extern bool print_group;
extern bool print_failed;
extern bool print_passed;

extern int  count_passed;
extern int  count_total;


int test_sim();
int test_sasm();

int gen_sim();
int gen_sasm();
