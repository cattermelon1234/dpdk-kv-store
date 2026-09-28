/*
 * validate and decode requests, encode replies
 */

// Ethernet header | IP header | UDP header | my protocl header | key | value

#include "protocol.hpp"

#include <arpa/inet.h>
#include <cstring>

namespace {

// The caller must check that output has room for the field at offset.
inline void write_u16(std::span<std::byte> output, std::size_t offset,
                      std::uint16_t value) {
  const std::uint16_t wire_value = htons(value);
  std::memcpy(output.data() + offset, &wire_value, sizeof(wire_value));
}

inline void write_u32(std::span<std::byte> output, std::size_t offset,
                      std::uint32_t value) {
  const std::uint32_t wire_value = htonl(value);
  std::memcpy(output.data() + offset, &wire_value, sizeof(wire_value));
}

} // namespace

// payload is the full UDP request [header][key][value]
DecodeError decode_request(std::span<const std::byte> payload,
                           RequestView &out) {
  if (payload.size() < request_wire::header_size) {
    return DecodeError::TOO_SHORT;
  }
  auto version =
      std::to_integer<std::uint8_t>(payload[request_wire::version_offset]);
  if (version != protocol_version) {
    return DecodeError::UNSUPPORTED_VERSION;
  }

  auto opcode =
      std::to_integer<std::uint8_t>(payload[request_wire::opcode_offset]);
  if (opcode != static_cast<std::uint8_t>(Opcode::GET) &&
      opcode != static_cast<std::uint8_t>(Opcode::PUT)) {
    return DecodeError::INVALID_OPCODE;
  }

  std::uint16_t key_length =
      (std::to_integer<std::uint16_t>(payload[request_wire::key_length_offset])
           << 8 |
       std::to_integer<std::uint16_t>(
           payload[request_wire::key_length_offset + 1]));

  std::uint16_t value_length =
      (std::to_integer<std::uint16_t>(
           payload[request_wire::value_length_offset])
           << 8 |
       std::to_integer<std::uint16_t>(
           payload[request_wire::value_length_offset + 1]));

  // Check the complete layout before constructing any borrowed views.
  const std::size_t expected_size = request_wire::header_size +
                                    static_cast<std::size_t>(key_length) +
                                    static_cast<std::size_t>(value_length);
  if (key_length == 0 || payload.size() != expected_size ||
      (opcode == static_cast<std::uint8_t>(Opcode::GET) && value_length != 0)) {
    return DecodeError::INVALID_LENGTH;
  }

  const auto id_offset = request_wire::request_id_offset;
  const std::uint32_t request_id =
      (std::to_integer<std::uint32_t>(payload[id_offset]) << 24) |
      (std::to_integer<std::uint32_t>(payload[id_offset + 1]) << 16) |
      (std::to_integer<std::uint32_t>(payload[id_offset + 2]) << 8) |
      std::to_integer<std::uint32_t>(payload[id_offset + 3]);

  out.opcode = static_cast<Opcode>(opcode);
  out.request_id = request_id;
  out.key = payload.subspan(request_wire::header_size, key_length);
  out.value =
      payload.subspan(request_wire::header_size + key_length, value_length);
  return DecodeError::NONE;
}

// output is the full UDP response [header][value]
EncodeError encode_response(const ResponseView &response,
                            std::span<std::byte> output,
                            std::size_t &bytes_written) {
  if (output.size() < response_wire::header_size) {
    return EncodeError::BUFFER_TOO_SMALL;
  }
  if (response.value.size() > MAX_VALUE_SIZE) {
    return EncodeError::INVALID_RESPONSE;
  }
  output[response_wire::version_offset] = std::byte{protocol_version};
  output[response_wire::status_offset] =
      static_cast<std::byte>(response.status);
  write_u16(output, response_wire::value_length_offset, response.value.size());
  write_u32(output, response_wire::response_id_offset, response.response_id);
  std::memcpy(output.data() + response_wire::header_size, response.value.data(),
              response.value.size());
  return EncodeError::NONE;
}
