#include "AttitudeEstimator.hpp"

#include <adcs/constants.hpp>

#include <gnc/constants.hpp>
#include <gnc/utilities.hpp>

#include <lin/core.hpp>
#include <lin/generators/constants.hpp>

AttitudeEstimator::AttitudeEstimator(StateFieldRegistry &registry,
        unsigned int offset) :
    TimedControlTask<void>(registry, "adcs_estimator", offset),
    b_body_rd_fp(FIND_READABLE_FIELD(lin::Vector3f, adcs_monitor.mag_vec)),
    s_body_rd_fp(FIND_READABLE_FIELD(lin::Vector3f, adcs_monitor.ssa_vec)),
    w_body_rd_fp(FIND_READABLE_FIELD(lin::Vector3f, adcs_monitor.gyr_vec)),
    pos_ecef_fp(FIND_READABLE_FIELD(lin::Vector3d, orbit_estimator.pos_ecef)),
    pan_time_fp(nullptr),  // pan_time_fp(FIND_READABLE_FIELD(unsigned long, pan_time);
    b_body_est_f("attitude_estimator.b_body", Serializer<lin::Vector3f>(
            adcs::imu::min_rd_mag, adcs::imu::max_rd_mag, 16*3)),
    s_body_est_f("attitude_estimator.s_body", Serializer<lin::Vector3f>(-1, 1, 16*3)),
    q_body_eci_est_f("attitude_estimator.q_body_eci", Serializer<lin::Vector4f>()),
    w_body_est_f("attitude_estimator.w_body", Serializer<lin::Vector3f>(-55, 55, 32*3)),
    estimator_state(),
    estimator_data(),
    estimate()
    {
    // Add out new statefields
    add_readable_field(b_body_est_f);
    add_readable_field(s_body_est_f);
    add_readable_field(q_body_eci_est_f);
    add_readable_field(w_body_est_f);

    // Default the outputs to NaNs
    b_body_est_f.set(lin::nans<lin::Vector3f>());
    s_body_est_f.set(lin::nans<lin::Vector3f>());
    q_body_eci_est_f.set(lin::nans<lin::Vector4f>());
    w_body_est_f.set(lin::nans<lin::Vector3f>());
}

void AttitudeEstimator::execute() {

    // Default all estimates to NaN
    b_body_est_f.set(lin::nans<lin::Vector3f>());
    s_body_est_f.set(lin::nans<lin::Vector3f>());
    q_body_eci_est_f.set(lin::nans<lin::Vector4f>());
    w_body_est_f.set(lin::nans<lin::Vector3f>());
    
    // Populate the attitude estimator's data struct
    estimator_data = gnc::AttitudeEstimatorData();
    estimator_data.t = ((double) pan_time_fp->get()) * 1.0e-9;
    estimator_data.r_ecef = pos_ecef_fp->get();
    estimator_data.b_body = b_body_rd_fp->get();
    estimator_data.s_body = s_body_rd_fp->get();
    estimator_data.w_body = w_body_rd_fp->get();

    // Call the estimator
    gnc::estimate_attitude(estimator_state, estimator_data, estimate);

    // Extract the estimated attitude and angular rate
    q_body_eci_est_f.set(estimate.q_body_eci);
    w_body_est_f.set(estimate.w_body);

    // Continue to estimate the rest of our parameters if the estimator was
    // successful

    // TODO : Complete this

    b_body_est_f.set(b_body_rd_fp->get());
    s_body_est_f.set(s_body_rd_fp->get());

    // if (!std::isnan(q_body_eci_est_f.get()(0))) {
    //     lin::Vector4f q_eci_body;
    //     gnc::utl::quat_conj(estimate.q_body_eci, q_eci_body);
    // }
}
