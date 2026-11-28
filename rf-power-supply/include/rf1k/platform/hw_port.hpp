#pragma once

#include "rf1k/types.hpp"

namespace rf1k {

/** Board HAL — stub on host, real drivers on STM32. */
class IHardware {
 public:
  virtual ~IHardware() = default;

  virtual float readPfW() = 0;
  virtual float readPrW() = 0;
  virtual float readVbusV() = 0;
  virtual float readTempPaC() = 0;
  virtual bool interlockOk() = 0;
  virtual bool faultLatched() = 0;

  virtual void setBusVoltage(float volts) = 0;
  virtual void setDdsAmplitude(float norm_0_1) = 0;
  virtual void rfEnable(bool on) = 0;
  virtual void faultClear() = 0;

  virtual void stepperMove(int axis, int delta_steps) = 0;
  virtual int stepperPos(int axis) = 0;
};

}  // namespace rf1k
