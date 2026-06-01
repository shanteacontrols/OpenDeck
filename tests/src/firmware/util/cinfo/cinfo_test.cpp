/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/shared/common.h"
#include "tests/shared/helpers/misc.h"

#include "firmware/src/signaling/signaling.h"
#include "firmware/src/util/cinfo/cinfo.h"

#include "zlibs/utils/misc/mutex.h"

#include <zephyr/kernel.h>

#include <cstddef>
#include <algorithm>
#include <memory>
#include <vector>

using namespace opendeck;
using namespace opendeck::firmware;

namespace
{
    using Block = database::Config::Block;

    struct ComponentInfoEvent
    {
        size_t block = 0;
        size_t index = 0;
    };

    class ComponentInfoTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            _component_info = std::make_unique<util::ComponentInfo>();
            _component_info->register_handler(
                [this](size_t block, size_t index)
                {
                    const zlibs::utils::misc::LockGuard lock(_lock);
                    _events.push_back(ComponentInfoEvent{
                        .block = block,
                        .index = index,
                    });
                });
        }

        void TearDown() override
        {
            signaling::clear_registry();
            _component_info.reset();
        }

        void publish(signaling::IoEventSource source, size_t index)
        {
            ASSERT_TRUE(signaling::publish(signaling::MidiIoSignal{
                .source          = source,
                .component_index = index,
            }));
            ASSERT_TRUE(zlibs::utils::signaling::drain());
        }

        bool wait_for_count(size_t expected)
        {
            return tests::wait_until(
                [this, expected]()
                {
                    return count() >= expected;
                },
                500,
                10);
        }

        size_t count() const
        {
            const zlibs::utils::misc::LockGuard lock(_lock);
            return _events.size();
        }

        std::vector<ComponentInfoEvent> events() const
        {
            const zlibs::utils::misc::LockGuard lock(_lock);
            return _events;
        }

        mutable zlibs::utils::misc::Mutex    _lock;
        std::vector<ComponentInfoEvent>      _events = {};
        std::unique_ptr<util::ComponentInfo> _component_info;
    };
}    // namespace

TEST_F(ComponentInfoTest, FirstComponentZeroEventIsQueued)
{
    publish(signaling::IoEventSource::Analog, 0);

    ASSERT_TRUE(wait_for_count(1));

    const auto emitted = events();
    ASSERT_EQ(1, emitted.size());
    EXPECT_EQ(static_cast<size_t>(Block::Analog), emitted.at(0).block);
    EXPECT_EQ(0, emitted.at(0).index);
}

TEST_F(ComponentInfoTest, RepeatedSameComponentWithinThrottleWindowIsDropped)
{
    publish(signaling::IoEventSource::Analog, 0);
    ASSERT_TRUE(wait_for_count(1));

    publish(signaling::IoEventSource::Analog, 0);

    EXPECT_FALSE(wait_for_count(2));
    EXPECT_EQ(1, count());
}

TEST_F(ComponentInfoTest, RepeatedSameComponentAfterThrottleWindowIsQueued)
{
    publish(signaling::IoEventSource::Analog, 0);
    ASSERT_TRUE(wait_for_count(1));

    k_msleep(550);
    publish(signaling::IoEventSource::Analog, 0);

    ASSERT_TRUE(wait_for_count(2));

    const auto emitted = events();
    ASSERT_EQ(2, emitted.size());
    EXPECT_EQ(static_cast<size_t>(Block::Analog), emitted.at(1).block);
    EXPECT_EQ(0, emitted.at(1).index);
}

TEST_F(ComponentInfoTest, DifferentComponentInSameBlockIsQueued)
{
    publish(signaling::IoEventSource::Analog, 0);
    ASSERT_TRUE(wait_for_count(1));

    publish(signaling::IoEventSource::Analog, 1);

    ASSERT_TRUE(wait_for_count(2));

    const auto emitted = events();
    ASSERT_EQ(2, emitted.size());
    EXPECT_EQ(static_cast<size_t>(Block::Analog), emitted.at(1).block);
    EXPECT_EQ(1, emitted.at(1).index);
}

TEST_F(ComponentInfoTest, DifferentBlocksKeepIndependentPendingSlots)
{
    publish(signaling::IoEventSource::Analog, 0);
    publish(signaling::IoEventSource::Switch, 0);

    ASSERT_TRUE(wait_for_count(2));

    const auto emitted = events();
    ASSERT_EQ(2, emitted.size());

    const auto has_analog = std::any_of(emitted.begin(),
                                        emitted.end(),
                                        [](const ComponentInfoEvent& event)
                                        {
                                            return (event.block == static_cast<size_t>(Block::Analog)) &&
                                                   (event.index == 0);
                                        });
    const auto has_switch = std::any_of(emitted.begin(),
                                        emitted.end(),
                                        [](const ComponentInfoEvent& event)
                                        {
                                            return (event.block == static_cast<size_t>(Block::Switches)) &&
                                                   (event.index == 0);
                                        });

    EXPECT_TRUE(has_analog);
    EXPECT_TRUE(has_switch);
}
