#ifndef ATTITUDE_ESTIMATOR_HPP_
#define ATTITUDE_ESTIMATOR_HPP_

#include "TimedControlTask.hpp"

#include <gnc_attitude_estimation.hpp>
#include <lin/core/vector/vector.hpp>

/**
 * @brief Determine attitude and body angular rate using the current time, orbit
 * estimate, and adcs sensor input.
 */
class AttitudeEstimator : public TimedControlTask<void> {
   public:
    /**
     * @brief Construct a new attitude estimator.
     * 
     * @param registry
     * @param offset
     */
    AttitudeEstimator(StateFieldRegistry& registry, unsigned int offset);
    virtual ~AttitudeEstimator() = default;

    /**
     * @brief Update out current estimate of attitude and angular rate.
     */
    void execute() override;

   protected:
    // Inputs from the adcs box monitor
    ReadableStateField<lin::Vector3f> const *const b_body_rd_fp;
    ReadableStateField<lin::Vector3f> const *const s_body_rd_fp;
    ReadableStateField<lin::Vector3f> const *const w_body_rd_fp;

    // Inputs from the orbit estimator
    ReadableStateField<lin::Vector3d> const *const pos_ecef_fp;

    // TODO : Determine where this is from
    ReadableStateField<lin::Vector3f> const *const pan_time_fp;

    // Output estimates
    ReadableStateField<lin::Vector3f> b_body_est_f;
    ReadableStateField<lin::Vector3f> s_body_est_f;
    ReadableStateField<lin::Vector4f> q_body_eci_est_f;
    ReadableStateField<lin::Vector3f> w_body_est_f;

    // Structs for the psim attitude estimator adapter
    gnc::AttitudeEstimatorState estimator_state;
    gnc::AttitudeEstimatorData  estimator_data;
    gnc::AttitudeEstimate       estimate;
};

#endif
