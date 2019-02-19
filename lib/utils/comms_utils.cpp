/** @file comms_utils.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Implementation of utilities for compressing and 
 * decompressing data for downlinking and processing uplinks.
 */

#include "comms_utils.hpp"

void Comms::trim_temperature(int t, std::bitset<9>* result) {
    trim_int(t, -40, 125, result);
}

int Comms::expand_temperature(const std::bitset<9>& t) {
    return expand_int(t, -40, 125);
}

void Comms::trim_quaternion(const std::array<float, 4>& q, std::bitset<29>* result) {
    std::bitset<9> quat_element_representations[3];
    std::array<float, 4> q_mags; // Magnitudes of elements in quaternion
    for(int i = 0; i < 4; i++) q_mags[i] = std::abs(q[i]);

    int max_element_idx = 0;
    float max_element_value = std::max(std::max(q_mags[0], q_mags[1]), std::max(q_mags[2], q_mags[3]));
    if (q_mags[0] == max_element_value) max_element_idx = 0;
    if (q_mags[1] == max_element_value) max_element_idx = 1;
    if (q_mags[2] == max_element_value) max_element_idx = 2;
    if (q_mags[3] == max_element_value) max_element_idx = 3;
    int quat_number = 0; // The current compressed vector bitset that we're modifying
    for(int i = 0; i < 4; i++) {
        if (i == max_element_idx) {
            std::bitset<2> largest_element_representation(i);
            result->set(0, largest_element_representation[0]);
            result->set(1, largest_element_representation[1]);
        }
        else {
            trim_float(q[quat_number], -sqrt(2), sqrt(2),  &quat_element_representations[quat_number]);
            quat_number++;
        }
    }
    for(int i = 0; i < 9; i++) {
        result->set(i+2, quat_element_representations[0][i]);
        result->set(i+11, quat_element_representations[1][i]);
        result->set(i+20, quat_element_representations[2][i]);
    }
}

void Comms::expand_quaternion(const std::bitset<29>& q, std::array<float, 4>* result) {
    int missing_element = (q[0] << 1) + q[1];
    (*result)[missing_element] = 1;
    int j = 0; // Currently processed packed quaternion element
    for(int i = 0; i < 4; i++) {
        if (i != missing_element) {
            std::bitset<9> quat_element_packed;
            for(int k = 0; k < 9; k++) quat_element_packed.set(k, q[2 + j*9 + k]);
            (*result)[i] = expand_float(quat_element_packed, -sqrt(2), sqrt(2));
            (*result)[missing_element] -= pow((*result)[i], 2);
        }
    }
    (*result)[missing_element] = sqrt((*result)[missing_element]);
}

void Comms::trim_gps_time(const msg_gps_time_t& gpstime, std::bitset<48>* result) {
    std::bitset<16> wn((unsigned short int) gpstime.wn);
    std::bitset<32> tow(gpstime.tow);
    for(int i = 0; i < 16; i++)
        result->set(i, wn[i]);
    for(int i = 0; i < 32; i++)
        result->set(i + 16, tow[i]);
}

void Comms::expand_gps_time(const std::bitset<48>& gpstime, msg_gps_time_t* result) {
    std::bitset<16> wn;
    std::bitset<32> tow;
    for(int i = 0; i < 16; i++)
        wn.set(i, gpstime[i]);
    for(int i = 0; i < 32; i++)
        tow.set(i, gpstime[16+i]);
    result->wn = (unsigned int) wn.to_ulong();
    result->tow = (unsigned int) tow.to_ulong();
}