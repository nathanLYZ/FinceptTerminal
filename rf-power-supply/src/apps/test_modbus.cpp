#include "rf1k/modbus.hpp"

#include <cstdio>
#include <cstring>

int main() {
  rf1k::ModbusMap map;
  map.init();

  std::uint8_t req[8] = {1, 0x06, 0x00, 0x01, 0x01, 0xF4, 0, 0};  // power=500
  auto crc = rf1k::ModbusMap::crc16(req, 6);
  req[6] = static_cast<std::uint8_t>(crc & 0xff);
  req[7] = static_cast<std::uint8_t>(crc >> 8);

  std::uint8_t resp[64];
  int n = map.processRtu(req, 8, resp, 64);
  if (n < 6 || map.holding(rf1k::HPowerSetW) != 500) {
    std::printf("FAIL write holding\n");
    return 1;
  }

  rf1k::Telemetry tm;
  tm.pf_w = 498.2f;
  tm.pr_w = 12.f;
  tm.vswr = 1.37f;
  map.exportTelemetry(rf1k::State::Run, tm, 0);

  std::uint8_t rd[8] = {1, 0x04, 0x01, 0x00, 0x00, 0x03, 0, 0};
  crc = rf1k::ModbusMap::crc16(rd, 6);
  rd[6] = static_cast<std::uint8_t>(crc & 0xff);
  rd[7] = static_cast<std::uint8_t>(crc >> 8);
  n = map.processRtu(rd, 8, resp, 64);
  if (n < 9) {
    std::printf("FAIL read input len=%d\n", n);
    return 1;
  }
  const auto state = static_cast<std::uint16_t>((resp[3] << 8) | resp[4]);
  const auto pf = static_cast<std::uint16_t>((resp[5] << 8) | resp[6]);
  if (state != 2 || pf != 498) {
    std::printf("FAIL values state=%u pf=%u\n", state, pf);
    return 1;
  }
  std::printf("ok modbus c++ tests passed\n");
  return 0;
}
