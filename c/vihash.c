/*
 * Author: Evan Shipman
 * Title : ViHash
 * Descr.: A custom hash visualization algorithmrithm similar to the one used by
 *         ssh-keygen
 * Vers. : 1.0
 */

#include <stdlib.h>
#include <string.h>

#include <stdio.h>

#include <openssl/sha.h>

#define uint8_t unsigned char
#define uint32_t unsigned int
#define uint64_t unsigned long

#define ROWS 8
#define COLS 16

#define BUFF_SIZE 1026

/* The supported hash algorithms */
enum algorithm {sha1, sha256, sha512};

/* The alphabet of symbols to print into the art */
const uint8_t *ALPHABET = " .0+^ERI";
const uint8_t ALPHABET_LENGTH = 8;

/* The version to print at the top of the art's box */
const uint8_t *VERSION = "ViHash 1.0";

/* Hash functions from <openssl/sha.h> */
extern uint8_t* SHA1(const uint8_t*, size_t, uint8_t*);
extern uint8_t* SHA256(const uint8_t*, size_t, uint8_t*);
extern uint8_t* SHA512(const uint8_t*, size_t, uint8_t*);

/*
 * Hashes the input data and returns the malloc'd result.
 * Returns a pointer to the hash of the input data on success,
 * returns NULL on failure.
 */
uint8_t* hash(
    enum algorithm hash_algo,
    uint8_t *data, uint64_t length, 
    uint8_t **hash, uint64_t *hash_len)
{
    /* Declare a function pointer for the hash algorithm */
    uint8_t* (*hash_function)(const uint8_t*, size_t, uint8_t*);

    /* Validate user inputs */
    if (data == NULL)
        return NULL;
    else if (length == 0)
        return NULL;
    else if (hash == NULL || *hash != NULL)
        return NULL;
    else if (hash_len == NULL)
        return NULL;

    /* Set the length of the hash output and the algorithm to use */
    switch (hash_algo) {
        case sha1:
            *hash_len = SHA_DIGEST_LENGTH;
            hash_function = SHA1;
            break;
        case sha256:
            *hash_len = SHA256_DIGEST_LENGTH;
            hash_function = SHA256;
            break;
        case sha512:
            *hash_len = SHA512_DIGEST_LENGTH;
            hash_function = SHA512;
            break;
        default:
            *hash_len = SHA256_DIGEST_LENGTH;
            hash_function = SHA256;
            break;
    }
    
    /* Attempt to allocate space for the hash, then hash the data */
    if (((*hash) = (uint8_t*) malloc((*hash_len) * sizeof(uint8_t))) != NULL)
        (*hash_function)(data, length, *hash);
    else
        return NULL;
    
    /* Return a pointer to the hash */
    return *hash;
}

/*
 * Converts 4-bits to a coordinate on the grid.
 * Returns 0 for success, 1 for failure.
 */
int nibble_to_coords(
    uint8_t nibble,
    uint32_t *x,
    uint32_t *y,
    uint8_t *increment)
{
    int retval;

    /* Validate inputs */
    if (x == NULL)
        return 1;
    else if (y == NULL)
        return 1;
    else if (increment == NULL)
        return 1;

    /* If the most significant bit is 1, increment the next location */
    if (nibble & 0x8)
        *increment = 1;
    else
        *increment = 0;
    

    /* Mask off the MSB */
    nibble &= 0x7;

    /* Determine the new X coordinate */
    if (nibble < 3)
        *x = (*x - 1) % ROWS;
    else if (nibble > 3 && nibble < 7)
        *x = (*x + 1) % ROWS;

    /* Determine the new Y coordinate */
    if (nibble == 0 || nibble > 5)
        *y = (*y - 1) % COLS;
    else if (nibble > 1 && nibble < 5)
        *y = (*y + 1) % COLS;
    
}

/*
 * Converts the input data to a 2D char array of the ViHash art.
 * Returns a pointer to the 2D array on success,
 * returns NULL on failure.
 */
