#!/usr/bin/python3
# Author: Evan Shipman
# Title : ViHash
# Descr.: A custom hash visualization algorithm similar to the one used by
#         ssh-keygen. This version supports colored output, using ANSI escape
#         codes to handle colored output.
# Vers. : 1.0

import hashlib

SUPPORTED_ALGORITHMS = {'md5', 'sha1', 'sha256', 'sha512', 'sha3 256', 'sha3 512'}
ALPHABET = [' ', '.', '0', '+', '^', 'E', 'R', 'I']
COLORS = ['\033[48:5:0m', '\033[48:5:196m', '\033[48:5:202m', '\033[48:5:220m', '\033[48:5:10m', '\033[48:5:12m', '\033[48:5:4m', '\033[48:5:5m']

COLS = 16
ROWS = 8

# Takes a the name of the hash algorithm and a byte array of the data to be hash
# Returns the hash of the data
def hash(algo, data):
    if (algo not in hashlib.algorithms_available) or (algo not in SUPPORTED_ALGORITHMS):
        print("Error: Unsupported algorithm '" + algo + "'")
        return data
    h = hashlib.new(algo)
    h.update(data)
    return h.digest()

# Convert 4-bits to a direction to move in the board:
# 2 3 4
# 1 X 5
# 0 7 6
# Set increment to true if we should increment the next location
def nibble_to_coords(nibble, x, y):
    new_x = None
    new_y = None
    increment = None

    if ((nibble % 8) < 3):
        new_x = x - 1
    elif ((nibble % 8) == 3 or (nibble % 8) == 7):
        new_x = x
    else:
        new_x = x + 1

    if ((nibble % 8) == 0 or (nibble % 8) > 5):
        new_y = y - 1
    elif ((nibble % 8) == 1 or (nibble % 8) == 5):
        new_y = y
    else:
        new_y = y + 1

    if (nibble >> 3) == 0:
        increment = False
    else:
        increment = True
    
    return (new_x % ROWS), (new_y % COLS), increment

# Generate the 2D visualization according to different modes:
# Mode 0 - Just symbols
# Mode 1 - Colored symbols
# Mode 2 - Just colors
def generate(data, mode = 0):
    # Allocate the board and get the initial x and y
    board = [[0 for i in range(COLS)] for j in range(ROWS)]
    x = data[0] >> 4
    y = data[0] & 0x7

    # Should not allow data too short to be visualized
    if (len(data) < 2):
        return board

    for i in range(len(data) - 1):
        
        # Handle the upper half of the byte
        x, y, inc = nibble_to_coords(data[i] >> 4, x, y)
        if (inc):
            board[x][y] += 1
        else:
            board[x][y] -= 1

        # Handle the lower half of the byte
        x, y, inc = nibble_to_coords(data[i] & 0xF, x, y)
        if (inc):
            board[x][y] += 1
        else:
            board[x][y] -= 1
    
    # Print the visualization with a pretty box
    if (COLS == 16):
        print("+---ViHash 1.0---+")
    else:
        print("+", end=""); print("-" * COLS, end="+\n")

    for i in range(ROWS):
        print("|", end="")
        for j in range(COLS):
            if (mode > 0):
                print(COLORS[board[i][j] % len(ALPHABET)], end="")
            if (mode == 2):
                print(" ", end="")
            if (mode < 2):
                print(ALPHABET[board[i][j] % len(ALPHABET)], end="")
            if (mode > 0):
                print(COLORS[0], end="")
        print("|")
    
    if (COLS == 16):
        print("+----------------+")
    else:
        print("+", end=""); print("-" * COLS, end="+\n")

# Piece it all together, hash the data with the specified algorithm and print
# the visualization to the screen
def vihash(algo, data, mode):
    generate(hash(algo, data), mode)

# Print the bytes in a fingerprint-like fashion
def print_pretty(data):
    for i in range(len(data)):
        print(hex(data[i]).split('x')[-1], end="")
        if i < len(data) - 1:
            print(":", end="")
    print()

hash_algo = 'sha256'

while True:
    print()
    text = input("Input something: ")

    data = hash(hash_algo, text.encode())
    print("Fingerprint: ", end="")
    print_pretty(data)

    print("Mode 0:")
    vihash(hash_algo, text.encode(), 0)

    print("Mode 2:")
    vihash(hash_algo, text.encode(), 2)