#include "rf1k/modbus.hpp"

#include <cstring>

namespace rf1k {

void ModbusMap::init() {
  holding_.fill(0);
  input_.fill(0);
  holding_[HRampWS] = 200;
  holding_[HPreflTripW] = 200;
  holding_[HCmdWord] = CmdMatchAuto;
}

void ModbusMap::exportTelemetry(State state, const Telemetry& tm,
                                std::uint32_t fault) {
  input_[IState] = static_cast<std::uint16_t>(state);
  input_[IPfW] = static_cast<std::uint16_t>(tm.pf_w < 0 ? 0 : tm.pf_w + 0.5f);
  input_[IPrW] = static_cast<std::uint16_t>(tm.pr_w < 0 ? 0 : tm.pr_w + 0.5f);
  input_[IVswrX100] =
      static_cast<std::uint16_t>(tm.vswr * 100.f + 0.5f);
  input_[IVbusMv] =
      static_cast<std::uint16_t>(tm.vbus_v * 1000.f + 0.5f);
  input_[ITempPaX10] =
      static_cast<std::uint16_t>(tm.temp_pa_c * 10.f + 0.5f);
  input_[IFault] = static_cast<std::uint16_t>(fault);
  input_[ICsPos] = static_cast<std::uint16_t>(tm.cs_pos);
  input_[ICpPos] = static_cast<std::uint16_t>(tm.cp_pos);
}

void ModbusMap::applyHolding(Setpoints& sp) {
  const auto cmd = holding_[HCmdWord];
  sp.rf_on = (cmd & CmdRfOn) != 0;
  sp.clear_fault = (cmd & CmdClearFault) != 0;
  sp.match_auto = (cmd & CmdMatchAuto) != 0;
  sp.power_set_w = static_cast<float>(holding_[HPowerSetW]);
  sp.ramp_w_s = static_cast<float>(holding_[HRampWS]);
  sp.prefl_trip_w = static_cast<float>(holding_[HPreflTripW]);
  if (cmd & CmdClearFault) {
    holding_[HCmdWord] =
        static_cast<std::uint16_t>(cmd & ~CmdClearFault);
  }
}

std::uint16_t ModbusMap::crc16(const std::uint8_t* data, int len) {
  std::uint16_t crc = 0xFFFF;
  for (int i = 0; i < len; ++i) {
    crc ^= data[i];
    for (int b = 0; b < 8; ++b) {
      crc = (crc & 1) ? static_cast<std::uint16_t>((crc >> 1) ^ 0xA001)
                      : static_cast<std::uint16_t>(crc >> 1);
    }
  }
  return crc;
}

static void put_be(std::uint8_t* p, std::uint16_t v) {
  p[0] = static_cast<std::uint8_t>(v >> 8);
  p[1] = static_cast<std::uint8_t>(v & 0xff);
}

int ModbusMap::handlePdu(const std::uint8_t* req, int req_len,
                         std::uint8_t* resp, int resp_max) {
  if (req_len < 2 || resp_max < 5) return -1;
  resp[0] = req[0];
  resp[1] = req[1];
  const auto fn = req[1];

  if (fn == 0x03 || fn == 0x04) {
    if (req_len < 6) return -1;
    const std::uint16_t start =
        static_cast<std::uint16_t>((req[2] << 8) | req[3]);
    const std::uint16_t qty =
        static_cast<std::uint16_t>((req[4] << 8) | req[5]);
    if (fn == 0x03) {
      if (start + qty > HCount || qty == 0) {
        resp[1] = static_cast<std::uint8_t>(fn | 0x80);
        resp[2] = 0x02;
        return 3;
      }
      resp[2] = static_cast<std::uint8_t>(qty * 2);
      for (std::uint16_t i = 0; i < qty; ++i)
        put_be(&resp[3 + i * 2], holding_[start + i]);
      return 3 + qty * 2;
    }
    // Input regs: protocol absolute 0x0100+
    if (start < 0x0100 || start > 0x0108 || qty == 0 ||
        (start - 0x0100 + qty) > ICount) {
      resp[1] = static_cast<std::uint8_t>(fn | 0x80);
      resp[2] = 0x02;
      return 3;
    }
    const int idx0 = static_cast<int>(start - 0x0100);
    resp[2] = static_cast<std::uint8_t>(qty * 2);
    for (std::uint16_t i = 0; i < qty; ++i)
      put_be(&resp[3 + i * 2], input_[idx0 + i]);
    return 3 + qty * 2;
  }

  if (fn == 0x06) {
    if (req_len < 6) return -1;
    const std::uint16_t reg =
        static_cast<std::uint16_t>((req[2] << 8) | req[3]);
    const std::uint16_t val =
        static_cast<std::uint16_t>((req[4] << 8) | req[5]);
    if (reg >= HCount) {
      resp[1] = static_cast<std::uint8_t>(fn | 0x80);
      resp[2] = 0x02;
      return 3;
    }
    holding_[reg] = val;
    std::memcpy(resp, req, 6);
    return 6;
  }

  resp[1] = static_cast<std::uint8_t>(fn | 0x80);
  resp[2] = 0x01;
  return 3;
}

int ModbusMap::processRtu(const std::uint8_t* frame, int len, std::uint8_t* out,
                          int out_max, std::uint8_t slave) {
  if (len < 4) return -1;
  const auto crc_rx =
      static_cast<std::uint16_t>(frame[len - 2] | (frame[len - 1] << 8));
  if (crc_rx != crc16(frame, len - 2)) return -2;
  if (frame[0] != slave && frame[0] != 0) return 0;
  const int body = handlePdu(frame, len - 2, out, out_max - 2);
  if (body < 0) return body;
  const auto crc = crc16(out, body);
  out[body] = static_cast<std::uint8_t>(crc & 0xff);
  out[body + 1] = static_cast<std::uint8_t>(crc >> 8);
  return body + 2;
}

}  // namespace rf1k
