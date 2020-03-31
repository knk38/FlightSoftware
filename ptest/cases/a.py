# SpinMotorsCase. Gets satellite read to spin motors.
from .base import SingleSatOnlyCase
import time

class A(SingleSatOnlyCase):
    def setup_case_singlesat(self):
        # self.sim.flight_controller.write_state("pan.state", 11) # Mission State = Manual
        # self.sim.flight_controller.write_state("adcs.state", 5) # ADCS State = Manual
        # self.sim.flight_controller.write_state("adcs_cmd.rwa_mode", 1) # Speed Control
        # self.sim.flight_controller.write_state("adcs_cmd.rwa_speed_cmd", "0, 0, 0") # 0 speed to begin with
        # self.sim.flight_controller.write_state("dcdc.ADCSMotor_cmd", "true")

        # motors spin wheels green

        self.ws("cycle.auto", True)

        print(self.rs("gomspace.vbatt"))

        self.ws("adcs_cmd.havt_reset7", True)
        self.ws("adcs_cmd.havt_reset8", True)
        self.ws("adcs_cmd.havt_reset9", True)

        self.ws("dcdc.ADCSMotor_cmd", True)
        self.ws("adcs_cmd.rwa_speed_cmd", [10,10,10])
        self.ws("adcs_cmd.rwa_mode", self.rwa_modes.get_by_name("RWA_SPEED_CTRL"))
        self.ws("pan.state", self.mission_states.get_by_name("manual"))
        self.ws("adcs.state", self.adcs_states.get_by_name("point_manual"))

        print(self.rs("gomspace.vbatt"))
        print(self.rs("pan.cycle_no"))

        # time.sleep(5)

        # self.cycle()

        # time.sleep(5)

        # self.cycle()

    def run_case_singlesat(self):
        self.cycle_no = self.sim.flight_controller.read_state("pan.cycle_no")
        self.finish()
