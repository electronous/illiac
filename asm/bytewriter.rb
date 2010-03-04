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

def abss
	new_inst = ZeroOperandInst.new
	new_inst.opcode = 0b10010100
	new_inst.flag   = 1
	ByteWriter.instance.add_instruction(new_inst)
end

def absl
	new_inst = ZeroOperandInst.new
	new_inst.opcode = 0b10010101
	new_inst.flag   = 1
	ByteWriter.instance.add_instruction(new_inst)
end

def dupb
	new_inst = ZeroOperandInst.new
	new_inst.opcode = 0b00100100
	new_inst.flag   = 0
	ByteWriter.instance.add_instruction(new_inst)
end

def duphw
	new_inst = ZeroOperandInst.new
	new_inst.opcode = 0b00100101
	new_inst.flag   = 0
	ByteWriter.instance.add_instruction(new_inst)
end

def dupw
	new_inst = ZeroOperandInst.new
	new_inst.opcode = 0b00100110
	new_inst.flag   = 0
	ByteWriter.instance.add_instruction(new_inst)
end

def sluffb
	new_inst = ZeroOperandInst.new
	new_inst.opcode = 0b00100000
	new_inst.flag   = 0
	ByteWriter.instance.add_instruction(new_inst)
end

def sluffhw
	new_inst = ZeroOperandInst.new
	new_inst.opcode = 0b00100001
	new_inst.flag   = 0
	ByteWriter.instance.add_instruction(new_inst)
end

def sluffw
	new_inst = ZeroOperandInst.new
	new_inst.opcode = 0b00100001
	new_inst.flag   = 0
	ByteWriter.instance.add_instruction(new_inst)
end

def xchb
	new_inst = ZeroOperandInst.new
	new_inst.opcode = 0b00101000
	new_inst.flag   = 0
	ByteWriter.instance.add_instruction(new_inst)
end

def xchhw
	new_inst = ZeroOperandInst.new
	new_inst.opcode = 0b00101001
	new_inst.flag   = 0
	ByteWriter.instance.add_instruction(new_inst)
end

def xchw
	new_inst = ZeroOperandInst.new
	new_inst.opcode = 0b00101001
	new_inst.flag   = 0
	ByteWriter.instance.add_instruction(new_inst)
end

def oneb
	new_inst = ZeroOperandInst.new
	new_inst.opcode = 0b00000100
	new_inst.flag   = 0
	ByteWriter.instance.add_instruction(new_inst)
end

def hcf
	new_inst = ZeroOperandInst.new
	new_inst.opcode = 0b11111111
	new_inst.flag   = 0
	ByteWriter.instance.add_instruction(new_inst)
end
