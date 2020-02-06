#include "test_fixture.hpp"

TestFixture::TestFixture(mission_state_t initial_state) : registry()
{
    adcs_ang_momentum_fp = registry.create_internal_field<lin::Vector3f>(
                                "attitude_estimator.h_body");
    adcs_paired_fp = registry.create_writable_field<bool>("adcs.paired");

    radio_state_fp = registry.create_internal_field<unsigned char>("radio.state");
    last_checkin_cycle_fp = registry.create_internal_field<unsigned int>(
                                "radio.last_comms_ccno");

    prop_state_fp = registry.create_readable_field<unsigned char>("prop.state", 2);

    piksi_mode_fp = registry.create_readable_field<unsigned char>("piksi.state", 4);
    propagated_baseline_pos_fp = registry.create_readable_vector_field<double>(
                                    "orbit.baseline_pos", 0, 100000, 100);

    docked_fp = registry.create_readable_field<bool>("docksys.docked");

    // Initialize these variables
    const float nan_f = std::numeric_limits<float>::quiet_NaN();
    const double nan_d = std::numeric_limits<double>::quiet_NaN();
    adcs_ang_momentum_fp->set({nan_f,nan_f,nan_f});
    radio_state_fp->set(static_cast<unsigned char>(radio_state_t::disabled));
    last_checkin_cycle_fp->set(0);
    prop_state_fp->set(static_cast<unsigned char>(prop_state_t::disabled));
    propagated_baseline_pos_fp->set({nan_d,nan_d,nan_d});
    docked_fp->set(false);

    mission_manager = std::make_unique<MissionManager>(registry, 0);

    // Check that mission manager creates its expected fields
    detumble_safety_factor_fp = registry.find_writable_field_t<double>("detumble_safety_factor");
    close_approach_trigger_dist_fp = registry.find_writable_field_t<double>("trigger_dist.close_approach");
    docking_trigger_dist_fp = registry.find_writable_field_t<double>("trigger_dist.docking");
    max_radio_silence_duration_fp = registry.find_writable_field_t<unsigned int>("max_radio_silence");
    adcs_state_fp = registry.find_writable_field_t<unsigned char>("adcs.state");
    docking_config_cmd_fp = registry.find_writable_field_t<bool>("docksys.config_cmd");
    mission_state_fp = registry.find_writable_field_t<unsigned char>("pan.state");
    is_deployed_fp = registry.find_readable_field_t<bool>("pan.deployed");
    deployment_wait_elapsed_fp = registry.find_readable_field_t<unsigned int>(
                                    "pan.deployment.elapsed");
    sat_designation_fp = registry.find_writable_field_t<unsigned char>("pan.sat_designation");

    // Set initial state.
    mission_state_fp->set(static_cast<unsigned char>(initial_state));
}

// Set and assert functions for various mission states.

void TestFixture::set(mission_state_t state) {
    mission_state_fp->set(static_cast<unsigned char>(state));
}

void TestFixture::set(adcs_state_t state) {
    adcs_state_fp->set(static_cast<unsigned char>(state));
}

void TestFixture::set(prop_state_t state) {
    prop_state_fp->set(static_cast<unsigned char>(state));
}

void TestFixture::set(radio_state_t state) {
    radio_state_fp->set(static_cast<unsigned char>(state));
}

void TestFixture::set(sat_designation_t designation) {
    sat_designation_fp->set(static_cast<unsigned char>(designation));
}

// Step forward the state machine by 1 control cycle.
void TestFixture::step() { mission_manager->execute(); }

void TestFixture::set_ccno(unsigned int ccno) {
    mission_manager->control_cycle_count = ccno;
}

// Create a hardware fault that necessitates a transition to safe hold or initialization hold.
void TestFixture::set_hardware_fault(bool faulted) {
    // TODO
}

// Set the distance between the two satellites.
void TestFixture::set_sat_distance(double dist) { propagated_baseline_pos_fp->set({dist, 0, 0}); }


// Set the angular rate of the spacecraft.
void TestFixture::set_ang_rate(float rate) {
    adcs_ang_momentum_fp->set({rate, 0, 0}); // TODO will need to change this once the inertia tensor
                                             // is added to GNC constants.
}

adcs_state_t TestFixture::adcs_states[8] = {adcs_state_t::detumble, adcs_state_t::limited,
        adcs_state_t::point_docking, adcs_state_t::point_manual, adcs_state_t::point_standby,
        adcs_state_t::startup, adcs_state_t::zero_L, adcs_state_t::zero_torque};

prop_state_t TestFixture::prop_states[6] = {prop_state_t::disabled, prop_state_t::idle,
    prop_state_t::await_firing,
    prop_state_t::pressurizing,
    prop_state_t::firing,
    prop_state_t::venting};
