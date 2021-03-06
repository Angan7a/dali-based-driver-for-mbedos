/* DALI Driver
 * Copyright (c) 2018 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DALI_DRIVER_H
#define DALI_DRIVER_H

#include "manchester/encoder.h"
#include "mbed.h"

// Special commands that do not address a specific device
// These values will be used as address byte in DALI command
enum SpecialCommandOpAddr {
    SEARCHADDRH = 0xB1,
    SEARCHADDRM = 0xB3,
    SEARCHADDRL = 0xB5,
    DTR0 = 0xA3,
    DTR1 = 0xC3,
    DTR2 = 0xC5,
    INITIALISE = 0xA5,
    RANDOMISE = 0xA7,
    PROGRAM_SHORT_ADDR = 0xB7,
    QUERY_SHORT_ADDR = 0xBB,
    COMPARE = 0xA9,
    TERMINATE = 0xA1,
    ENABLE_DEVICE_TYPE = 0xC1,
    WITHDRAW = 0xAB
};

// Command op codes
enum CommandOpCodes {
    GO_TO_SCENE = 0x10,
    OFF = 0x00,
    ON_AND_STEP_UP = 0x08,
    QUERY_GEAR_GROUPS_L = 0xC0, // get lower byte of gear groups status
    QUERY_GEAR_GROUPS_H = 0xC1, // get upper byte of gear groups status
    QUERY_ACTUAL_LEVEL = 0xA0,
    QUERY_ERROR = 0x90,
    QUERY_PHM = 0x9A,
    QUERY_FADE = 0xA5,
    QUERY_COLOR_TYPE_FEATURES = 0xF9,
    QUERY_SCENE_LEVEL = 0xB0,
    READ_MEM_LOC = 0xC5,
    SET_TEMP_RGB_DIM = 0xEB,
    SET_TEMP_TEMPC = 0xE7,
    SET_TEMP_WAF_DIM = 0xEC,
    COLOR_ACTIVATE = 0xE2,

    // Commands below are "send twice"
    SET_SCENE = 0x40,
    SET_FADE_TIME = 0x2E,
    SET_FADE_RATE = 0x2F,
    SET_MIN_LEVEL = 0x2B,
    REMOVE_FROM_SCENE = 0x50,
    REMOVE_FROM_GROUP = 0x70,
    STORE_DTR_AS_SCENE =0x40,
    ADD_TO_GROUP = 0x60,
    SET_SHORT_ADDR = 0x80,
    SET_MAX_LEVEL = 0x2A
};

enum InstanceType { GENERIC = 0, OCCUPANCY = 3, LIGHT = 4, BUTTON = 1 };
enum ColorType { RGB, TEMPERATURE, UNSUPPORTED };

#define YES 0xFF

class DALIDriver {
public:
    /** Constructor DALIDriver
     *
     *  @param out_pin      Output pin for the DALI commands
     *  @param in_pin       Input pin for responses from DALI slaves
     *  @param baud         Signal baud rate
     *  @param idle_state   The default state of the line (high for DALI)
     */
    DALIDriver(PinName out_pin, PinName in_pin, int baud = 1200,
               bool idle_state = 0);

    ~DALIDriver();

    /** Initialise the driver
     *
     *   @returns    the number of logical units on the bus
     *
     */
    int init();

    /** Initialise the luminaires on the bus (give them addresses)
     *
     *   @returns    the number of luminaires on the bus
     *
     */
    int init_lights();

    /** Initialise the inputs on the bus (give them addresses)
     *
     *   @returns    the number of input devices on the bus
     *
     */
    int init_inputs();

    /** Attach a callback when input event is generated
     *
     *   @param status_cb callback to take in the 32 bit event message
     */
    void attach(mbed::Callback<void(uint32_t)> status_cb);

    /** Detach the callback
     */
    void detach();

    /** Reattach the callback
     */
    void reattach();

    /** Send a standard command on the bus
     *
     *   @param address     The address byte for command
     *   @param opcode      The opcode byte
     *
     */
    void send_command_standard(uint8_t address, uint8_t opcode);

    /** Send a standard command on the bus to input devices
     *
     *   @param address      The address byte for command
     *   @param instance     The instance byte for command
     *   @param opcode       The opcode byte
     *
     */
    void send_command_standard_input(uint8_t address, uint8_t instance,
                                     uint8_t opcode);

    /** Send a special command on the bus
     *
     *   @param address     The special command opcode from SpecialCommandOpAddr
     * enum
     *   @param opcode      The data for the command
     *
     */
    void send_command_special(uint8_t address, uint8_t opcode);

    /** Send a special command on the bus to input devices
     *
     *   @param instance     The instance byte for command
     *   @param opcode       The opcode byte
     *
     */
    void send_command_special_input(uint8_t instance, uint8_t opcode);

    /** Send a direct arc power command on the bus
     *
     *   @param address     The address byte for command
     *   @param opcode      The opcode byte
     *
     */
    void send_command_direct(uint8_t address, uint8_t opcode);

    /** Get the address of a group
     *
     *   @param group_number    The group number [0-15]
     *   @returns
     *       8 bit address of the group
     *
     */
    uint8_t get_group_addr(uint8_t group_number);

    /** Add a device to a group
     *
     *   @param addr    8 bit device address
     *   @param group   The group number [0-15]
     *   @returns
     *       true if command success
     *
     */
    bool add_to_group(uint8_t addr, uint8_t group);

    /** Remove a device from a group
     *
     *   @param addr    8 bit device address
     *   @param group   The group number [0-15]
     *   @returns
     *       true if command success
     *
     */
    bool remove_from_group(uint8_t addr, uint8_t group);

    /** Set the light output for a device/group
     *
     *   @param addr    8 bit address (device or group)
     *   @param level   Light output level [0,254]
     *   NOTE: Refer to section 9.3 of iec62386-102 for dimming curve
     *
     */
    void set_level(uint8_t addr, uint8_t level);

    /** Turn a device/group off
     *
     *   @param addr    8 bit address (device or group)
     *
     */
    void turn_off(uint8_t addr);

    /** Turn a device/group on
     *
     *   @param addr    8 bit address (device or group)
     *
     */
    void turn_on(uint8_t addr);

    /** Get the current light level
     *
     *   @param addr    8 bit address (device or group)
     *   @returns
     *       Light level [0, 254] from QUERY ACTUAL LEVEL command
     *
     */
    uint8_t get_level(uint8_t addr);

    /** Get the current error status
     *
     *   @param addr    8 bit address (device or group)
     *   @returns
     *       Error status from QUERY ERROR command
     *
     */
    uint8_t get_error(uint8_t addr);

    /** Get the fade time and fade rate
     *
     *   @param addr    8 bit address (device or group)
     *   @returns
     *       The 8 bit representation of the fade time/fade rate from the QUERY
     * FADE TIME/FADE RATE command The answer shall be XXXX YYYYb, where XXXXb
     * equals fadeTime and YYYYb equals fadeRate
     *
     */
    uint8_t get_fade(uint8_t addr);

    /** Get the number of instances on an input device
     *
     * @param addr   8 bit address of the input device
     * @returns
     *       The number of instances on an input device 0 to 31
     *
     */
    uint32_t query_instances(uint8_t addr);

    /** Get the color type features
    *
    * @param addr 8 bit address of the light
    *
    * @returns 
    *       8 bit number represnting color type features (page 38 iec62386-209
    *   bit 0       xy-coordinate capable       (0 = No) 
    *   bit 1       Color temperature capable   (0 = No)
    *   bit 2..4    Number of primary colors    ([0, 6])
    *   bit 5..7    Number RGBWAF channels      ([0,6])
    *
    */
    uint8_t query_color_type_features(uint8_t addr);

    ColorType get_color_type(uint8_t addr);

    /** Query if the light is capable of color temperature
    *
    * @param addr 8 bit address of the light
    *
    * @returns boolean representing support 
    *
    */ 
    bool query_temperature_capable(uint8_t addr);

    /** Query number of rgbwaf channels 
    *
    * @param addr 8 bit address of the light
    *
    * @returns integer number of channels 
    *
    */ 
    uint8_t query_rgbwaf_channels(uint8_t addr);

    /** Set the color
    *
    *   @param addr 8 bit address of the light
    *   @param r    level of red [0,254]
    *   @param g    level of green [0,254]
    *   @param b    level of blue [0,254]
    *   @param dim  level of dim [0,254]
    *
    */ 
    void set_color(uint8_t addr, uint8_t r, uint8_t g, uint8_t b, uint8_t dim = 0);

    /** Set the color scene
    *
    *   @param addr 8 bit address of the light
    *   @param addr 8 bit scene number 
    *   @param r    level of red [0,254]
    *   @param g    level of green [0,254]
    *   @param b    level of blue [0,254]
    *   @param dim  level of dim [0,254]
    *
    */ 
    void set_color_scene(uint8_t addr, uint8_t scene, uint8_t r, uint8_t g, uint8_t b, uint8_t dim = 0);

    /** Set the color
    *
    *   @param addr     8 bit address of the light
    *   @param temp     light temperature in kelvin [2500,7042]
    *
    */
    void set_color(uint8_t addr, uint16_t temp);

    /** Set the color scene
    *
    *   @param addr     8 bit address of the light
    *   @param addr 8 bit scene number 
    *   @param temp     light temperature in kelvin [2500,7042]
    *
    */
    void set_color_scene(uint8_t addr, uint8_t scene, uint16_t temp);


    /** Set the event scheme -- section 9.6.3 of iec62386-103
     * 0 (default) -Instance addressing, using instance type and number.
     * 1 - Device addressing, using short address and instance type.
     * 2 - Device/instance addressing, using short address and instance number.
     * 3 - Device group addressing, using device group and instance type.
     * 4 - Instance group addressing, using instance group and type.
     *
     *   @param address      The address byte for command
     *   @param instance     The instance byte for command
     *   @param scheme        The scheme [0, 4]
     *
     */
    void set_event_scheme(uint8_t addr, uint8_t inst, uint8_t scheme);

    /** Set the event scheme -- section 9.6.4 of iec62386-103
     *
     *   @param address      The address byte for command
     *   @param instance     The instance byte for command
     *   @param filter       The filter for DTR0
     *
     */
    void set_event_filter(uint8_t addr, uint8_t inst, uint8_t filter);

    /** Get the instance type -- 9.4.3 of iec62386-103
     *
     *   @param address      The address byte for command
     *   @param instance     The instance byte for command
     *   @returns
     *       The instance type number [0,31], see InstanceType enum for values
     *
     */
    uint8_t get_instance_type(uint8_t addr, uint8_t inst);

    /** Get the instance status
     *
     *   @param address      The address byte for command
     *   @param instance     The instance byte for command
     *   @returns
     *       status -- 255 for enabled, 0 for disabled
     *
     */
    uint8_t get_instance_status(uint8_t addr, uint8_t inst);

    /** Disable the instance
     *
     *   @param address      The address byte for command
     *   @param instance     The instance byte for command
     *
     */
    void disable_instance(uint8_t addr, uint8_t inst);

    /** Enable the instance
     *
     *   @param address      The address byte for command
     *   @param instance     The instance byte for command
     *
     */
    void enable_instance(uint8_t addr, uint8_t inst);

    /** Get the temperature from a sensor
     *
     *   @param address      The address byte for command
     *   @param instance     The instance byte for command
     *   @returns
     *       The temperature in celcius
     *
     */
    float get_temperature(uint8_t addr, uint8_t instance);

    /** Get the humidity from a sensor
     *
     *   @param address      The address byte for command
     *   @param instance     The instance byte for command
     *   @returns
     *       The humidity percentage
     *
     */
    float get_humidity(uint8_t addr, uint8_t instance);

    /** Set quiet mode status (event messages on/off
     *
     * @param on     whether quiet mode is on or off
     *
     */
    void quiet_mode(bool on);

    /** Get the physical minimum
     *
     *   @param addr    8 bit address (device or group)
     *   @returns
     *       Physical minimum light output the control gear can operate at [0,
     * 254] from QUERY PHYSICAL MINIMUM command
     *
     */
    uint8_t get_phm(uint8_t addr);

    /** Set the fade rate for a device/group
     *
     *   @param addr    8 bit address (device or group)
     *   @param rate    Fade rate [1, 15]
     *   NOTE: Refer to section 9.5.3 of iec62386-102 for fade rate calculation
     *
     */
    void set_fade_rate(uint8_t addr, uint8_t rate);

    /** Set the fade time for a device/group
     *
     *   @param addr    8 bit address (device or group)
     *   @param rate    Fade time [1, 15]
     *   NOTE: Refer to section 9.5.2 of iec62386-102 for fade time calculation
     *
     */
    void set_fade_time(uint8_t addr, uint8_t time);

    /** Set the light output for a scene
     *
     *   @param addr    8 bit address (device or group)
     *   @param scene   scene number [0, 15]
     *   @param level   Light output level [0,254]
     *
     */
    void set_scene(uint8_t addr, uint8_t scene, uint8_t level);

    /** Remove device/group from scene
     *
     *   @param addr    8 bit address (device or group)
     *   @param scene   scene number [0, 15]
     *
     */
    void remove_from_scene(uint8_t addr, uint8_t scene);

    /** Go to a scene
     *
     *   @param addr    8 bit address (device or group)
     *   @param scene   scene number [0, 15]
     *
     */
    void go_to_scene(uint8_t addr, uint8_t scene);

    /** Call recv on the bus
     *
     *   @returns    the messagein the recv buffer for the bus (encoder class)
     *
     */
    uint32_t recv();

    /** Parse the event message
     *
     *   @param msg      the 32 bit event message
     *   @returns        the event_msg struct containing addr, instance type,
     * and event info
     *
     */
    event_msg parse_event(uint32_t msg);

    static const uint8_t broadcast_addr = 0xFF;

    // The encoder for the bus signals
    ManchesterEncoder encoder;

    int get_num_lights()
    {
        return num_lights;
    }

    int get_num_inputs()
    {
        return num_inputs;
    }

    int get_input_addr_start()
    {
        return num_lights;
    }

