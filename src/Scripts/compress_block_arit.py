import os
import pickle
import sys

# Adiciona a pasta src/ ao path para importar de Compressor/
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

from Compressor.ArithmeticEncoder import ArithmeticEncoder
from Compressor.SimpleFrequencyTable import SimpleFrequencyTable

# Caminho relativo à raiz do projeto
BASE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "../.."))
DATA_DIR = os.path.join(BASE_DIR, "Block_Files")
OUTPUT_DIR = os.path.join(BASE_DIR, "Results_Compression")

# Entradas e saídas
bxe_path = os.path.join(DATA_DIR, "encoded_output.bxe")
compressed_path = os.path.join(OUTPUT_DIR, "compressed.bxe.arith")
freqs_path = os.path.join(OUTPUT_DIR, "freqs.pkl")
blocks_path = os.path.join(OUTPUT_DIR, "arit_block_sizes.pkl")

os.makedirs(OUTPUT_DIR, exist_ok=True)

# Leitura dos blocos e eventos
block_sizes = []
event_bytes = bytearray()

with open(bxe_path, "rb") as f:
    while True:
        header = f.read(2)
        if not header:
            break
        num_events = int.from_bytes(header, "little")
        block_sizes.append(num_events)
        event_data = f.read(num_events * 6)
        event_bytes.extend(event_data)

# Conta frequência de cada byte (0–255)
freqs = [max(1, event_bytes.count(b)) for b in range(256)]  # garante mínimo 1

# Cria tabela de frequências
freq_table = SimpleFrequencyTable(freqs)

# Codifica os dados
compressed_bits = []

bitout = ArithmeticEncoder(32, compressed_bits.append)
for b in event_bytes:
    bitout.write(freq_table, b)
bitout.finish()

# Converte bits para bytes
compressed_bytes = bytearray()
byte = 0
count = 0
for bit in compressed_bits:
    byte = (byte << 1) | bit
    count += 1
    if count == 8:
        compressed_bytes.append(byte)
        byte = 0
        count = 0
if count > 0:
    compressed_bytes.append(byte << (8 - count))  # pad final

# Salva os arquivos
with open(compressed_path, "wb") as f:
    f.write(compressed_bytes)

with open(blocks_path, "wb") as f:
    pickle.dump(block_sizes, f)

with open(freqs_path, "wb") as f:
    pickle.dump(freqs, f)

# Estatísticas
print(f"Original size: {len(event_bytes)} bytes")
print(f"Compressed size: {len(compressed_bytes)} bytes")
print(f"Compression ratio: {len(compressed_bytes) / len(event_bytes):.2%}")
