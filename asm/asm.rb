#!/usr/bin/ruby

require 'illiac_asm.rb'
require 'stringio'

def assemble input
	Illiac.assemble input
end

def write_out assembly, output
	output.write(assembly)
end

if ARGV.nil? or ARGV.size < 1
	abort "Usage: #{$0} infile"# outfile"
end

infile = File.open(ARGV.shift, 'r')
#outfile = File.open(ARGV.shift, 'w')

#infile = StringIO.new(%[
#asdf:
#		oneb
#		dupb
#		dupb
#		dupb
#		absl
#		hcf
#		abss
#		dupb
#		.word :baz
#foo:	sluffw
#		.stringz "lolol"
#baz:	.word :asdf
#		10.times {|x| label "\#{(x+?a).chr}"; .word x }
#					  ], 'r')
outfile = $stdout

asm = assemble infile
write_out asm, outfile

