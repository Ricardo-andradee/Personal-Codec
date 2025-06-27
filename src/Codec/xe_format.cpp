/**********************************************************************************************************************
 * MIT License                                                                                                        *
 *                                                                                                                    *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and                  *
 * associated documentation files (the “Software”), to deal in the Software without restriction,                      *
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,              *
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,              *
 * subject to the following conditions:                                                                               *
 *                                                                                                                    *
 * The above copyright notice and this permission notice shall be included in all copies or substantial               *
 * portions of the Software.                                                                                          *
 *                                                                                                                    *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,                                *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND               *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES               *
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN                *
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 **********************************************************************************************************************/

#include <sstream>
#include <iomanip>
#include "xe_format.h"
#include "jpeg_xe_canonical_raw_event_format_ctc_header.h"

namespace XEFormat {

FieldsDefinition FieldsDefinition::make_reference() {
    FieldsDefinition ref_def;
    ref_def.event_size = 48;
    ref_def.event_size_bytes = static_cast<std::uint8_t>(ref_def.event_size/8);
    ref_def.event_type_bit_size = 2;

    ref_def.absts.abstimestamp = 46;

    ref_def.cd_ev.relativetimestamp = 23;
    ref_def.cd_ev.polarity = 1;
    ref_def.cd_ev.x = 11;
    ref_def.cd_ev.y = 11;

    ref_def.tr_ev.relativetimestamp = 23;
    ref_def.tr_ev.polarity = 1;
    ref_def.tr_ev.triggerid = 8;
    ref_def.tr_ev.padding = 14;

    return ref_def;
}

namespace Decoder {

bool assert_jpegxe_canonical_header(std::istream &is) {
    const std::string ref_jpegxe_canonical_hex_header = REFERENCE_JPEG_XE_CANONICAL_RAW_EVENT_FORMAT_CTC_HEX_HEADER;
    const std::size_t ref_nfields = (ref_jpegxe_canonical_hex_header.size()-24)/8;
    
    std::stringstream ss;
    char c;
    auto parse_binary_char_in_hex_fct = [&is,&c,&ss]() -> bool {
        if(!is.read(&c, 1)) {
            return false;
        }
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
        return true;
    };

    for(int i=0; i<11; ++i) {
        if(!parse_binary_char_in_hex_fct()) {
            return false;
        }
    }
    if(!parse_binary_char_in_hex_fct()) {
        return false;
    }
    const std::size_t nfields = static_cast<std::uint8_t>(c);
    if(nfields != ref_nfields) {
        return false;
    }
    for(std::size_t i = 0; i < nfields; ++i) {
        for(std::size_t j = 0; j < 4; ++j) {
            if(!parse_binary_char_in_hex_fct()) {
                return false;
            }
        }
    }
    const std::string read_hex_header = ss.str();
    return ref_jpegxe_canonical_hex_header == read_hex_header;
}

bool read_next_encoded_event(std::istream &is, const FieldsDefinition &fdef, encoded_event_t &read_encoded_event) {
    assert(fdef.event_size <= 64);  // implementation only supports up to 64 bits events
    read_encoded_event = 0;
    for(int i=0; i<fdef.event_size_bytes; ++i) {
        std::uint8_t read_byte;
        if(!is.read((char*) &read_byte, 1)) {
            return false;
        }
        read_encoded_event = (read_encoded_event << 8) + read_byte;
    }
    return true;
}

EventType decode_event_type(encoded_event_t encoded_event, const FieldsDefinition &fdef) {
    const std::uint8_t ev_type = static_cast<std::uint8_t>(encoded_event & ((static_cast<std::uint64_t>(1)<<fdef.event_type_bit_size)-1));
    switch (ev_type) {
        case static_cast<std::uint8_t>(EventType::ABSTimeStamp):
            return EventType::ABSTimeStamp;
        case static_cast<std::uint8_t>(EventType::CD):
            return EventType::CD;
        case static_cast<std::uint8_t>(EventType::Trigger):
            return EventType::Trigger;
        default:
            throw std::runtime_error("Event type not supported!");
    }
}

timestamp_t decode_event_timestamp(encoded_event_t encoded_event, const FieldsDefinition &fdef) {
    switch(decode_event_type(encoded_event, fdef)) {
        case ABSTimeStamp:
            return (encoded_event >> fdef.event_type_bit_size) & ((static_cast<std::uint64_t>(1)<<fdef.absts.abstimestamp)-1);
        case CD:
            return (encoded_event >> fdef.event_type_bit_size) & ((static_cast<std::uint64_t>(1)<<fdef.cd_ev.relativetimestamp)-1);
        case Trigger:
            return (encoded_event >> fdef.event_type_bit_size) & ((static_cast<std::uint64_t>(1)<<fdef.tr_ev.relativetimestamp)-1);
        default:
            throw std::runtime_error("Event type not supported!");
    }
}

CDEvent decode_event_cd(encoded_event_t encoded_event, timestamp_t abs_time_base, const FieldsDefinition &fdef) {
    assert(decode_event_type(encoded_event, fdef)==EventType::CD);
    CDEvent ev;
    std::uint64_t p_encoded_event = encoded_event >> fdef.event_type_bit_size;
    ev.timestamp = abs_time_base + (p_encoded_event & ((static_cast<std::uint64_t>(1)<<fdef.cd_ev.relativetimestamp)-1));
    p_encoded_event = p_encoded_event >> fdef.cd_ev.relativetimestamp;
    ev.polarity = p_encoded_event & ((static_cast<std::uint64_t>(1)<<fdef.cd_ev.polarity)-1);
    p_encoded_event = p_encoded_event >> fdef.cd_ev.polarity;
    ev.x = p_encoded_event & ((static_cast<std::uint64_t>(1)<<fdef.cd_ev.x)-1);
    p_encoded_event = p_encoded_event >> fdef.cd_ev.x;
    ev.y = p_encoded_event & ((static_cast<std::uint64_t>(1)<<fdef.cd_ev.y)-1);
    return ev;
}

TriggerEvent decode_event_trigger(encoded_event_t encoded_event, timestamp_t abs_time_base, const FieldsDefinition &fdef) {
    assert(decode_event_type(encoded_event, fdef)==EventType::Trigger);
    TriggerEvent ev;
    std::uint64_t p_encoded_event = encoded_event >> fdef.event_type_bit_size;
    ev.timestamp = abs_time_base + (p_encoded_event & ((static_cast<std::uint64_t>(1)<<fdef.tr_ev.relativetimestamp)-1));
    p_encoded_event = p_encoded_event >> fdef.tr_ev.relativetimestamp;
    ev.polarity = p_encoded_event & ((static_cast<std::uint64_t>(1)<<fdef.tr_ev.polarity)-1);
    p_encoded_event = p_encoded_event >> fdef.tr_ev.polarity;
    ev.triggerid = p_encoded_event & ((static_cast<std::uint64_t>(1)<<fdef.tr_ev.triggerid)-1);
    ev.padding = 0;
    return ev;

}

} //namespace Decoder

namespace Encoder {

void initialize_jpegxe_canonical_file(timestamp_t abs_time_base, const FieldsDefinition &fdef, std::ostream &os) {
    const std::string ref_jpegxe_canonical_hex_header = REFERENCE_JPEG_XE_CANONICAL_RAW_EVENT_FORMAT_CTC_HEX_HEADER;
    assert(ref_jpegxe_canonical_hex_header.size()%2 == 0);  // hex string should have an even number of characters
    for(std::size_t i=0; i<ref_jpegxe_canonical_hex_header.size(); i+=2) {
        const std::string byte_str = ref_jpegxe_canonical_hex_header.substr(i, 2);
        const std::uint8_t byte = static_cast<std::uint8_t>(std::stoi(byte_str, nullptr, 16));
        os.write((char*) &byte, 1);
    }
    const encoded_event_t encoded_event = encode_event_absts(abs_time_base, fdef);
    write_encoded_event(os, fdef, encoded_event);
}

void write_encoded_event(std::ostream &os, const FieldsDefinition &fdef, const encoded_event_t &encoded_event) {
    for(int i=fdef.event_size_bytes-1; i>=0; --i) {
        const std::uint8_t byte = static_cast<std::uint8_t>((encoded_event >> (i*8)) & 0xFF);
        os.write((char*) &byte, 1);
    }
}

encoded_event_t encode_event_absts(timestamp_t abs_time_base, const FieldsDefinition &fdef) {
    assert(abs_time_base < (static_cast<std::uint64_t>(1)<<fdef.absts.abstimestamp));
    encoded_event_t encoded_event = abs_time_base;
    encoded_event = EventType::ABSTimeStamp + (encoded_event << fdef.event_type_bit_size);
    return encoded_event;
}

encoded_event_t encode_event_cd(const CDEvent &event_cd, timestamp_t abs_time_base, const FieldsDefinition &fdef) {
    assert(event_cd.timestamp >= abs_time_base);
    assert(event_cd.timestamp - abs_time_base < (static_cast<std::uint64_t>(1)<<fdef.cd_ev.relativetimestamp));
    assert(event_cd.polarity < (static_cast<std::uint64_t>(1)<<fdef.cd_ev.polarity));
    assert(event_cd.x < (static_cast<std::uint64_t>(1)<<fdef.cd_ev.x));
    assert(event_cd.y < (static_cast<std::uint64_t>(1)<<fdef.cd_ev.y));
    encoded_event_t encoded_event = event_cd.y;
    encoded_event = event_cd.x + (encoded_event << fdef.cd_ev.x);
    encoded_event = event_cd.polarity + (encoded_event << fdef.cd_ev.polarity);
    encoded_event = (event_cd.timestamp - abs_time_base) + (encoded_event << fdef.cd_ev.relativetimestamp);
    encoded_event = EventType::CD + (encoded_event << fdef.event_type_bit_size);
    return encoded_event;
}

encoded_event_t encode_event_trigger(const TriggerEvent &event_trigger, timestamp_t abs_time_base, const FieldsDefinition &fdef) {
    assert(event_trigger.timestamp >= abs_time_base);
    assert(event_trigger.timestamp - abs_time_base < (static_cast<std::uint64_t>(1)<<fdef.tr_ev.relativetimestamp));
    assert(event_trigger.polarity < (static_cast<std::uint64_t>(1)<<fdef.tr_ev.polarity));
    assert(event_trigger.triggerid < (static_cast<std::uint64_t>(1)<<fdef.tr_ev.triggerid));
    encoded_event_t encoded_event = event_trigger.triggerid;
    encoded_event = event_trigger.polarity + (encoded_event << fdef.tr_ev.polarity);
    encoded_event = (event_trigger.timestamp - abs_time_base) + (encoded_event << fdef.tr_ev.relativetimestamp);
    encoded_event = EventType::Trigger + (encoded_event << fdef.event_type_bit_size);
    return encoded_event;
}

bool update_absolute_time_base(timestamp_t &abs_time_base, timestamp_t next_timestamp, const FieldsDefinition &fdef) {
    assert(fdef.cd_ev.relativetimestamp == fdef.tr_ev.relativetimestamp);
    assert(abs_time_base <= next_timestamp);
    bool updated = false;
    const timestamp_t max_rel_ts = (static_cast<std::uint64_t>(1)<<fdef.cd_ev.relativetimestamp);
    while(abs_time_base + max_rel_ts <= next_timestamp) {
        abs_time_base += max_rel_ts;
        updated = true;
    }
    return updated;
}

void write_event_cd(const CDEvent &event_cd, timestamp_t &abs_time_base, const FieldsDefinition &fdef, std::ostream &os) {
    if(update_absolute_time_base(abs_time_base, event_cd.timestamp, fdef)) {
        const encoded_event_t encoded_event = encode_event_absts(abs_time_base, fdef);
        write_encoded_event(os, fdef, encoded_event);
    }
    const encoded_event_t encoded_event = encode_event_cd(event_cd, abs_time_base, fdef);
    write_encoded_event(os, fdef, encoded_event);
}

void write_event_trigger(const TriggerEvent &event_trigger, timestamp_t &abs_time_base, const FieldsDefinition &fdef, std::ostream &os) {
    if(update_absolute_time_base(abs_time_base, event_trigger.timestamp, fdef)) {
        const encoded_event_t encoded_event = encode_event_absts(abs_time_base, fdef);
        write_encoded_event(os, fdef, encoded_event);
    }
    const encoded_event_t encoded_event = encode_event_trigger(event_trigger, abs_time_base, fdef);
    write_encoded_event(os, fdef, encoded_event);
}

} //namespace Encoder

} //namespace XEFormat

