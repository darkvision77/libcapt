#include "Protocol.hpp"
#include "Core/CaptPacket.hpp"
#include "Core/PacketBuilder.hpp"
#include "Enums.hpp"
#include "ExtendedStatus.hpp"
#include "Core/PacketReader.hpp"
#include "ProtocolError.hpp"
#include <cstdint>
#include <iomanip>
#include <ios>
#include <sstream>

namespace Capt::Protocol {
    inline static void checkOpcode(uint16_t actual, uint16_t expected) {
        if (actual != expected) {
            std::ostringstream ss("unexpected response opcode: 0x");
            ss << std::hex << std::setfill('0') << std::uppercase << std::setw(4) << actual << std::nouppercase;
            ss << " (expected 0x" << std::uppercase << std::setw(4) << expected << ')';
            throw ProtocolError(ss.str());
        }
    }

    void IC_BEGIN_PAGE(std::ostream& stream, const PageParams& params) {
        PacketBuilder()
            .AppendUint16(0) // const uint16
            .AppendUint16(0x03fc) // TargetModel
            .AppendByte(params.PaperSize)
            .AppendByte(0) // const byte
            .AppendByte(0) // InputSlot = auto
            .AppendByte(0) // const byte
            .AppendByte(params.TonerDensity)
            .AppendByte(params.TonerDensity)
            .AppendByte(params.TonerDensity)
            .AppendByte(params.TonerDensity)
            .AppendByte(params.Mode)
            .AppendByte(params.Resolution)
            .AppendByte(3) // const byte
            .AppendByte(1) // const byte
            .AppendByte(1) // const byte
            .AppendByte(1) // const byte
            .AppendByte(params.SmoothEnable ? 2 : 0)
            .AppendByte(params.TonerSaving ? 1 : 0)
            .AppendByte(0) // const byte
            .AppendByte(0) // const byte
            .AppendUint16(params.MarginLeft)
            .AppendUint16(params.MarginTop)
            .AppendUint16(params.ImageLineSize)
            .AppendUint16(params.ImageLines)
            .AppendUint16(params.PaperWidth)
            .AppendUint16(params.PaperHeight)
            .WriteTo(stream, 0xD0A0) << std::flush;
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

    ExtendedStatus PC_GET_EXTENDED_STATUS(std::iostream& stream) {
        CaptPacket::WriteTo(stream, 0xA0A0) << std::flush;

        CaptPacket packet;
        stream >> packet;
        checkOpcode(packet.Opcode, 0xA0A0);
        PacketReader reader = PacketReader(packet);

        ExtendedStatus result;
        result.Basic = static_cast<BasicStatus>(reader.ReadByte());
        if ((result.Basic & BasicStatus::ERROR_BIT) != 0) {
            std::ostringstream ss("PC_GET_EXTENDED_STATUS returned error: 0x");
            ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(result.Basic);
            throw ProtocolError(ss.str());
        }
        result.Changed = reader.ReadByte();
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

    PrinterInfo PC_GET_PRINTER_INFO(std::iostream& stream) {
        CaptPacket::WriteTo(stream, 0xA1A1) << std::flush;

        CaptPacket packet;
        stream >> packet;
        checkOpcode(packet.Opcode, 0xA1A1);
        PacketReader reader = PacketReader(packet);

        PrinterInfo result;
        reader.ReadByte(); // local_15
        reader.ReadByte(); // local_16
        result.DeviceId = reader.ReadByte();
        result.Type = reader.ReadByte();
        result.VersionMajor = reader.ReadByte();
        result.VersionMinor = reader.ReadByte();
        result.BlockSize = reader.ReadUint16();
        result.Buffers = reader.ReadUint16();
        // other unknown fields
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
        PacketBuilder()
            .AppendUint32(0xadeadbee)
            .AppendUint16(pageNumber)
            .AppendByte(0) // const byte
            .AppendByte(0) // const byte
            .WriteTo(stream, 0xE0A5) << std::flush;

        CaptPacket packet;
        stream >> packet;
        checkOpcode(packet.Opcode, 0xE0A5);
        PacketReader reader = PacketReader(packet);
        uint8_t err = reader.ReadByte();
        return err;
    }

    uint8_t PCR_CLEANING(std::iostream& stream) {
        PacketBuilder()
            .AppendUint32(0xadeadbee)
            .AppendUint16(1) // const byte
            .AppendByte(0) // const byte
            .AppendByte(0) // const byte
            .WriteTo(stream, 0xE0AD) << std::flush;

        CaptPacket packet;
        stream >> packet;
        checkOpcode(packet.Opcode, 0xE0AD);
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
