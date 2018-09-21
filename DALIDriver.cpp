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

#include "DALIDriver.h"

DALIDriver::DALIDriver(PinName out_pin, PinName in_pin, int baud, bool idle_state)
    : encoder(out_pin, in_pin, baud, idle_state)
{
}

DALIDriver::~DALIDriver() 
{
}

bool DALIDriver::add_to_group(uint8_t addr, uint8_t group)
{
    // Send the command to add to group
    send_twice(addr, ADD_TO_GROUP + group);
    // Query upper or lower bits of gearGroups 16 bit variable
    uint8_t cmd = group < 8 ? QUERY_GEAR_GROUPS_L : QUERY_GEAR_GROUPS_H;
    // Send query command
    send_command_standard(addr, cmd);
    // Receive gearGroups variable
    uint8_t resp = encoder.recv(); 
    // Group bit will be set if this light is a memeber of that group
    uint8_t mask = 1 << (group % 8);
    bool contained = resp & mask;
    // Return whether light is part of group
    return contained;
}

bool DALIDriver::remove_from_group(uint8_t addr, uint8_t group)
{
    // Send the command to remove from group
    send_twice(addr, REMOVE_FROM_GROUP + group);
    // Query upper or lower bits of gearGroups 16 bit variable
    uint8_t cmd = group < 8 ? QUERY_GEAR_GROUPS_L : QUERY_GEAR_GROUPS_H;
    // Send query command
    send_command_standard(addr, cmd);
    // Receive gearGroups variable
    uint8_t resp = encoder.recv(); 
    // Group bit will be set if this light is a memeber of that group
    uint8_t mask = 1 << (group % 8);
    bool contained = resp & mask;
    // Return whether light is not part of group
    return !contained;
}

void DALIDriver::set_level(uint8_t addr, uint8_t level) {
    send_command_direct(addr, level);
}

void DALIDriver::turn_off(uint8_t addr) {
    send_command_standard(addr, OFF);
}

uint8_t DALIDriver::get_level(uint8_t addr) {
    send_command_standard(addr, QUERY_ACTUAL_LEVEL);
    uint8_t resp = encoder.recv(); 
    return resp;
}

uint8_t DALIDriver::get_phm(uint8_t addr) {
    send_command_standard(addr, QUERY_PHM);
    uint8_t resp = encoder.recv(); 
    return resp;
}

uint8_t DALIDriver::get_fade(uint8_t addr) {
    send_command_standard(addr, QUERY_FADE);
    uint8_t resp = encoder.recv(); 
    return resp;
}

void DALIDriver::turn_on(uint8_t addr) {
    send_command_standard(addr, ON_AND_STEP_UP);
}

void DALIDriver::send_twice(uint8_t addr, uint8_t opcode) { 
    send_command_standard(addr, opcode);
    send_command_standard(addr, opcode);
}

void DALIDriver::set_fade_time(uint8_t addr, uint8_t time) {
    // Send twice command
    send_command_special(DTR0, time);
    send_twice(addr, SET_FADE_TIME); 
}

void DALIDriver::set_fade_rate(uint8_t addr, uint8_t rate) {
    // Send twice command
    send_command_special(DTR0, rate);
    send_twice(addr, SET_FADE_RATE);
}

void DALIDriver::set_scene(uint8_t addr, uint8_t scene, uint8_t level) {
    send_command_special(DTR0, level);
    // Send twice command
    send_twice(addr, SET_SCENE + scene); 
}

void DALIDriver::remove_from_scene(uint8_t addr, uint8_t scene) {
    send_twice(addr, REMOVE_FROM_SCENE + scene);
}

void DALIDriver::go_to_scene(uint8_t addr, uint8_t scene) {
    send_twice(addr, GO_TO_SCENE + scene);
}

void DALIDriver::attach(mbed::Callback<void(uint32_t)> status_cb) {
    encoder.attach(status_cb);
}

void DALIDriver::send_command_special(uint8_t address, uint8_t opcode) 
{
    encoder.send(((uint16_t)address << 8) | opcode);
}

void DALIDriver::send_command_special_input(uint8_t instance, uint8_t opcode) 
{
    encoder.send_24(((uint32_t)0xC1 << 16) | ((uint16_t)instance << 8) | opcode);
}

void DALIDriver::send_command_standard(uint8_t address, uint8_t opcode) 
{
    // Get the upper bit
    uint8_t mask = address & 0x80;
    // Change address to have 1 in LSb to signify 'standard command'
    address = mask | ((address << 1) + 1);
    encoder.send(((uint16_t)address << 8) | opcode);
}

