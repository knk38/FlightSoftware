#include "AttitudeComputer.hpp"

#include <gnc/constants.hpp>
#include <gnc/utilities.hpp>

#include <lin/core.hpp>
#include <lin/generators/constants.hpp>

AttitudeEstimator::AttitudeEstimator(StateFieldRegistry& registry, unsigned int offset) :
    TimedControlTask<void>(registery, offset)
    // TODO
{
    // TODO
}

void AttitudeEstimator::execute() {
  
}
