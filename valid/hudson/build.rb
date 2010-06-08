#!/usr/bin/ruby

$LOAD_PATH.push('metabuild/lib')
require 'metabuild'
include Metabuild

options = Options.new({ "target"        => "k1",
                        "clone"         => ".",
                        "processor"     => "processor",
                        "mds"           => "mds",
                        "open64"        => "open64",
                        "parallel-make" => "yes",
                      })

repo = Git.new(options["clone"])

build = Target.new("build", repo, [])
valid = Target.new("valid", repo, [build])
install = Target.new("install", repo, [valid])

b = Builder.new("gdb", options, [build, valid, install])

def cpu_number ()
  num_cpu = 0
  File.open("/proc/cpuinfo") { |f| f.each { |l| l =~ /processor/ and num_cpu += 1 }}
  num_cpu
end

arch = options["target"]
workspace = options["workspace"]
gdb_clone =  workspace + "/" + options["clone"]
processor_clone =  workspace + "/" + options["processor"]
mds_clone =  workspace + "/" + options["mds"]
open64_clone =  workspace + "/" + options["open64"]

build_path = gdb_clone + "/" + arch + "_build"

prefix = options["prefix"].empty? ? "#{build_path}/release" : options["prefix"]
make_j = options["parallel-make"] == "yes" ? "-j#{cpu_number}" : ""

b.logtitle = "Report for GDB build, arch = #{arch}"

b.default_targets = [build]

case arch
when "k1"
  build_target = "k1-elf"
  mds_path = processor_clone  + "/#{arch}-family/build/BE/GDB/#{arch}"
when "st200" 
  build_target = "lx-stm-elf32"
  mds_path = mds_clone + "/#{arch}-family/build/BE/GBU/#{arch}"
else 
  raise "Unknown Target #{arch}"
end

build.add_result build_path

b.target("build") do 

  if( arch == "k1" )
    create_goto_dir! build_path

    b.run(:cmd => "../configure --target=#{build_target} --program-prefix=#{arch}- --disable-werror --without-python --with-libexpat-prefix=$PWD/../bundled_libraries/expat --with-bugurl=no --prefix=#{prefix}")
    b.run(:cmd => "make clean")
    b.run(:cmd => "make #{make_j} MDS_BE_DIR=#{mds_path}")
  end
end

b.target("install") do

  if( arch == "k1" )
    cd build_path
    b.run(:cmd => "make install")  if( arch == "k1" )
  end

end

b.target("valid") do

  if( arch == "k1" )
    Dir.chdir build_path + "/gdb/testsuite"

    b.run(:cmd => "[ $USER != \"hudson\" ] || killall -9 gdb_stub; true")
    b.run(:cmd => "LANG=C PATH=#{open64_clone}/osprey/targia32_#{arch}/devimage/bin:$PATH DEJAGNU=../../../gdb/testsuite/site.exp runtest --target_board=k1-iss  gdb.base/*.exp gdb.mi/*.exp gdb.kalray/*.exp; true")
    b.valid(:cmd => "../../../gdb/testsuite/regtest.rb ../../../gdb/testsuite/gdb.sum.ref gdb.sum")
  end
end

b.launch
