#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <string>
#include <cstdint>
#include "../Codec/xe_format.h"

using namespace XEFormat;

struct BlockHeader {
    uint16_t num_events;
};

int main(int argc, char* argv[]) {
    
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " INPUT_XE_FILE NUM_EVENTS_TO_READ (0 = ALL)" << std::endl;
        return 1;
    }

    std::ifstream input_file(argv[1], std::ios::binary);
    if (!input_file) {
        std::cerr << "Cannot open input file: " << argv[1] << std::endl;
        return 1;
    }

    int max_events = std::atoi(argv[2]);
    if (max_events < 0) {
        std::cerr << "Invalid number of events to read: " << argv[2] << std::endl;
        return 1;
    }

    //necessário para poder ler cada evento
    const FieldsDefinition fields_def = FieldsDefinition::make_reference();
    //array de eventos 
    std::vector<encoded_event_t> all_events;

    while (input_file) {
        //define evento 
        encoded_event_t encoded_event;
        
        //se não tiver mais eventos
        if (!Decoder::read_next_encoded_event(input_file, fields_def, encoded_event)) break;

        //vai colocando os eventos em all_events
        all_events.push_back(encoded_event);
        
        //se valor lido de eventos passar os dados pelo utilizador
        if (max_events > 0 && all_events.size() >= static_cast<size_t>(max_events))
            break;
    }

    //fecha ficheiro de input
    input_file.close();
    
    //numero final de eventos lidos
    std::cout << "Total events read: " << all_events.size() << std::endl;

    //para escrever para o ficheiro .bxe em modo binario
    std::ofstream output_file("../../Block_Files/encoded_output.bxe", std::ios::binary);
    if (!output_file) {
        std::cerr << "Cannot open output file." << std::endl;
        return 1;
    }

    //tamanho de cada bloco(x eventos)
    const size_t block_size = 1024;
    
    //para iterar sobre all_events
    size_t index = 0;

    //id do bloco
    size_t block_id = 0;

    while (index < all_events.size()) {
        //eventos que faltam processar
        size_t events_remaining = all_events.size() - index;

        //numero de eventos que vao entrar em cada bloco quando faltar menos de 1024 último bloco terá menos
        size_t events_in_block = std::min(events_remaining, block_size);

        //cabeçalho de bloco contendo o numero de eventos dentro do bloco(ver o número maximo de eventos por blocos)
        BlockHeader header{static_cast<uint16_t>(events_in_block)};

        //escreve o cabeçalho no ficheiro
        output_file.write(reinterpret_cast<const char*>(&header), sizeof(header));

        //escreve cada evento individual no ficheiro binário
        for (size_t i = 0; i < events_in_block; ++i) {
            Encoder::write_encoded_event(output_file, fields_def, all_events[index + i]);
        }

        //incrementar o numero de eventos previamente organizados
        index += events_in_block;

        //seguir para o proximo bloco
        ++block_id;
    }
    //fecha o ficheiro
    output_file.close();
    std::cout << "Wrote " << all_events.size() << " events into " << block_id << " blocks to the file encoded_output.bxe" << std::endl;

    return 0;
}
