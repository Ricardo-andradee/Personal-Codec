import os
import pickle
import sys

# Adiciona a pasta src/ ao path para importar de Compressor/
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

from Compressor.ArithmeticDecoder import ArithmeticDecoder
from Compressor.SimpleFrequencyTable import SimpleFrequencyTable

BASE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "../.."))
RESULTS_DIR = os.path.join(BASE_DIR, "Results_Compression")

# Caminhos
compressed_path = os.path.join(RESULTS_DIR, "compressed.bxe.arith")
freqs_path = os.path.join(RESULTS_DIR, "freqs.pkl")
blocks_path = os.path.join(RESULTS_DIR, "arit_block_sizes.pkl")
output_path = os.path.join(RESULTS_DIR, "reconstructed_arit.bxe")


os.makedirs(RESULTS_DIR, exist_ok=True)


# Carregar dados
with open(compressed_path, "rb") as f:
    compressed_bytes = f.read()

with open(freqs_path, "rb") as f:
    freqs = pickle.load(f)

with open(blocks_path, "rb") as f:
    block_sizes = pickle.load(f)

# Reconstrói a tabela de frequências
freq_table = SimpleFrequencyTable(freqs)

# Converte os bytes de volta para bits
bit_stream = []
for byte in compressed_bytes:
    for i in reversed(range(8)):
        bit_stream.append((byte >> i) & 1)

bit_iter = iter(bit_stream)

# Decodifica os símbolos
decoder = ArithmeticDecoder(32, bit_iter)
total_events = sum(block_sizes)
decoded_bytes = bytearray()

for _ in range(total_events * 6):
    symbol = decoder.read(freq_table)
    decoded_bytes.append(symbol)

# Recria o .bxe com headers por bloco
with open(output_path, "wb") as f:
    offset = 0
    for num_events in block_sizes:
        f.write(num_events.to_bytes(2, "little"))
        block_len = num_events * 6
        f.write(decoded_bytes[offset:offset + block_len])
        offset += block_len

print(f"Decompressed {total_events} events into {output_path}")
