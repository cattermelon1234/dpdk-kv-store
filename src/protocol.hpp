/* 
 * Request Layout
 * 0: version (1)
 * 1: opcode (1 = GET, 2 = PUT)
 * 2-3: key length (2 bytes)
 * 4-5: vallue length (2 bytes)
 * 6-9: request id (4 bytes, int)
 * 10-10+key_length: key bytes (key_length)
 * 10+key_length-10+key_length+value_length: value bytes (value_length)
 *
 * Response Layout
 * 0: 0/1 Version
 * 1: status (0/1) fulfilled, err
 * 2-3: value length (2 bytes)
 * 4-7: request id (4 bytes, int)
 * 8-8+value_length: value bytes 
 */ 

#include <span>
#include <cstddef>
#include <cstdint>
#define MAX_VALUE_SIZE 65535

inline constexpr std::uint8_t protocol_version = 1;

namespace request_wire {
  inline constexpr std::size_t version_offset = 0;
  inline constexpr std::size_t opcode_offset = 1;
  inline constexpr std::size_t key_length_offset = 2;
  inline constexpr std::size_t value_length_offset = 4;
  inline constexpr std::size_t request_id_offset = 6;
  inline constexpr std::size_t header_size = 10;
}

namespace response_wire {
  inline constexpr std::size_t version_offset = 0;
  inline constexpr std::size_t status_offset = 1;
  inline constexpr std::size_t value_length_offset = 2;
  inline constexpr std::size_t response_id_offset = 4;
  inline constexpr std::size_t header_size = 8;
}

enum class Opcode : std::uint8_t {
  GET = 1,
  PUT = 2
};

enum class Status : std::uint8_t {
  OK = 0,
  NOT_FOUND = 1,
  FULL = 2
};

enum class DecodeError {
  NONE,
  TOO_SHORT,
  UNSUPPORTED_VERSION,
  INVALID_OPCODE,
  INVALID_LENGTH,
};

enum class EncodeError {
  NONE,
  BUFFER_TOO_SMALL,
  INVALID_RESPONSE,
};

struct RequestView {
  Opcode opcode;
  std::uint32_t request_id;
  std::span<const std::byte> key;
  std::span<const std::byte> value;
};

struct ResponseView {
  Status status;
  std::uint32_t response_id;
  std::span<const std::byte> value;
};

// payload is the full UDP request [header][key][value]
// On success, out borrows payload storage. On failure, out is unchanged.
DecodeError decode_request(
    std::span<const std::byte> payload,
    RequestView& out
);

// output is the full UDP response [header][value]
// Input value and output storage must not overlap. No ownership is transferred.
// On failure, output is unchanged and bytes_written is zero.
EncodeError encode_response(
    const ResponseView& response, 
    std::span<std::byte> output, 
    std::size_t& bytes_written
);
