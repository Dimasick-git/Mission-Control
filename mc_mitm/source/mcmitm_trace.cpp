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
#include "mcmitm_trace.hpp"
#include <atomic>
#include <cstdarg>
#include <cstdio>

namespace ams::mitm::trace {

    namespace {

        constexpr const char TracePath[] = "sdmc:/config/MissionControl/sleep_trace.log";
        constexpr size_t LineCount = 128;
        constexpr size_t LineSize = 120;
        constexpr s64 MaxFileSize = 256_KB;

        constinit os::SdkMutex g_lock;
        constinit char g_lines[LineCount][LineSize];
        constinit size_t g_head = 0;
        constinit size_t g_count = 0;
        constinit u32 g_dropped = 0;
        // Only the trace thread flushes.
        constinit char g_flush_buffer[LineCount * (LineSize + 1) + 64];
        constinit std::atomic<bool> g_suspended = false;

        constexpr size_t ThreadStackSize = 0x4000;
        alignas(os::ThreadStackAlignment) constinit u8 g_thread_stack[ThreadStackSize];
        constinit os::ThreadType g_thread;

    }

    void Print(const char *fmt, ...) {
        const u64 ms = os::ConvertToTimeSpan(os::GetSystemTick()).GetMilliSeconds();

        std::scoped_lock lk(g_lock);
        if (g_count == LineCount) {
            ++g_dropped;
            return;
        }

        char *line = g_lines[(g_head + g_count) % LineCount];
        const int n = std::snprintf(line, LineSize, "%lu.%03lu ", static_cast<unsigned long>(ms / 1000), static_cast<unsigned long>(ms % 1000));
        va_list args;
        va_start(args, fmt);
        std::vsnprintf(line + n, LineSize - n, fmt, args);
        va_end(args);
        ++g_count;
    }

    void SetSuspended(bool suspended) {
        g_suspended = suspended;
    }

    void Flush() {
        char *buffer = g_flush_buffer;
        constexpr size_t BufferSize = sizeof(g_flush_buffer);
        size_t size = 0;
        {
            std::scoped_lock lk(g_lock);
            if (g_count == 0 && g_dropped == 0) {
                return;
            }
            for (; g_count > 0; --g_count, g_head = (g_head + 1) % LineCount) {
                size += std::snprintf(buffer + size, BufferSize - size, "%s\n", g_lines[g_head]);
            }
            if (g_dropped) {
                size += std::snprintf(buffer + size, BufferSize - size, "(dropped %u lines)\n", g_dropped);
                g_dropped = 0;
            }
        }

        fs::FileHandle file;
        if (R_FAILED(fs::OpenFile(std::addressof(file), TracePath, fs::OpenMode_Write | fs::OpenMode_AllowAppend))) {
            if (R_FAILED(fs::CreateFile(TracePath, 0)) ||
                R_FAILED(fs::OpenFile(std::addressof(file), TracePath, fs::OpenMode_Write | fs::OpenMode_AllowAppend))) {
                return;
            }
        }
        ON_SCOPE_EXIT { fs::CloseFile(file); };

        s64 offset = 0;
        if (R_FAILED(fs::GetFileSize(std::addressof(offset), file))) {
            return;
        }
        if (offset > MaxFileSize) {
            // Start over rather than growing without bound.
            if (R_FAILED(fs::SetFileSize(file, 0))) {
                return;
            }
            offset = 0;
        }
        static_cast<void>(fs::WriteFile(file, offset, buffer, size, fs::WriteOption::Flush));
    }

    namespace {

        void TraceThreadFunc(void *) {
            for (;;) {
                os::SleepThread(TimeSpan::FromSeconds(1));
                // While the console goes to sleep the SD card may already be suspending: keep lines for after wake.
                if (!g_suspended) {
                    Flush();
                }
            }
        }

    }

    void Initialize() {
        if (R_SUCCEEDED(os::CreateThread(std::addressof(g_thread), TraceThreadFunc, nullptr, g_thread_stack, ThreadStackSize, 21))) {
            os::SetThreadNamePointer(std::addressof(g_thread), "mc::TraceThread");
            os::StartThread(std::addressof(g_thread));
        }
    }

}
