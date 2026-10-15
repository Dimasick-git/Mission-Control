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
#pragma once
#include <stratosphere.hpp>

// Sleep diagnostics: power state requests, bluetooth interface calls and events that wait on their reader.
// Lines go to a ring in memory; a trace thread flushes them to sdmc:/config/MissionControl/sleep_trace.log once a
// second (not while suspended), so no caller touches the SD card.
namespace ams::mitm::trace {

    void Print(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
    void Initialize();
    void SetSuspended(bool suspended);
    void Flush();

}
