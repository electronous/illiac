require 'rubygems'
require 'bindata'

module Illiac
	@@labels = {}
	def self.labels
		return @@labels
	end
	def self.assemble input
		IlliacAsm.new(input)
	end
	class Instruction < BinData::Record
		endian :little
		uint8 :opcode_b
		bit7
		bit1 :flag_b
		class InstructionArgumentError < StandardError; end
		class UndefinedLabel < StandardError; end

		# Get a metaclass for this class
		def self.metaclass; class << self; self; end; end

		# Advanced metaprogramming code for nice, clean instructions
		def self.parameters( *arr )
			return @parameters if arr.empty?

			# 1. Set up accessors for each variable
			attr_accessor *arr

			# 2. Add a new class method to for each param
			arr.each do |a|
				metaclass.instance_eval do
					define_method( a ) do |val|
						@parameters ||= {}
						@parameters[a] = val
					end
				end
			end

			# 3. For each instruction, the `initialize' method
			#    should use the default arg for each param.
			class_eval do
				define_method( :initialize ) do |*args|
					super()
					self.class.parameters.each do |k,v|
						instance_variable_set("@#{k}", v)
					end
					if args.size != instance_variable_get('@arguments')
						raise InstructionArgumentError 
					end
					self.opcode_b = @opcode
					self.flag_b = @flag
					@labels = Illiac::labels
					@args = args
				end
			end
		end

		def size
			1
		end

		def to_str
			@opcode.to_s + @args.reduce('') do |m,a|
				if a.instance_of? Symbol
					a = @labels[a]
					raise UndefinedLabel if a.nil?
				end
				m << a.to_s
			end
		end

		# Instruction parameters are read-only
		parameters :opcode, :flag, :arguments
	end


	class Abss < Instruction
		opcode 0b10010100
		flag 1
		arguments 0
	end

	class Absl < Instruction
		opcode 0b10010101
		flag 1
		arguments 0
	end

	class Dupb < Instruction
		opcode 0b00100100
		flag 0
		arguments 0
	end

	class Duphw < Instruction
		opcode 0b00100101
		flag 0
		arguments 0
	end

	class Dupw < Instruction
		opcode 0b00100110
		flag 0
		arguments 0
	end

	class Sluffb < Instruction
		opcode 0b00100000
		flag 0
		arguments 0
	end

	class Sluffhw < Instruction
		opcode 0b00100001
		flag 0
		arguments 0
	end

	class Sluffw < Instruction
		opcode 0b00100010
		flag 0
		arguments 0
	end

	class Xchb < Instruction
		opcode 0b00101000
		flag 0
		arguments 0
	end

	class Xchhw < Instruction
		opcode 0b00101001
		flag 0
		arguments 0
	end

	class Xchw < Instruction
		opcode 0b00101010
		flag 0
		arguments 0
	end

	class Oneb < Instruction
		opcode 0b00000100
		flag 0
		arguments 0
	end

	class Hcf < Instruction
		opcode 0b11111111
		flag 0
		arguments 0
	end

#	class PseudoOp < Instruction
#		endian :little
#		def to_s
#			@args.reduce('') do |m,a|
#				if a.instance_of? Symbol
#					a = @labels[a]
#					raise UndefinedLabel if a.nil?
#				end
#				m << a.to_s
#			end
#		end
#
#		def size
#			@args.reduce(0){|m,a| m += a.to_s.size}
#		end
#	end
#
#	class Stringz < PseudoOp
#		array :string, :type => :uint8
#		arguments 1
#	end
#
#	class Word < PseudoOp
#		opcode 0
#		flag 0
#		arguments 1
#	end
#
#	class HalfWord < PseudoOp
#		opcode 0
#		flag 0
#		arguments 1
#	end
#
#	class Byte < PseudoOp
#		opcode 0
#		flag 0
#		arguments 1
#	end

	class IlliacAsm
		Illiac.constants.each do |klass|
			if Illiac.const_get(klass).superclass == Illiac::Instruction
				instance_eval do
					define_method(klass.downcase) do |*args|
						inst = Illiac.const_get(klass).new(*args)
						@asm << inst
						@lineno += inst.size
					end
				end
			end
			#if Illiac.const_get(klass).superclass == Illiac::PseudoOp
			#	instance_eval do
			#		define_method("pseudo_#{klass.downcase}") do |*args|
			#			inst = Illiac.const_get(klass).new(*args)
			#			@asm << inst
			#			@lineno += inst.size
			#		end
			#	end
			#end
		end

		def label l
			@labels[l] = @lineno
		end

		def initialize infile
			@asm = []
			@lineno = 0
			@labels = Illiac.labels
			asm = infile.read
			prepro = asm.gsub(/^(.*\s+)?(\S+):/, '\1label :\2;').gsub(/^(.*\s+)\.(\S+)/, '\1 pseudo_\2')
			#puts prepro
			eval prepro
		end

		def to_s
			@asm.reduce('') {|m,a| m << a.to_binary_s}
		end
	end
end
