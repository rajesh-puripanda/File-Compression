#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <cstring>
#include <zstd.h>

using namespace std;

int main() {
    const char* input_file = "hamlet.txt";

    /* =======================
       READ INPUT FILE
       ======================= */
    std::ifstream in(input_file, std::ios::binary | std::ios::ate);
    if (!in) {
        std::perror("Failed to open input file");
        return 1;
    }

    std::size_t input_size = static_cast<std::size_t>(in.tellg());
    in.seekg(0, std::ios::beg);

    std::vector<char> input(input_size);
    in.read(input.data(), input_size);
    in.close();

    /* =======================
       COMPRESS
       ======================= */
    std::size_t max_compressed = ZSTD_compressBound(input_size);
    std::vector<char> compressed(max_compressed);

    std::size_t compressed_size = ZSTD_compress(
        compressed.data(),
        max_compressed,
        input.data(),
        input_size,
        3
    );

    if (ZSTD_isError(compressed_size)) {
        std::cerr << "Compression error: "
                  << ZSTD_getErrorName(compressed_size) << "\n";
        return 1;
    }

    std::ofstream cf("output.zst", std::ios::binary);
    cf.write(compressed.data(), compressed_size);
    cf.close();

    double ratio = static_cast<double>(compressed_size) /
                   static_cast<double>(input_size);
    double savings = (1.0 - ratio) * 100.0;

    std::cout << "\n";
    std::cout << "+---------------------------+--------------------------+\n";
    std::cout << "| ZSTD COMPRESSION REPORT   |                          |\n";
    std::cout << "+---------------------------+--------------------------+\n";
    std::cout << "| File                      | output.zst               |\n";
    std::cout << "| Original Size             | "
              << std::setw(10) << input_size << " bytes        |\n";
    std::cout << "| Compressed Size           | "
              << std::setw(10) << compressed_size << " bytes        |\n";
    std::cout << "| Compression Ratio         | "
              << std::setw(10) << std::fixed << std::setprecision(3)
              << ratio << "              |\n";
    std::cout << "| Space Saved               | "
              << std::setw(9) << std::setprecision(2)
              << savings << " %             |\n";
    std::cout << "+---------------------------+--------------------------+\n";

    if (compressed_size >= input_size) {
        std::cout << "Note: File is too small to benefit from compression.\n";
    }

    /* =======================
       DECOMPRESS
       ======================= */
    unsigned long long decompressed_size =
        ZSTD_getFrameContentSize(compressed.data(), compressed_size);

    if (decompressed_size == ZSTD_CONTENTSIZE_ERROR ||
        decompressed_size == ZSTD_CONTENTSIZE_UNKNOWN) {
        std::cerr << "Invalid or unsupported zstd frame\n";
        return 1;
    }

    std::vector<char> decompressed(decompressed_size);

    std::size_t result = ZSTD_decompress(
        decompressed.data(),
        decompressed_size,
        compressed.data(),
        compressed_size
    );

    if (ZSTD_isError(result)) {
        std::cerr << "Decompression error: "
                  << ZSTD_getErrorName(result) << "\n";
        return 1;
    }

    std::ofstream df("output.dec", std::ios::binary);
    df.write(decompressed.data(), decompressed_size);
    df.close();

    /* =======================
       VERIFICATION
       ======================= */
    bool verified =
        (decompressed_size == input_size) &&
        (std::memcmp(input.data(), decompressed.data(), input_size) == 0);

    std::cout << "\n";
    std::cout << "+---------------------------+--------------------------+\n";
    std::cout << "| ZSTD DECOMPRESSION CHECK  |                          |\n";
    std::cout << "+---------------------------+--------------------------+\n";
    std::cout << "| Decompressed Size         | "
              << std::setw(10) << decompressed_size
              << " bytes        |\n";
    std::cout << "| Verification              | "
              << (verified ? "PASSED" : "FAILED")
              << "                     |\n";
    std::cout << "+---------------------------+--------------------------+\n";

    return verified ? 0 : 1;
}
