#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>
#define LONESHA256_IMPLEMENTATION
#include "lonesha256.h"

#define BLOCK_SIZE 32 // 256 bits = 32 bytes

void mostyn_reduction(const uint8_t input[BLOCK_SIZE], uint8_t output[BLOCK_SIZE / 2]) {
    int out_idx = 0;
    uint8_t out_byte = 0;
    
    for (int i = 0; i < 32; i++) {
        uint8_t byte = input[i];
        
        // XOR each pair of bits and generate a 4-bit result
        uint8_t result = (((byte & 0x80) >> 7) ^ ((byte & 0x40) >> 6)) << 3 |
                         (((byte & 0x20) >> 5) ^ ((byte & 0x10) >> 4)) << 2 |
                         (((byte & 0x08) >> 3) ^ ((byte & 0x04) >> 2)) << 1 |
                         (((byte & 0x02) >> 1) ^ (byte & 0x01));
        
        // Pack two 4-bit results into each byte of the output array
        if (i % 2 == 0) {
            out_byte = result << 4;  // Store as the high nibble
        } else {
            output[out_idx++] = out_byte | result;  // Combine with low nibble
        }
    }
}

void xor(uint8_t plaintext[BLOCK_SIZE], const uint8_t xor_key[BLOCK_SIZE]) {
    for (size_t i = 0; i < BLOCK_SIZE; i++) {
        plaintext[i] ^= xor_key[i];
    }
}

void permute(uint8_t plaintext[BLOCK_SIZE], const uint8_t permutation_key[BLOCK_SIZE / 2]) {
    uint8_t temp_buffer[BLOCK_SIZE];
    uint8_t indices[BLOCK_SIZE / 2];

    // Initialize indices with 0, 1, 2, ..., 15
    for (size_t i = 0; i < BLOCK_SIZE / 2; i++) {
        indices[i] = i;
    }

    // Perform a stable sort on the indices based on the permutation_key
    for (size_t i = 0; i < (BLOCK_SIZE / 2) - 1; i++) {
        for (size_t j = 0; j < (BLOCK_SIZE / 2) - 1 - i; j++) {
            if (permutation_key[indices[j]] > permutation_key[indices[j + 1]]) {
                uint8_t temp = indices[j];
                indices[j] = indices[j + 1];
                indices[j + 1] = temp;
            }
        }
    }

    // Permute the buffer based on the sorted indices
    for (size_t i = 0; i < BLOCK_SIZE / 2; i++) {
        memcpy(&temp_buffer[i * 2], &plaintext[indices[i] * 2], 2);
    }

    // Copy the permuted buffer back to the original buffer
    memcpy(plaintext, temp_buffer, BLOCK_SIZE);
}

void unpermute(uint8_t ciphertext[BLOCK_SIZE], const uint8_t permutation_key[BLOCK_SIZE / 2]) {
    uint8_t temp_buffer[BLOCK_SIZE];
    uint8_t indices[BLOCK_SIZE / 2];

    // Initialize indices with 0, 1, 2, ..., 15
    for (size_t i = 0; i < BLOCK_SIZE / 2; i++) {
        indices[i] = i;
    }

    // Perform a stable sort on the indices based on the permutation_key
    for (size_t i = 0; i < (BLOCK_SIZE / 2) - 1; i++) {
        for (size_t j = 0; j < (BLOCK_SIZE / 2) - 1 - i; j++) {
            if (permutation_key[indices[j]] > permutation_key[indices[j + 1]]) {
                uint8_t temp = indices[j];
                indices[j] = indices[j + 1];
                indices[j + 1] = temp;
            }
        }
    }

    // Reverse permutation using the sorted indices
    for (size_t i = 0; i < BLOCK_SIZE / 2; i++) {
        memcpy(&temp_buffer[indices[i] * 2], &ciphertext[i * 2], 2);
    }

    // Copy the unpermuted buffer back to the original buffer
    memcpy(ciphertext, temp_buffer, BLOCK_SIZE);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <key> <[e]ncrypt|[d]ecrypt> <in_file> <out_file>\n", argv[0]);
        return 1;
    }

    bool encrypt;
    if ((strcmp(argv[2], "e") == 0 || strcmp(argv[2], "encrypt") == 0)) {
        encrypt = true;
    } else if (argc > 2 && (strcmp(argv[2], "d") == 0 || strcmp(argv[2], "decrypt") == 0)) {
        encrypt = false;
    } else {
        fprintf(stderr, "Provide function in proper format: <[e]ncrypt|[d]ecrypt>");
        return 1;
    }

    // Read the integer value from the command line
    uint32_t initial_key = (uint32_t)atoi(argv[1]);
    // Read the file name from the command line
    char *in_filename = argv[3];
    char *out_filename = argv[4];

    FILE *in_file = fopen(in_filename, "rb");
    if (!in_file) {
        perror("Failed to open input file");
        return 1;
    }

    FILE *out_file = fopen(out_filename, "wb");
    if (!out_file) {
        perror("Failed to open output file");
        fclose(in_file);
        return 1;
    }

    int round_count = 0;
    uint8_t xor_key[BLOCK_SIZE];
    uint8_t permutation_key[BLOCK_SIZE / 2];

    unsigned char buffer[BLOCK_SIZE];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, BLOCK_SIZE, in_file)) > 0) {
        // Pad the remaining buffer with zeros if needed
        if (bytesRead < BLOCK_SIZE) {
            memset(buffer + bytesRead, 0, BLOCK_SIZE - bytesRead);
        }

        if (round_count == 0) {
            lonesha256(xor_key, (unsigned char *)&initial_key, strlen((unsigned char *)&initial_key));
        } else {
            lonesha256(xor_key, xor_key, BLOCK_SIZE);
        }
        mostyn_reduction(xor_key, permutation_key);

        if (encrypt) {
            permute(buffer, permutation_key);
            xor(buffer, xor_key);
        } else {
            xor(buffer, xor_key);
            unpermute(buffer, permutation_key);
        }
        
        // Write the 32-byte block to the output file
        if (fwrite(buffer, 1, BLOCK_SIZE, out_file) != BLOCK_SIZE) {
            perror("Failed to write to output file");
            fclose(in_file);
            fclose(out_file);
            return 1;
        }
        
        round_count++;
    }

    if (ferror(in_file)) {
        perror("Error reading input file");
    }

    fclose(in_file);
    fclose(out_file);
    printf("Fin.\n");
    return 0;
}