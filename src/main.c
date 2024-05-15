#include <shy_file.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, const char** argv) {
  size_t fileCap = 8;
  size_t fileCount = 0;
  const char** inputFiles = calloc(fileCap, sizeof(char*));
  const char* outputFile = NULL;
  int clvl = 14;

  if (argc < 3) {
    printf("Usage: cshy <operation> <filepath1> [filepath2] [...] -o <outputFileName> -c <compressionLevel>\n");
    printf("Operations:\n");
    printf("\tpack - Packs the specified files into a .shy file.\n");
    printf("\tunpack - Unpacks a .shy-file.");
    printf("\nOptions:\n");
    printf("\t-o - Specify the name of your .shy-archive.\n");
    printf("\t-c - Specify compression level. (Standard: %i)\n\tPossible compression options: 1 - 20.\n\tZstd \'--ultra\' compression (level > 20) not supported.\n", clvl);
    printf("\nExample:\n");
    printf("\tcshy pack file1.txt file2.txt\n");
    printf("\tcshy unpack archive.shy\n");
    return 1;
  }

  for (int i = 2; i < argc; i++) {
    if (!strcmp(argv[i], "-o")) {
      outputFile = argv[++i];
    } else if (!strcmp(argv[i], "-c")) {
      int arg = atoi(argv[++i]);
      if (arg > 20 | arg < 1) {
        printf("Invalid compression level! Valid levels range from 1 to 20.\n");
        exit(1);
      }
      clvl = arg;
    } else {
      // This is essentially array.push()
      inputFiles[fileCount] = argv[i];
      fileCount += 1;
      if (fileCount > fileCap) {
        fileCap *= 2;
        inputFiles = reallocarray(inputFiles, fileCap, sizeof(char*));
      }
    }
  }
  
  if (!outputFile) {
    printf("No output name specified. Choosing default.\n");
    outputFile = "shy_out";
  } 

  if(!strcmp(argv[1], "pack")) {
    printf("Packing files...\n");
    shy_file file = shy_file_create(inputFiles, fileCount);
    char nameBuff[512];

    snprintf(nameBuff, sizeof(nameBuff), "%s.shy", outputFile);
    shy_file_save(file, nameBuff, clvl);

  } else if (!strcmp(argv[1], "unpack")) {
    if (argc > 3) {
      printf("Error: Only one input file allowed while unpacking!\n");
      return 1;
    }
    printf("Unpacking. Reading shyfile.\n");
    shy_file file = shy_file_read(inputFiles[0]);
    printf("Writing Files.\n");
    shy_file_unpack(file, outputFile);
  }
  
  printf("Done.\n");
  return 0;
}
