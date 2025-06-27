#huffmann coding
from dahuffman import HuffmanCodec

#usado para salvar a tabela e os blocos
import pickle

#para os caminhos
import os


# Caminho relativo à raiz do projeto
BASE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "../.."))
DATA_DIR = os.path.join(BASE_DIR, "Block_Files")
OUTPUT_DIR = os.path.join(BASE_DIR, "Results_Compression")

# Entradas e saídas
bxe_path = os.path.join(DATA_DIR, "encoded_output.bxe")
compressed_path = os.path.join(OUTPUT_DIR, "compressed.bxe.huff")
table_path = os.path.join(OUTPUT_DIR, "huffman_table.pkl")
blocks_path = os.path.join(OUTPUT_DIR, "huff_block_sizes.pkl")

# Criar diretório de saída se necessário
os.makedirs(OUTPUT_DIR, exist_ok=True)
#armazena o número de eventos por bloco
block_sizes = []

#armazena todos os bytes de eventos combinados de todos os blocos
event_bytes = bytearray()

#Ler o .bxe e extrair os eventos em ordem
with open(bxe_path, "rb") as f:
    while True:
        #le 2 bytes do cabeçalho do bloco (16 bits)
        header = f.read(2)
        
        #se header vazio, acaba o loop
        if not header:
            break

        #little-endian porque maioria dos sistemas modernos no C++ escrevem em little-endian
        num_events = int.from_bytes(header, "little")

        #adicionar valor à lista block_sizes para recriar os blocos originais na descodificação (1024)
        block_sizes.append(num_events)

        #1024 * 6
        event_data = f.read(num_events * 6)

        #adiciona dados lidos à lista completa de eventos, no final do loop todos os eventos sao concatenados em sequencia
        event_bytes.extend(event_data)

#Criar a tabela de Huffman fixa a partir dos bytes
codec = HuffmanCodec.from_data(event_bytes)

#Codificar todos os eventos
compressed = codec.encode(event_bytes)

#Guardar ficheiros na pasta decoder
with open(compressed_path, "wb") as f:
    f.write(compressed)

with open(table_path, "wb") as f:
    pickle.dump(codec, f)

with open(blocks_path, "wb") as f:
    pickle.dump(block_sizes, f)

#Mostrar estatísticas
print(f"Original size: {len(event_bytes)} bytes")
print(f"Compressed size: {len(compressed)} bytes")
print(f"Compression ratio: {len(compressed) / len(event_bytes):.2%}")
