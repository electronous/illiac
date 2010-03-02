#! /usr/bin/env ruby

require 'bytewriter'

infile  = ARGV[0]
outfile = ARGV[1]

if infile.nil?
	abort("You must specify an input asm file")
end

if outfile.nil?
	abort("You must specify an output bin file")
end

load infile

ByteWriter.instance.write_out(outfile)
