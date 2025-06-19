import matplotlib.pyplot as plt
import math

def parse_blocks(filename):
    with open(filename, 'r') as file:
        lines = file.readlines()

    blocks = []
    current_block = []

    for line in lines:
        line = line.strip()
        if line.startswith('>'):
            if current_block:
                blocks.append(current_block)
                current_block = []
        elif line.startswith('7E8'):
            current_block.append(line[3:])  
    
    if current_block:
        blocks.append(current_block)

    return blocks

def blocks_to_bytes(blocks):
    byte_blocks = []

    for block in blocks:
        block_bytes = []
        for line in block:
            bytes_line = [int(line[i:i+2], 16) for i in range(0, len(line), 2)]
            block_bytes.extend(bytes_line)
        byte_blocks.append(block_bytes)

    return byte_blocks



import matplotlib.pyplot as plt
import math

import matplotlib.pyplot as plt

def plot_byte_evolution(byte_blocks):
    selected_indices = [35,36,37,38,39] #is byte 36 of this grouping!!! for sure, the last non zero couple of nibble

    plt.figure(figsize=(8, 8), dpi=100)  # 800x800 pixels

    for byte_index in selected_indices:
        byte_values = [
            block[byte_index] if byte_index < len(block) else None
            for block in byte_blocks
        ]
        byte_values_40 = [x - 40 for x in byte_values]
        plt.plot(byte_values_40, label=f'Byte {byte_index}', marker='o')

    plt.title("Byte 14")
    plt.xlabel("Block Index")
    plt.ylabel("Byte Value")
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    plt.show()




# === MAIN SCRIPT ===
filename = "C:\\Users\\AngeloP\\Downloads\\16guigno_clean_partial.txt"  
blocks = parse_blocks(filename)
byte_blocks = blocks_to_bytes(blocks)
plot_byte_evolution(byte_blocks)