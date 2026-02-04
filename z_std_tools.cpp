#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>

#include <zstd.h>
#include <openssl/sha.h>

/* =========================
   CONSTANTS
   ========================= */

static const size_t BUFFER_SIZE = 1 << 16; // 64 KB

/* =========================
   UTILITIES
   ========================= */

bool ends_with(const std::string& s, const std::string& ext) {
    if (s.size() < ext.size()) return false;
    return s.compare(s.size() - ext.size(), ext.size(), ext) == 0;
}

bool is_office_file(const std::string& name) {
    return ends_with(name, ".docx") ||
           ends_with(name, ".xlsx") ||
           ends_with(name, ".pptx");
}

void sha256_file(const std::string& path, unsigned char hash[SHA256_DIGEST_LENGTH]) {
    std::ifstream f(path, std::ios::binary);
    SHA256_CTX ctx;
    SHA256_Init(&ctx);

    char buf[BUFFER_SIZE];
    while (f.good()) {
        f.read(buf, sizeof(buf));
        SHA256_Update(&ctx, buf, f.gcount());
    }

    SHA256_Final(hash, &ctx);
}

void print_hash(const unsigned char h[SHA256_DIGEST_LENGTH]) {
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        printf("%02x", h[i]);
}

/* =========================
   STREAMING COMPRESSION
   ========================= */

bool compress_file(const std::string& in, const std::string& out) {
    FILE* fin = fopen(in.c_str(), "rb");
    FILE* fout = fopen(out.c_str(), "wb");
    if (!fin || !fout) return false;

    ZSTD_CCtx* cctx = ZSTD_createCCtx();
    ZSTD_CCtx_setParameter(cctx, ZSTD_c_compressionLevel, 5);
    ZSTD_CCtx_setParameter(cctx, ZSTD_c_nbWorkers, 4); // multithread

    char inbuf[BUFFER_SIZE];
    char outbuf[BUFFER_SIZE];

    ZSTD_inBuffer input = { inbuf, 0, 0 };
    ZSTD_outBuffer output = { outbuf, sizeof(outbuf), 0 };

    while ((input.size = fread(inbuf, 1, sizeof(inbuf), fin)) > 0) {
        input.pos = 0;
        while (input.pos < input.size) {
            output.pos = 0;
            size_t ret = ZSTD_compressStream(cctx, &output, &input);
            if (ZSTD_isError(ret)) return false;
            fwrite(outbuf, 1, output.pos, fout);
        }
    }

    size_t ret;
    do {
        output.pos = 0;
        ret = ZSTD_endStream(cctx, &output);
        fwrite(outbuf, 1, output.pos, fout);
    } while (ret != 0);

    ZSTD_freeCCtx(cctx);
    fclose(fin);
    fclose(fout);
    return true;
}

/* =========================
   STREAMING DECOMPRESSION
   ========================= */

bool decompress_file(const std::string& in, const std::string& out) {
    FILE* fin = fopen(in.c_str(), "rb");
    FILE* fout = fopen(out.c_str(), "wb");
    if (!fin || !fout) return false;

    ZSTD_DCtx* dctx = ZSTD_createDCtx();

    char inbuf[BUFFER_SIZE];
    char outbuf[BUFFER_SIZE];

    ZSTD_inBuffer input = { inbuf, 0, 0 };
    ZSTD_outBuffer output = { outbuf, sizeof(outbuf), 0 };

    while ((input.size = fread(inbuf, 1, sizeof(inbuf), fin)) > 0) {
        input.pos = 0;
        while (input.pos < input.size) {
            output.pos = 0;
            size_t ret = ZSTD_decompressStream(dctx, &output, &input);
            if (ZSTD_isError(ret)) return false;
            fwrite(outbuf, 1, output.pos, fout);
        }
    }

    ZSTD_freeDCtx(dctx);
    fclose(fin);
    fclose(fout);
    return true;
}

/* =========================
   MAIN
   ========================= */

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file>\n";
        return 1;
    }

    std::string input = argv[1];
    bool decompress = ends_with(input, ".zst");

    if (!decompress && is_office_file(input)) {
        std::cout << "Notice: Office files are already ZIP-compressed.\n";
        std::cout << "Expected compression gain is minimal.\n\n";
    }

    std::string output = decompress ? "output.dec" : "output.zst";

    unsigned char hash_before[SHA256_DIGEST_LENGTH];
    unsigned char hash_after[SHA256_DIGEST_LENGTH];

    if (!decompress)
        sha256_file(input, hash_before);

    bool ok = decompress
        ? decompress_file(input, output)
        : compress_file(input, output);

    if (!ok) {
        std::cerr << "Operation failed\n";
        return 1;
    }

    if (decompress) {
        sha256_file(output, hash_after);
        std::cout << "\nSHA-256 (decompressed): ";
        print_hash(hash_after);
        std::cout << "\n";
    }

    std::cout << "\nOperation completed successfully\n";
    return 0;
}
