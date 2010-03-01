require 'rubygems'
require 'bindata'

class ZeroOperand < BinData::Record
    endian :little

    uint8  :opcode
    bit1   :flag
end

class ByteWriter
    def initialize(outfile)
        @outfile = outfile
    end

    def DUPB
        new_op = ZeroOperand.new
        new_op.opcode = 0b00100100
        new_op.flag   = 1
        @instructions.push(new_op)
    end

    def go(&block)
        @instructions = Array.new
        File.open(@outfile, "w") do |@fh|
            yield self

            @instructions.each do |instruction|
                @fh.write(instruction.to_binary_s)
            end
        end
    end
end
