#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zstd.h>

int main() {
    FILE* f = fopen("hamlet.txt", "rb");
    if (!f) {
        perror("Failed to open input.txt");
        return 1;
    }

    fseek(f, 0, SEEK_END);
    size_t input_size = ftell(f);
    rewind(f);

    void* input = malloc(input_size);
    fread(input, 1, input_size, f);
    fclose(f);

    /* =======================
       COMPRESS
       ======================= */
    size_t max_compressed = ZSTD_compressBound(input_size);
    void* compressed = malloc(max_compressed);

    size_t compressed_size = ZSTD_compress(
        compressed,
        max_compressed,
        input,
        input_size,
        3
    );

    if (ZSTD_isError(compressed_size)) {
        printf("Compression error: %s\n",
               ZSTD_getErrorName(compressed_size));
        return 1;
    }

    FILE* cf = fopen("output.zst", "wb");
    fwrite(compressed, 1, compressed_size, cf);
    fclose(cf);

    double ratio = (double)compressed_size / (double)input_size;
    double savings = (1.0 - ratio) * 100.0;

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

    /* 
       DECOMPRESS
        */
    unsigned long long decompressed_size =
        ZSTD_getFrameContentSize(compressed, compressed_size);

    if (decompressed_size == ZSTD_CONTENTSIZE_ERROR ||
        decompressed_size == ZSTD_CONTENTSIZE_UNKNOWN) {
        printf("Invalid or unsupported zstd frame\n");
        return 1;
    }

    void* decompressed = malloc(decompressed_size);

    size_t result = ZSTD_decompress(
        decompressed,
        decompressed_size,
        compressed,
        compressed_size
    );

    if (ZSTD_isError(result)) {
        printf("Decompression error: %s\n",
               ZSTD_getErrorName(result));
        return 1;
    }

    FILE* df = fopen("output.dec", "wb");
    fwrite(decompressed, 1, decompressed_size, df);
    fclose(df);

    /* 
       VERIFICATION
        */
    int verified = (decompressed_size == input_size) &&
                   (memcmp(input, decompressed, input_size) == 0);

    printf("\n");
    printf("+---------------------------+--------------------------+\n");
    printf("| ZSTD DECOMPRESSION CHECK  |                          |\n");
    printf("+---------------------------+--------------------------+\n");
    printf("| Decompressed Size         | %10llu bytes        |\n",
           decompressed_size);
    printf("| Verification              | %-24s |\n",
           verified ? "PASSED" : "FAILED");
    printf("+---------------------------+--------------------------+\n");

    /* 
       CLEANUP
        */
    free(input);
    free(compressed);
    free(decompressed);

    return verified ? 0 : 1;
}
