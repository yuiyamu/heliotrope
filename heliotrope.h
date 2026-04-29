#ifndef HELIOTROPE_H_
#define HELIOTROPE_H_

#include <stdbool.h>
#include <stdio.h>

enum HelioReturnCode {
  SUCCESS,
  FILE_NOT_EXIST,
  FILE_INVALID,
  INVALID_COMPRESSION_METHOD,
  DEFLATE_ERROR,
  FILESYSTEM_ERROR
};

struct HelioFile {
  size_t uncompressed_size;
  size_t compressed_size;
  unsigned char *compressed_data; //only need to hold compressed in memory~
  
  char *file_name;
  size_t file_name_len;
};

void helio_mkdir(const char *dir_path);
bool helio_dir_exists(const char *directory);
char *helio_get_path(const char *prev_dir, const char *name);
char **helio_list_dir(const char *directory, bool recusrive);

enum HelioReturnCode helio_extract(char *filename, bool verbose);
enum HelioReturnCode helio_compress(char *folder_path, char *filename, char *extension, bool verbose);

void *safe_alloc(void *ptr, size_t bytes);
void *safe_calloc(size_t num_elements, size_t element_size);

#endif
