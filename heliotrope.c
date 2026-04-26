#include "heliotrope.h"

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

static uint16_t two_byte_to_int(const unsigned char byte_1, const unsigned char byte_2) {
    return byte_1 | (byte_2 << 8);
}

static uint32_t four_byte_to_int(const unsigned char byte_1, const unsigned char byte_2, const unsigned char byte_3, const unsigned char byte_4) {
    return byte_1 | (byte_2 << 8) | (byte_3 << 16) | (byte_4 << 24);
}

#define MAX_CHUNK 131072

static const unsigned char eocd_header[] = {0x50, 0x4b, 0x05, 0x06};
static const unsigned char cd_header[] = {0x50, 0x4b, 0x01, 0x02};
static const unsigned char local_file_header[] = {0x50, 0x4b, 0x03, 0x04};
static enum HelioReturnCode extract_file(FILE *file, uint32_t offset, const char *directory_path) {
    fseek(file, offset, SEEK_SET);
    unsigned char local_header[30] = {0};
    unsigned char header_compare[4] = {0};

    size_t fread_return = fread(local_header, 1, 30, file);
    memcpy(header_compare, local_header, 4);
    if (fread_return != 30 || memcmp(header_compare, local_file_header, sizeof(header_compare)) != 0) {
        return FILE_INVALID;
    }

    //compression method check~
    uint16_t compression_method = two_byte_to_int(local_header[8], local_header[9]);
    if (compression_method != 8 && compression_method != 0) {
        return INVALID_COMPRESSION_METHOD; //anything else is not valid~
    }

    uint16_t file_name_len = two_byte_to_int(local_header[26], local_header[27]);
    uint16_t extra_field_len = two_byte_to_int(local_header[28], local_header[29]);
    uint32_t compressed_size = four_byte_to_int(local_header[18], local_header[19], local_header[20], local_header[21]);
    uint32_t uncompressed_size = four_byte_to_int(local_header[22], local_header[23], local_header[24], local_header[25]);
    unsigned char file_name[file_name_len + 1];
    fread(file_name, 1, file_name_len, file);
    file_name[file_name_len] = '\0';

    if (compressed_size == 0) { //size of 0, must be a folder!
        char *full_folder_path = helio_get_path(directory_path, (char *)file_name);
        if (!full_folder_path) {
            return FILE_INVALID;
        }
        helio_mkdir(full_folder_path);
        free(full_folder_path);
    }

    //now we're at the compressed data :0~!!
    fseek(file, extra_field_len, SEEK_CUR);

    size_t input_chunk_size = MAX_CHUNK;
    size_t output_chunk_size = MAX_CHUNK;
    if (compressed_size < MAX_CHUNK) {
        input_chunk_size = compressed_size; //for files smaller, this saves a bit of memory~
    }
    if (uncompressed_size < MAX_CHUNK) {
        output_chunk_size = uncompressed_size; //for files smaller, this saves a bit of memory~
    }
    unsigned char input_buf[input_chunk_size];
    unsigned char output_buf[output_chunk_size];

    //let's create the file >.<
    char *file_path = helio_get_path(directory_path, (char *)file_name);
    if (!file_path) {
        return FILE_INVALID;
    }
    FILE *new_file = fopen(file_path, "w");
    free(file_path);

    if (compression_method == 8) {
        //for all of the compressed data, we literally just feed it to zlib and it takes care of things for us =w=
        //we really just have to worry about parsing the .zip format, not huffman tree fuckery
        z_stream strm = {0};
        int inflate_ret = inflateInit2(&strm, -MAX_WBITS);
        if (inflate_ret != Z_OK) {
            return DEFLATE_ERROR; //not checking every single error >_<,, but its daijoubu
        }

        size_t remaining_bytes = compressed_size; //max at first
        while (inflate_ret != Z_STREAM_END && remaining_bytes > 0) { //0 bytes left to read
            size_t bytes_to_read = remaining_bytes < input_chunk_size? remaining_bytes : input_chunk_size;
            strm.avail_in = fread(input_buf, 1, bytes_to_read, file);
            strm.next_in = input_buf;

            //this do while loop case is something i honestly don't understand very well,, but i can't be bothered to rewrite
            do {
                strm.next_out = output_buf;
                strm.avail_out = output_chunk_size;
                inflate_ret = inflate(&strm, Z_NO_FLUSH);
                if (inflate_ret != Z_OK && inflate_ret != Z_STREAM_END) {
                    return DEFLATE_ERROR;
                }
                size_t bytes_out = output_chunk_size - strm.avail_out;
                fwrite(output_buf, 1, bytes_out, new_file);
            } while (strm.avail_out == 0);
        }
        inflateEnd(&strm);
    } else { //if uncompressed, no use for any of this =w=
        size_t remaining_bytes = compressed_size;
        while (remaining_bytes > 0) {
            size_t to_read = remaining_bytes < output_chunk_size? remaining_bytes : output_chunk_size;
            size_t bytes_read = fread(output_buf, 1, to_read, file);
            if (bytes_read == 0) break;

            fwrite(output_buf, 1, bytes_read, new_file);
            remaining_bytes -= bytes_read;
        }
    }

    return SUCCESS;
}

