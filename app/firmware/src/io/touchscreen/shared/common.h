/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/common/shared/common.h"
#include "firmware/src/io/touchscreen/drivers/count.h"

namespace opendeck::io::touchscreen
{
    /**
     * @brief Flattened collection of touchscreen components.
     */
    class Collection : public io::common::BaseCollection<OPENDECK_TOUCHSCREEN_COMPONENT_COUNT>
    {
        public:
        /**
         * @brief Prevents instantiation of this compile-time touchscreen collection descriptor.
         */
        Collection() = delete;
    };

    /**
     * @brief Touchscreen setting parameters stored in the database.
     */
    enum class Setting : uint8_t
    {
        Enable,
        Model,
        Brightness,
        InitialScreen,
        Reserved,
        Count
    };

    /**
     * @brief Event types that can be produced by touchscreen models.
     */
    enum class TsEvent : uint8_t
    {
        None,
        Switch,
    };

    /**
     * @brief Press phases reported by touchscreen models.
     */
    enum class PressType : uint8_t
    {
        None,
        Initial,
        Hold
    };

    /**
     * @brief Supported logical touchscreen brightness levels.
     */
    enum class Brightness : uint8_t
    {
        Level10,
        Level25,
        Level50,
        Level75,
        Level80,
        Level90,
        Level100
    };

    /**
     * @brief Touchscreen model implementations supported by the firmware.
     */
    enum class ModelType : uint8_t
    {
        Nextion,
        Count
    };

    /**
     * @brief Screen-space icon description used by touchscreen models.
     */
    struct Icon
    {
        uint16_t x_pos      = 0;
        uint16_t y_pos      = 0;
        uint16_t width      = 0;
        uint16_t height     = 0;
        uint16_t on_screen  = 0;
        uint16_t off_screen = 0;
    };

    /**
     * @brief Generic touchscreen event payload shared between models and the controller.
     */
    struct Data
    {
        PressType press_type   = PressType::None;
        size_t    switch_index = 0;
        bool      switch_state = false;
        uint16_t  x_pos        = 0;
        uint16_t  y_pos        = 0;
    };

    /**
     * @brief Interface implemented by concrete touchscreen model backends.
     */
    class Model
    {
        public:
        virtual ~Model() = default;

        /**
         * @brief Initializes the model backend.
         *
         * @return `true` if initialization succeeded, otherwise `false`.
         */
        virtual bool init() = 0;

        /**
         * @brief Deinitializes the model backend.
         *
         * @return `true` if deinitialization succeeded, otherwise `false`.
         */
        virtual bool deinit() = 0;

        /**
         * @brief Switches the model to the requested screen.
         *
         * @param index Screen index to show.
         *
         * @return `true` if the request succeeded, otherwise `false`.
         */
        virtual bool set_screen(size_t index) = 0;

        /**
         * @brief Polls the model for the next event.
         *
         * @param ts_data Output storage populated with any decoded event data.
         *
         * @return Decoded touchscreen event type.
         */
        virtual TsEvent update(Data& ts_data) = 0;

        /**
         * @brief Updates one icon region on the touchscreen.
         *
         * @param icon Icon descriptor to render.
         * @param state Requested icon state.
         */
        virtual void set_icon_state(Icon& icon, bool state) = 0;

        /**
         * @brief Applies a logical brightness level.
         *
         * @param brightness Brightness level to apply.
         *
         * @return `true` if the brightness change succeeded, otherwise `false`.
         */
        virtual bool set_brightness(Brightness brightness) = 0;

        protected:
        static constexpr size_t BUFFER_SIZE = 50;

        static inline uint8_t rx_buffer[BUFFER_SIZE] = {};
        static inline size_t  buffer_count           = 0;
    };
}    // namespace opendeck::io::touchscreen
