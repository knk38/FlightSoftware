#include <unity.h>
#define PROP_TEST
#include <PropulsionSystem.hpp>
#include <Arduino.h>

/**
 * test_prop.cpp
 * Hardware test for the Propulsion System
 */ 
#define ASSERT_TRUE(x, msg){\
UNITY_TEST_ASSERT(x, __LINE__, msg);}
#define ASSERT_FALSE(x, msg){\
UNITY_TEST_ASSERT(x, __LINE__, msg);}

void check_all_valves_closed();
void is_schedule_expected(const std::array<unsigned int, 4> &expected_schedule);
void reset_tank();
void fill_tank(); 
void thrust();
bool is_at_threshold_pressure();
bool is_valve_open(size_t valve_pin);

const std::array<unsigned int, 4> zero_schedule = {0, 0, 0, 0};

Devices::PropulsionSystem prop_system;
// -llibc -lc
void set_firing_schedule(unsigned int a, unsigned int b, unsigned int c, unsigned int d)
{
    prop_system.tank2.schedule[0] = a;
    prop_system.tank2.schedule[1] = b;
    prop_system.tank2.schedule[2] = c;
    prop_system.tank2.schedule[3] = d;
}

void test_initialization()
{
    // schedule should be initialized to 0
    is_schedule_expected(zero_schedule);
    // all valves should be closed
    check_all_valves_closed();
    // there should be no interrupts at startup
    TEST_ASSERT_FALSE(prop_system.is_enabled);
    // prop system should still be considered functional
    TEST_ASSERT_TRUE(prop_system.is_functional());
}

// Make sure that the current schedule matches the provided schedule
void is_schedule_expected(const std::array<unsigned int, 4> &expected_schedule)
{
    for (size_t i = 0; i < 4; ++i)
        TEST_ASSERT_EQUAL(expected_schedule[i], prop_system.tank2.schedule[i]);
}

// Make sure all pins low
void check_all_valves_closed()
{
    for (size_t i = 0; i < 6; ++i)
        TEST_ASSERT_EQUAL(LOW, is_valve_open(i));
}

// True if specified valve is open
bool is_valve_open(size_t valve_pin)
{
    return digitalRead(valve_pin);
}

void test_is_functional()
{
    TEST_ASSERT_TRUE(prop_system.is_functional());
}

// test that we can turn on the timer
void test_enable()
{
    // TODO: how to check that interval timer is on?
    TEST_ASSERT_TRUE(prop_system.is_functional());
    set_firing_schedule(0, 0, 0, 0);
    prop_system.enable();
    // is_enabled is only ever set by enable() or disable()
    ASSERT_TRUE(prop_system.is_enabled, "interval timer should be on");
    TEST_ASSERT_TRUE(prop_system.is_functional());
    check_all_valves_closed();
    ASSERT_FALSE(prop_system.tank2.valve_lock.is_free(), 
    "thrust valves should be locked");
    delay(6); // make sure the interrupt didn't turn on the valves
    // enable should not open any valves since schedule is 0
    check_all_valves_closed();
}

// should disable interval and turn off all thrust valves
void test_disable()
{
    prop_system.disable();
    // TODO: how to check that intervaltimer is off
    ASSERT_FALSE(prop_system.is_enabled, "interval timer should be off");
    TEST_ASSERT_TRUE(prop_system.is_functional());
    // check that schedule is 0 and valves are all off
    is_schedule_expected(zero_schedule);
    check_all_valves_closed();
}

void test_reset()
{
    set_firing_schedule(12, 1000, 40, 200);
    // possibly dangerous
    prop_system.enable();
    // reset stops the timer
    prop_system.reset();
    check_all_valves_closed();
    is_schedule_expected(zero_schedule);
}

// test that we can open and close the tank valves
void test_open_tank1_valve()
{
    prop_system.open_valve(prop_system.tank1, 0);
    ASSERT_TRUE(is_valve_open(0), "tank1 valve 0 should be open");
    delay(1000); // fire for 1 second
    prop_system.close_valve(prop_system.tank1, 0);
    ASSERT_FALSE(is_valve_open(0), "tank1 valve 0 should be closed"); // make sure it is closed
}

void test_tank_lock()
{
    prop_system.open_valve(prop_system.tank1, 1);
    ASSERT_FALSE(prop_system.tank2.valve_lock.is_free(), "tank valves should be locked");
    delay(1000);
    prop_system.close_valve(prop_system.tank1, 1);
    ASSERT_FALSE (prop_system.tank2.valve_lock.is_free(), "tank should still be locked");
    delay(1000* 5);
    ASSERT_FALSE (prop_system.tank2.valve_lock.is_free(), "tank should still be locked");
    delay(1000*5); 
    ASSERT_TRUE (prop_system.tank2.valve_lock.is_free(), "tank should be unlocked");
}

void test_tank1_enforce_lock()
{
    prop_system.open_valve(prop_system.tank1, 0);
    ASSERT_FALSE(prop_system.open_valve(prop_system.tank2, 1), "Request to open tank1 valve 1 should be denied");
    prop_system.close_valve(prop_system.tank1, 0);
    ASSERT_FALSE(prop_system.open_valve(prop_system.tank2, 1), "Closing valve does not reset valve timer");
    delay(1000*10);
    ASSERT_TRUE(prop_system.open_valve(prop_system.tank2, 1), "Valve lock should be unlocked now");
    delay(1000);
    prop_system.close_valve(prop_system.tank2, 1);
}

