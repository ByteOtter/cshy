#include <stdint.h>

// Define macro for magic sanitization.
#define SHY_MAGIC "SHY\xFF"

// Shy file format header.
typedef struct {
  uint32_t magic;
  uint64_t file_cnt;
  uint64_t data_size;
  uint64_t str_size;
} shy_hdr;

// Shy file contents header.
typedef struct {
  uint64_t off;
  uint64_t size;
  uint64_t path_off;
} shy_entry;

// Shy file format struct.
typedef struct {
  shy_hdr header;
  shy_entry* entries;
  uint8_t* data;
  char* paths;
} shy_file;

// Read existing SHY-File
shy_file shy_file_read(const char* path);
