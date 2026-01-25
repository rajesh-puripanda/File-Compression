#include <stdio.h>
#include <zstd.h>


int main() {
    printf("ZSTD version: %s\n", ZSTD_versionString()); // ZSTD version: 1.5.7
    return 0;
}
