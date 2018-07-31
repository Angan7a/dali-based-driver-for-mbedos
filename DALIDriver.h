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

#include "mbed.h"
#include "manchester/encoder.h"

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
    COMPARE = 0xA9,
    WITHDRAW = 0xAB
};

// Command op codes 
enum CommandOpCodes {
    READ_MEM_LOC = 0xC5
};

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
    DALIDriver(PinName out_pin, PinName in_pin, int baud = 1200, bool idle_state = 1);

    /** Initialise the driver 
    *
    *   @returns    true if successful 
    *       
    */
    bool init();

private:

    /** Assign addresses to the logical units on the bus 
    *
    *   @returns    The number of logical units found on bus
    *
    *   NOTE: This process is copied from page 82 of iec62386-102
    */ 
    int assign_addresses();

    /** Set the controller search address 
    * This address will be used in search commands to determine what 
    * control units this address or a numerically lower address
    *
    *   @param val    Search address valued (only lower 24 bits are used)
    *
    */ 
    void set_search_address(uint32_t val);

    /** Send a command on the bus 
    *
    *   @param address     The address byte for command
    *   @param opcode      The opcode byte 
    *
    */ 
    void send_command(uint8_t address, uint8_t opcode);

    
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

    // The encoder for the bus signals
    ManchesterEncoder encoder;
    // The number of logical units on the bus
    int num_logical_units;
};

#endif
