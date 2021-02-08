/* mbed Microcontroller Library
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
 
#include <mbed.h>
#include "ble/BLE.h"
extern "C"{
  #include "SEGGER_RTT.h"
}

/* for seeding random number generator */
static bool seeded = false;

inline void print_error(ble_error_t error, const char* msg)
{
    SEGGER_RTT_printf(0, "%s: ", msg);
    switch(error) {
        case BLE_ERROR_NOT_FOUND:
            SEGGER_RTT_printf(0, "BLE_ERROR_NOT_FOUND: Not found");
            break;
        case BLE_ERROR_NONE:
            SEGGER_RTT_printf(0, "BLE_ERROR_NONE: No error");
            break;
        case BLE_ERROR_BUFFER_OVERFLOW:
            SEGGER_RTT_printf(0, "BLE_ERROR_BUFFER_OVERFLOW: The requested action would cause a buffer overflow and has been aborted");
            break;
        case BLE_ERROR_NOT_IMPLEMENTED:
            SEGGER_RTT_printf(0, "BLE_ERROR_NOT_IMPLEMENTED: Requested a feature that isn't yet implement or isn't supported by the target HW");
            break;
        case BLE_ERROR_PARAM_OUT_OF_RANGE:
            SEGGER_RTT_printf(0, "BLE_ERROR_PARAM_OUT_OF_RANGE: One of the supplied parameters is outside the valid range");
            break;
        case BLE_ERROR_INVALID_PARAM:
            SEGGER_RTT_printf(0, "BLE_ERROR_INVALID_PARAM: One of the supplied parameters is invalid");
            break;
        case BLE_STACK_BUSY:
            SEGGER_RTT_printf(0, "BLE_STACK_BUSY: The stack is busy");
            break;
        case BLE_ERROR_INVALID_STATE:
            SEGGER_RTT_printf(0, "BLE_ERROR_INVALID_STATE: Invalid state");
            break;
        case BLE_ERROR_NO_MEM:
            SEGGER_RTT_printf(0, "BLE_ERROR_NO_MEM: Out of Memory");
            break;
        case BLE_ERROR_OPERATION_NOT_PERMITTED:
            SEGGER_RTT_printf(0, "BLE_ERROR_OPERATION_NOT_PERMITTED");
            break;
        case BLE_ERROR_INITIALIZATION_INCOMPLETE:
            SEGGER_RTT_printf(0, "BLE_ERROR_INITIALIZATION_INCOMPLETE");
            break;
        case BLE_ERROR_ALREADY_INITIALIZED:
            SEGGER_RTT_printf(0, "BLE_ERROR_ALREADY_INITIALIZED");
            break;
        case BLE_ERROR_UNSPECIFIED:
            SEGGER_RTT_printf(0, "BLE_ERROR_UNSPECIFIED: Unknown error");
            break;
        case BLE_ERROR_INTERNAL_STACK_FAILURE:
            SEGGER_RTT_printf(0, "BLE_ERROR_INTERNAL_STACK_FAILURE: internal stack faillure");
            break;
    }
    SEGGER_RTT_printf(0, "\r\n");
}
 
/** print device address to the terminal */
inline void print_address(const uint8_t *addr)
{
    SEGGER_RTT_printf(0, "%02x:%02x:%02x:%02x:%02x:%02x\r\n",
           addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
}
 
inline void print_mac_address()
{
    /* Print out device MAC address to the console*/
    ble::own_address_type_t addr_type;
    ble::address_t address;
    BLE::Instance().gap().getAddress(addr_type, address);
    SEGGER_RTT_printf(0, "DEVICE MAC ADDRESS: ");
    print_address((uint8_t *)&address);
 
    if (!seeded) {
        seeded = true;
        /* use the address as a seed */
        uint8_t* random_data = (uint8_t*)&address;
        srand(*((unsigned int*)random_data));
    }
}
 
inline const char* phy_to_string(ble::phy_t phy) {
    switch(phy.value()) {
        case ble::phy_t::LE_1M:
            return "LE 1M";
        case ble::phy_t::LE_2M:
            return "LE 2M";
        case ble::phy_t::LE_CODED:
            return "LE coded";
        default:
            return "invalid PHY";
    }
}
 
