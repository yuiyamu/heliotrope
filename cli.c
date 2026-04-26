#include "heliotrope.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

int main(int argc, char **argv) {
    printf("heliotrope - version 1.0.0\nan incredibly simple zip extractor, made by yuiyamu\n\n");

  if (argc == 1) {
    printf("no input files.\n");
    return 1;
  } else if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
    printf("[file] with no other arguments extracts to a newly created folder with the same name.\n");
    printf("[file] [folder] will extract to a specified location.\n");
  } else if (argc == 2 || argc == 3) { //one argument other than help, must be a file
    char *filename = argv[1];
    char *dir = argv[1];
    if (argc == 3) {
      dir = argv[2];
    }
    
    int ret_code = 1;
    printf("extracting %s...\n", argv[1]);
    enum HelioReturnCode extract_code = helio_extract(filename, true);
    switch (extract_code) {
      case SUCCESS: {
        printf("success.\n");
        ret_code = 0;
        break;
      }
      case FILE_NOT_EXIST: {
        printf("provided file does not exist.\n");
        break;
      }
      case FILE_INVALID: {
        printf("provided file is not a valid zip file.\n");
        break;
      }
      case INVALID_COMPRESSION_METHOD: {
        printf("provided file uses a compression method not supported by heliotrope.\n");
        break;
      }
      case DEFLATE_ERROR: {
        printf("provided zip file failed to be parsed by the deflate algorithm.\n");
        break;
      }
    }
    return ret_code;
  } else {
    printf("unknown parameters.\n");
    return 1;
  }
}