// Open tank valve for 1 second and wait 10 s to make sure no one can fire
void fire_tank_valve(size_t valve_pin)
{
    if (valve_pin > 2)
    {
        ASSERT_FALSE(1, "Tank Valve pins are indexed at 0 and 1");
    }
    while ( !prop_system.open_valve(prop_system.tank1, 1) ) 
        delay(140);

    delay(1000);
    prop_system.close_valve(prop_system.tank1, 1);
}

// TODO: not sure how to test this
void test_set_thrust_valve_schedule()
{
    TEST_ASSERT_FALSE(prop_system.is_enabled);
    if (prop_system.tank2.get_pressure() == 0)
    {
        fill_tank();
    }
    static const std::array<unsigned int, 4> schedule = {4, 2, 3, 4};
    prop_system.tank2.set_schedule(schedule);
    prop_system.enable();
    TEST_ASSERT_TRUE(is_valve_open(2) && is_valve_open(3) && is_valve_open(4) && is_valve_open(5))
    delay(1000);
    check_all_valves_closed();
}

// Fill tank to threshold value
void fill_tank()
{
    // Fire a maximum of 20 times for 1 s every 10s until we have threshold_presure
    for ( size_t i = 0; is_at_threshold_pressure() && i < 20; ++i)
    {
        prop_system.open_valve(prop_system.tank1, 0);
        delay(1000);
        prop_system.close_valve(prop_system.tank1, 0);
        delay(1000* 10); // fire every 10 seconds
    }
    ASSERT_TRUE(is_at_threshold_pressure(), "tank 2 pressure should be above threshold pressure")
}

bool is_at_threshold_pressure()
{
    return prop_system.tank2.get_pressure() > prop_system.tank2.threshold_pressure;
}

// Empty out the outer tank so that pressure is 0
void reset_tank()
{
    // this is bad
    check_all_valves_closed(); // make sure all outer valves are closed
    while (prop_system.tank2.get_pressure() > 1.0f) // arbitrary delta
        thrust();
    // ASSERT_TRUE(prop_system.get_pressure() < prop_system.pressure_delta, 
    // "tank 2 pressure should be empty");
}

void thrust()
{
    ASSERT_FALSE(prop_system.is_enabled, 
    "interval timer should not be on when we have not yet started thrust");
    static const std::array<unsigned int, 4> schedule = {4, 2, 3, 6};
    ASSERT_FALSE(is_valve_open(0) || is_valve_open(1), 
    "Both tank valves should be closed");
    prop_system.tank2.set_schedule(schedule);
    prop_system.enable();
    delay(120); // outer valves have 120 seconds to finish thrust
    TEST_ASSERT_FALSE(prop_system.is_enabled);
    prop_system.disable();
}

// Test that opening inner valve increases the pressure
// Test that opening outer valves decrease pressure
void test_get_pressure()
{
    auto old_pressure = prop_system.tank2.get_pressure();
    prop_system.open_valve(prop_system.tank1, 0);
    delay(1000); // 1 seconds
    prop_system.close_valve(prop_system.tank1, 0);
    ASSERT_TRUE(prop_system.tank2.get_pressure() > old_pressure, 
    "pressure of tank 2 should be higher after filling");

    old_pressure = prop_system.tank2.get_pressure();
    reset_tank();
    ASSERT_TRUE(prop_system.tank2.get_pressure() < old_pressure,
    "pressure of tank 2 should be lower after firing");
}

// Test that the backup valve can fill the tank to threshold value
void test_backup_valve()
{
    auto old_pressure = prop_system.tank2.get_pressure();
    prop_system.open_valve(prop_system.tank1, 1);
    delay(1000);
    ASSERT_TRUE(is_valve_open(1), "tank 1 valve 1 should be opened");
    prop_system.close_valve(prop_system.tank1, 1);
    ASSERT_TRUE(prop_system.tank2.get_pressure() > old_pressure, 
    "pressure of tank 2 should be higher after filling");
    reset_tank();
    ASSERT_TRUE(prop_system.tank2.get_pressure() < old_pressure,
    "pressure of tank 2 should be lower after firing");
}

void test_get_temp_inner()
{
    // temperature increases with pressure
    reset_tank();
    auto old_temp = prop_system.tank1.get_temp();

    fill_tank();
    ASSERT_TRUE( prop_system.tank1.get_temp() < old_temp, 
    "temp of tank 1 should be lower after filling");
    old_temp = prop_system.tank1.get_temp();

    reset_tank();
    ASSERT_TRUE( prop_system.tank1.get_temp() > old_temp, 
    "temp of tank 1 should be the same after firing");
}

void test_get_temp_outer()
{
    reset_tank();
    auto old_temp = prop_system.tank2.get_temp();
    fill_tank();
    auto new_temp = prop_system.tank2.get_temp();
    ASSERT_TRUE(new_temp > old_temp, 
    "temp of tank 2 should be higher after filling");
    old_temp = new_temp;
    thrust();
    ASSERT_TRUE(prop_system.tank2.get_temp() < old_temp, 
    "temp of tank 2 should be lower after firing");
}

void setup() {
    delay(5000);
    Serial.begin(9600);
    pinMode(13, OUTPUT);
    prop_system.setup();
    UNITY_BEGIN();
    RUN_TEST(test_initialization);
    RUN_TEST(test_is_functional);
    RUN_TEST(test_open_tank1_valve);
    RUN_TEST(test_tank_lock);
    RUN_TEST(test_tank1_enforce_lock);
    RUN_TEST(test_set_thrust_valve_schedule);
    RUN_TEST(test_enable);
    RUN_TEST(test_disable);
    RUN_TEST(test_reset);
    RUN_TEST(test_get_pressure);
    RUN_TEST(test_backup_valve);
    RUN_TEST(test_get_temp_inner);
    RUN_TEST(test_get_temp_outer);
    UNITY_END();
}

void loop()
{

}