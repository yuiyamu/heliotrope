#ifndef HELIOTROPE_H_
#define HELIOTROPE_H_

#include <stdbool.h>
#include <stdio.h>

enum HelioReturnCode {
  SUCCESS,
  FILE_NOT_EXIST,
  FILE_INVALID,
  INVALID_COMPRESSION_METHOD,
  DEFLATE_ERROR
};

struct HelioFile {
  size_t uncompressed_size;
  size_t compressed_size;
  unsigned char *uncompressed_data;
  unsigned char *compressed_data;
  
  char *file_name;
  size_t file_name_len;
};

void helio_mkdir(const char *dir_path);
bool helio_dir_exists(const char *directory);
char *helio_get_path(const char *prev_dir, const char *name);

enum HelioReturnCode helio_extract(char *filename, bool verbose);

#endif
