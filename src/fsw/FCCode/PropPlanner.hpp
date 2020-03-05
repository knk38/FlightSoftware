#ifndef PROP_PLANNER_HPP_
#define PROP_PLANNER_HPP_

class PropPlanner : public TimedControlTask<void>() {
  public:
    PropPlanner(StateFieldRegistry& r);
    void execute() override;

  private:
    // Outputs to PropController
    ReadableStateField<unsigned int>* cycles_until_firing_fp;
    ReadableStateField<unsigned int>* sched_valve1_fp;
    ReadableStateField<unsigned int>* sched_valve2_fp;
    ReadableStateField<unsigned int>* sched_valve3_fp;
    ReadableStateField<unsigned int>* sched_valve4_fp;

    // OrbitController output
    struct OrbitControllerOutput {
        bool start_pressurizing;
        bool stop_pressurizing;
        std::array<unsigned int, 4> valve_scheds;
    }
};

#endif
