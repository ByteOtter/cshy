#include <errno.h>
#include <shy_file.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Define READ macro to avoid repetitive fread calls.
#define READ(var) fread(&var, sizeof(var), 1, f)
#define WRITE(var) fwrite(&var, sizeof(var), 1, f)

shy_file shy_file_read(const char* path) {
  shy_file result;
  
  FILE* f = fopen(path, "r");
  
  // Exit with error if file does not exist.
  if(!f) {
    printf("An error occured: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  
  // Read header.
  READ(result.header.magic);

  // Check if magic is the same as we expect.
  // Reinterpret the memory of the magic string value as a uint32_t.
  if (result.header.magic != *(uint32_t*) SHY_MAGIC) {
    printf("File is not a SHY-File!\n");
    exit(EXIT_FAILURE);
  }

  READ(result.header.file_cnt);
  READ(result.header.data_size);

  // Read entries.
  result.entries = calloc(result.header.file_cnt, sizeof(shy_entry));

  for (uint64_t i = 0; i < result.header.file_cnt; i++) {
    READ(result.entries[i].off);
    READ(result.entries[i].size);
    READ(result.entries[i].path_off);
  }

  result.data = calloc(result.header.data_size, sizeof(uint8_t));
  fread(result.data, sizeof(uint8_t), result.header.data_size, f); // Read all bytes from f.
  
  // Read file names
  result.paths = calloc(result.header.str_size, sizeof(char));
  fread(result.paths, sizeof(char), result.header.str_size, f);
  
  fclose(f);
  return result;
}

void shy_file_save(shy_file files, const char* path) {
  FILE* f = fopen(path, "w");

  if (!f) {
    printf("An error occured when writing to file: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  
  if (files.header.magic != *(uint32_t*) SHY_MAGIC) {
    printf("File is not a SHY-File!\n");
    exit(EXIT_FAILURE);
  }

  WRITE(files.header.magic);
  WRITE(files.header.file_cnt);
  WRITE(files.header.data_size);

  for (uint64_t i = 0; i < files.header.file_cnt; i++) {
    WRITE(files.entries[i].off);
    WRITE(files.entries[i].size);
    WRITE(files.entries[i].path_off);
  }

  fwrite(files.data, sizeof(uint8_t), files.header.data_size, f);

  fwrite(files.paths, sizeof(char), files.header.str_size, f);
  
  fclose(f);
}

shy_file shy_file_create(const char** file_paths, size_t file_cnt) {
  shy_file result;
  size_t data_size = 0;
  size_t str_size = 0;

  result.header.magic = *(uint32_t*) SHY_MAGIC;
  result.header.file_cnt = file_cnt;
  result.entries = calloc(file_cnt, sizeof(shy_entry));
  
  for (size_t i = 0; i < file_cnt; i++) {
    // Get total length of files.
    FILE* f = fopen(file_paths[i], "r");
    fseek(f, 0, SEEK_END);
    size_t file_size = ftell(f);
    result.entries[i].size = file_size;
    result.entries[i].off = data_size;
    data_size += file_size;
    fclose(f);
    
    result.entries[i].path_off = str_size;
    // Get length of path string including nullbyte.
    str_size += strlen(file_paths[i])+1;
  }

  result.data = calloc(data_size, sizeof(uint8_t));
  uint8_t* cur = result.data;

  for (size_t i = 0; i < file_cnt; i++) {
    // Read the file as bytes into memory.
    FILE* f = fopen(file_paths[i], "r");
    fseek(f, 0, SEEK_END);
    size_t filesize = ftell(f);
    fseek(f, 0, SEEK_SET);
    fread(cur, filesize, 1, f);
    cur += filesize;
    fclose(f);
  }
  
  return result;
}

