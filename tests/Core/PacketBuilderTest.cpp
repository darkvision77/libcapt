#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdexcept>
#include "libcapt/Core/PacketBuilder.hpp"

using testing::ElementsAre;
using namespace Capt;

TEST(PacketBuilder, AppendByte) {
    PacketBuilder builder(0x1234);
    builder.AppendByte(1);
    builder.AppendByte(2);
    builder.AppendByte(3);
    ASSERT_EQ(builder.Packet.Opcode, 0x1234);
    EXPECT_THAT(builder.Packet.Payload, ElementsAre(1, 2, 3));
}

TEST(PacketBuilder, AppendUint16) {
    PacketBuilder builder(0x1234);
    builder.AppendUint16(0x1234);
    builder.AppendUint16(0x1200);
    builder.AppendUint16(0x0034);
    ASSERT_EQ(builder.Packet.Opcode, 0x1234);
    EXPECT_THAT(builder.Packet.Payload, ElementsAre(0x34, 0x12, 0x00, 0x12, 0x34, 0x00));
}

TEST(PacketBuilder, AppendUint32) {
    PacketBuilder builder(0x1234);
    builder.AppendUint32(0xffaabbcc);
    builder.AppendUint32(0x00fccf00);
    ASSERT_EQ(builder.Packet.Opcode, 0x1234);
    EXPECT_THAT(builder.Packet.Payload, ElementsAre(0xcc, 0xbb, 0xaa, 0xff, 0x00, 0xcf, 0xfc, 0x00));
}

TEST(PacketBuilder, AppendBytes) {
    std::vector<uint8_t> data{0xff, 0x0f, 0xaa};
    PacketBuilder builder(0x1234);
    builder.AppendByte(1);
    builder.AppendByte(2);
    builder.AppendBytes(data.data(), data.size());
    builder.AppendByte(3);
    ASSERT_EQ(builder.Packet.Opcode, 0x1234);
    EXPECT_THAT(builder.Packet.Payload, ElementsAre(1, 2, 0xff, 0x0f, 0xaa, 3));
}

TEST(PacketBuilder, Overflow) {
    EXPECT_THROW({
        std::vector<uint8_t> data(0xffff - 7);
        PacketBuilder builder(0x1234);
        builder.AppendBytes(data.data(), data.size());
        builder.AppendUint32(1);
    }, std::overflow_error);
}

TEST(PacketBuilder, AppendBytesOverflow) {
    EXPECT_THROW({
        std::vector<uint8_t> data(0xffff);
        PacketBuilder builder(0x1234);
        builder.AppendBytes(data.data(), data.size());
    }, std::overflow_error);
}
