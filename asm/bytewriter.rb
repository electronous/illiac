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
    def initialize(outfile)
        @outfile = outfile
    end

    def go(&block)
        File.open(@outfile, "w") do |@out_fh|
            block.call
        end
    end

    def DUPB()
        out = ZeroOperand.new
        out.opcode = 0b00100100
        out.flag   = 1
        @out_fh.write(out)
    end
end
