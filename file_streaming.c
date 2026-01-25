#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zstd.h>

#define CHUNK_SIZE (128 * 1024)

void compress_stream(const char* in_name, const char* out_name) {
    FILE* in = fopen(in_name, "rb");
    FILE* out = fopen(out_name, "wb");

    if (!in || !out) {
        perror("File open failed");
        exit(1);
    }

    ZSTD_CStream* cstream = ZSTD_createCStream();
    ZSTD_initCStream(cstream, 3);

    void* in_buf = malloc(CHUNK_SIZE);
    void* out_buf = malloc(ZSTD_CStreamOutSize());

    size_t read;
    while ((read = fread(in_buf, 1, CHUNK_SIZE, in)) > 0) {
        ZSTD_inBuffer input = { in_buf, read, 0 };

        while (input.pos < input.size) {
            ZSTD_outBuffer output = {
                out_buf,
                ZSTD_CStreamOutSize(),
                0
            };

            ZSTD_compressStream(cstream, &output, &input);
            fwrite(out_buf, 1, output.pos, out);
        }
    }

    /* flush */
    int finished = 0;
    while (!finished) {
        ZSTD_outBuffer output = {
            out_buf,
            ZSTD_CStreamOutSize(),
            0
        };

        size_t remaining = ZSTD_endStream(cstream, &output);
        fwrite(out_buf, 1, output.pos, out);
        finished = (remaining == 0);
    }

    fclose(in);
    fclose(out);
    free(in_buf);
    free(out_buf);
    ZSTD_freeCStream(cstream);
}

void decompress_stream(const char* in_name, const char* out_name) {
    FILE* in = fopen(in_name, "rb");
    FILE* out = fopen(out_name, "wb");

    if (!in || !out) {
        perror("File open failed");
        exit(1);
    }

    ZSTD_DStream* dstream = ZSTD_createDStream();
    ZSTD_initDStream(dstream);

    void* in_buf = malloc(ZSTD_DStreamInSize());
    void* out_buf = malloc(ZSTD_DStreamOutSize());

    size_t read;
    while ((read = fread(in_buf, 1, ZSTD_DStreamInSize(), in)) > 0) {
        ZSTD_inBuffer input = { in_buf, read, 0 };

        while (input.pos < input.size) {
            ZSTD_outBuffer output = {
                out_buf,
                ZSTD_DStreamOutSize(),
                0
            };

            ZSTD_decompressStream(dstream, &output, &input);
            fwrite(out_buf, 1, output.pos, out);
        }
    }

    fclose(in);
    fclose(out);
    free(in_buf);
    free(out_buf);
    ZSTD_freeDStream(dstream);
}

int main() {
    compress_stream("shakespeare_works_streaming.txt", "output.zst");
    decompress_stream("output.zst", "shakespeare_works_streaming.dec");

    printf("\nStreaming compression + decompression completed.\n");
    printf("Files:\n");
    printf("  shakespeare_works_streaming.txt  → output.zst → shakespeare_works_streaming.dec\n");

    return 0;
}
