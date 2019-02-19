/** @file comms_utils.inl
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Implementation of utilities for compressing and 
 * decompressing data for downlinking and processing uplinks.
 */

#include "comms_utils.hpp"

template<unsigned int packet_bit_size>
unsigned int Comms::add_packet_checksum(std::bitset<packet_bit_size>& packet,
                                unsigned int& packet_ptr,
                                std::array<char, (unsigned int) ceilf(packet_bit_size / 8)>* dest) {
    // Copy data over to destination char array
    std::string packet_string = packet.to_string();
    std::copy(packet_string.begin(), packet_string.end(), dest->data());
    // Compute checksum
    FastCRC32 crc32;
    unsigned int crc32_value = crc32.crc32((const uint8_t*) dest->data(), ((packet_bit_size - 1) - packet_ptr) / 8);
    // Copy checksum into destination array
    std::bitset<32> crc32_value_representation(crc32_value);
    std::string crc32_value_string = crc32_value_representation.to_string();
    std::copy(packet_string.begin(), packet_string.end(), dest->data() + (unsigned int) ceilf(packet_bit_size / 8));
    return crc32_value;
}

template<unsigned int max_size>
void Comms::trim_float(float f, float min, float max, std::bitset<max_size>* result) {
    if(f > max) f = max;
    if(f < min) f = min;
    float resolution = (max - min) / pow(2, max_size);
    unsigned int result_int = (unsigned int) ((f - min) / resolution);
    std::bitset<max_size> result_copy(result_int);
    *result = result_copy;
}

template<unsigned int max_size>
float Comms::expand_float(const std::bitset<max_size>& f, float min, float max) {
    unsigned long f_bits = f.to_ullong();
    float resolution = (max - min) / pow(2, max_size);
    return min + resolution * f_bits;
}

template<unsigned int max_vec_size, REQUIRES(max_vec_size > 20)>
void Comms::trim_vector(const std::array<float, 3>& v, float min_magnitude, float max_magnitude, std::bitset<max_vec_size>* result) {
    float mag = sqrt(pow(v[0],2)+pow(v[1],2)+pow(v[2],2));
    std::bitset<max_vec_size - 20> magnitude_representation;
    trim_float(mag, min_magnitude, max_magnitude, &magnitude_representation);

    // Compress unit vector into two compressed floats
    std::bitset<20> float_representation;
    std::bitset<9> vec_element_representations[2];
    std::array<float, 3> v_mags; // Magnitudes of elements in vector
    for(int i = 0; i < 3; i++) v_mags[i] = std::abs(v[i]);
    
    int max_element_idx = 0;
    float max_element_value = std::max(std::max(v_mags[0], v_mags[1]), v_mags[2]);
    if (v_mags[0] == max_element_value) max_element_idx = 0;
    if (v_mags[1] == max_element_value) max_element_idx = 1;
    if (v_mags[2] == max_element_value) max_element_idx = 2;
    int vec_number = 0; // The current compressed vector bitset that we're modifying
    for(int i = 0; i < 3; i++) {
        if (i == max_element_idx) {
            std::bitset<2> largest_element_representation(i);
            float_representation.set(0, largest_element_representation[0]);
            float_representation.set(1, largest_element_representation[1]);
        }
        else {
            float v_element_scaled = v[vec_number] / mag;
            trim_float(v_element_scaled, -sqrt(2), sqrt(2), &vec_element_representations[vec_number]);
            vec_number++;
        }
    }
    // Add floats to final bit representation
    for(int i = 0; i < 9; i++) {
        float_representation.set(i+2, vec_element_representations[0][i]);
        float_representation.set(i+11, vec_element_representations[1][i]);
    }
    for(unsigned int i = 0; i < max_vec_size - 20; i++)
        (*result).set(i, magnitude_representation[i]);
    for(int i = 0; i < 20; i++)
        (*result).set(i + max_vec_size - 20, float_representation[i]);
}

template<unsigned int max_vec_size, REQUIRES(max_vec_size > 20)>
void Comms::trim_vector(const std::array<float, 3>& v, float max_magnitude, std::bitset<max_vec_size>* result) {
    trim_vector(v, 0, max_magnitude, result);
}

template<unsigned int max_vec_size, REQUIRES(max_vec_size > 20)>
void Comms::expand_vector(const std::bitset<max_vec_size>& v, float min_magnitude, float max_magnitude, std::array<float, 3>* result) {
    std::bitset<max_vec_size - 20> magnitude_packed;
    for(int i = 0; i < max_vec_size - 20; i++) magnitude_packed.set(i, v[i]);
    float magnitude = expand_float(magnitude_packed, min_magnitude, max_magnitude);

    unsigned int missing_component = v[max_vec_size-20] << 1 + v[max_vec_size-20+1];
    (*result)[missing_component] = 1;
    int j = 0; // Index of current component being processed
    for(int i = 0; i < 3; i++) {
        if (i != missing_component) {
            std::bitset<9> vec_component_packed;
            for(int k = 0; k < 9; k++) vec_component_packed.set(k, v[max_vec_size-20+2+j*9+k]);
            (*result)[i] = expand_float(vec_component_packed, -sqrt(2), sqrt(2));
            (*result)[missing_component] -= pow((*result)[i],2);
            j++;
        }
    }
    (*result)[missing_component] = sqrt((*result)[missing_component]);

    for(int i = 0; i < 3; i++)
        (*result)[i] *= magnitude;
}

template<unsigned int max_vec_size, REQUIRES(max_vec_size > 20)>
void Comms::expand_vector(const std::bitset<max_vec_size>& v, float max_magnitude, std::array<float, 3>* result) {
    expand_vector(v, 0, max_magnitude, result);
}

template<unsigned int max_int_size>
void Comms::trim_int(int i, int min, int max, std::bitset<max_int_size>* result) {
    if (i > max) i = max;
    if (i < min) i = min;
    unsigned int resolution = (max - min) / pow(2, max_int_size);
    unsigned int result_int = (i - min) / resolution;
    std::bitset<max_int_size> result_copy(result_int);
    *result = result_copy;
}

template<unsigned int max_int_size>
int Comms::expand_int(const std::bitset<max_int_size>& result, int min, int max) {
    unsigned int resolution = (max - min) / pow(2, max_int_size);
    return min + result.to_ulong() * resolution;
}