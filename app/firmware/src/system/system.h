/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"
#include "config.h"
#include "layout.h"
#include "signaling/signaling.h"
#include "protocol/midi/midi.h"
#include "threads.h"
#include "util/cinfo/cinfo.h"

#include "zlibs/utils/misc/kwork_delayable.h"
#include "zlibs/utils/misc/ring_buffer.h"

namespace opendeck::sys
{
    /**
     * @brief Top-level system coordinator for initialization, backup/restore, and reboot flows.
     */
    class System
    {
        public:
        /**
         * @brief Constructs the top-level system coordinator.
         *
         * @param hwa Hardware abstraction used for system-level operations.
         */
        explicit System(Hwa& hwa);

        /**
         * @brief Initializes the system coordinator and all managed components.
         *
         * @return `true` if initialization succeeded, otherwise `false`.
         */
        bool init();

        private:
        /**
         * @brief Indicates whether the system is currently idle, backing up, or restoring configuration.
         */
        enum class BackupRestoreState : uint8_t
        {
            None,
            Backup,
            Restore
        };

        /**
         * @brief Tracks the current step of the incremental restore state machine.
         */
        enum class BackupPhase : uint8_t
        {
            Idle,
            RestoreStart,
            PresetSelect,
            PresetData,
            RestoreCurrentPreset,
            RestoreEnd,
            FinalAck,
        };

        /**
         * @brief Tracks incremental backup and restore traversal state.
         */
        struct BackupSession
        {
            bool        active         = false;
            BackupPhase phase          = BackupPhase::Idle;
            uint8_t     current_preset = 0;
            uint8_t     preset_index   = 0;
            uint8_t     block_index    = 0;
            uint8_t     section_index  = 0;
        };

        /**
         * @brief Tracks staged forced-refresh progress across top-level I/O subsystems.
         */
        struct ForcedRefreshSession
        {
            bool              active          = false;
            size_t            io_index        = 0;
            size_t            component_index = 0;
            ForcedRefreshType type            = {};
        };

        /**
         * @brief SysEx data handler used by the system coordinator.
         */
        class SysExDataHandler : public zlibs::utils::sysex_conf::DataHandler
        {
            public:
            /**
             * @brief Constructs the SysEx data handler bound to one system coordinator instance.
             *
             * @param system System coordinator that services SysEx requests.
             */
            explicit SysExDataHandler(System& system)
                : _system(system)
            {}

            /**
             * @brief Reads one SysEx configuration value.
             *
             * @param block SysEx configuration block index.
             * @param section SysEx configuration section index within `block`.
             * @param index Entry index within `section`.
             * @param value Output storage for the returned value.
             *
             * @return SysEx read status code describing the result.
             */
            zlibs::utils::sysex_conf::Status get(uint8_t block, uint8_t section, uint16_t index, uint16_t& value) override;

            /**
             * @brief Writes one SysEx configuration value.
             *
             * @param block SysEx configuration block index.
             * @param section SysEx configuration section index within `block`.
             * @param index Entry index within `section`.
             * @param value Value to store.
             *
             * @return SysEx write status code describing the result.
             */
            zlibs::utils::sysex_conf::Status set(uint8_t block, uint8_t section, uint16_t index, uint16_t value) override;

            /**
             * @brief Handles one custom SysEx request routed through the system coordinator.
             *
             * @param request Custom request identifier.
             * @param custom_response Output payload for the custom response.
             *
             * @return SysEx status code describing the request result.
             */
            zlibs::utils::sysex_conf::Status custom_request(uint16_t request, CustomResponse& custom_response) override;

            /**
             * @brief Sends one SysEx response packet through the system coordinator.
             *
             * @param packet UMP packet containing the response payload.
             *
             * @return `true` if the response was queued or sent successfully, otherwise `false`.
             */
            bool send_response(const midi_ump& packet) override;

            private:
            System& _system;
        };

        /**
         * @brief Database event handlers owned by the system coordinator.
         */
        class DatabaseHandlers : public database::Handlers
        {
            public:
            /**
             * @brief Constructs the database callback adapter bound to one system coordinator instance.
             *
             * @param system System coordinator that consumes database events.
             */
            explicit DatabaseHandlers(System& system)
                : _system(system)
            {}

            /**
             * @brief Notifies the system coordinator that the active preset changed.
             *
             * @param preset Newly selected preset index.
             */
            void preset_change(uint8_t preset) override;

            /**
             * @brief Notifies the system coordinator that a factory reset is starting.
             */
            void factory_reset_start() override;

            /**
             * @brief Notifies the system coordinator that a factory reset finished.
             */
            void factory_reset_done() override;

            /**
             * @brief Notifies the system coordinator that the database finished initializing.
             */
            void initialized() override;

            private:
            System& _system;
        };

        static constexpr zlibs::utils::sysex_conf::ManufacturerId SYS_EX_MANUFACTURER_ID = {
            Config::SYSEX_MANUFACTURER_ID_0,
            Config::SYSEX_MANUFACTURER_ID_1,
            Config::SYSEX_MANUFACTURER_ID_2
        };

        static constexpr size_t   RESTORE_QUEUE_PACKET_COUNT       = 2048;
        static constexpr uint32_t BACKUP_PROCESSING_START_DELAY_MS = 100;
        static constexpr uint32_t BACKUP_RESTORE_STEP_DELAY_MS     = 4;

