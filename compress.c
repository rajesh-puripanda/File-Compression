#include <stdio.h>
#include <stdlib.h>
#include <zstd.h>
#include <locale.h>

int main() {
       setlocale(LC_ALL, ""); // Enable UTF-8 support for pretty printing
    FILE* f = fopen("hamlet.txt", "rb");
    if (!f) {
        perror("Failed to open input file");
        return 1;
    }
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

    FILE* out = fopen("hamlet.zst", "wb");
    fwrite(compressed, 1, compressed_size, out);
    fclose(out);

    free(input);
    free(compressed);
double ratio = input_size ? 
    (double)compressed_size / (double)input_size : 0.0; // Avoid division by zero

    double savings = (1.0 - ratio) * 100.0;

    // printf("Compressed %zu bytes -> %zu bytes\n",
    //        input_size, compressed_size);
printf("\n");
printf("+---------------------------+--------------------------+\n");
printf("| ZSTD COMPRESSION REPORT   |                          |\n");
printf("+---------------------------+--------------------------+\n");
printf("| File                      | %-24s |\n", "output.zst");
printf("| Original Size             | %10zu bytes        |\n", input_size);
printf("| Compressed Size           | %10zu bytes        |\n", compressed_size);
printf("| Compression Ratio         | %10.3f              |\n", ratio);
printf("| Space Saved               | %9.2f %%             |\n", savings);
printf("+---------------------------+--------------------------+\n");

if (compressed_size >= input_size) {
    printf("Note: File is too small to benefit from compression.\n");
}
    return 0;
}
