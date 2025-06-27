#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include "../codec/xe_format.h"

using namespace XEFormat;

struct BlockHeader {
    uint16_t num_events;
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " INPUT_BXE_FILE OUTPUT_XE_FILE" << std::endl;
        return 1;
    }

    const char* bxe_filename = argv[1];
    const char* output_filename = argv[2];

    std::ifstream input_file(bxe_filename, std::ios::binary);
    if (!input_file) {
        std::cerr << "Cannot open input .bxe file: " << bxe_filename << std::endl;
        return 1;
    }

    std::ofstream output_file(output_filename, std::ios::binary);
    if (!output_file) {
        std::cerr << "Cannot open output .xe file: " << output_filename << std::endl;
        return 1;
    }

    const FieldsDefinition fields_def = FieldsDefinition::make_reference();
    timestamp_t abs_time_base = 0;  // valor neutro se desconhecido

    // Inicializa o header canônico JPEG_XE
    Encoder::initialize_jpegxe_canonical_file(abs_time_base, fields_def, output_file);

    // Escreve o evento ABS inicial com timestamp base
    encoded_event_t abs_event = Encoder::encode_event_absts(abs_time_base, fields_def);
    Encoder::write_encoded_event(output_file, fields_def, abs_event);

    // ---- Ler blocos e reescrever eventos ----
    while (input_file) {
        BlockHeader header;
        if (!input_file.read(reinterpret_cast<char*>(&header), sizeof(header))) break;

        for (int i = 0; i < header.num_events; ++i) {
            uint8_t buffer[6];
            if (!input_file.read(reinterpret_cast<char*>(buffer), 6)) {
                std::cerr << "Unexpected EOF while reading event." << std::endl;
                return 1;
            }

            encoded_event_t encoded = 0;
            for (int b = 0; b < 6; ++b) {
                encoded |= static_cast<encoded_event_t>(buffer[5 - b]) << (b * 8);
            }
            Encoder::write_encoded_event(output_file, fields_def, encoded);
        }
    }

    input_file.close();
    output_file.close();

    std::cout << "Reconstructed .xe file with generated header written to " << output_filename << std::endl;
    return 0;
}
