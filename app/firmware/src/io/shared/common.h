/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <inttypes.h>
#include <stddef.h>

namespace opendeck::io::common
{
    /**
     * @brief Packed-byte storage location for one bit-addressed I/O state.
     */
    struct BitStorageLocation
    {
        uint8_t array_index = 0;
        uint8_t bit_index   = 0;
    };

    /**
     * @brief Returns the packed-byte array slot and bit index for one flattened component index.
     *
     * @tparam BitsPerStorageUnit Number of bits stored in each backing array element.
     * @param index Flattened component index to map into packed storage.
     *
     * @return Packed-bit storage location for the requested index.
     */
    template<size_t BitsPerStorageUnit>
    static constexpr BitStorageLocation bit_storage_location(size_t index)
    {
        return {
            .array_index = static_cast<uint8_t>(index / BitsPerStorageUnit),
            .bit_index   = static_cast<uint8_t>(index % BitsPerStorageUnit),
        };
    }

    /**
     * @brief Selects how an I/O component should be treated during initialization sequencing.
     */
    enum class InitAction : uint8_t
    {
        AsIs,
        Init,
        DeInit
    };

    /**
     * @brief Compile-time helper that describes grouped component collections.
     *
     * @tparam T Per-group element counts.
     */
    template<size_t... T>
    class BaseCollection
    {
        public:
        /**
         * @brief Prevents instantiation of this compile-time collection descriptor.
         */
        BaseCollection() = delete;

        /**
         * @brief Returns the number of groups in the collection.
         *
         * @return Number of compile-time groups.
         */
        static constexpr size_t groups()
        {
            return sizeof...(T);
        }

        /**
         * @brief Returns the total number of elements across all groups.
         *
         * @return Total element count.
         */
        static constexpr size_t size()
        {
            return (T + ...);
        }

        /**
         * @brief Returns the number of elements in one group.
         *
         * @param group Group index to query.
         *
         * @return Element count for the requested group.
         */
        static constexpr size_t size(size_t group)
        {
            constexpr size_t VALUES[] = { T... };
            return VALUES[group];
        }

        /**
         * @brief Returns the global start index of a group within the flattened collection.
         *
         * @param group Group index to query.
         *
         * @return First flattened index belonging to the requested group.
         */
        static constexpr size_t start_index(size_t group)
        {
            size_t index = 0;

            for (size_t i = 0; i < group; i++)
            {
                index += size(i);
            }

            return index;
        }
    };

    /**
     * @brief Interface for subsystems that expose shared hardware allocations.
     */
    class Allocatable
    {
        public:
        /**
         * @brief Identifies an allocatable shared hardware interface.
         */
        enum class Interface : uint8_t
        {
            Uart,
        };

        virtual ~Allocatable() = default;

        /**
         * @brief Returns whether the requested interface is already allocated.
         *
         * @param interface Shared interface to query.
         *
         * @return `true` when the interface is allocated, otherwise `false`.
         */
        virtual bool allocated(Interface interface) = 0;
    };
}    // namespace opendeck::io::common
