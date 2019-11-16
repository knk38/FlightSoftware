#pragma once

#include <ControlTask.hpp>
#include <Piksi.hpp>
#include <string>

/**
 * Piksi / Piksi Control Task States
 */
static constexpr int SPP = 0;
static constexpr int FIXED_RTK = 1;
static constexpr int FLOAT_RTK = 2;

static constexpr int NO_FIX = 3;
static constexpr int SYNC_ERROR = 4;
static constexpr int DATA_ERROR = 5;
static constexpr int NSAT_ERROR = 6;
static constexpr int NO_DATA = 7;
static constexpr int TIME_LIMIT = 8;

static constexpr int DEAD = 9;

class PiksiControlTask : public ControlTask<void>
{
public:
#ifndef DESKTOP
  PiksiControlTask(StateFieldRegistry &registry) : ControlTask<int>(registry),
                                                   piksi("Piksi", Serial4) {}

#else
  PiksiControlTask(StateFieldRegistry &registry);
  Devices::Piksi piksi;

#endif
  /** 
   * execute is overriden from ControlTask 
   * Calling execute() when the state is IDLE generates no effects. 
  */
  void execute();

  /** Returns the current state of the Piksi */
  int get_current_state() const;

protected:
  #ifndef DESKTOP
  Devices::Piksi piksi;
  #endif

  Serializer<d_vector_t> pos_sr;
  ReadableStateField<d_vector_t> pos_f;

  Serializer<d_vector_t> vel_sr;
  ReadableStateField<d_vector_t> vel_f;

  Serializer<d_vector_t> baseline_pos_sr;
  ReadableStateField<d_vector_t> baseline_pos_f;

  Serializer<int> currentState_sr;
  ReadableStateField<int> currentState_f;

  std::array<double, 3> pos;
  std::array<double, 3> vel;
  std::array<double, 3> baseline_pos;
  msg_gps_time_t time;

  unsigned int tow_past = 0;
  unsigned int iar;

  unsigned int pos_past;
  unsigned int pos_tow;

  unsigned int vel_past;
  unsigned int vel_tow;

  unsigned int baseline_past;
  unsigned int baseline_tow;

private:

  int currentState; // the state of the Piksi

  // the number of cycles since a successful / meaningful read operation
  unsigned int since_good_cycles = 0;

};