#include <errno.h>
#include <shy_file.h>
#include <gcrypt.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <zstd.h>

// Define READ macro to avoid repetitive fread calls.
#define READ(var) fread(&var, sizeof(var), 1, f)
#define WRITE(var) fwrite(&var, sizeof(var), 1, f)
// Align integer to an address boundary.
// If they are already aligned, do nothing.
#define ALIGN(x, by) (x % by == 0 ? x : x + (by - (x % by)))


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
  READ(result.header.str_size);

  // Read entries.
  result.entries = calloc(result.header.file_cnt, sizeof(shy_entry));

  for (uint64_t i = 0; i < result.header.file_cnt; i++) {
    READ(result.entries[i].off);
    READ(result.entries[i].size);
    READ(result.entries[i].path_off);
    READ(result.entries[i].unc_size);
  }
 

  // Read file data.
  for (uint64_t i = 0; i < result.header.file_cnt; i++) {
    uint8_t* buff = calloc(result.entries[i].size, sizeof(uint8_t));
    fread(buff, sizeof(*buff), result.entries[i].size, f);

    result.entries[i].data = calloc(result.entries[i].unc_size, sizeof(uint8_t));
    
    size_t err = ZSTD_decompress(result.entries[i].data, result.entries[i].unc_size, buff, result.entries[i].size);

    if (ZSTD_isError(err)) {
      printf("Error while decompressing file! %s\n", ZSTD_getErrorName(err));
      exit(1);
    }
    free(buff); 
  }
  
  // Read file names
  result.paths = calloc(result.header.str_size, sizeof(char));
  fread(result.paths, sizeof(char), result.header.str_size, f);
  
  fclose(f);
  return result;
}

void shy_file_save(shy_file files, const char* path, int clvl) {
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
  WRITE(files.header.str_size);

  uint8_t** file_bodies = calloc(files.header.file_cnt, sizeof(uint8_t*));

  for (uint64_t i = 0; i < files.header.file_cnt; i++) {
    printf("Compressing files with compression level '%i'. File %lu/%lu...\n", clvl, i+1, files.header.file_cnt);
    file_bodies[i] = calloc(ZSTD_compressBound(files.entries[i].unc_size), sizeof(uint8_t));
    size_t err = ZSTD_compress(file_bodies[i], ZSTD_compressBound(files.entries[i].unc_size), files.entries[i].data,files.entries[i].unc_size, clvl);
   
    if (ZSTD_isError(err)) {
      printf("Error while trying to compress source files! %s! (line: %i)\n", ZSTD_getErrorName(err), __LINE__);
      exit(1);
    }
    files.entries[i].size = err;

    WRITE(files.entries[i].off);
    WRITE(files.entries[i].size);
    WRITE(files.entries[i].path_off);
    WRITE(files.entries[i].unc_size);
 }

  for (uint64_t i = 0; i < files.header.file_cnt; i++) {
    fwrite(file_bodies[i], sizeof(uint8_t), files.entries[i].size, f);
 }

  fwrite(files.paths, sizeof(char), files.header.str_size, f);
  
  fclose(f);
}

/* Encrypt file using AES.
 * 
 * */
shy_file shy_file_encrypt(const char* key, shy_file file) {
 // Encrypt each entry of the file.
  for (size_t i = 0; i < file.header.file_cnt; i++) {
     // Get a salt of 32 bit length.
    void* keyBuff = calloc(16, sizeof(char));
    size_t salt_len = 32;
    const void* salt = gcry_random_bytes(salt_len, GCRY_STRONG_RANDOM);
    gcry_kdf_derive(key, strlen(key), GCRY_KDF_PBKDF2, GCRY_MD_SHA256, salt, salt_len, 100000, sizeof(keyBuff), keyBuff);
    gcry_cipher_hd_t enkey;
    gcry_cipher_open(&enkey, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_GCM, GCRY_CIPHER_SECURE);
    // Create random initialization vector
    size_t iv_len = 32;
    const void* iv = gcry_random_bytes(iv_len, GCRY_STRONG_RANDOM);
    gcry_cipher_setiv(enkey, iv, iv_len);
    gcry_cipher_setkey(enkey, keyBuff, sizeof(keyBuff));
    shy_entry* curr_ent = &file.entries[i];

    const size_t aligned_size = ALIGN(curr_ent->size, 16);
    char* enc_ent = calloc(aligned_size, 1);
    gcry_cipher_encrypt(enkey, enc_ent, aligned_size, curr_ent->data, curr_ent->size);
    free(curr_ent->data);
    file.entries[i].data = (uint8_t*) enc_ent;
  }
  return file;
}

shy_file shy_file_decrypt(const char* key, shy_file file) {

}

shy_file shy_file_create(const char** file_paths, size_t file_cnt, const char* psw) {
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


  result.header.str_size = str_size;

  for (size_t i = 0; i < file_cnt; i++) {
    // Read the file as bytes into memory.
    FILE* f = fopen(file_paths[i], "r");
    fseek(f, 0, SEEK_END);
    size_t filesize = ftell(f);
    fseek(f, 0, SEEK_SET);
    // Create buffer for compressed data.
    result.entries[i].data = calloc(filesize, sizeof(uint8_t));
    result.entries[i].unc_size = filesize;
    fread(result.entries[i].data, filesize, 1, f);
    fclose(f);
  }
  
  result.paths = calloc(str_size, sizeof(char));
  char* cursor = result.paths;

  for (size_t i = 0; i < file_cnt; i++) {
    strcpy(cursor, file_paths[i]);
    cursor += strlen(file_paths[i])+1;
  }

  if (psw) {
    shy_file_encrypt(psw, result);
  }

  return result;
}

static void _mkdir(const char *dir) {
    char tmp[256];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp),"%s",dir);
    len = strlen(tmp);
    if (tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for (p = tmp + 1; *p; p++)
        if (*p == '/') {
            *p = 0;
            mkdir(tmp, S_IRWXU);
            *p = '/';
        }
    mkdir(tmp, S_IRWXU);
}

// Unpack original files from shyfile.
//
// - Call shy_file_read to get the structure of the file.
// - For each file in the shyfile extract its data and path.
// - Write the files to its *original* location.
//
// TODO:
// - Add argument to unpack at current location
// - Add argument to unpack in new folder with the shyfile name.
void shy_file_unpack(shy_file f, const char* shyFileName) {

  // Iterate through the entries of the shyfile.
  for (uint64_t i = 0; i < f.header.file_cnt; i++) {
    // set current entry.
    shy_entry curr_ent = f.entries[i];
    // Get path from list of paths or data. Do so by adding the current entries path offset.
    char destBuff[512]; // Stack buffer 512 Bytes for path handling.
    char* dest_path = f.paths + curr_ent.path_off;
    snprintf(destBuff, sizeof(destBuff), "output/%s/", shyFileName);
    _mkdir(destBuff);
    snprintf(destBuff + strlen(destBuff), sizeof(destBuff), "%s", dest_path);

    // Open new file to write into.
    FILE* file = fopen(destBuff, "w");
    if (!file) {
      printf("Error while opening target file! %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
   
    // Write data
    fwrite(curr_ent.data, 1, curr_ent.unc_size, file);
    fclose(file);
  }
}

