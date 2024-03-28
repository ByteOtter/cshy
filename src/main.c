#include <shy_file.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, const char** argv) {
  size_t fileCap = 8;
  size_t fileCount = 0;
  const char** inputFiles = calloc(fileCap, sizeof(char*));
  const char* outputFile = NULL;

  if (argc < 3) {
    printf("Usage: cshy <operation> <filepath1> [filepath2] [...] -o <outputFileName>\n");
    printf("Operations:\n");
    printf("\tpack - Packs the specified files into a .shy file.\n");
    printf("\tunpack - Unpacks a .shy-file.");
    printf("\nExample:\n");
    printf("\tcshy pack file1.txt file2.txt\n");
    printf("\tcshy unpack archvie.shy\n");
    return 1;
  }

  for (int i = 2; i < argc; i++) {
    if (!strcmp(argv[i], "-o")) {
      outputFile = argv[++i];
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
    shy_file_save(file, nameBuff);

  } else if (!strcmp(argv[1], "unpack")) {
    // TODO catch if more than one input files specified!
    printf("Unpacking. Reading shyfile.\n");
    shy_file file = shy_file_read(inputFiles[0]);
    printf("Writing Files.\n");
    shy_file_unpack(file, outputFile);
  }

  return 0;
}