        Hwa&                                                                        _hwa;
        DatabaseHandlers                                                            _database_handlers;
        SysExDataHandler                                                            _sysex_data_handler;
        zlibs::utils::misc::KworkDelayable                                          _forced_refresh_work;
        zlibs::utils::misc::KworkDelayable                                          _io_resume_work;
        zlibs::utils::misc::KworkDelayable                                          _factory_reset_work;
        zlibs::utils::misc::KworkDelayable                                          _backup_work;
        zlibs::utils::misc::KworkDelayable                                          _restore_work;
        zlibs::utils::misc::KworkDelayable                                          _reboot_work;
        zlibs::utils::misc::KworkDelayable                                          _sysex_conf_close_work;
        zlibs::utils::sysex_conf::SysExConf                                         _sysex_conf;
        util::ComponentInfo                                                         _cinfo;
        Layout                                                                      _layout;
        BackupRestoreState                                                          _backup_restore_state     = BackupRestoreState::None;
        BackupSession                                                               _backup_session           = {};
        bool                                                                        _components_initialized   = false;
        uint32_t                                                                    _backup_generated_packets = 0;
        fw_selector::FwType                                                         _reboot_type              = fw_selector::FwType::Application;
        ForcedRefreshSession                                                        _forced_refresh_session   = {};
        zlibs::utils::misc::RingBuffer<RESTORE_QUEUE_PACKET_COUNT, false, midi_ump> _restore_queue            = {};

        /**
         * @brief Initializes all registered I/O and protocol collections from the hardware backend.
         */
        void collection_init();

        /**
         * @brief Deinitializes all registered I/O and protocol collections from the hardware backend.
         */
        void collection_deinit();

        /**
         * @brief Starts an asynchronous backup session.
         */
        void start_backup();

        /**
         * @brief Advances the backup state machine by one step.
         */
        void run_backup_step();

        /**
         * @brief Finalizes the active backup session.
         */
        void finish_backup();

        /**
         * @brief Schedules a factory reset from the system workqueue.
         */
        void schedule_factory_reset();

        /**
         * @brief Executes the scheduled factory reset.
         */
        void run_factory_reset();

        /**
         * @brief Starts an asynchronous restore session.
         */
        void start_restore();

        /**
         * @brief Finalizes the active restore session.
         */
        void finish_restore();

        /**
         * @brief Queues one restore packet for deferred processing.
         *
         * @param packet UMP packet to queue.
         *
         * @return `true` if the packet was queued, otherwise `false`.
         */
        bool queue_restore_packet(const midi_ump& packet);

        /**
         * @brief Advances the restore state machine by one step.
         */
        void run_restore_step();

        /**
         * @brief Emits one SysEx backup request for the selected block and section.
         *
         * @param block SysEx configuration block index.
         * @param section SysEx configuration section index within `block`.
         *
         * @return `true` if the request was emitted successfully, otherwise `false`.
         */
        bool emit_backup_request(uint8_t block, uint8_t section);

        /**
         * @brief Emits a preset-change event toward the managed components.
         *
         * @param preset Preset index to activate.
         */
        void emit_preset_change(uint8_t preset);

        /**
         * @brief Starts a staged forced-refresh session.
         */
        void start_forced_refresh();

        /**
         * @brief Schedules a staged forced refresh of the specified type.
         *
         * @param type Reason for the forced refresh.
         * @param delay_ms Delay before the staged refresh starts.
         */
        void schedule_forced_refresh(ForcedRefreshType type, uint32_t delay_ms);

        /**
         * @brief Finalizes the current staged forced-refresh session.
         */
        void finish_forced_refresh();

        /**
         * @brief Finds the next backup section that should be exported.
         *
         * @param block Updated with the next block index on success.
         * @param section Updated with the next section index on success.
         *
         * @return `true` if another section is available, otherwise `false`.
         */
        bool find_next_backup_section(uint8_t& block, uint8_t& section) const;

        /**
         * @brief Forces dependent components to refresh their state from current configuration.
         */
        void force_component_refresh();

        /**
         * @brief Schedules a reboot into the selected firmware target.
         *
         * @param type Firmware target to boot after restart.
         */
        void schedule_reboot(fw_selector::FwType type);

        /**
         * @brief Updates configuration-session timeout tracking after incoming SysEx.
         *
         * @param was_open Configuration-session state before processing the incoming packet.
         */
        void update_sysex_configuration_session(bool was_open);

        /**
         * @brief Forces an inactive configuration session closed after timeout.
         */
        void close_inactive_sysex_configuration_session();

        /**
         * @brief Emits a configuration-session state change.
         */
        void publish_configuration_session_state(signaling::SystemEvent event);

        /**
         * @brief Reads one global system configuration value.
         *
         * @param section Global configuration section to read.
         * @param index Entry index within `section`.
         * @param value Output storage for the returned value.
         *
         * @return Optional status code describing the read result.
         */
        std::optional<uint8_t> sys_config_get(sys::Config::Section::Global section, size_t index, uint16_t& value);

        /**
         * @brief Writes one global system configuration value.
         *
         * @param section Global configuration section to update.
         * @param index Entry index within `section`.
         * @param value Value to store.
         *
         * @return Optional status code describing the write result.
         */
        std::optional<uint8_t> sys_config_set(sys::Config::Section::Global section, size_t index, uint16_t value);
    };
}    // namespace opendeck::sys
