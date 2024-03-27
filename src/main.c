#include <shy_file.h>
#include <stdio.h>

int main(int argc, const char** argv) {
  if (argc < 2) {
    fprintf(stderr, "Invalid number of arguments!");
    return 1;
  }
  shy_file file = shy_file_create(argv+1, argc-1);
  shy_file_save(file, "test.shy");

  return 0;
}