uint8_t** travel(
    uint8_t *input,
    uint64_t input_len,
    uint8_t ***output,
    uint64_t *output_len)
{
    int i, j,
        x, y;
    uint8_t increment;

    /* Validate inputs */
    if (input == NULL)
        return NULL;
    else if (input_len < 2)
        return NULL;
    else if (output != NULL && *output != NULL && **output != NULL)
        return NULL;
    
    /* Allocate space for the ViHash art */
    if (((*output) = (uint8_t**) malloc(ROWS * sizeof(uint8_t*))) == NULL)
        return NULL;
    
    /* Allocate each row. If any mallocs fail, free everything and return NULL */
    for (i = 0; i < ROWS; i++) {
        (*output)[i] = (uint8_t*) malloc(COLS * sizeof(uint8_t));
        if ((*output)[i] == NULL) {
            for (j = 0; j < i; j++) {
                free((*output)[i]);
                free(*output);
            }
            return NULL;
        } else {
            memset((*output)[i], 0, COLS * sizeof(uint8_t));
        }
    }

    /* Compute teh output length */
    *output_len = COLS * ROWS * sizeof(uint8_t);

    /* Set the starting positions */
    x = (input[0] >> 4) % COLS;
    y = (input[0] & 0xF) % ROWS;

    /*
     * Loop through the data, traversing the matrix, and either incrementing or
     * decrementing.
     */
    for (i = 1; i < input_len; i++) {
        /* Handle the upper half of the byte */
        nibble_to_coords(input[i] >> 4, &x, &y, &increment);
        (*output)[x % ROWS][y % COLS] += (increment) ? 1 : -1;

        /* Handle the lower half of the byte */
        nibble_to_coords(input[i] & 0xF, &x, &y, &increment);
        (*output)[x % ROWS][y % COLS] += (increment) ? 1 : -1;
    }

    /* Replace all the numbers in the matrix with the alphabet representation */
    for (i = 0; i < ROWS; i++)
        for (j = 0; j < COLS; j++)
            (*output)[i][j] = ALPHABET[(*output)[i][j] % ALPHABET_LENGTH];

    return *output;
}

/*
 * Prints out the ViHash art to the file. If any part of the map is NULL,
 * nothing is printed to prevent output mangling.
 */
void fprint_map(FILE *stream, uint8_t ***map)
{
    int i, j;

    /* Input Validation */
    if (stream == NULL)
        return;
    else if (map == NULL)
        return;
    for (i = 0; i < ROWS; i++)
        if ((*map)[i] == NULL)
            return;
    
    if (COLS >= strlen(VERSION)) {
        printf("+");
        for (i = 0; i < (COLS - strlen(VERSION)) / 2; i++)
            printf("-");
        printf("%s", VERSION);
        for (i = (COLS - strlen(VERSION)) / 2; i < COLS - strlen(VERSION); i++)
            printf("-");
        printf("+\n");
    }
    for (i = 0; i < ROWS; i++) {
        printf("|");
        for (j = 0; j < COLS; j++)
            fprintf(stream, "%c", (*map)[i][j]);
        printf("|\n");
    }
    printf("+");
    for (i = 0; i < COLS; i++)
        printf("-");
    printf("+\n");
}

/*
 * Wrapper for fprint_map to print to standard output.
 */
void print_map(uint8_t ***map)
{
    fprint_map(stdout, map);
}

int main(int argc, char **argv)
{
    /* Declare the local variables */
    uint8_t string[BUFF_SIZE];
    enum algorithm hash_algorithm;
    uint8_t *hash_val;
    uint64_t hash_len;

    uint64_t map_len;
    uint8_t **map;
    
    int i;

    /* Initialize the arguments for the hash function */
    hash_algorithm = sha1;
    hash_val = NULL;
    hash_len = 0;

    /* Initialize the ViHash art variables */
    map_len = 0;
    map = NULL;

    /* Clear out the string's memory */
    memset(string, 0, BUFF_SIZE);

    /* Get the user's input and remove the ending newline */
    printf("Please enter a string (up to %d characters): ", BUFF_SIZE - 2);
    fgets(string, BUFF_SIZE, stdin);
    string[strlen(string) - 1] = '\0';
    
    /* Hash the input string and store it in hash_val */
    hash(hash_algorithm, string, strlen(string), &hash_val, &hash_len);

    /* If hash() returned NULL, something wrong happened */
    if (hash_val == NULL) {
        fprintf(stderr, "Error: Could not compute hash of input data\n");
        return 1;
    }

    /* Print the hash of the string */
    printf("Hash('%s') = ", string);
    for (i = 0; i < hash_len; i++)
        printf("%x", hash_val[i]);
    printf("\n");

    /* Compute the ViHash art */
    map = travel(hash_val, hash_len, &map, &map_len);
    
    /* If the result is NULL, something wrong happened */
    if (map == NULL) {
        fprintf(stderr, "Error: Could not compute ViHash art\n");
        return 1;
    }

    /* Print the art to stdout */
    print_map(&map);

    /* Clean up memory */
    free(hash_val);

    for (i = 0; i < ROWS; i++)
        free(map[i]);
    free(map);

    return 0;
}