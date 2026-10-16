/*
 * Copyright (c) 2020-2026 ndeadly
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "bluetooth_ble.hpp"
#include "../btdrv_mitm_flags.hpp"

namespace ams::bluetooth::ble {

    namespace {

        constinit os::SdkMutex g_event_data_lock;
        constinit bluetooth::BleEventInfo g_event_info;
        constinit bluetooth::BleEventType g_current_event_type;

        os::SystemEvent g_system_event;
        os::SystemEvent g_system_event_fwd(os::EventClearMode_AutoClear, true);
        os::SystemEvent g_system_event_user_fwd(os::EventClearMode_AutoClear, true);

        os::Event g_init_event(os::EventClearMode_ManualClear);
        os::Event g_data_read_event(os::EventClearMode_AutoClear);

    }

    bool IsInitialized() {
        return g_init_event.TryWait();
    }

    void SignalInitialized() {
        g_init_event.Signal();
    }

    void WaitInitialized() {
        g_init_event.Wait();
    }

    os::SystemEvent *GetSystemEvent() {
        return &g_system_event;
    }

    os::SystemEvent *GetForwardEvent() {
        return &g_system_event_fwd;
    }

    os::SystemEvent *GetUserForwardEvent() {
        return &g_system_event_user_fwd;
    }

    Result GetEventInfo(bluetooth::BleEventType *type, void *buffer, size_t size) {
        std::scoped_lock lk(g_event_data_lock);

        // The client may ask for more than our event struct (the bluetooth service's event can be larger on newer
        // firmware). Never copy past g_event_info: give what we have and zero the rest. Copying `size` bytes from
        // g_event_info handed the reader whatever globals follow it, and the result changed with every build.
        *type = g_current_event_type;
        const size_t copy = std::min(size, sizeof(g_event_info));
        std::memcpy(buffer, &g_event_info, copy);
        if (size > copy) {
            std::memset(static_cast<u8 *>(buffer) + copy, 0, size - copy);
        }

        g_data_read_event.Signal();
        
        R_SUCCEED();
    }

    void HandleEvent() {
        {
            std::scoped_lock lk(g_event_data_lock);
            R_ABORT_UNLESS(btdrvGetBleManagedEventInfo(&g_event_info, sizeof(bluetooth::BleEventInfo), &g_current_event_type));
        }

        if (!g_redirect_ble_events) {
            g_system_event_fwd.Signal();
            g_data_read_event.Wait();
        }

        if (g_system_event_user_fwd.GetBase()->state) {
            g_system_event_user_fwd.Signal();
        }
    }

}
