#include "protocol.hpp"

#include <array>
#include <cassert>
#include <vector>

int main() {
  using B = std::byte;
  const std::array value{B{'c'}, B{'a'}, B{'t'}};
  ResponseView response{Status::OK, 0x12345678, value};
  std::array<B, 12> output{};
  output.fill(B{0xee});
  std::size_t written = 999;

  assert(encode_response(response, output, written) == EncodeError::NONE);
  assert(written == 11);
  const std::array expected{B{1}, B{0}, B{0}, B{3}, B{0x12}, B{0x34},
                            B{0x56}, B{0x78}, B{'c'}, B{'a'}, B{'t'}, B{0xee}};
  assert(output == expected);

  auto expect_error = [&](std::span<B> destination, EncodeError error) {
    const auto before = output;
    written = 999;
    assert(encode_response(response, destination, written) == error);
    assert(written == 0);
    assert(output == before);
  };
  expect_error({}, EncodeError::BUFFER_TOO_SMALL);
  expect_error(std::span<B>(output).first(8), EncodeError::BUFFER_TOO_SMALL);
  expect_error(std::span<B>(output).first(10), EncodeError::BUFFER_TOO_SMALL);

  for (Status status : {Status::NOT_FOUND, Status::FULL}) {
    response.status = status;
    expect_error(output, EncodeError::INVALID_RESPONSE);
  }
  response.status = static_cast<Status>(255);
  expect_error(output, EncodeError::INVALID_RESPONSE);

  response.value = {};
  for (Status status : {Status::OK, Status::NOT_FOUND, Status::FULL}) {
    response.status = status;
    assert(encode_response(response, std::span<B>(output).first(8), written) ==
           EncodeError::NONE);
    assert(written == 8);
    assert(output[1] == static_cast<B>(status));
    assert(output[2] == B{0} && output[3] == B{0});
  }

  // Exercise both bytes of the length and its representable limit.
  std::vector<B> large_value(MAX_VALUE_SIZE, B{0xab});
  std::vector<B> large_output(response_wire::header_size + large_value.size());
  response = {Status::OK, 42, large_value};
  assert(encode_response(response, large_output, written) == EncodeError::NONE);
  assert(written == large_output.size());
  assert(large_output[2] == B{0xff} && large_output[3] == B{0xff});
  assert(large_output.back() == B{0xab});
  large_value.push_back(B{0});
  response.value = large_value;
  expect_error(output, EncodeError::INVALID_RESPONSE);
}
