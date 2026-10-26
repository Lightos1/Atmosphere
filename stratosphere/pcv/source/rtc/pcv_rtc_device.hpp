/*
 * Copyright (c) Atmosphère-NX
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
#include "pcv_rtc_registers.hpp"
#include "pcv_rtc_i2c.hpp"
#include "pcv_rtc_reset_flag.hpp"

namespace ams::pcv::rtc {

    struct RtcRegisters {
        u32 sec;
        u32 min;
        u32 hour;
        u32 weekday;
        u32 month;
        u32 year;
        u32 day;
    };

    struct HwTimeRegisters {
        u8 sec;
        u8 min;
        u8 hour;
        u8 weekday;
        u8 month;
        u8 year;
        u8 day;
    };
    static_assert(sizeof(HwTimeRegisters) == Max77620RtcTimeRegCount);

    class RtcDevice {
        public:
            static constexpr int AlarmCount = 2;
        private:
            static constexpr HwTimeRegisters DefaultTimeRegisters = {
                .sec     = 0,
                .min     = 0,
                .hour    = 0,
                .weekday = 1,
                .month   = 1,
                .year    = 0,
                .day     = 1,
            };
        private:
            static constexpr s32 SecondsPerMinute = 60;
            static constexpr s32 SecondsPerHour   = SecondsPerMinute * 60;
            static constexpr s32 SecondsPerDay    = SecondsPerHour * 24;

            static constexpr s32 MinutesPerHour = 60;
            static constexpr s32 HoursPerDay    = 24;

            static constexpr s32 MonthsPerYear = 12;
            static constexpr s32 March         = 3;

            /* Accounting for leap years: 4 years = 1461 days, a century = 36525 days. */
            static constexpr s32 DaysPerYear           = 365;
            static constexpr s32 DaysPer4Years         = DaysPerYear   * 4 + 1;
            static constexpr s32 DaysPerCentury        = DaysPer4Years * 25;

            static constexpr s32 DaysPer400Years       = DaysPerCentury * 4 - 3;
            static constexpr s32 JulianDaysPer400Years = DaysPerCentury * 4;
            static constexpr s32 LeapDayInQuad         = DaysPer4Years - 1;
            static constexpr s32 YearsPerLeapBlock     = 4;
            static constexpr s32 CenturiesPerLeapCycle = 4;
            static constexpr s32 YearsPerCentury       = 100;
            static constexpr s32 YearsPer400Years      = YearsPerCentury * CenturiesPerLeapCycle;

            /* Five months starting in March are 153 days long. */
            static constexpr s32 DaysPer5Months  = 31 + 30 + 31 + 30 + 31;
            static constexpr s32 MonthsPerPeriod = 5;
            static constexpr s32 MonthRounding   = 2;

            /* RTC counts years from 2000, and 2000-03-01 is 31 + 29 days after start of the new year. */
            static constexpr s32 DaysFromEpochToFirstMarch = 60;

            /* Using march as the first month. */
            static constexpr s32 MarchBasedFebruaryIdx = 11;
            static constexpr s32 MarchBasedDecemberIdx = 9;

            static constexpr s32 LeapDayOfMonth = 29;

            /* NOTE: Nintendo uses an 0x1F mask for hours, might be a typo? */
            static constexpr u8 TimeHourMask = 0x1F;
        private:
            i2c::I2cSession &m_session;
            os::SdkMutex    m_lock;
            bool            m_ready = false;
            bool            m_alarm_enabled[AlarmCount] = {};
            bool            m_reset_on_shutdown = false;
        private:
            Result ReadRtcRegisters(u8 reg, u8 count, u8 *out) {
                std::scoped_lock lock(m_lock);
                constexpr u32 MaxAttempts = 4;

                /* Trigger an update. */
                R_TRY(I2cWrite(m_session, Max77620RtcUpdate0Reg, Max77620RtcReadUpdate));

                /* Wait until ready. */
                bool ready = false;
                for (u32 attempt = 0; attempt < MaxAttempts; ++attempt) {
                    svc::SleepThread(20'000'000);

                    const u8 cmd = Max77620RtcUpdate1Reg;
                    u8 update    = 0;
                    R_TRY(I2cRead(m_session, std::addressof(update), sizeof(update), std::addressof(cmd), sizeof(cmd)));

                    if ((update & Max77620RtcUpdate1ReadReady) != 0) {
                        ready = true;
                        break;
                    }
                }
                R_UNLESS(ready, ResultRtcAccessFailed());

                /* Read the requested registers. */
                for (u8 i = 0; i < count; ++i) {
                    const u8 cmd = reg + i;
                    R_TRY(I2cRead(m_session, out + i, sizeof(u8), std::addressof(cmd), sizeof(cmd)));
                }

                return ResultSuccess();
            }

            Result WriteRtcRegisters(u8 reg, u8 count, const u8 *data) {
                std::scoped_lock lock(m_lock);

                /* Write the data. */
                for (u8 i = 0; i < count; ++i) {
                    R_TRY(I2cWrite(m_session, reg + i, data[i]));
                }

                /* Trigger an update. */
                R_TRY(I2cWrite(m_session, Max77620RtcUpdate0Reg, Max77620RtcWriteUpdate));

                /* Wait until the write flag is cleared. */
                u8 out = 0;
                R_TRY(I2cReadUntilFlagClear(m_session, out, Max77620RtcUpdate0Reg, Max77620RtcWriteUpdate));

                return ResultSuccess();
            }

            ALWAYS_INLINE void ClearAlarmEnableBits(HwTimeRegisters &alarm) {
                alarm.sec     &= Max77620RtcSecValueMask;
                alarm.min     &= Max77620RtcMinValueMask;
                alarm.hour    &= Max77620RtcHourValueMask;
                alarm.weekday &= Max77620RtcWeekdayValueMask;
                alarm.month   &= Max77620RtcMonthValueMask;
                alarm.year    &= Max77620RtcYearValueMask;
                alarm.day     &= Max77620RtcDayValueMask;
            }

            ALWAYS_INLINE void SetAlarmEnableBits(HwTimeRegisters &alarm) {
                alarm.sec   |= Max77620RtcAlarmEnMask;
                alarm.min   |= Max77620RtcAlarmEnMask;
                alarm.hour  |= Max77620RtcAlarmEnMask;
                alarm.month |= Max77620RtcAlarmEnMask;
                alarm.year  |= Max77620RtcAlarmEnMask;
                alarm.day   |= Max77620RtcAlarmEnMask;
            }

            ALWAYS_INLINE u8 GetAlarmRegister(u32 alarm_id) const {
                return alarm_id ? Max77620RtcAlarm2Reg : Max77620RtcAlarm1Reg;
            }

            ALWAYS_INLINE RtcRegisters ConvertToRtcRegisters(u64 in_seconds) {
                /* NOTE: Nintendo truncates the input to 32 bits. */
                const s32 total_seconds  = static_cast<s32>(in_seconds);
                const s32 days           = total_seconds / SecondsPerDay;
                const s32 seconds_of_day = total_seconds % SecondsPerDay;

                /* Split the days since March 1st into 400-year blocks, rounding down for dates before 2000-03-01. */
                /* NOTE: Nintendo uses 146100 (4 * DaysPerCentury) here, which treats 2100 as a leap year (obviously incorrect). */
                /* This doesn't matter though, because the RTC year only covers up to 2099. */
                const s32 days_since_march = days - DaysFromEpochToFirstMarch;

                s32 block        = days_since_march / JulianDaysPer400Years;
                s32 day_in_block = days_since_march % JulianDaysPer400Years;
                if (day_in_block < 0) {
                    --block;
                    day_in_block += JulianDaysPer400Years;
                }

                /* Split the block into centuries, 4-year groups and years. */
                const s32 century        = day_in_block / DaysPerCentury;
                const s32 day_in_century = day_in_block % DaysPerCentury;
                const s32 quad           = day_in_century / DaysPer4Years;
                const s32 day_in_quad    = day_in_century % DaysPer4Years;
                const s32 year_in_quad   = day_in_quad / DaysPerYear;
                const s32 day_in_year    = day_in_quad % DaysPerYear;

                /* Feb 29th is the very last day of a 4-year group, and would otherwise look like day 0 of year 4. */
                const bool is_leap_day = (day_in_quad == LeapDayInQuad);

                /* Month counted from March (0) to February (11). */
                const s32 march_month = is_leap_day ? MarchBasedFebruaryIdx : (MonthsPerPeriod * day_in_year + MonthRounding) / DaysPer5Months;
                const s32 day_of_month = is_leap_day ? LeapDayOfMonth : (day_in_year + 1) - (DaysPer5Months * march_month + MonthRounding) / MonthsPerPeriod;

                /* Years counted from March 2000. January and February belong to the next calendar year. */
                s32 year = YearsPer400Years * block + YearsPerCentury * century + YearsPerLeapBlock * quad + year_in_quad - (is_leap_day ? 1 : 0);
                s32 month;
                if (march_month > MarchBasedDecemberIdx) {
                    ++year;
                    month = march_month - MarchBasedDecemberIdx;
                } else {
                    month = march_month + March;
                }

                RtcRegisters time = {};
                time.sec     = seconds_of_day % SecondsPerMinute;
                time.min     = (seconds_of_day % SecondsPerHour) / SecondsPerMinute;
                time.hour    = seconds_of_day / SecondsPerHour;
                time.weekday = 0;
                time.day     = day_of_month;
                time.month   = month;
                time.year    = year % 100;

                return time;
            }

            Result IsRtcModeConfigured(bool *out) {
                u8 control = 0;
                R_TRY(ReadRtcRegisters(Max77620RtcControlReg, 1, std::addressof(control)));

                *out = (control & (Max77620RtcBinFormat | Max77620Rtc24h)) == Max77620Rtc24h;

                return ResultSuccess();
            }

            Result ResetToDefaults() {
                const u8 *defaults = reinterpret_cast<const u8 *>(std::addressof(DefaultTimeRegisters));

                R_TRY(WriteRtcRegisters(Max77620RtcSecReg,    sizeof(DefaultTimeRegisters), defaults));
                R_TRY(WriteRtcRegisters(Max77620RtcAlarm1Reg, sizeof(DefaultTimeRegisters), defaults));
                R_TRY(WriteRtcRegisters(Max77620RtcAlarm2Reg, sizeof(DefaultTimeRegisters), defaults));

                return ResultSuccess();
            }

            Result SetAlarmEnabledImpl(u32 alarm_id, bool enable) {
                AMS_ASSERT(alarm_id < AlarmCount);

                /* Get the interrupt mask. */
                u8 interrupt_mask = 0;
                R_TRY(ReadRtcRegisters(Max77620RtcIntMReg, 1, std::addressof(interrupt_mask)));

                const u8 alarm_mask     = alarm_id ? Max77620RtcAlarm2Mask : Max77620RtcAlarm1Mask;
                const u8 alarm_register = GetAlarmRegister(alarm_id);

                /* A set bit masks the interrupt. */
                if (enable) {
                    interrupt_mask &= ~alarm_mask;
                } else {
                    interrupt_mask |= alarm_mask;
                }

                m_alarm_enabled[alarm_id] = enable;

                /* Read the current alarm registers */
                HwTimeRegisters time_registers = {};
                if (R_SUCCEEDED(ReadRtcRegisters(alarm_register, sizeof(time_registers), reinterpret_cast<u8 *>(std::addressof(time_registers))))) {
                    ClearAlarmEnableBits(time_registers);
                } else {
                    time_registers = {};
                }

                /* Set all alarm registers except on weekdays. */
                if (m_alarm_enabled[alarm_id]) {
                    SetAlarmEnableBits(time_registers);
                }

                /* NOTE: Nintendo ignores the return value of this. */
                R_DISCARD(WriteRtcRegisters(alarm_register, sizeof(time_registers), reinterpret_cast<const u8 *>(std::addressof(time_registers))));

                /* Write back the interrupt mask and cache it. */
                R_TRY(WriteRtcRegisters(Max77620RtcIntMReg, 1, std::addressof(interrupt_mask)));

                return ResultSuccess();
            }

            Result ReadTime(RtcRegisters *out) {
                R_UNLESS(IsReady(),      ResultDeviceNotReady());
                R_UNLESS(out != nullptr, ResultInvalidArgument());

                /* Read the alarm time registers. */
                HwTimeRegisters regs = {};
                R_TRY(ReadRtcRegisters(Max77620RtcSecReg, sizeof(regs), reinterpret_cast<u8 *>(std::addressof(regs))));

                out->sec     = regs.sec     & Max77620RtcSecValueMask;
                out->min     = regs.min     & Max77620RtcMinValueMask;
                out->hour    = regs.hour    & Max77620RtcHourValueMask;
                out->weekday = regs.weekday & Max77620RtcWeekdayValueMask;
                out->month   = regs.month   & Max77620RtcMonthValueMask;
                out->year    = regs.year    & Max77620RtcYearValueMask;
                out->day     = regs.day     & Max77620RtcDayValueMask;

                return ResultSuccess();
            }

            Result SetTime(RtcRegisters *in_regs) {
                R_UNLESS(IsReady(),          ResultDeviceNotReady());
                R_UNLESS(in_regs != nullptr, ResultInvalidArgument());

                /* Write the alarm time registers. */
                HwTimeRegisters regs = {
                    .sec     = static_cast<u8>(in_regs->sec),
                    .min     = static_cast<u8>(in_regs->min),
                    .hour    = static_cast<u8>(in_regs->hour),
                    .weekday = static_cast<u8>(in_regs->weekday),
                    .month   = static_cast<u8>(in_regs->month),
                    .year    = static_cast<u8>(in_regs->year),
                    .day     = static_cast<u8>(in_regs->day),
                };

                return WriteRtcRegisters(Max77620RtcSecReg, sizeof(regs), reinterpret_cast<u8 *>(std::addressof(regs)));
            }
        public:
            explicit constexpr RtcDevice(i2c::I2cSession &session) : m_session(session) { }

            Result Initialize() {
                /* Set rtc interrupt. */
                u8 data = Max77620RtcIntMaskAll;
                R_TRY(WriteRtcRegisters(Max77620RtcIntMReg, 1, std::addressof(data)));

                /* Read the interrupt status and preceed to not use it. */
                R_TRY(ReadRtcRegisters(Max77620RtcIntReg, 1, std::addressof(data)));

                data = Max77620RtcControlMBinFormat | Max77620RtcControlM24h;
                R_TRY(WriteRtcRegisters(Max77620RtcControlMReg, 1, std::addressof(data)));

                /* If the rtc is in 24h mode with BCD, it must already be set up. */
                bool configured = false;
                R_TRY(IsRtcModeConfigured(std::addressof(configured)));

                /* Disable the alarms if it's not configured. */
                if (!configured) {
                    R_TRY(SetAlarmEnabledImpl(0, false));
                    R_TRY(SetAlarmEnabledImpl(1, false));
                }

                /* Set 24h format. */
                data = Max77620Rtc24h;
                R_TRY(WriteRtcRegisters(Max77620RtcControlReg, 1, std::addressof(data)));

                /* Reset time and alarms if it's not configured. */
                if (!configured) {
                    R_TRY(ResetToDefaults());
                }

                /* Unmask the control interrupts. */
                data = Max77620RtcControlMUnmaskAll;
                R_TRY(WriteRtcRegisters(Max77620RtcControlMReg, 1, std::addressof(data)));

                /* Cache the enabled alarms. */
                for (u32 i = 0; i < AlarmCount; ++i) {
                    HwTimeRegisters alarm = {};
                    R_TRY(ReadRtcRegisters(i ? Max77620RtcAlarm2Reg : Max77620RtcAlarm1Reg, sizeof(alarm), reinterpret_cast<u8 *>(std::addressof(alarm))));
                    m_alarm_enabled[i] = (alarm.sec & Max77620RtcAlarmEnMask) != 0;
                }

                /* Rewrite the interrupt mask with whatever it currently holds. */
                R_TRY(ReadRtcRegisters(Max77620RtcIntMReg,  1, std::addressof(data)));
                R_TRY(WriteRtcRegisters(Max77620RtcIntMReg, 1, std::addressof(data)));

                m_ready = true;

                return ResultSuccess();
            }

            ALWAYS_INLINE bool IsReady() const { return m_ready; }

            Result WriteAlarmTime(u32 alarm_id, const RtcRegisters &time) {
                R_UNLESS(IsReady(),             ResultDeviceNotReady());
                R_UNLESS(alarm_id < AlarmCount, ResultInvalidArgument());

                const u8 alarm_register = GetAlarmRegister(alarm_id);

                HwTimeRegisters alarm = {
                    .sec     = static_cast<u8>(time.sec     & Max77620RtcSecValueMask),
                    .min     = static_cast<u8>(time.min     & Max77620RtcMinValueMask),
                    .hour    = static_cast<u8>(time.hour    & Max77620RtcHourValueMask),
                    .weekday = static_cast<u8>(time.weekday & Max77620RtcWeekdayValueMask),
                    .month   = static_cast<u8>(time.month   & Max77620RtcMonthValueMask),
                    .year    = static_cast<u8>(time.year    & Max77620RtcYearValueMask),
                    .day     = static_cast<u8>(time.day     & Max77620RtcDayValueMask),
                };

                if (m_alarm_enabled[alarm_id]) {
                    SetAlarmEnableBits(alarm);
                }

                return WriteRtcRegisters(alarm_register, sizeof(alarm), reinterpret_cast<const u8 *>(std::addressof(alarm)));
            }

            Result EnableAlarm(u32 alarm_id, u64 in_seconds) {
                const RtcRegisters time = ConvertToRtcRegisters(in_seconds);

                /* Write the alarm time. */
                R_UNLESS(R_SUCCEEDED(WriteAlarmTime(alarm_id, time)), ResultAlarmAccessFailed());

                /* Enable the alarm. */
                R_UNLESS(R_SUCCEEDED(SetAlarmEnabledImpl(alarm_id, true)), ResultAlarmAccessFailed());

                return ResultSuccess();
            }

            Result DisableAlarm(u32 alarm_id) {
                R_UNLESS(alarm_id < AlarmCount, ResultInvalidArgument());

                /* Disable the alarm. */
                R_UNLESS(R_SUCCEEDED(SetAlarmEnabledImpl(alarm_id, false)), ResultAlarmAccessFailed());

                constexpr RtcRegisters DisabledAlarmTime = {
                    .sec     = 59,
                    .min     = 59,
                    .hour    = 23,
                    .weekday = 0,
                    .month   = 2,
                    .year    = 99,
                    .day     = 30,
                };

                /* Set the alarm time to default. */
                R_UNLESS(R_SUCCEEDED(WriteAlarmTime(alarm_id, DisabledAlarmTime)), ResultAlarmAccessFailed());

                return ResultSuccess();
            }

            Result GetTimeInSeconds(u64 *out_time) {
                R_UNLESS(out_time != nullptr, ResultInvalidArgument());

                /* Read the time from the RTC. */
                RtcRegisters time = {};
                R_UNLESS(R_SUCCEEDED(ReadTime(std::addressof(time))), ResultRtcAccessFailed());

                const s32 year   = static_cast<u8>(time.year);
                const s32 month  = time.month & Max77620RtcMonthValueMask;
                const s32 day    = time.day   & Max77620RtcDayValueMask;
                const s32 hour   = time.hour  & TimeHourMask;
                const s32 minute = time.min   & Max77620RtcMinValueMask;
                const s32 second = time.sec   & Max77620RtcSecValueMask;

                /* Treat March as the first month, so the leap day falls at the end of the year. */
                const bool is_jan_or_feb  = month < March;
                const s32 shifted_year    = is_jan_or_feb ? year - 1 : year;
                const s32 shifted_month   = is_jan_or_feb ? month + (MonthsPerYear - March) : month - March;
                const s32 century         = shifted_year / YearsPerCentury;
                const s32 year_in_century = shifted_year % YearsPerCentury;

                const s32 truncation_fix = (shifted_year >= 0) ? 0 : -1;

                /* Days since 2000-01-01. */
                const s32 days = (day - 1) + DaysFromEpochToFirstMarch + truncation_fix + (DaysPer4Years * year_in_century) / YearsPerLeapBlock
                                           + (DaysPer400Years * century) / CenturiesPerLeapCycle + (DaysPer5Months * shifted_month + MonthRounding) / MonthsPerPeriod;

                *out_time = ((static_cast<s64>(days) * HoursPerDay + hour) * MinutesPerHour + minute) * SecondsPerMinute + second;

                return ResultSuccess();
            }

            Result SetTimeInSeconds(u64 in_seconds) {
                RtcRegisters time = ConvertToRtcRegisters(in_seconds);

                /* Set the time. */
                R_UNLESS(R_SUCCEEDED(SetTime(std::addressof(time))), ResultRtcAccessFailed());

                return ResultSuccess();
            }

            Result SetUpResetOnShutdown() {
                /* Mask control interrupts */
                u8 data = Max77620RtcControlM24h;
                R_TRY(WriteRtcRegisters(Max77620RtcControlMReg, 1, std::addressof(data)));

                data = Max77620RtcControlBcd12h;
                R_TRY(WriteRtcRegisters(Max77620RtcControlReg, 1, std::addressof(data)));

                /* NOTE: Nintendo not lock here either. */
                const u8 *defaults = reinterpret_cast<const u8 *>(std::addressof(DefaultTimeRegisters));
                constexpr u8 RtcBases[] = { Max77620RtcSecReg, Max77620RtcAlarm1Reg, Max77620RtcAlarm2Reg };

                /* Write the defaults. */
                for (u32 b = 0; b < util::size(RtcBases); ++b) {
                    for (u8 i = 0; i < sizeof(DefaultTimeRegisters); ++i) {
                        const bool last = (b == util::size(RtcBases) - 1) && (i == sizeof(DefaultTimeRegisters) - 1);

                        if (!last) {
                            R_TRY(I2cWrite(m_session, RtcBases[b] + i, defaults[i]));
                        } else {
                            R_TRY(WriteRtcRegisters(RtcBases[b] + i, 1, defaults + i));
                        }
                    }
                }

                data = Max77620RtcControlMUnmaskAll;
                R_TRY(WriteRtcRegisters(Max77620RtcControlMReg, 1, std::addressof(data)));

                return ResultSuccess();
            }

            Result SetResetOnShutdown(u8 do_reset) {
                m_reset_on_shutdown = do_reset;

                return ResultSuccess();
            }

            Result GetResetDetected(u8 *reset_detected) {
                R_UNLESS(reset_detected != nullptr, ResultInvalidArgument());

                *reset_detected = GetExternalRtcResetFlag();

                return ResultSuccess();
            }

            Result ClearResetDetected() {
                SetExternalRtcResetFlag(false);

                bool flag = GetExternalRtcResetFlag();

                R_ABORT_UNLESS(!flag);

                return ResultSuccess();
            }

            Result ApplyResetOnShutdownIfFlagged() {
                R_SUCCEED_IF(!m_reset_on_shutdown);

                return SetUpResetOnShutdown();
            }
    };

}
