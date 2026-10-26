#pragma once
#include <stratosphere.hpp>
#include "pcv_rtc_i2c.hpp"

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

    struct AlarmTimeRegister {
        u8 sec;
        u8 min;
        u8 hour;
        u8 weekday;
        u8 month;
        u8 year;
        u8 day;
    };
    static_assert(sizeof(AlarmTimeRegister) == Max77620RtcTimeRegCount);

    class RtcDevice {
        public:
            static constexpr int AlarmCount = 2;
        private:
            static constexpr AlarmTimeRegister DefaultTimeRegister = {
                .sec     = 0,
                .min     = 0,
                .hour    = 0,
                .weekday = 1,
                .month   = 1,
                .year    = 0,
                .day     = 1,
            };

        private:
            i2c::I2cSession &m_session;
            os::SdkMutex    m_lock;
            u8              m_saved_int_mask = 0;
            bool            m_ready = false;
            bool            m_alarm_enabled[AlarmCount] = {};
        private:
            Result ReadRegisters(u8 reg, u8 count, u8 *out) {
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
                R_UNLESS(ready, ResultReadUpdateTimeout());

                /* Read the requested registers. */
                for (u8 i = 0; i < count; ++i) {
                    const u8 cmd = reg + i;
                    R_TRY(I2cRead(m_session, out + i, sizeof(u8), std::addressof(cmd), sizeof(cmd)));
                }

                return ResultSuccess();
            }

            Result WriteRegisters(u8 reg, u8 count, const u8 *data) {
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

            ALWAYS_INLINE void ClearAlarmEnableBits(AlarmTimeRegister &alarm) {
                alarm.sec     &= Max77620RtcSecValueMask;
                alarm.min     &= Max77620RtcMinValueMask;
                alarm.hour    &= Max77620RtcHourValueMask;
                alarm.weekday &= Max77620RtcWeekdayValueMask;
                alarm.month   &= Max77620RtcMonthValueMask;
                alarm.year    &= Max77620RtcYearValueMask;
                alarm.day     &= Max77620RtcDayValueMask;
            }

            ALWAYS_INLINE void SetAlarmEnableBits(AlarmTimeRegister &alarm) {
                alarm.sec   |= Max77620RtcAlarmEnMask;
                alarm.min   |= Max77620RtcAlarmEnMask;
                alarm.hour  |= Max77620RtcAlarmEnMask;
                alarm.month |= Max77620RtcAlarmEnMask;
                alarm.year  |= Max77620RtcAlarmEnMask;
                alarm.day   |= Max77620RtcAlarmEnMask;
            }

            ALWAYS_INLINE u8 GetAlarmRegister(u32 alarm_id) const {
                return alarm_id ? Max77620RtcAlarm2Reg  : Max77620RtcAlarm1Reg;
            }

            Result IsRtcModeConfigured(bool *out) {
                u8 control = 0;
                R_TRY(ReadRegisters(Max77620RtcControlReg, 1, std::addressof(control)));

                *out = (control & (Max77620RtcBinFormat | Max77620Rtc24h)) == Max77620Rtc24h;

                return ResultSuccess();
            }

            Result ResetToDefaults() {
                const u8 *defaults = reinterpret_cast<const u8 *>(std::addressof(DefaultTimeRegister));

                R_TRY(WriteRegisters(Max77620RtcSecReg,    sizeof(DefaultTimeRegister), defaults));
                R_TRY(WriteRegisters(Max77620RtcAlarm1Reg, sizeof(DefaultTimeRegister), defaults));
                R_TRY(WriteRegisters(Max77620RtcAlarm2Reg, sizeof(DefaultTimeRegister), defaults));

                return ResultSuccess();
            }

            Result SetAlarmEnabledImpl(u32 alarm_id, bool enable) {
                AMS_ASSERT(alarm_id < AlarmCount);

                /* Get the interrupt mask. */
                u8 interrupt_mask = 0;
                R_TRY(ReadRegisters(Max77620RtcIntMReg, 1, std::addressof(interrupt_mask)));

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
                AlarmTimeRegister time_registers = {};
                if (R_SUCCEEDED(ReadRegisters(alarm_register, sizeof(time_registers), reinterpret_cast<u8 *>(std::addressof(time_registers))))) {
                    ClearAlarmEnableBits(time_registers);
                } else {
                    time_registers = {};
                }

                /* Set all alarm registers except on weekdays. */
                if (m_alarm_enabled[alarm_id]) {
                    SetAlarmEnableBits(time_registers);
                }

                /* NOTE: Nintendo ignores the return value of this. */
                R_DISCARD(WriteRegisters(alarm_register, sizeof(time_registers), reinterpret_cast<const u8 *>(std::addressof(time_registers))));

                /* Write back the interrupt mask and cache it. */
                R_TRY(WriteRegisters(Max77620RtcIntMReg, 1, std::addressof(interrupt_mask)));
                m_saved_int_mask = interrupt_mask;

                return ResultSuccess();
            }
        public:
            explicit constexpr RtcDevice(i2c::I2cSession &session) : m_session(session) { }

            Result Initialize() {
                /* Set rtc interrupt. */
                u8 data = 0xFF;
                R_TRY(WriteRegisters(Max77620RtcIntMReg, 1, std::addressof(data)));

                /* Read the interrupt status and preceed to not use it. */
                R_TRY(ReadRegisters(Max77620RtcIntReg, 1, std::addressof(data)));

                /* Todo: Figure out what 3 means. */
                data = 3;
                R_TRY(WriteRegisters(Max77620RtcControlMReg, 1, std::addressof(data)));

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
                R_TRY(WriteRegisters(Max77620RtcControlReg, 1, std::addressof(data)));

                /* Reset time and alarms if it's not configured. */
                if (!configured) {
                    R_TRY(ResetToDefaults());
                }

                /* Unmask the control interrupts. */
                data = 0;
                R_TRY(WriteRegisters(Max77620RtcControlMReg, 1, std::addressof(data)));

                /* Cache the enabled alarms. */
                for (u32 i = 0; i < AlarmCount; ++i) {
                    AlarmTimeRegister alarm = {};
                    R_TRY(ReadRegisters(i ? Max77620RtcAlarm2Reg : Max77620RtcAlarm1Reg, sizeof(alarm), reinterpret_cast<u8 *>(std::addressof(alarm))));
                    m_alarm_enabled[i] = (alarm.sec & Max77620RtcAlarmEnMask) != 0;
                }

                /* Rewrite the interrupt mask with whatever it currently holds. */
                R_TRY(ReadRegisters(Max77620RtcIntMReg, 1, std::addressof(data)));
                R_TRY(WriteRegisters(Max77620RtcIntMReg, 1, std::addressof(data)));

                m_ready = true;

                return ResultSuccess();
            }

            Result Finalize() {
                AMS_ABORT("TODO");
            }

            ALWAYS_INLINE bool IsReady() const { return m_ready; }

            Result ReadTime(RtcRegisters *out) {
                (void) out;
                AMS_ABORT("TODO");
            }

            Result WriteAlarmTime(u32 alarm_id, const RtcRegisters &time) {
                R_UNLESS(IsReady(), ResultDeviceNotReady());
                R_UNLESS(alarm_id < AlarmCount, ResultInvalidArguments());

                const u8 alarm_register = GetAlarmRegister(alarm_id);

                AlarmTimeRegister alarm = {
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

                return WriteRegisters(alarm_register, sizeof(alarm), reinterpret_cast<const u8 *>(std::addressof(alarm)));
            }

            Result DisableAlarm(u32 alarm_id) {
                R_UNLESS(alarm_id < AlarmCount, ResultInvalidArguments());
                R_UNLESS(R_SUCCEEDED(SetAlarmEnabledImpl(alarm_id, false)), ResultAlarmDisableFailed());

                constexpr RtcRegisters DisabledAlarmTime = {
                    .sec     = 59,
                    .min     = 59,
                    .hour    = 23,
                    .weekday = 0,
                    .month   = 2,
                    .year    = 99,
                    .day     = 30,
                };

                R_UNLESS(R_SUCCEEDED(WriteAlarmTime(alarm_id, DisabledAlarmTime)), ResultAlarmDisableFailed());

                return ResultSuccess();
            }
    };

}
