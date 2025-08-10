#include "Protocol.hpp"
#include "Core/PacketBuilder.hpp"
#include "Enums.hpp"
#include "ExtendedStatus.hpp"
#include "Core/PacketReader.hpp"
#include <cassert>
#include <cstdint>

namespace Capt::Protocol {
    void IC_BEGIN_PAGE(std::ostream& stream, const PageParams& params) {
        Capt::PacketBuilder builder(0xD0A0);
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
        builder.Packet.WriteTo(stream);
        stream.flush();
    }

    void IC_BEGIN_DATA(std::ostream& stream) {
        Capt::CaptPacket(0xD0A1).WriteTo(stream);
        stream.flush();
    }

    void IC_END_PAGE(std::ostream& stream) {
        Capt::CaptPacket(0xD0A2).WriteTo(stream);
        stream.flush();
    }

    void IC_VIDEO_DATA(std::ostream& stream, uint8_t* data, std::size_t count) {
        // TODO: optimize?
        Capt::CaptPacket packet(0xC0A0, std::vector<uint8_t>(data, data + count));
        packet.WriteTo(stream);
        stream.flush();
    }

    ExtendedStatus PC_GET_EXTENDED_STATUS(std::iostream& stream) {
        Capt::CaptPacket(0xA0A0).WriteTo(stream);
        stream.flush();

        Capt::CaptPacket packet = Capt::CaptPacket::ReadFrom(stream);
        Capt::PacketReader reader = Capt::PacketReader(packet);

        ExtendedStatus result;
        result.Basic = static_cast<BasicStatus>(reader.ReadByte());
        assert((result.Basic & BasicStatus::ERROR_BIT) == 0);
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
        Capt::CaptPacket(0xE0A0).WriteTo(stream);
        stream.flush();

        Capt::CaptPacket packet = Capt::CaptPacket::ReadFrom(stream);
        assert(packet.Opcode == 0xE0A0);
        Capt::PacketReader reader = Capt::PacketReader(packet);
        BasicStatus status = static_cast<BasicStatus>(reader.ReadByte());
        uint8_t ch = reader.ReadByte();
        if (changed != nullptr) {
            *changed = ch;
        }
        return status;
    }

    uint8_t PCR_GO_ONLINE(std::iostream& stream, uint16_t pageNumber) {
        Capt::PacketBuilder builder(0xE0A5);
        builder.AppendUint32(0xadeadbee);
        builder.AppendUint16(pageNumber);
        builder.AppendByte(0); // const byte
        builder.AppendByte(0); // const byte
        builder.Packet.WriteTo(stream);
        stream.flush();

        Capt::CaptPacket packet = Capt::CaptPacket::ReadFrom(stream);
        assert(packet.Opcode == builder.Packet.Opcode);
        Capt::PacketReader reader = Capt::PacketReader(packet);
        uint8_t err = reader.ReadByte();
        return err;
    }

    uint8_t PCR_CLEANING(std::iostream& stream) {
        Capt::PacketBuilder builder(0xE0AD);
        builder.AppendUint32(0xadeadbee);
        builder.AppendUint16(1); // const byte
        builder.AppendByte(0); // const byte
        builder.AppendByte(0); // const byte
        builder.Packet.WriteTo(stream);
        stream.flush();

        Capt::CaptPacket packet = Capt::CaptPacket::ReadFrom(stream);
        assert(packet.Opcode == builder.Packet.Opcode);
        Capt::PacketReader reader = Capt::PacketReader(packet);
        uint8_t err = reader.ReadByte();
        // there is one more byte ignored by the original software
        return err;
    }

    static uint8_t execCmd(std::iostream& stream, uint16_t opcode) {
        Capt::CaptPacket(opcode).WriteTo(stream);
        stream.flush();

        Capt::CaptPacket packet = Capt::CaptPacket::ReadFrom(stream);
        assert(packet.Opcode == opcode);
        Capt::PacketReader reader = Capt::PacketReader(packet);
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
