require 'rubygems'
require 'bindata'

class ZeroOperand < BinData::Record
    endian :little

    uint8  :opcode
    bit1   :flag

    def to_s(*args)
        return to_binary_s(*args)
    end
end

class ByteWriter
    def initialize(fh)
        @fh = fh
    end

    def DUPB()
        out = ZeroOperand.new
        out.opcode = 0b00100100
        out.flag   = 1
        @fh.write(out)
    end
end
