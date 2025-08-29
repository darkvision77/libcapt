#include "libcapt/Core/PacketReader.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace Capt;

TEST(PacketReader, ReadByte) {
    CaptPacket packet(0x1234, {1, 250, 0});
    PacketReader reader(packet);
    ASSERT_EQ(reader.ReadByte(), 1);
    ASSERT_EQ(reader.ReadByte(), 250);
    ASSERT_EQ(reader.ReadByte(), 0);
}

TEST(PacketReader, ReadUint16) {
    CaptPacket packet(0x1234, {0x42, 0xff, 0xfe, 0});
    PacketReader reader(packet);
    ASSERT_EQ(reader.ReadUint16(), 0xff42);
    ASSERT_EQ(reader.ReadUint16(), 0x00fe);
}

TEST(PacketReader, ReadUint32) {
    CaptPacket packet(0x1234, {0x42, 0xff, 0xfe, 0});
    PacketReader reader(packet);
    ASSERT_EQ(reader.ReadUint32(), 0x00feff42);
}

TEST(PacketReader, Basic) {
    CaptPacket packet(0x1234, {1, 0xf2, 0x42, 0xff, 0xfe, 0, 1});
    PacketReader reader(packet);
    ASSERT_EQ(reader.ReadByte(), 1);
    ASSERT_EQ(reader.ReadUint16(), 0x42f2);
    ASSERT_EQ(reader.ReadUint32(), 0x0100feff);
}

TEST(PacketReader, ReadByteEOF) {
    EXPECT_THROW({
        CaptPacket packet(0x1234, {0});
        PacketReader reader(packet);
        reader.ReadByte();
        reader.ReadByte();
    }, std::out_of_range);
}

TEST(PacketReader, ReadUint16EOF) {
    EXPECT_THROW({
        CaptPacket packet(0x1234, {0});
        PacketReader reader(packet);
        reader.ReadUint16();
    }, std::out_of_range);
}

TEST(PacketReader, ReadUint32EOF) {
    EXPECT_THROW({
        CaptPacket packet(0x1234, {0});
        PacketReader reader(packet);
        reader.ReadUint32();
    }, std::out_of_range);
}

TEST(PacketReader, ReadBytesZero) {
    CaptPacket packet(0x1234, {});
    PacketReader reader(packet);
    ASSERT_EQ(reader.ReadBytes(0).size(), 0);
}

TEST(PacketReader, ReadBytes) {
    CaptPacket packet(0x1234, {1, 0xf2, 0x42, 0xff, 0xfe, 0, 1});
    PacketReader reader(packet);
    ASSERT_THAT(reader.ReadBytes(1), testing::ElementsAre(1));
    ASSERT_THAT(reader.ReadBytes(2), testing::ElementsAre(0xf2, 0x42));
    ASSERT_THAT(reader.ReadBytes(4), testing::ElementsAre(0xff, 0xfe, 0, 1));
}

TEST(PacketReader, ReadBytesEOF) {
    EXPECT_THROW({
        CaptPacket packet(0x1234, {0});
        PacketReader reader(packet);
        reader.ReadBytes(1);
        reader.ReadBytes(1);
    }, std::out_of_range);

    EXPECT_THROW({
        CaptPacket packet(0x1234, {0});
        PacketReader reader(packet);
        reader.ReadBytes(2);
    }, std::out_of_range);
}
