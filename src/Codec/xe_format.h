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

#pragma once

#include <vector>
#include <istream>
#include <cstdint>
#include <cmath>
#include <cassert>
#include "common_structures.h"

namespace XEFormat {

enum EventFieldTypes : uint8_t {
    Reserved        = 0x00, // Not to be used.
    Polarity        = 0x01, // Specifies an event polarity. Considered “abstract” and typically just 1 bit. Generates an event.
    XCoord          = 0x02, // Specifies the originating X coordinate.
    YCoord          = 0x03, // Specifies the originating Y coordinate.
    ExtTriggerID    = 0x04, // Specifies the originating trigger address or ID. Generates an event.
    SensorID        = 0x05, // Specifies a sensor ID where the event was generated. This allows for systems with 2 or more sensors.
    Padding         = 0x06, // Allows for the definition of custom padding fields. Bits inside padding fields are set to zero.
    ABSTimeStampLSB = 0x10, // Sets the lower bits of the active timestamp base (other bits are untouched).
    ABSTimeStampMSB = 0x11, // Combines with the LSB of absolute timestamp base.
    RelTimeStamp    = 0x12, // Offsets to the absolute timestamp base.
};

enum EventType : uint8_t {
    CD              = 0x00,
    Trigger         = 0x01,
    ABSTimeStamp    = 0x02,
};

struct CDEvent {
    timestamp_t timestamp;
    unsigned int polarity;
    unsigned int x;
    unsigned int y;

    bool operator==(const CDEvent &other) const {
        return timestamp==other.timestamp && polarity==other.polarity && x==other.x && y==other.y;
    }
};

struct TriggerEvent {
    timestamp_t timestamp;
    unsigned int polarity;
    unsigned int triggerid;
    unsigned int padding;
    bool operator==(const TriggerEvent &other) const {
        return timestamp==other.timestamp && polarity==other.polarity && triggerid==other.triggerid;
    }
};

struct ABSTimeStampEvent {
    timestamp_t timestamp;
};

struct Sizes_CDEvent {
    unsigned int relativetimestamp;
    unsigned int polarity;
    unsigned int x;
    unsigned int y;
};

struct Sizes_TriggerEvent {
    unsigned int relativetimestamp;
    unsigned int polarity;
    unsigned int triggerid;
    unsigned int padding;
};

struct Sizes_ABSTimeStamp {
    unsigned int abstimestamp;
};

struct FieldsDefinition {
    std::uint8_t event_size;
    std::uint8_t event_size_bytes;
    std::uint8_t event_type_bit_size;
    
    Sizes_ABSTimeStamp absts;
    Sizes_CDEvent cd_ev;
    Sizes_TriggerEvent tr_ev;

