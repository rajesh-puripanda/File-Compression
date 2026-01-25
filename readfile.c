#include <stdio.h>
#include <stdlib.h>

unsigned char* read_file(const char* filename, size_t* size) {
    FILE* f = fopen(filename, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    *size = ftell(f);
    rewind(f);

    unsigned char* buffer = malloc(*size);
    fread(buffer, 1, *size, f);

    fclose(f);
    return buffer;
}