private:
    void set_color_temp(uint8_t addr, uint16_t temp);
    void set_color_temp(uint8_t addr, uint8_t r, uint8_t g, uint8_t b, uint8_t dim = 0);

    // Some commands must be sent twice, utility function to do that
    void send_twice(uint8_t addr, uint8_t opcode);

    /** Assign addresses to the luminaires on the bus
     *
     *   @returns    The number of input devices found on bus
     *
     *   NOTE: This process is mostly copied from page 82 of iec62386-102
     *   The addresses will be in the range [0, number units - 1]
     */
    int assign_addresses(bool reset = false);

    /** Assign addresses to the input devices on the bus
     *
     *   @param num_found    The number of luminaires already found (so that the
     * starting address is last_luminaire + 1)
     *   @returns            The number of unput devices found on bus
     *
     *   NOTE: This process is mostly copied from page 82 of iec62386-102
     *   The addresses will be in the range [0, number units - 1]
     */
    int assign_addresses_input(bool reset = false, int num_found = 0);

    /** Assign addresses to the logical units on the bus
     *
     *   @returns    The number of logical units found on bus
     *
     *   NOTE: This process is mostly copied from page 82 of iec62386-102
     *   The addresses will be in the range [0, number units - 1]
     */
    int get_highest_address();

    /** Set the controller search address for luminaires
     * This address will be used in search commands to determine what
     * control units have this address or a numerically lower address
     *
     *   @param val    Search address valued (only lower 24 bits are used)
     *
     */
    void set_search_address(uint32_t val);

    /** Set the controller search address  for input devices
     * This address will be used in search commands to determine what
     * control units have this address or a numerically lower address
     *
     *   @param val    Search address valued (only lower 24 bits are used)
     *
     */
    void set_search_address_input(uint32_t val);

    /** Check the response from the bus
     *
     *   @param expected    Expected response from the bus
     *   @returns
     *       True if response matches expected response
     *
     */
    bool check_response(uint8_t expected);

    /** Get the index of a control unit
     *
     *   @param addr     The address of the device
     *   @returns        The index of the device
     */
    int getIndexOfLogicalUnit(uint8_t addr);

    // The number of logical units on the bus
    int num_logical_units;
    // Number of luminaires on the bus
    int num_lights;
    // Number of input devices on the bus
    int num_inputs;
    // Address where input devices start
    int inputs_start;
};

#endif
