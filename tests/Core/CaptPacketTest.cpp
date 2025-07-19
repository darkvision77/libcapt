#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "libcapt/Core/CaptPacket.hpp"
#include "MemoryStream.hpp"

using testing::ElementsAreArray;
using testing::ElementsAre;
using namespace Capt;

class CaptPacketTest : public testing::Test {
protected:
    MemoryStream buffer;
    CaptPacket packet;

    void WriteOpcode(uint16_t opcode) {
        this->packet = CaptPacket(opcode);
        this->packet.WriteTo(this->buffer);
        ASSERT_EQ(this->buffer.Buffer().size(), 4);
    }

    void WritePayload(uint16_t opcode, std::vector<uint8_t> payload) {
        this->packet = CaptPacket(opcode, payload);
        this->packet.WriteTo(this->buffer);
        ASSERT_EQ(this->buffer.Buffer().size(), 4 + payload.size());
    }

    void Read(std::vector<uint8_t> data) {
        this->buffer = MemoryStream(data);
        this->packet = CaptPacket::ReadFrom(this->buffer);
        ASSERT_EQ(this->packet.Size(), data.size());
        ASSERT_EQ(this->packet.Payload.size(), data.size() - 4);
    }
};

TEST_F(CaptPacketTest, WriteNoPayload) {
    WriteOpcode(0x1234);
    EXPECT_THAT(buffer.Buffer(), ElementsAre(0x34, 0x12, 0x04, 0x00));
}

TEST_F(CaptPacketTest, WritePayload) {
    WritePayload(0x1234, {1, 2});
    EXPECT_THAT(buffer.Buffer(), ElementsAre(0x34, 0x12, 6, 0, 1, 2));
}

TEST_F(CaptPacketTest, WritePayload2) {
    WritePayload(0x1234, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    EXPECT_THAT(buffer.Buffer(), ElementsAre(0x34, 0x12, 14, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10));
}

TEST_F(CaptPacketTest, WriteLongPayload) {
    std::vector<uint8_t> payload;
    std::vector<uint8_t> expected{0x34, 0x12, 24, 2};
    for (std::size_t i = 0; i < 532; i++) {
        payload.push_back(i & 0xff);
        expected.push_back(i & 0xff);
    }
    WritePayload(0x1234, payload);
    EXPECT_THAT(buffer.Buffer(), ElementsAreArray(expected));
}

TEST_F(CaptPacketTest, WriteOverflow) {
    EXPECT_THROW({
        std::vector<uint8_t> payload(65535 - 4 + 1);
        WritePayload(0x1234, payload);
    }, std::overflow_error);
}

TEST_F(CaptPacketTest, ReadNoPayload) {
    Read({0x34, 0x12, 0x04, 0x00});
    ASSERT_EQ(packet.Opcode, 0x1234);
}

TEST_F(CaptPacketTest, ReadPayload) {
    Read({0x34, 0x12, 6, 0, 1, 2});
    ASSERT_EQ(packet.Opcode, 0x1234);
    EXPECT_THAT(packet.Payload, ElementsAre(1, 2));
}

TEST_F(CaptPacketTest, ReadPayload2) {
    Read({0x34, 0x12, 14, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    ASSERT_EQ(packet.Opcode, 0x1234);
    EXPECT_THAT(packet.Payload, ElementsAre(1, 2, 3, 4, 5, 6, 7, 8, 9, 10));
}

TEST_F(CaptPacketTest, ReadLongPayload) {
    std::vector<uint8_t> payload;
    std::vector<uint8_t> data{0x34, 0x12, 24, 2};
    for (std::size_t i = 0; i < 532; i++) {
        payload.push_back(i & 0xff);
        data.push_back(i & 0xff);
    }
    Read(data);
    ASSERT_EQ(packet.Opcode, 0x1234);
    EXPECT_THAT(packet.Payload, ElementsAreArray(payload));
}
