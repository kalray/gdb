#!/usr/bin/ruby

require 'rubygems'
require 'metabuild'
include Metabuild

options = Options.new({"target" => "k1",
                       "parallel-make" => "yes"
                      })

repo = Git.new(options["clone"])

build = Target.new("build", repo, [])
valid = Target.new("valid", repo, [build])

b = Builder.new("gdb", options, [build, valid])

def cpu_number ()
  num_cpu = 0
  File.open("/proc/cpuinfo") { |f| f.each { |l| l =~ /processor/ and num_cpu += 1 }}
  num_cpu
end

target = options["target"]
workspace = options["workspace"]
make_j = options["parallel-make"] == "yes" ? "-j#{cpu_number}" : ""

exit 0 if not target == "k1"

b.logtitle = "Report for GDB build, arch = #{target}"

b.default_targets = [build]

case target
when "k1"
  build_target = "k1-elf"
  mds_path = workspace + "/documents/Processor/#{target}-family/build/BE/GDB/#{target}"
when "st200" 
  build_target = "lx-stm-elf32"
  mds_path = workspace + "/mds/#{target}-family/build/BE/GBU/#{target}"
else 
  raise "Unknown Target #{target}"
end

build_path = workspace + "/" + options["clone"] + "/" + target + "_build"

build.add_result build_path

b.target("build") do 

  create_goto_dir! build_path

  b.run(:cmd => "../configure --target=#{build_target} --program-prefix=#{target}- --disable-werror --prefix=#{build_path}/release")
  b.run(:cmd => "make clean")
  b.run(:cmd => "make #{make_j} MDS_BE_DIR=#{mds_path}")
  b.run(:cmd => "make install")
end

b.target("valid") do

end

b.launch




