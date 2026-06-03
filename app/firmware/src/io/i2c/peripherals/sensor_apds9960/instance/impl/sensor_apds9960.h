/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "firmware/src/io/i2c/peripherals/sensor_apds9960/instance/impl/deps.h"
#include "firmware/src/io/i2c/shared/value_filter.h"
#include "firmware/src/signaling/signaling.h"
#include "firmware/src/system/shared/config.h"

#include <array>
#include <optional>

namespace opendeck::firmware::io::i2c::sensor_apds9960
{
    /**
     * @brief APDS9960 proximity, gesture, and color sensor detector.
     */
    class SensorApds9960 : public Peripheral
    {
        public:
        /**
         * @brief Constructs the APDS9960 peripheral bound to the shared I2C backend.
         *
         * @param hwa Hardware abstraction used to communicate with the sensor.
         * @param database Database used to read and update APDS9960 settings.
         */
        SensorApds9960(Hwa&      hwa,
                       Database& database);

        /**
         * @brief Initializes APDS9960 at one supported I2C address.
         *
         * @param address_index Index into the supported I2C address list.
         *
         * @return `true` if initialization completed.
         */
        bool init(size_t address_index) override;

        /**
         * @brief Performs one periodic sensor update.
         *
         * @return `true` while the sensor is initialized, otherwise `false`.
         */
        bool update() override;

        /**
         * @brief Deinitializes the sensor runtime state.
         *
         * @return `true` if the sensor was deinitialized.
         */
        bool deinit() override;

        /**
         * @brief Returns the peripheral name used in diagnostics.
         *
         * @return Static peripheral name.
         */
        constexpr std::string_view name() const override;

        /**
         * @brief Returns supported APDS9960 I2C addresses.
         *
         * @return Candidate 7-bit I2C addresses.
         */
        std::span<const uint8_t> i2c_addresses() const override;

        private:
        static constexpr uint8_t  PROXIMITY_IDLE_THRESHOLD           = 16;
        static constexpr uint8_t  PROXIMITY_MOVING_THRESHOLD         = 8;
        static constexpr uint8_t  PROXIMITY_CONFIRMATION_SAMPLES     = 2;
        static constexpr uint16_t AMBIENT_LIGHT_SEND_THRESHOLD       = 16;
        static constexpr uint8_t  AMBIENT_LIGHT_CONFIRMATION_SAMPLES = 2;
        static constexpr uint16_t RGB_IDLE_THRESHOLD                 = 4;
        static constexpr uint16_t RGB_MOVING_THRESHOLD               = 4;
        static constexpr uint8_t  RGB_CONFIRMATION_SAMPLES           = 2;
        static constexpr uint8_t  GESTURE_FIFO_MAX_SAMPLES           = 32;
        static constexpr uint8_t  GESTURE_EDGE_THRESHOLD             = 13;
        static constexpr int64_t  GESTURE_EDGE_TIMEOUT_MS            = 500;
        static constexpr int64_t  GESTURE_SEND_COOLDOWN_MS           = 700;
        static constexpr uint8_t  READ_FAILURE_LIMIT                 = 3;

        Hwa&           _hwa;
        Database&      _database;
        bool           _found                      = false;
        bool           _initialized                = false;
        uint8_t        _read_failure_count         = 0;
        size_t         _selected_i2c_address_index = 0;
        ValueFilter<1> _proximity_filter           = {};
        ValueFilter<1> _ambient_light_filter       = {};
        ValueFilter<3> _rgb_filter                 = {};
        int64_t        _last_gesture_send_ms       = 0;
        int64_t        _gesture_start_ms           = 0;
        bool           _gesture_up_started         = false;
        bool           _gesture_down_started       = false;
        bool           _gesture_left_started       = false;
        bool           _gesture_right_started      = false;

        /**
         * @brief Configures proximity, gesture, and color sensing.
         *
         * @return `true` if the sensor accepted the configuration.
         */
        bool configure_sensor();

        /**
         * @brief Returns the selected APDS9960 I2C address.
         *
         * @return Selected 7-bit I2C address.
         */
        uint8_t i2c_address() const;

        /**
         * @brief Returns true when a specific APDS9960 output is enabled.
         *
         * @param setting Output setting to check.
         *
         * @return `true` if enabled in the database.
         */
        bool output_enabled(Setting setting);

        /**
         * @brief Returns true when APDS9960 gesture output is enabled.
         *
         * @return `true` when gesture output is enabled.
         */
        bool gesture_engine_enabled();

        /**
         * @brief Builds the APDS9960 CONTROL register value.
         *
         * @return Register value composed from configured proximity gain and defaults.
         */
        uint8_t control_register_value();

        /**
         * @brief Writes one APDS9960 register.
         *
         * @param reg Register address.
         * @param value Value to write.
         *
         * @return `true` if the write succeeded.
         */
        bool write_register(uint8_t reg, uint8_t value);