enum HelioReturnCode helio_extract(char *filename, bool verbose) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) return FILE_NOT_EXIST;
    //we don't look at the start of the zip file, since technically the first file offset could be anywhere
    //most zip files will have the first file start at, well, the first byte - but this doesn't have to be the case

    //time to actually parse this shit >:3 since .osz files have no comments, everything will just be a fixed end - 22
    //to where the start of the Embodiment of Central Directory is
    fseek(file, -22, SEEK_END);
    unsigned char eocd[22] = {0};
    size_t fread_return = fread(eocd, 1, 22, file);

    unsigned char header_compare[4] = {0};
    memcpy(header_compare, eocd, 4);
    if (fread_return != 22 || memcmp(header_compare, eocd_header, sizeof(header_compare)) != 0) {
        fclose(file);
        return FILE_INVALID;
    }

    //okay, time to read the eocd. all we really want is the number of records, size, and offset of start =w=
    //stuff like disk number is beyond the scope of this project
    uint16_t num_files = two_byte_to_int(eocd[10], eocd[11]); //number of total records
    uint32_t central_directory_size = four_byte_to_int(eocd[12], eocd[13], eocd[14], eocd[15]);
    uint32_t central_directory_start_offset = four_byte_to_int(eocd[16], eocd[17], eocd[18], eocd[19]);

    //that's everything we need from the eocd~ time to actually read the central directory!
    fseek(file, central_directory_start_offset, SEEK_SET);
    unsigned char central_directory[central_directory_size];
    fread_return = fread(central_directory, 1, central_directory_size, file);

    memcpy(header_compare, central_directory, 4);
    if (fread_return != central_directory_size || memcmp(header_compare, cd_header, sizeof(header_compare))) { //we can assume each file entry after is valid probably lol
        fclose(file);
        return FILE_INVALID;
    }

    //making output directory, additional directories inside must be made seperately~
    char *folder_name = strdup(filename);
    folder_name[strrchr(filename, '.') - filename] = '\0';
    helio_mkdir(folder_name);

    //each file has its own little central directory >_<!! we need to get all the values we want from her~
    size_t offset = 0;
    for (int i = 0; i < num_files; i++) {
        uint32_t crc32 = four_byte_to_int(central_directory[16 + offset], central_directory[17 + offset], central_directory[18 + offset], central_directory[19 + offset]);
        uint16_t file_name_len = two_byte_to_int(central_directory[28 + offset], central_directory[29 + offset]);
        uint16_t extra_field_len = two_byte_to_int(central_directory[30 + offset], central_directory[31 + offset]);
        uint16_t file_comment_len = two_byte_to_int(central_directory[32 + offset], central_directory[33 + offset]);
        uint32_t file_offset = four_byte_to_int(central_directory[42 + offset], central_directory[43 + offset], central_directory[44 + offset], central_directory[45 + offset]);

        unsigned char filename[file_name_len + 1];
        memcpy(filename, central_directory + 46 + offset, file_name_len);
        filename[file_name_len] = '\0';
        if (verbose) {
            printf("./%s/%s\n", folder_name, filename);
        }

        enum HelioReturnCode file_code = extract_file(file, file_offset, folder_name);
        if (file_code != SUCCESS) {
            return file_code;
        }

        offset += 46 + file_name_len + extra_field_len + file_comment_len;
    }

    free(folder_name);

    return SUCCESS;
}

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define MKDIR(dir) _mkdir(dir)
#else
#include <sys/stat.h>
#include <unistd.h>
#define MKDIR(dir) mkdir(dir, 0755);
#endif

void helio_mkdir(const char *dir_path) { //makes parent directories too :3
    char *dir_copy = strdup(dir_path);  // make a copy we can modify
    char *char_ptr = NULL;
    int mk_return = 0;
    if (dir_copy[strlen(dir_copy) - 1] == '/') { //we usually shouldn't get this with a slash at the end, but just in case~
        dir_copy[strlen(dir_copy) - 1] = 0;
    }

    for (char_ptr = dir_copy + 1; *char_ptr; char_ptr++) {
        if (*char_ptr == '/') { //we increase what character we're on until we get to a / :3
            *char_ptr = 0;
            mk_return = MKDIR(dir_copy);
            if (mk_return != 0 && errno != EEXIST) {
                fprintf(stderr, "%s while trying to create directory %s >.<\n", strerror(errno), dir_path);
            }
            *char_ptr = '/';
        }
    }

    mk_return = MKDIR(dir_copy); //now we can make the final path yayyyy
    if (mk_return != 0 && errno != EEXIST) {
        fprintf(stderr, "%s while trying to create directory %s >.<\n", strerror(errno), dir_path);
    }

    free(dir_copy);
}

bool helio_dir_exists(const char *directory) {
    #ifdef _WIN32
    DWORD attrib = GetFileAttributesA(directory); //fucked up windows shit
    return (attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY));
    #else
    struct stat dir_stat;
    return (stat(directory, &dir_stat) == 0 && S_ISDIR(dir_stat.st_mode));
    #endif
}

//super fucking useful!! caller must free >_<,,
char *helio_get_path(const char *prev_dir, const char *name) {
    size_t new_path_len = strlen(prev_dir) + strlen(name) + 2; //two new characters - / and \0
    char *path = calloc(1, new_path_len);

    int length_written = sprintf(path, "%s/%s", prev_dir, name);
    if (length_written != (int)(new_path_len - 1)) { //im kinda sus of this but maybe this is just okay
        free(path);
        return NULL;
    }

    return path;
}
