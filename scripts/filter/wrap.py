import gdb # type: ignore
import os

CMDFILE = os.environ["WRAP_CMDFILE"]
OUTFILE = os.environ["WRAP_OUTFILE"]
INFILE = os.environ["WRAP_INFILE"]
FILTEREXE = os.environ["WRAP_FILTEREXE"]

ARGS = [
    "--CNTblModel=0",
    "--PageSize=A4",
    "--Resolution=600",
    "--CNTonerSaving=False",
    "--CNSuperSmooth=True",
    INFILE,
]

cmdfs = open(CMDFILE, mode="w", encoding="utf8", buffering=1)
lastCmd = None

class ShiftBufferBreakpoint(gdb.Breakpoint):
    def stop(self) -> bool:
        remain = int(gdb.parse_and_eval("(int)$edi"))
        count = int(gdb.parse_and_eval("(int)$eax"))
        addr = int(gdb.parse_and_eval("*(int**)($ebp - 0x480 + 4)")) + 9
        # cmdfs.write(f"\naddr={hex(addr)} value={hex(int(gdb.parse_and_eval(f"*(unsigned char*){hex(addr)}")))} count={count} remain={remain}")
        buffer = gdb.parse_and_eval(f"*(unsigned char(*)[{count + (remain - count)}])({addr})")
        gdb.execute(f"set *(unsigned char(*)[{remain - count}])({addr}) = {{{str.join(', ', map(hex, buffer.bytes[count:]))}}}")
        return False

class CommandBreakpoint(gdb.Breakpoint):
    def __init__(self, name: str, *args, **kwargs) -> None:
        super().__init__(*args, **kwargs)
        self.name = name

    def stop(self) -> bool:
        global lastCmd
        if lastCmd is not None:
            cmdfs.write("\n")
        cmdfs.write(self.name)
        lastCmd = self.name
        return False

class WriteBreakpoint(gdb.Breakpoint):
    def stop(self) -> bool:
        assert lastCmd is not None
        value = int(gdb.parse_and_eval("*(int*)($ebp + 0x8 + 4)"))
        cmdfs.write(" " + hex(value))
        return False

class WriteArrayBreakpoint(gdb.Breakpoint):
    def stop(self) -> bool:
        assert lastCmd is not None
        count = int(gdb.parse_and_eval("*(int*)($ebp + 0xc + 4)"))
        assert count > 0
        extendCount = 2
        buffer = gdb.parse_and_eval(f"**(unsigned char(**)[{count + extendCount}])($ebp + 0x8 + 4)")
        cmdfs.write(f" len={count} " + str.join(" ", map(hex, buffer.bytes[:-extendCount])))
        cmdfs.write(f" extended: " + str.join(" ", map(hex, buffer.bytes[count:])))
        return False

class NoMarginsBreakpoint(gdb.Breakpoint):
    def stop(self) -> bool:
        gdb.execute("set *(int*)($ecx + 0x190) = 0") # top
        gdb.execute("set *(int*)($ecx + 0x194) = 0") # bottom
        gdb.execute("set *(int*)($ecx + 0x198) = 0") # left
        gdb.execute("set *(int*)($ecx + 0x19c) = 0") # right
        return False

gdb.execute("set verbose off")
# gdb.execute(f"set logging file {LOGFILE}")
# gdb.execute("set logging overwrite on")
# gdb.execute("set logging redirect on")
# gdb.execute("set logging enabled on")

gdb.execute(f"file {FILTEREXE}")
gdb.execute(f"set args {str.join(' ', ARGS)} > {OUTFILE}")

CommandBreakpoint("CopyThenRepeat", "*0x804f03b")
CommandBreakpoint("CopyThenRaw", "*0x804f0a0")
CommandBreakpoint("Extend", "*0x804f102")
CommandBreakpoint("RepeatXLong", "*0x804f17a")
CommandBreakpoint("CopyThenRepeatLong", "*0x804f227")
CommandBreakpoint("RepeatThenRawLong", "*0x804f2e2")
CommandBreakpoint("CopyThenRawLong", "*0x804f3a9")
CommandBreakpoint("CopyLong", "*0x804f6d9")
CommandBreakpoint("EOP", "*0x804f847")
CommandBreakpoint("RepeatX", "*0x804f9ea")
CommandBreakpoint("CopyShort", "*0x804fb66")
CommandBreakpoint("EOL", "*0x804fd61")
CommandBreakpoint("NOP", "*0x804fd81")
CommandBreakpoint("RepeatThenRaw", "*0x804fe8d")

# Fix captfilter's *ThenRaw bug
ShiftBufferBreakpoint("*0x804fdc2")
ShiftBufferBreakpoint("*0x804fb48")
ShiftBufferBreakpoint("*0x804fc77")
ShiftBufferBreakpoint("*0x804fc9d")
ShiftBufferBreakpoint("*0x804fe45")
ShiftBufferBreakpoint("*0x804f79e")
ShiftBufferBreakpoint("*0x804f7c1")

# WriteBreakpoint("*0x804ec48")
# WriteArrayBreakpoint("*0x804ec80")

NoMarginsBreakpoint("*0x080496cc")

gdb.execute("run")
cmdfs.write("\n")
cmdfs.close()

exitcode = int(gdb.parse_and_eval("$_exitcode"))
assert exitcode == 0, f"non-zero exit code: {exitcode}"
