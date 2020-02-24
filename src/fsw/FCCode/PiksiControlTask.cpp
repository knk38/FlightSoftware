#include "PiksiControlTask.hpp"
#include <limits>
#include <cmath>
#include "radio_state_t.enum"

PiksiControlTask::PiksiControlTask(StateFieldRegistry &registry, 
    unsigned int offset, Devices::Piksi &_piksi) 
    : TimedControlTask<void>(registry, "piksi", offset),
    piksi(_piksi),
    pos_f("piksi.pos", Serializer<d_vector_t>(0,100000,100)),
    vel_f("piksi.vel", Serializer<d_vector_t>(0,100000,100)),
    baseline_pos_f("piksi.baseline_pos", Serializer<d_vector_t>(0,100000,100)),
    current_state_f("piksi.state", Serializer<unsigned int>(4)),
    fix_error_count_f("piksi.fix_error_count", Serializer<unsigned int>(1001)),
    time_f("piksi.time", Serializer<gps_time_t>()),
    last_fix_time_f("piksi.last_fix_time"),
    bool_sr(),
    data_mute_f("piksi.data_mute", bool_sr),
    //if we haven't had a good reading in ~120 seconds the piksi is probably dead
    piksi_fault("piksi.fault", 1000, control_cycle_count)
    {
        add_readable_field(pos_f);
        add_readable_field(vel_f);
        add_readable_field(baseline_pos_f);
        add_readable_field(current_state_f);
        add_readable_field(fix_error_count_f);
        add_readable_field(time_f);
        add_internal_field(last_fix_time_f);
        add_writable_field(data_mute_f);

        piksi_fault.add_to_registry(registry);

        //register callbacks and begin the serial port
        piksi.setup();

        // Set initial values
        data_mute_f.set(true);
        current_state_f.set(static_cast<unsigned int>(piksi_mode_t::no_fix));
        nan_return();
    }

void PiksiControlTask::nan_return(){
    constexpr double nan = std::numeric_limits<double>::quiet_NaN();
    pos_f.set({nan, nan, nan});
    vel_f.set({nan, nan, nan});
    baseline_pos_f.set({nan, nan, nan});

    time = gps_time_t();
    time_f.set(time);
}

void PiksiControlTask::init(){
    radio_state_fp = find_writable_field<unsigned char>("radio.state", __FILE__, __LINE__);
}

void PiksiControlTask::execute()
{
    int read_out = piksi.read_all();

    //4 means no bytes
    //3 means CRC error on serial
    //5 means timing error exceed
    if(read_out == 3|| read_out == 4|| read_out == 5)
        piksi_fault.signal();
    else {
        piksi_fault.unsignal();
    }

    if(read_out == 5){
        current_state_f.set(static_cast<unsigned int>(piksi_mode_t::time_limit_error));
        nan_return();
        return;
    }

    else if(read_out == 4){
        current_state_f.set(static_cast<unsigned int>(piksi_mode_t::no_data_error));
        nan_return();
        return;
    }

    else if(read_out == 3){
        current_state_f.set(static_cast<unsigned int>(piksi_mode_t::crc_error));
        nan_return();
        return;
    }

    else if(read_out == 2){
        current_state_f.set(static_cast<unsigned int>(piksi_mode_t::no_fix));
        nan_return();
        return;
    }

    else if(read_out == 1 || read_out == 0){
        //get time from driver, then dump into a gps_time_t
        piksi.get_gps_time(&msg_time);
        time = gps_time_t(msg_time);

        piksi.get_pos_ecef(&pos_tow, &pos);
        piksi.get_vel_ecef(&vel_tow, &vel);

        if(read_out == 1)
            piksi.get_baseline_ecef(&baseline_tow, &baseline_pos);

        bool check_time;
        if(read_out == 1)
            check_time = time.tow == pos_tow && time.tow == vel_tow && time.tow == baseline_tow;
        else
            check_time = time.tow == pos_tow && time.tow == vel_tow;

        if(!check_time){
            //error caused by times not matching up
            //indicitave of getting only part of the next scream

            current_state_f.set(static_cast<unsigned int>(piksi_mode_t::sync_error));
            nan_return();
            piksi_fault.signal();
            return;
        }

        int nsats = piksi.get_pos_ecef_nsats();
        if(nsats < 4){
            current_state_f.set(static_cast<unsigned int>(piksi_mode_t::nsat_error));
            nan_return();
            piksi_fault.signal();
            return;
        }

        if(read_out == 0) {
            current_state_f.set(static_cast<unsigned int>(piksi_mode_t::spp));
            last_fix_time_f.set(get_system_time());
        }
        if(read_out == 1){
            int baseline_flag = piksi.get_baseline_ecef_flags();
            if(baseline_flag == 1){
                current_state_f.set(static_cast<unsigned int>(piksi_mode_t::fixed_rtk));
                last_fix_time_f.set(get_system_time());
            }
            else if(baseline_flag == 0){
                current_state_f.set(static_cast<unsigned int>(piksi_mode_t::float_rtk));
                last_fix_time_f.set(get_system_time());
            }
            else{
                //baseline flag unexpected value
                current_state_f.set(static_cast<unsigned int>(piksi_mode_t::data_error));
                nan_return();
                piksi_fault.signal();
                return;
            }
        }

        //set values in data statefields
        time_f.set(time);
        pos_f.set(pos);
        vel_f.set(vel);
        if(read_out == 1){
            baseline_pos_f.set(baseline_pos);
        }
        
        piksi_fault.unsignal();

        // mute piksi
        if(data_mute_f.get() && radio_state_fp->get() == static_cast<unsigned int>(radio_state_t::transceive))
            nan_return();
    }

    //if read_out is unexpected value which it shouldn't do lol
    else{
        current_state_f.set(static_cast<unsigned int>(piksi_mode_t::data_error));
        nan_return();
        piksi_fault.signal();
        return;
    }
}
