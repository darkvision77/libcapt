#include "Protocol.hpp"
#include "Core/CaptPacket.hpp"
#include "Core/PacketBuilder.hpp"
#include "Enums.hpp"
#include "ExtendedStatus.hpp"
#include "Core/PacketReader.hpp"
#include "ProtocolError.hpp"
#include <cstdint>
#include <expected>
#include <format>

namespace Capt::Protocol {
    static constexpr void checkOpcode(uint16_t actual, uint16_t expected) {
        if (actual != expected) {
            throw ProtocolError(std::format("unexpected response opcode: 0x{:04X} (expected 0x{:04X})", actual, expected));
        }
    }

    void IC_BEGIN_PAGE(std::ostream& stream, const PageParams& params) {
        PacketBuilder builder(0xD0A0);
        builder.AppendUint16(0); // const uint16
        builder.AppendUint16(0x03fc); // TargetModel
        builder.AppendByte(params.PaperSize);
        builder.AppendByte(0); // const byte
        builder.AppendByte(0); // InputSlot = auto
        builder.AppendByte(0); // const byte
        builder.AppendByte(params.TonerDensity);
        builder.AppendByte(params.TonerDensity);
        builder.AppendByte(params.TonerDensity);
        builder.AppendByte(params.TonerDensity);
        builder.AppendByte(params.Mode);
        builder.AppendByte(params.Resolution);
        builder.AppendByte(3); // const byte
        builder.AppendByte(1); // const byte
        builder.AppendByte(1); // const byte
        builder.AppendByte(1); // const byte
        builder.AppendByte(params.SmoothEnable ? 2 : 0);
        builder.AppendByte(params.TonerSaving ? 1 : 0);
        builder.AppendByte(0); // const byte
        builder.AppendByte(0); // const byte
        builder.AppendUint16(params.MarginLeft);
        builder.AppendUint16(params.MarginTop);
        builder.AppendUint16(params.ImageLineSize);
        builder.AppendUint16(params.ImageLines);
        builder.AppendUint16(params.PaperWidth);
        builder.AppendUint16(params.PaperHeight);
        stream << builder.Packet << std::flush;
    }

    void IC_BEGIN_DATA(std::ostream& stream) {
        CaptPacket::WriteTo(stream, 0xD0A1) << std::flush;
    }

    void IC_END_PAGE(std::ostream& stream) {
        CaptPacket::WriteTo(stream, 0xD0A2) << std::flush;
    }

    void IC_VIDEO_DATA(std::ostream& stream, std::span<const uint8_t> data) {
        CaptPacket::WriteTo(stream, 0xC0A0, data) << std::flush;
    }

    std::expected<ExtendedStatus, BasicStatus> PC_GET_EXTENDED_STATUS(std::iostream& stream) {
        CaptPacket::WriteTo(stream, 0xA0A0) << std::flush;

        CaptPacket packet;
        stream >> packet;
        checkOpcode(packet.Opcode, 0xA0A0);
        PacketReader reader = PacketReader(packet);

        ExtendedStatus result;
        result.Basic = static_cast<BasicStatus>(reader.ReadByte());
        if ((result.Basic & BasicStatus::ERROR_BIT) != 0) {
            return std::unexpected(result.Basic);
        }
        reader.ReadByte(); // param_1 + 0x279
        result.Aux = static_cast<AuxStatus>(reader.ReadByte());
        result.Controller = static_cast<ControllerStatus>(reader.ReadByte());
        result.PaperAvailableBits = reader.ReadByte();
        reader.ReadByte(); // param_1 + 0x27d
        result.Engine = static_cast<EngineReadyStatus>(reader.ReadUint16());
        result.Start = reader.ReadUint16();
        result.Printing = reader.ReadUint16();
        result.Shipped = reader.ReadUint16();
        result.Printed = reader.ReadUint16();
        return result;
    }

    BasicStatus PCR_GET_BASIC_STATUS(std::iostream& stream, uint8_t* changed) {
        CaptPacket::WriteTo(stream, 0xE0A0) << std::flush;

        CaptPacket packet;
        stream >> packet;
        checkOpcode(packet.Opcode, 0xE0A0);
        PacketReader reader = PacketReader(packet);
        BasicStatus status = static_cast<BasicStatus>(reader.ReadByte());
        uint8_t ch = reader.ReadByte();
        if (changed != nullptr) {
            *changed = ch;
        }
        return status;
    }

    uint8_t PCR_GO_ONLINE(std::iostream& stream, uint16_t pageNumber) {
        PacketBuilder builder(0xE0A5);
        builder.AppendUint32(0xadeadbee);
        builder.AppendUint16(pageNumber);
        builder.AppendByte(0); // const byte
        builder.AppendByte(0); // const byte
        stream << builder.Packet << std::flush;

        CaptPacket packet;
        stream >> packet;
        checkOpcode(packet.Opcode, builder.Packet.Opcode);
        PacketReader reader = PacketReader(packet);
        uint8_t err = reader.ReadByte();
        return err;
    }

    uint8_t PCR_CLEANING(std::iostream& stream) {
        PacketBuilder builder(0xE0AD);
        builder.AppendUint32(0xadeadbee);
        builder.AppendUint16(1); // const byte
        builder.AppendByte(0); // const byte
        builder.AppendByte(0); // const byte
        stream << builder.Packet << std::flush;

        CaptPacket packet;
        stream >> packet;
        checkOpcode(packet.Opcode, builder.Packet.Opcode);
        PacketReader reader = PacketReader(packet);
        uint8_t err = reader.ReadByte();
        // there is one more byte ignored by the original software
        return err;
    }

    static uint8_t execCmd(std::iostream& stream, uint16_t opcode) {
        CaptPacket::WriteTo(stream, opcode) << std::flush;

        CaptPacket packet;
        stream >> packet;
        checkOpcode(packet.Opcode, opcode);
        PacketReader reader = PacketReader(packet);
        uint8_t err = reader.ReadByte();
        return err;
    }

    uint8_t PC_RESERVE_UNIT(std::iostream& stream) { return execCmd(stream, 0xA2A0); }
    uint8_t PCR_DISCARD_DATA(std::iostream& stream) { return execCmd(stream, 0xE0A4); }
    uint8_t PCR_CLEAR_ERROR(std::iostream& stream) { return execCmd(stream, 0xE0A2); }
    uint8_t PCR_GO_OFFLINE(std::iostream& stream) { return execCmd(stream, 0xE0A6); }
    uint8_t PCR_RELEASE_UNIT(std::iostream& stream) { return execCmd(stream, 0xE0A9); }
    uint8_t PCR_CLEAR_MISPRINT(std::iostream& stream) { return execCmd(stream, 0xE0A3); }
    uint8_t PCR_RESET_ENGINE(std::iostream& stream) { return execCmd(stream, 0xE0A1); }
}
