#include <errno.h>
#include <shy_file.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define READ macro to avoid repetitive fread calls.
#define READ(var) fread(&var, sizeof(var), 1, f)

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
    printf("File is not a SHY-File!");
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

  return result;
}