        /**
         * @brief Reads one APDS9960 register.
         *
         * @param reg Register address.
         *
         * @return Register value, or empty if the read failed.
         */
        std::optional<uint8_t> read_register(uint8_t reg);

        /**
         * @brief Reads one little-endian 16-bit APDS9960 register pair.
         *
         * @param reg Low-byte register address.
         *
         * @return Register value, or empty if the read failed.
         */
        std::optional<uint16_t> read_register_le16(uint8_t reg);

        /**
         * @brief Reads one APDS9960 register block.
         *
         * @param reg First register address.
         * @param buffer Output buffer.
         *
         * @return `true` if the read succeeded.
         */
        bool read_register_block(uint8_t reg, std::span<uint8_t> buffer);

        /**
         * @brief Clears consecutive I2C failure state after a successful transfer.
         */
        void record_i2c_success();

        /**
         * @brief Records one failed I2C transfer and disconnects the sensor after repeated failures.
         */
        void record_i2c_failure();

        /**
         * @brief Moves the sensor into a disconnected state until a later retry succeeds.
         */
        void mark_disconnected();

        /**
         * @brief Reads and publishes one proximity value when ready.
         *
         * @return `true` if the sensor is still usable.
         */
        bool read_proximity();

        /**
         * @brief Reads and publishes one gesture event when ready.
         *
         * @return `true` if the sensor is still usable.
         */
        bool read_gesture();

        /**
         * @brief Reads and publishes one RGB value tuple when ready.
         *
         * @return `true` if the sensor is still usable.
         */
        bool read_rgb();

        /**
         * @brief Reads and publishes one ambient light value when ready.
         *
         * @return `true` if the sensor is still usable.
         */
        bool read_ambient_light();

        /**
         * @brief Reads and decodes one gesture event from the sensor FIFO if available.
         *
         * Gesture decoding moves through the following states:
         * - Step 1: verify that the sensor is powered on and gesture sensing is enabled.
         * - Step 2: check the gesture status register and return immediately when the sensor's
         *           internal gesture FIFO has no valid samples.
         * - Step 3: read how many U/D/L/R sample groups are waiting in the sensor's gesture FIFO.
         *           Values above the sensor's 32-sample FIFO capacity are treated as invalid and
         *           limited to 32 before reading the samples.
         * - Step 4: discard any half-detected gesture edge that has waited too long for its
         *           matching opposite edge.
         * - Step 5: process each FIFO sample as one U/D/L/R sample group until a complete direction is
         *           decoded or the FIFO data is exhausted.
         *
         * @return Gesture direction, or empty if no complete gesture was available.
         */
        std::optional<opendeck::firmware::signaling::OscSensorGesture> decode_gesture_fifo();

        /**
         * @brief Processes one U/D/L/R gesture sample group using edge-state detection.
         *
         * A directional gesture is treated as two edges. The first strong imbalance between
         * opposing photodiodes stores the possible start direction. A later strong imbalance in
         * the opposite direction completes the gesture. For example, an up/down imbalance followed
         * by the opposite up/down imbalance produces an up or down gesture. The same rule is used
         * for left/right gestures.
         *
         * @param up Up photodiode count.
         * @param down Down photodiode count.
         * @param left Left photodiode count.
         * @param right Right photodiode count.
         *
         * @return Gesture direction, or empty if the edge pair is incomplete.
         */
        std::optional<opendeck::firmware::signaling::OscSensorGesture> process_gesture_sample(uint8_t up, uint8_t down, uint8_t left, uint8_t right);

        /**
         * @brief Resets transient gesture decoding counters.
         */
        void reset_gesture_counts();

        /**
         * @brief Resets gesture edge tracking.
         */
        void reset_gesture_edge_state();

        /**
         * @brief Clears debounced continuous-output state.
         */
        void reset_value_states();

        /**
         * @brief Publishes the last committed continuous-output values.
         */
        void publish_value_states();

        /**
         * @brief Serves SysEx configuration reads for the APDS9960 section.
         *
         * @param section I2C configuration section being read.
         * @param index Parameter index within the section.
         * @param value Output storage for the returned value.
         *
         * @return Status code for handled requests, otherwise `std::nullopt`.
         */
        std::optional<uint8_t> sys_config_get(sys::Config::Section::I2c section, size_t index, uint16_t& value);

        /**
         * @brief Serves SysEx configuration writes for the APDS9960 section.
         *
         * @param section I2C configuration section being written.
         * @param index Parameter index within the section.
         * @param value Value to write.
         *
         * @return Status code for handled requests, otherwise `std::nullopt`.
         */
        std::optional<uint8_t> sys_config_set(sys::Config::Section::I2c section, size_t index, uint16_t value);
    };
}    // namespace opendeck::firmware::io::i2c::sensor_apds9960
