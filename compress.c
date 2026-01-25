#include <stdio.h>
#include <stdlib.h>
#include <zstd.h>

int main() {
    FILE* f = fopen("input.txt", "rb");

    fseek(f, 0, SEEK_END);
    size_t input_size = ftell(f);
    rewind(f);

    void* input = malloc(input_size);
    fread(input, 1, input_size, f);
    fclose(f);

    size_t max_size = ZSTD_compressBound(input_size);
    void* compressed = malloc(max_size);

    size_t compressed_size = ZSTD_compress(
        compressed,
        max_size,
        input,
        input_size,
        3   // compression level
    );

    if (ZSTD_isError(compressed_size)) {
        printf("Compression error: %s\n",
               ZSTD_getErrorName(compressed_size));
        return 1;
    }

    FILE* out = fopen("output.zst", "wb");
    fwrite(compressed, 1, compressed_size, out);
    fclose(out);

    free(input);
    free(compressed);

    printf("Compressed %zu bytes -> %zu bytes\n",
           input_size, compressed_size);

    return 0;
}