    static FieldsDefinition make_reference();
};

using encoded_event_t = std::uint64_t;

namespace Decoder {

/// @brief  Asserts that the header of the input stream matches the reference JPEG_XE canonical raw event format header.
/// @param is input stream to load the header from.
/// @return true if the header matches the reference JPEG_XE canonical raw event format header, false otherwise.
bool assert_jpegxe_canonical_header(std::istream &is);

/// @brief  Reads the next encoded event from the input stream.
/// @param is input stream to read the encoded event from.
/// @param fdef fields definition.
/// @param read_encoded_event output parameter to store the read encoded event.
/// @return true if the encoded event was successfully read, false otherwise.
bool read_next_encoded_event(std::istream &is, const FieldsDefinition &fdef, encoded_event_t &read_encoded_event);

/// @brief  Decodes the event type from the input encoded event.
/// @param encoded_event encoded event.
/// @param fdef fields definition.
/// @return the decoded event type.
EventType decode_event_type(encoded_event_t encoded_event, const FieldsDefinition &fdef);

/// @brief  Decodes the event timestamp from the input encoded event.
/// @param encoded_event encoded event.
/// @param fdef fields definition.
/// @return the decoded event timestamp.
timestamp_t decode_event_timestamp(encoded_event_t encoded_event, const FieldsDefinition &fdef);

/// @brief  Decodes a CD event from the input encoded event.
/// @param encoded_event encoded event.
/// @param abs_time_base the absolute time-base used to shift the event relative timestamp.
/// @param fdef fields definition.
/// @return the decoded CD event.
CDEvent decode_event_cd(encoded_event_t encoded_event, timestamp_t abs_time_base, const FieldsDefinition &fdef);

/// @brief  Decodes a trigger event from the input encoded event.
/// @param encoded_event encoded event.
/// @param abs_time_base the absolute time-base used to shift the event relative timestamp.
/// @param fdef fields definition.
/// @return the decoded trigger event.
TriggerEvent decode_event_trigger(encoded_event_t encoded_event, timestamp_t abs_time_base, const FieldsDefinition &fdef);

} // namespace Decoder

namespace Encoder {

/// @brief  Initializes a reference JPEG_XE canonical event file by writing the CTC header and the initial absolute time base event to the output stream.
/// @param abs_time_base the absolute time-base to be encoded.
/// @param fdef fields definition.
/// @param os output stream to write the header to.
void initialize_jpegxe_canonical_file(timestamp_t abs_time_base, const FieldsDefinition &fdef, std::ostream &os);

/// @brief  Write the input encoded event to the output stream.
/// @param os output stream to write the encoded event to.
/// @param fdef fields definition.
/// @param encoded_event encoded event to be written.
void write_encoded_event(std::ostream &os, const FieldsDefinition &fdef, const encoded_event_t &encoded_event);

/// @brief  Encodes an absolute time base event.
/// @param abs_time_base the absolute time-base to be encoded.
/// @param fdef fields definition.
/// @return the encoded absolute time base event.
encoded_event_t encode_event_absts(timestamp_t abs_time_base, const FieldsDefinition &fdef);

/// @brief  Encodes a CD event.
/// @param event_cd the CD event to encode.
/// @param abs_time_base the absolute time-base used to shift the event relative timestamp. Must be lower or equal to the event timestamp.
/// @param fdef fields definition.
/// @return the encoded CD event.
encoded_event_t encode_event_cd(const CDEvent &event_cd, timestamp_t abs_time_base, const FieldsDefinition &fdef);

/// @brief  Encodes a trigger event.
/// @param event_trigger the trigger event to encode.
/// @param abs_time_base the absolute time-base used to shift the event relative timestamp. Must be lower or equal to the event timestamp.
/// @param fdef fields definition.
/// @return the encoded trigger event.
encoded_event_t encode_event_trigger(const TriggerEvent &event_trigger, timestamp_t abs_time_base, const FieldsDefinition &fdef);

/// @brief  Updates the absolute time base to enable encoding the specified next timestamp.
/// @param abs_time_base the absolute time-base used to shift the event relative timestamp.
/// @param next_timestamp the next timestamp to be encoded.
/// @param fdef fields definition.
/// @return true if the absolute time base was updated, false otherwise.
bool update_absolute_time_base(timestamp_t &abs_time_base, timestamp_t next_timestamp, const FieldsDefinition &fdef);

/// @brief  Encodes and writes a CD event to the output stream, updating the absolute timebase if necessary.
/// @param event_cd the CD event to encode.
/// @param abs_time_base the absolute time-base used to shift the event relative timestamp.
/// @param fdef fields definition.
/// @param os output stream to write the encoded event to.
void write_event_cd(const CDEvent &event_cd, timestamp_t &abs_time_base, const FieldsDefinition &fdef, std::ostream &os);

/// @brief  Encodes and writes a trigger event to the output stream, updating the absolute timebase if necessary.
/// @param event_trigger the trigger event to encode.
/// @param abs_time_base the absolute time-base used to shift the event relative timestamp.
/// @param fdef fields definition.
/// @param os output stream to write the encoded event to.
void write_event_trigger(const TriggerEvent &event_trigger, timestamp_t &abs_time_base, const FieldsDefinition &fdef, std::ostream &os);

} // namespace Encoder

} // namespace XEFormat