void DALIDriver::send_command_direct(uint8_t address, uint8_t opcode) 
{
    // Get the upper bit
    uint8_t mask = address & 0x80;
    // Change address to have 0 in LSb to signify 'direct arc power'
    address = mask | (address << 1);
    encoder.send(((uint16_t)address << 8) | opcode);
}

bool DALIDriver::check_response(uint8_t expected) 
{
    int response = encoder.recv();
    if (response < 0) 
        return false;
    return (response == expected);
}

int DALIDriver::getIndexOfLogicalUnit(uint8_t addr)
{
    send_command_special(DTR1, 0x00);
    send_command_special(DTR0, 0x1A);
    send_command_special(READ_MEM_LOC, (addr << 1) + 1);
    return encoder.recv();
} 

void DALIDriver::set_search_address(uint32_t val) 
{
    send_command_special(SEARCHADDRH, val >> 16);
    send_command_special(SEARCHADDRM, (val >> 8) & (0x00FF));
    send_command_special(SEARCHADDRL, val & 0x0000FF);
}

void DALIDriver::set_search_address_input(uint32_t val) 
{
    send_command_special_input(0x05, val >> 16);
    send_command_special_input(0x06, (val >> 8) & (0x00FF));
    send_command_special_input(0x07, val & 0x0000FF);
}

uint8_t DALIDriver::get_group_addr(uint8_t group_number)
{
    uint8_t mask = 1 << 7;
    // Make MSb a 1 to signify >1 device being addressed
    return mask | group_number;
}

int DALIDriver::init() 
{
    // TODO: does this need to happen every time controller boots?
    num_logical_units = assign_addresses();
    num_logical_units += assign_addresses_input(true, num_logical_units);
    return num_logical_units;
}

int DALIDriver::get_highest_address() {
    int highestAssigned = -1;
    // Start initialization phase
    send_command_special(INITIALISE, 0x00);
    send_command_special(INITIALISE, 0x00);
    // Assign all units a random address
    send_command_special(RANDOMISE, 0x00);
    send_command_special(RANDOMISE, 0x00);
    wait_ms(100);

    bool found_non_mask =  false;

    while(true) {
        // Set the search address to the highest range
        set_search_address(0xFFFFFF);
        // Compare logical units search address to global search address
        send_command_special(COMPARE, 0x00);
        // Check if any device responds yes
        bool yes = check_response(YES);       
        // If no devices are unassigned (all withdrawn), we are done
        if (!yes) {
            break;
        }
        uint32_t searchAddr = 0xFFFFFF;
        for(int i = 23; i>=0; i--) {
            uint32_t mask = 1 << i;
            searchAddr = searchAddr & (~mask);
            // Set a new search address
            set_search_address(searchAddr);
            send_command_special(COMPARE, 0x00);
            // Check if any devices match
            bool yes = check_response(YES);
            if(!yes) {
                //No unit here, revert the mask
                searchAddr = searchAddr | mask;
            }
            // If yes, then we found at least one device
        }
        set_search_address(searchAddr);
        send_command_special(COMPARE, 0x00);
        yes = check_response(YES);
        if (yes) {
            // Get the current short address
            send_command_special(QUERY_SHORT_ADDR, 0x00);
            uint8_t short_addr = encoder.recv();
            if (short_addr != 0xFF) {
                short_addr = short_addr >> 1;
                if(short_addr > highestAssigned) { 
                    highestAssigned = short_addr;
                }
            }
            // Tell unit to withdraw (no longer respond to search queries)
            send_command_special(WITHDRAW, 0x00);
        }
        // Refresh initialization state
        send_command_special(INITIALISE, 0x00);
        send_command_special(INITIALISE, 0x00);
    }
		
    send_command_special(TERMINATE, 0x00);
    return highestAssigned;
}

