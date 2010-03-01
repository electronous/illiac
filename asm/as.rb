#! /usr/bin/env ruby

require 'bytewriter'

infile  = ARGV[0]
outfile = ARGV[1]

if infile == nil:
    abort("You must specify an input asm file")
end

if outfile == nil:
    abort("You must specify an output bin file")
end

ByteWriter.new(outfile).go do |writer|
    File.open(infile, "r").each do |line|
        instruction = line.split()
        opcode      = instruction[0]
        operands    = instruction[1..-1]

        if not writer.respond_to?(opcode):
            abort("Invalid opcode: " + opcode)
        end

        writer.send(opcode, *operands)
    end
end
