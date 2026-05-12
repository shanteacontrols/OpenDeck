/*
 * Copyright (c) 2026 Igor Petrovic
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tests/common.h"
#include "tests/helpers/database.h"
#include "tests/helpers/misc.h"

#include "database/builder.h"
#include "protocol/mdns/hwa_test.h"
#include "protocol/mdns/mdns.h"
#include "signaling/signaling.h"
#include "util/configurable/configurable.h"
#include "zlibs/utils/misc/mutex.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

using namespace opendeck;
using namespace opendeck::protocol;

namespace
{
    std::string make_expected_name()
    {
        std::string name = "opendeck-";

        for (const auto* character = OPENDECK_TARGET; *character != '\0'; character++)
        {
            if (std::isalnum(static_cast<unsigned char>(*character)) != 0)
            {
                name.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(*character))));
                continue;
            }

            name.push_back('-');
        }

        name += "-08090a0b.local";

        return name;
    }

    class NetworkIdentityCollector
    {
        public:
        NetworkIdentityCollector()
        {
            signaling::subscribe<signaling::NetworkIdentitySignal>(
                [this](const signaling::NetworkIdentitySignal& signal)
                {
                    const zlibs::utils::misc::LockGuard lock(_mutex);

                    _signals.push_back({
                        .name         = std::string(signal.name()),
                        .ipv4_address = std::string(signal.ipv4_address()),
                    });
                });
        }

        struct Signal
        {
            std::string name         = {};
            std::string ipv4_address = {};
        };

        std::vector<Signal> signals() const
        {
            const zlibs::utils::misc::LockGuard lock(_mutex);
            return _signals;
        }

        private:
        mutable zlibs::utils::misc::Mutex _mutex;
        std::vector<Signal>               _signals = {};
    };

    class MdnsTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            ASSERT_TRUE(_database_admin.init(_handlers));
            ASSERT_TRUE(_database_admin.factory_reset());
        }

        void TearDown() override
        {
            signaling::clear_registry();
            ConfigHandler.clear();
        }

        void set_custom_hostname(std::string_view hostname)
        {
            ASSERT_LT(hostname.size(), mdns::CUSTOM_HOSTNAME_DB_SIZE);

            for (size_t i = 0; i < hostname.size(); i++)
            {
                ASSERT_TRUE(_database.update(database::Config::Section::Common::MdnsHostname,
                                             i,
                                             static_cast<uint8_t>(hostname[i])));
            }
        }

        void wait_for_signals(const NetworkIdentityCollector& collector, size_t count)
        {
            ASSERT_TRUE(tests::wait_until(
                [&]()
                {
                    return collector.signals().size() == count;
                }));
        }

        tests::NoOpDatabaseHandlers _handlers;
        database::Builder           _database_builder;
        database::Admin&            _database_admin = _database_builder.instance();
        mdns::Database              _database       = mdns::Database(_database_admin);
        mdns::HwaTest               _hwa;
        mdns::Mdns                  _mdns = mdns::Mdns(_hwa, _database);
    };
}    // namespace

TEST_F(MdnsTest, PublishesNetworkIdentityOnInit)
{
    NetworkIdentityCollector collector;

    ASSERT_TRUE(_mdns.init());

    wait_for_signals(collector, 1);
    const auto signals = collector.signals();
    EXPECT_EQ(signals.front().name, make_expected_name());
    EXPECT_EQ(signals.front().ipv4_address, "192.168.1.112");
    EXPECT_EQ(_hwa.webconfig_instance + ".local", make_expected_name());
    EXPECT_EQ(_hwa.osc_instance + ".local", make_expected_name());
}

TEST_F(MdnsTest, UsesCustomHostnameFromCommonDatabase)
{
    set_custom_hostname("studio-left");
    NetworkIdentityCollector collector;

    ASSERT_TRUE(_mdns.init());

    wait_for_signals(collector, 1);
    EXPECT_EQ(collector.signals().front().name, "studio-left.local");
    EXPECT_EQ(_hwa.hostname, "studio-left");
    EXPECT_EQ(_hwa.webconfig_instance, "studio-left");
    EXPECT_EQ(_hwa.osc_instance, "studio-left");
}

TEST_F(MdnsTest, HandlesHostnameSysExConfig)
{
    uint16_t value = 0;

    ASSERT_TRUE(_mdns.init());

    EXPECT_EQ(ConfigHandler.get(sys::Config::Block::Global,
                                static_cast<uint8_t>(sys::Config::Section::Global::MdnsHostname),
                                0,
                                value),
              sys::Config::Status::Ack);
    EXPECT_EQ(value, 0);

    EXPECT_EQ(ConfigHandler.set(sys::Config::Block::Global,
                                static_cast<uint8_t>(sys::Config::Section::Global::MdnsHostname),
                                0,
                                's'),
              sys::Config::Status::Ack);
    EXPECT_EQ(ConfigHandler.set(sys::Config::Block::Global,
                                static_cast<uint8_t>(sys::Config::Section::Global::MdnsHostname),
                                1,
                                0),
              sys::Config::Status::Ack);

    EXPECT_EQ(ConfigHandler.get(sys::Config::Block::Global,
                                static_cast<uint8_t>(sys::Config::Section::Global::MdnsHostname),
                                0,
                                value),
              sys::Config::Status::Ack);
    EXPECT_EQ(value, 's');

    EXPECT_EQ(ConfigHandler.get(sys::Config::Block::Global,
                                static_cast<uint8_t>(sys::Config::Section::Global::MdnsHostname),
                                1,
                                value),
              sys::Config::Status::Ack);
    EXPECT_EQ(value, 0);
}

TEST_F(MdnsTest, FallsBackToGeneratedHostnameWhenCustomHostnameIsInvalid)
{
    set_custom_hostname("-bad");
    NetworkIdentityCollector collector;

    ASSERT_TRUE(_mdns.init());

    wait_for_signals(collector, 1);
    EXPECT_EQ(collector.signals().front().name, make_expected_name());
    EXPECT_EQ(_hwa.hostname + ".local", make_expected_name());
}

TEST_F(MdnsTest, RepublishesNetworkIdentityWhenIpChanges)
{
    NetworkIdentityCollector collector;

    ASSERT_TRUE(_mdns.init());
    wait_for_signals(collector, 1);

    _hwa.ip_address_value = "192.168.1.113";
    _hwa.trigger_ip_change();

    wait_for_signals(collector, 2);
    const auto signals = collector.signals();
    EXPECT_EQ(signals.back().name, make_expected_name());
    EXPECT_EQ(signals.back().ipv4_address, "192.168.1.113");
}

TEST_F(MdnsTest, StopsIpChangeCallbackOnDeinit)
{
    NetworkIdentityCollector collector;

    ASSERT_TRUE(_mdns.init());
    wait_for_signals(collector, 1);
    ASSERT_TRUE(_mdns.deinit());

    _hwa.ip_address_value = "192.168.1.113";
    _hwa.trigger_ip_change();

    ASSERT_EQ(collector.signals().size(), 1U);
}

TEST_F(MdnsTest, FailsWhenOscServiceCannotBeAdvertised)
{
    _hwa.osc_result = false;

    EXPECT_FALSE(_mdns.init());
}
