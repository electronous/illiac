require 'rubygems'
require 'bindata'
require 'singleton'

class ZeroOperandInst < BinData::Record
	endian :little

	uint8 :opcode
	bit1  :flag
end

class ByteWriter
	include Singleton

	def initialize
		@instructions = []
	end

	def add_instruction(instruction)
		@instructions << instruction
	end

	def write_out(outfile)
		File.open(outfile, "w") do |fh|
			@instructions.each do |instruction|
				fh.write(instruction.to_binary_s)
			end
		end
	end
end

def abs_short
	new_op = ZeroOperandInst.new
	new_op.opcode = 0b10010100
	new_op.flag   = 1
	ByteWriter.instance.add_instruction(new_op)
end

def abs_long
	new_op = ZeroOperandInst.new
	new_op.opcode = 0b10010101
	new_op.flag   = 1
	ByteWriter.instance.add_instruction(new_op)
end

def dupb
	new_op = ZeroOperandInst.new
	new_op.opcode = 0b00100100
	new_op.flag   = 0
	ByteWriter.instance.add_instruction(new_op)
end

def duphw
	new_op = ZeroOperandInst.new
	new_op.opcode = 0b00100101
	new_op.flag   = 0
	ByteWriter.instance.add_instruction(new_op)
end

def dupw
	new_op = ZeroOperandInst.new
	new_op.opcode = 0b00100110
	new_op.flag   = 0
	ByteWriter.instance.add_instruction(new_op)
end

def sluffb
	new_op = ZeroOperandInst.new
	new_op.opcode = 0b00100000
	new_op.flag   = 0
	ByteWriter.instance.add_instruction(new_op)
end

def sluffhw
	new_op = ZeroOperandInst.new
	new_op.opcode = 0b00100001
	new_op.flag   = 0
	ByteWriter.instance.add_instruction(new_op)
end

def sluffw
	new_op = ZeroOperandInst.new
	new_op.opcode = 0b00100001
	new_op.flag   = 0
	ByteWriter.instance.add_instruction(new_op)
end

def xchb
	new_op = ZeroOperandInst.new
	new_op.opcode = 0b00101000
	new_op.flag   = 0
	ByteWriter.instance.add_instruction(new_op)
end

def xchhw
	new_op = ZeroOperandInst.new
	new_op.opcode = 0b00101001
	new_op.flag   = 0
	ByteWriter.instance.add_instruction(new_op)
end

def xchw
	new_op = ZeroOperandInst.new
	new_op.opcode = 0b00101001
	new_op.flag   = 0
	ByteWriter.instance.add_instruction(new_op)
end
