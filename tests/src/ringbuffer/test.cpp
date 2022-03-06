#include "framework/Framework.h"
#include "core/src/general/RingBuffer.h"

namespace
{
    class RingBufferTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            buffer.reset();
        }

        static constexpr size_t                BUFFER_SIZE = 8;
        core::RingBuffer<uint8_t, BUFFER_SIZE> buffer;
    };
}    // namespace

TEST_F(RingBufferTest, Init)
{
    ASSERT_TRUE(buffer.isEmpty());
    ASSERT_FALSE(buffer.isFull());
    ASSERT_EQ(0, buffer.count());
}

TEST_F(RingBufferTest, Insertion)
{
    ASSERT_TRUE(buffer.insert(10));

    ASSERT_FALSE(buffer.isEmpty());
    ASSERT_FALSE(buffer.isFull());
    ASSERT_EQ(1, buffer.count());

    uint8_t value;

    ASSERT_TRUE(buffer.remove(value));
    ASSERT_EQ(10, value);

    ASSERT_EQ(0, buffer.count());
    ASSERT_TRUE(buffer.isEmpty());
    ASSERT_FALSE(buffer.isFull());

    // assign random value
    value = 147;

    // try to remove again
    ASSERT_FALSE(buffer.remove(value));

    // verify that the value hasn't changed
    ASSERT_EQ(147, value);

    // fill the entire buffer
    // buffer has room for one less element than specified
    for (size_t i = 0; i < BUFFER_SIZE - 1; i++)
    {
        ASSERT_TRUE(buffer.insert(10 + i));
    }

    ASSERT_FALSE(buffer.isEmpty());
    ASSERT_TRUE(buffer.isFull());
    ASSERT_EQ(BUFFER_SIZE - 1, buffer.count());

    for (size_t i = 0; i < BUFFER_SIZE - 1; i++)
    {
        ASSERT_TRUE(buffer.remove(value));
        ASSERT_EQ(10 + i, value);
        ASSERT_EQ(BUFFER_SIZE - 2 - i, buffer.count());
    }

    ASSERT_TRUE(buffer.isEmpty());
    ASSERT_FALSE(buffer.isFull());

    // verify that overwriting isn't possible
    for (size_t i = 0; i < BUFFER_SIZE - 1; i++)
    {
        ASSERT_TRUE(buffer.insert(10 + i));
    }

    ASSERT_FALSE(buffer.insert(10));

    ASSERT_FALSE(buffer.isEmpty());
    ASSERT_TRUE(buffer.isFull());

    buffer.reset();

    ASSERT_TRUE(buffer.isEmpty());
    ASSERT_FALSE(buffer.isFull());
    ASSERT_EQ(0, buffer.count());

    ASSERT_TRUE(buffer.insert(12));
    ASSERT_FALSE(buffer.isEmpty());
    ASSERT_FALSE(buffer.isFull());
    ASSERT_EQ(1, buffer.count());
    ASSERT_TRUE(buffer.remove(value));
    ASSERT_EQ(12, value);
}