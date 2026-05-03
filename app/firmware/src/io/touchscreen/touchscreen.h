/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "deps.h"
#include "system/config.h"
#include "signaling/signaling.h"
#include "io/base.h"
#include "threads.h"

#include "zlibs/utils/misc/mutex.h"

#include <zephyr/kernel.h>

#include <optional>

namespace opendeck::io::touchscreen
{
    /**
     * @brief Coordinates touchscreen model initialization, polling, and system integration.
     */
    class Touchscreen : public io::Base
    {
        public:
        /**
         * @brief Constructs a touchscreen controller bound to hardware and database services.
         *
         * @param hwa Hardware abstraction used to communicate with touchscreen models.
         * @param database Database interface used to read touchscreen configuration.
         */
        Touchscreen(Hwa&      hwa,
                    Database& database);

        /**
         * @brief Stops the touchscreen worker and unregisters all model instances.
         */
        ~Touchscreen() override;

        /**
         * @brief Initializes the configured touchscreen model and applies the stored startup state.
         *
         * @return `true` if the configured model was initialized successfully, otherwise `false`.
         */
        bool init() override;

        /**
         * @brief Stops the touchscreen worker thread.
         */
        void deinit() override;

        /**
         * @brief Registers a model implementation for later selection by configuration.
         *
         * @param model Model identifier to associate with the instance.
         * @param instance Model implementation to register, or `nullptr` to clear the slot.
         */
        static void register_model(ModelType model, Model* instance);

        private:
        static constexpr uint32_t THREAD_POLL_TIME_MS = 1;

        Hwa&                                                                    _hwa;
        Database&                                                               _database;
        zlibs::utils::misc::Mutex                                               _state_mutex;
        size_t                                                                  _active_screen_id = 0;
        bool                                                                    _initialized      = false;
        ModelType                                                               _active_model     = ModelType::Count;
        static inline std::array<Model*, static_cast<size_t>(ModelType::Count)> models            = {};
        threads::TouchscreenThread                                              _thread;
        struct k_sem                                                            _update_semaphore = {};

        /**
         * @brief Deinitializes the currently active model instance when one is running.
         *
         * @return `true` if no model was active or the active model deinitialized successfully, otherwise `false`.
         */
        bool deinit_model();

        /**
         * @brief Stops the touchscreen worker thread without invoking virtual dispatch.
         */
        void stop_thread();

        /**
         * @brief Signals the worker thread to process a touchscreen update.
         */
        void request_update();

        /**
         * @brief Waits for a pending touchscreen update request.
         *
         * @param timeout Maximum time to wait for a queued update.
         *
         * @return `true` if an update request was received before the timeout expired, otherwise `false`.
         */
        bool wait_for_update(k_timeout_t timeout);

        /**
         * @brief Polls the active model and dispatches any generated touchscreen events.
         */
        void process_update();

        /**
         * @brief Returns the registered implementation for the requested model identifier.
         *
         * @param model Model identifier to resolve.
         *
         * @return Pointer to the registered model implementation, or `nullptr` when no implementation is registered.
         */
        Model* model_instance(ModelType model);

        /**
         * @brief Returns whether a model is currently initialized and ready for use.
         *
         * @return `true` if a touchscreen model is active, otherwise `false`.
         */
        bool is_initialized() const;

        /**
         * @brief Switches the active model to the requested screen and publishes the change.
         *
         * @param index Screen index to show.
         */
        void set_screen(size_t index);

        /**
         * @brief Returns the most recently selected screen index.
         *
         * @return Active screen index tracked by the controller.
         */
        size_t active_screen();

        /**
         * @brief Updates a touchscreen icon when the current screen matches its configured pages.
         *
         * @param index Touchscreen component index whose icon should be updated.
         * @param state Icon state to render.
         */
        void set_icon_state(size_t index, bool state);

        /**
         * @brief Applies the requested brightness to the active model.
         *
         * @param brightness Brightness level to apply.
         *
         * @return `true` if the brightness change was accepted by the active model, otherwise `false`.
         */
        bool set_brightness(Brightness brightness);

        /**
         * @brief Handles a touchscreen button event and performs any configured page switch.
         *
         * @param button_index Touchscreen button index reported by the active model.
         * @param state Press state reported for the button.
         */
        void process_button(const size_t button_index, const bool state);

        /**
         * @brief Publishes the logical button event associated with a touchscreen component.
         *
         * @param index Touchscreen component index to publish.
         * @param state Button state to publish.
         */
        void button_handler(size_t index, bool state);

        /**
         * @brief Publishes a touchscreen screen-change notification.
         *
         * @param index Newly active screen index.
         */
        void screen_change_handler(size_t index);

        /**
         * @brief Serves SysEx configuration reads for the touchscreen block.
         *
         * @param section Touchscreen configuration section being read.
         * @param index Parameter index within the section.
         * @param value Output storage for the returned value.
         *
         * @return Status code for handled requests, otherwise `std::nullopt`.
         */
        std::optional<uint8_t> sys_config_get(sys::Config::Section::Touchscreen section, size_t index, uint16_t& value);

        /**
         * @brief Serves SysEx configuration writes for the touchscreen block.
         *
         * @param section Touchscreen configuration section being written.
         * @param index Parameter index within the section.
         * @param value Value to store.
         *
         * @return Status code for handled requests, otherwise `std::nullopt`.
         */
        std::optional<uint8_t> sys_config_set(sys::Config::Section::Touchscreen section, size_t index, uint16_t value);
    };
}    // namespace opendeck::io::touchscreen