// Return number of logical units on the bus
int DALIDriver::assign_addresses(bool reset) 
{
    int searchCompleted = false;
    uint8_t numAssignedShortAddresses = 0;
    int assignedAddresses[63] = {false}; 
    int highestAssigned = -1;

    if (!reset) {
        // Get the highest address already assigned
        int highest_addr = get_highest_address();
        if (highest_addr >= 0) {
            numAssignedShortAddresses = highest_addr + 1;
            for (int i = 0; i < numAssignedShortAddresses; i++) {
                assignedAddresses[i] = true;
            }
        }
    }
    // Start initialization phase for devices w/o a short address
    uint8_t opcode = reset ? 0x00 : 0xFF;
    send_command_special(INITIALISE, opcode);
    send_command_special(INITIALISE, opcode);
    // Assign all units a random address
    send_command_special(RANDOMISE, 0x00);
    send_command_special(RANDOMISE, 0x00);
    wait_ms(100);
    
    while(true) {
        // Set the search address to the highest range
        set_search_address(0xFFFFFF);
        // Compare logical units search address to global search address
        send_command_special(COMPARE, 0x00);
        // Check if any device responds yes
        bool yes = check_response(YES);       
        // If no devices are unassigned (all withdrawn), we are done
        if (!yes) {
            break;
        }
        if(numAssignedShortAddresses < 63) {
            uint32_t searchAddr = 0xFFFFFF;
            for(int i = 23; i>=0; i--) {
                uint32_t mask = 1 << i;
                searchAddr = searchAddr & (~mask);
                // Set a new search address
                set_search_address(searchAddr);
                send_command_special(COMPARE, 0x00);
                // Check if any devices match
                bool yes = check_response(YES);
                if(!yes) {
                    //No unit here, revert the mask
                    searchAddr = searchAddr | mask;
                }
                // If yes, then we found at least one device
            }
            set_search_address(searchAddr);
            send_command_special(COMPARE, 0x00);
            bool yes = check_response(YES);
            if (yes) {
                // We found a unit, let's program the short address with a new address
                // Give it a temporary short address
                uint8_t new_addr = numAssignedShortAddresses;
                if (new_addr < 63) {
                    if (assignedAddresses[new_addr] == true) {
                        // Duplicate addr?
                    }   
                    else {
                        // Program new address as short address
                        send_command_special(PROGRAM_SHORT_ADDR, (new_addr << 1) + 1);
                        // Tell unit to withdraw (no longer respond to search queries)
                        send_command_special(WITHDRAW, 0x00);
                        numAssignedShortAddresses++;
                        assignedAddresses[new_addr] = true;
                        if(new_addr > highestAssigned) 
                            highestAssigned = new_addr;
                    }
                }
                else {
                    // expected < 63 ?
                }
            }
            else {
                // No device found
            }
        }
        // Refresh initialization state
        send_command_special(INITIALISE, 0x00);
        send_command_special(INITIALISE, 0x00);
    }
		
    send_command_special(TERMINATE, 0x00);
    return numAssignedShortAddresses;

}

// Return number of logical units on the bus
int DALIDriver::assign_addresses_input(bool reset, int num_found) 
{
    int searchCompleted = false;
    uint8_t numAssignedShortAddresses = num_found;
    int assignedAddresses[63] = {false}; 
    int highestAssigned = -1;

    // Start initialization phase for devices w/o a short address
    uint8_t opcode = reset ? 0x00 : 0xFF;
    send_command_special_input(0x01, 0xFF);
    send_command_special_input(0x01, 0xFF);
    // Assign all units a random address
    send_command_special_input(0x02, 0x00);
    send_command_special_input(0x02, 0x00);
    wait_ms(100);
    
    while(true) {
        // Set the search address to the highest range
        set_search_address_input(0xFFFFFF);
        // Compare logical units search address to global search address
        send_command_special_input(0x03, 0x00);
        // Check if any device responds yes
        bool yes = check_response(YES);       
        // If no devices are unassigned (all withdrawn), we are done
        if (!yes) {
            break;
        }
        printf("hi\r\n");
        if(numAssignedShortAddresses < 63) {
            uint32_t searchAddr = 0xFFFFFF;
            for(int i = 23; i>=0; i--) {
                uint32_t mask = 1 << i;
                searchAddr = searchAddr & (~mask);
                // Set a new search address
                set_search_address_input(searchAddr);
                send_command_special_input(0x03, 0x00);
                // Check if any devices match
                bool yes = check_response(YES);
                if(!yes) {
                    //No unit here, revert the mask
                    searchAddr = searchAddr | mask;
                }
                // If yes, then we found at least one device
            }
            set_search_address_input(searchAddr);
            send_command_special_input(0x03, 0x00);
            bool yes = check_response(YES);
            if (yes) {
                // We found a unit, let's program the short address with a new address
                // Give it a temporary short address
                uint8_t new_addr = numAssignedShortAddresses;
                if (new_addr < 63) {
                    if (assignedAddresses[new_addr] == true) {
                        // Duplicate addr?
                    }   
                    else {
                        // Program new address as short address
                        send_command_special_input(0x08, (new_addr << 1) + 1);
                        // Tell unit to withdraw (no longer respond to search queries)
                        send_command_special_input(0x04, 0x00);
                        numAssignedShortAddresses++;
                        assignedAddresses[new_addr] = true;
                        if(new_addr > highestAssigned) 
                            highestAssigned = new_addr;
                    }
                }
                else {
                    // expected < 63 ?
                }
            }
            else {
                // No device found
            }
        }
        // Refresh initialization state
        send_command_special_input(0x01, 0x00);
        send_command_special_input(0x01, 0x00);
    }
		
    send_command_special_input(0x00, 0x00);
    return numAssignedShortAddresses;

}
