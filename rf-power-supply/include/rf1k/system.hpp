#pragma once

#include "rf1k/match/search.hpp"
#include "rf1k/modbus.hpp"
#include "rf1k/platform/hw_port.hpp"
#include "rf1k/types.hpp"

namespace rf1k {

/**
 * Top-level RF generator controller.
 * Call tickPower(dt) @ 1 kHz, tickMatch() @ 20–50 Hz, tickComm() as needed.
 */
class System {
 public:
  System(IHardware& hw, match::IActuator& match_hw);

  void init();
  void tickPower(float dt_s);
  void tickMatch();
  void tickComm();  // apply modbus holding, publish telemetry

  ModbusMap& modbus() { return mb_; }
  const ModbusMap& modbus() const { return mb_; }
  State state() const { return state_; }
  const Telemetry& telemetry() const { return tm_; }
  Setpoints& setpoints() { return sp_; }
  std::uint32_t fault() const { return fault_; }
  match::Search& matcher() { return matcher_; }

 private:
  void onFault(std::uint32_t bits);
  void clearFault();
  void stateMachine(float dt_s);
  void powerLoop(float dt_s);

  IHardware& hw_;
  Setpoints sp_{};
  Telemetry tm_{};
  State state_{State::Idle};
  std::uint32_t fault_{Fault::None};
  float ramp_power_w_{0.f};
  Pid power_pid_{};
  ModbusMap mb_{};
  match::Search matcher_;
};

}  // namespace rf1k
