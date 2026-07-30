#include "heliotrope.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>

static void print_help(void) {
  printf("Usage: [file] when extracting an archive,\n    or [directory] [file] when creating an archive.\n\n");
  printf("    -c, --create   creates an archive instead of extracting\n");
  printf("    -v, --verbose  uses verbose output\n");
}

static int parse_return_code(const enum HelioReturnCode extract_code) {
  int ret_code = 1;
  switch (extract_code) {
    case SUCCESS: {
      printf("success.\n");
      ret_code = 0;
      break;
    }
    case FILE_NOT_EXIST: {
      fprintf(stderr, "provided file does not exist.\n");
      break;
    }
    case FILE_INVALID: {
      fprintf(stderr, "provided file is not a valid zip file.\n");
      break;
    }
    case INVALID_COMPRESSION_METHOD: {
      fprintf(stderr, "provided file uses a compression method not supported by heliotrope.\n");
      break;
    }
    case DEFLATE_ERROR: {
      fprintf(stderr, "provided zip file failed to be parsed by the deflate algorithm.\n");
      break;
    }
    case FILESYSTEM_ERROR: {
      fprintf(stderr, "something went wrong when dealing with filesystem i/o.\ndo you have enough free space on your device, and permissions to the files?\n");
      break;
    }
  }

  return ret_code;
}

int main(int argc, char **argv) {
  printf("heliotrope - version 1.1.0\nan incredibly simple zip extractor, made by yuiyamu\n\n");

  bool create_archive = false;
  bool verbose = false;
  static struct option long_opts[] = { //for getopt_long, we need this kind of struct hehe~
    { "create",  no_argument, NULL, 'c' },
    { "verbose", no_argument, NULL, 'v' },
    { "help",    no_argument, NULL, 'h' },
    { NULL, 0, NULL, 0 }
  };

  int opt;
  while ((opt = getopt_long(argc, argv, "cvh", long_opts, NULL)) != -1) {
    switch (opt) {
      case 'c': {
        create_archive = true;
        break;
      }
      case 'v': {
        verbose = true;
        break;
      }
      case 'h': {
        print_help();
        return 0;
      }
      case '?': {
        print_help();
        return 1; //error, since they didn't know what da heck to put
      }
    }
  }

  int remaining_params = argc - optind;
  if (create_archive) {
    if (remaining_params == 0) {
      fprintf(stderr, "no input directory.\n");
      return 1;
    } else if (remaining_params > 2) {
      fprintf(stderr, "error: too many parameters.\n");
      return 1;
    }
    char *folder_path = argv[optind];
    char *filename = remaining_params > 1? argv[optind + 1] : argv[optind];
    return parse_return_code(helio_compress(folder_path, filename, ".zip", verbose));
  } else {
    if (remaining_params == 0) {
      fprintf(stderr, "no input file.\n");
      return 1;
    } else if (remaining_params > 1) {
      fprintf(stderr, "error: too many parameters | use --create to make an archive\n");
      return 1;
    }

    char *filename = argv[optind];
    printf("extracting %s...\n", filename);
    return parse_return_code(helio_extract(filename, verbose));
  }
}
