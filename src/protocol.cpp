/*
 * validate and decode requests, encode replies
 */

// Ethernet header | IP header | UDP header | my protocl header | key | value

#include "protocol.hpp"

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
                            std::span<const std::byte> output,
                            std::size_t &bytes_written);
