#pragma once

#include "rf1k/types.hpp"

#include <array>
#include <cstdint>

namespace rf1k {

enum MbHolding : std::uint16_t {
  HCmdWord = 0,
  HPowerSetW = 1,
  HRampWS = 2,
  HPreflTripW = 3,
  HCsPreset = 4,
  HCpPreset = 5,
  HCount = 6
};

enum MbCmd : std::uint16_t {
  CmdRfOn = 1u << 0,
  CmdClearFault = 1u << 1,
  CmdMatchAuto = 1u << 2
};

enum MbInput : std::uint16_t {
  IState = 0,
  IPfW = 1,
  IPrW = 2,
  IVswrX100 = 3,
  IVbusMv = 4,
  ITempPaX10 = 5,
  IFault = 6,
  ICsPos = 7,
  ICpPos = 8,
  ICount = 9
};

class ModbusMap {
 public:
  void init();
  void exportTelemetry(State state, const Telemetry& tm, std::uint32_t fault);
  void applyHolding(Setpoints& sp);

  std::uint16_t& holding(std::uint16_t i) { return holding_[i]; }
  std::uint16_t holding(std::uint16_t i) const { return holding_[i]; }
  std::uint16_t input(std::uint16_t i) const { return input_[i]; }

  int handlePdu(const std::uint8_t* req, int req_len, std::uint8_t* resp,
                int resp_max);

  static std::uint16_t crc16(const std::uint8_t* data, int len);

  int processRtu(const std::uint8_t* frame, int len, std::uint8_t* out,
                 int out_max, std::uint8_t slave = 1);

 private:
  std::array<std::uint16_t, HCount> holding_{};
  std::array<std::uint16_t, ICount> input_{};
};

}  // namespace rf1k
