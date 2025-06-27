#huffmann coding
import pickle

import os

#usado para salvar a tabela e os blocos
from dahuffman import HuffmanCodec

# Diretórios base
BASE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "../.."))
RESULTS_DIR = os.path.join(BASE_DIR, "Results_Compression")

# Caminhos dos ficheiros
compressed_path = os.path.join(RESULTS_DIR, "compressed.bxe.huff")
table_path = os.path.join(RESULTS_DIR, "huffman_table.pkl")
block_sizes_path = os.path.join(RESULTS_DIR, "huff_block_sizes.pkl")
output_bxe_path = os.path.join(RESULTS_DIR, "reconstructed_huff.bxe")

os.makedirs(RESULTS_DIR, exist_ok=True)

#carregar a tabela de Huffman
with open(table_path, "rb") as f:
    codec: HuffmanCodec = pickle.load(f)

#carregar os tamanhos dos blocos
with open(block_sizes_path, "rb") as f:
    block_sizes = pickle.load(f)

#ler o stream de dados comprimido
with open(compressed_path, "rb") as f:
    compressed_data = f.read()

#decodificar os dados (como lista de bytes)
decoded_bytes = codec.decode(compressed_data)

#converter para tipo bytes para poder escrever diretamente
decoded_bytes = bytes(decoded_bytes)

#recriar o .bxe com headers por bloco
with open(output_bxe_path, "wb") as f:
    offset = 0
    for num_events in block_sizes:
        #calcula quantos bytes de eventos virão a seguir
        block_len = num_events * 6

        #escreve o cabeçalho do bloco com 2 bytes
        f.write(num_events.to_bytes(2, "little"))

        #escreve os block_len bytes de eventos
        f.write(decoded_bytes[offset : offset + block_len])

        #avança o offset no buffer de decoded_bytes para o próximo bloco 
        offset += block_len

print(f"Reconstructed {len(block_sizes)} blocks into {output_bxe_path}")
