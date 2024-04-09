#include <stdint.h>
#include <stddef.h>
// Define macro for magic sanitization.
#define SHY_MAGIC "SHY\xFF"

// Shy file format header.
typedef struct {
  uint32_t magic;
  uint64_t file_cnt;
  uint64_t str_size;
} shy_hdr;

// Shy file contents header.
typedef struct {
  uint64_t off;
  uint64_t size;
  uint64_t path_off;
  uint64_t unc_size; // Uncompressed size. No compression if 0.
  uint8_t* data;
} shy_entry;

// Shy file format struct.
typedef struct {
  shy_hdr header;
  shy_entry* entries;
  char* paths;
} shy_file;

// Read existing SHY-File.
shy_file shy_file_read(const char* path);

// Save SHY-File to disk.
void shy_file_save(shy_file files, const char* path);

// Create SHY-File memory representation.
shy_file shy_file_create(const char** file_paths, size_t file_cnt);

void shy_file_unpack(shy_file f, const char* shyFileName);
