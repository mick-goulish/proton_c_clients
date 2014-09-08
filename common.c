#include "common.h"

#include <ctype.h>
#include <time.h>

#include <sys/time.h>

void print_timestamp(FILE *fp, char const *label) {
  struct timeval tv;
  gettimeofday(&tv, 0);
  struct tm *timeinfo = localtime(&tv.tv_sec);

  int seconds_today = 3600 * timeinfo->tm_hour +
    60 * timeinfo->tm_min + timeinfo->tm_sec;

  fprintf(fp, "time : %d.%.6ld : %s\n", seconds_today, tv.tv_usec, label);
}

void print_data(FILE *fp, char const *str, int len) {
  fputc('|', fp);
  for (int i = 0; i < len; ++i) {
    unsigned char c = *str++;
    if (isprint(c)) {
      fputc(c, fp);
    } else {
      fprintf(fp, "\\0x%.2X", c);
    }
  }
  fputs("|\n", fp);
}
