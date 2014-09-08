#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>

#define MY_BUF_SIZE  1000

void print_timestamp(FILE *fp, char const *label);
void print_data(FILE *fp, char const *str, int len);

#endif /* !__COMMON_H__ */
