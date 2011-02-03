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
                        "toolroot"      => "",
                        "version"       => ["unknown", "Version of the delivered GDB."],
                        "enable-gcc"    => ["yes", "Enable build of GCC."],
                        "enable-open64" => ["no", "Enable build of Open64."],
                      })

repo = Git.new(options["clone"])

clean = Target.new("clean", repo, [])
build = Target.new("build", repo, [])
valid = Target.new("valid", repo, [build])
install = Target.new("install", repo, [valid])
valid_valid = Target.new("gdb", repo, [])

b = Builder.new("gdb", options, [clean, build, valid, install, valid_valid])

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

enable_gcc = options["enable-gcc"]
enable_open64 = options["enable-open64"]

build_path = gdb_clone + "/" + arch + "_build"

prefix = options["prefix"].empty? ? "#{build_path}/release" : options["prefix"]
make_j = options["parallel-make"] == "yes" ? "-j#{cpu_number}" : ""

b.default_targets = [build]

case arch
when "k1"
  build_target = "k1-elf"
  family_path = processor_clone  + "/#{arch}-family/"
when "st200" 
  build_target = "lx-stm-elf32"
  family_path = mds_clone + "/#{arch}-family/"
else 
  raise "Unknown Target #{arch}"
end

build.add_result build_path

b.target("build") do 
  b.logtitle = "Report for GDB build, arch = #{arch}"

  machine_type = `uname -m`.chomp() == "x86_64" ? "64" : "32"

  if( arch == "k1" )
    create_goto_dir! build_path

    version = options["version"] + " " + `git rev-parse --verify --short HEAD 2> /dev/null`.chomp
    version += "-dirty" if not `git diff-index --name-only HEAD 2> /dev/null`.chomp.empty?

    b.run(:cmd => "echo #{machine_type}" )
    b.run(:cmd => "../configure --target=#{build_target} --program-prefix=#{arch}- --disable-werror --without-python --with-libexpat-prefix=$PWD/../bundled_libraries/expat#{machine_type} --with-bugurl=no --prefix=#{prefix}")
    b.run(:cmd => "make clean")
    b.run(:cmd => "make #{make_j} FAMDIR=#{family_path} ARCH=#{arch} KALRAY_VERSION=\"#{version}\"")
  end
end

b.target("clean") do 
  b.logtitle = "Report for GDB clean, arch = #{arch}"

  b.run("rm -rf #{build_path}")
end

b.target("install") do
  b.logtitle = "Report for GDB install, arch = #{arch}"

  if( arch == "k1" )
    cd build_path
    b.run(:cmd => "make install FAMDIR=#{family_path} ARCH=#{arch}")  if( arch == "k1" )
  end

end

b.target("valid") do
  b.logtitle = "Report for GDB valid, arch = #{arch}"

  if( arch == "k1" )
    Dir.chdir build_path + "/gdb/testsuite"
    
    b.run(:cmd => "LANG=C PATH=#{options["toolroot"]}/bin:$PATH DEJAGNU=../../../gdb/testsuite/site.exp runtest --target_board=k1-iss  gdb.base/*.exp gdb.mi/*.exp gdb.kalray/*.exp; true")
    if(enable_open64 == "yes") then
      b.valid(:cmd => "../../../gdb/testsuite/regtest.rb ../../../gdb/testsuite/gdb.sum.open64.ref gdb.sum")
    else 
      b.valid(:cmd => "../../../gdb/testsuite/regtest.rb ../../../gdb/testsuite/gdb.sum.ref gdb.sum")
    end
  end

end

b.target("gdb") do
  b.logtitle = "Report for GDB gdb, arch = #{arch}"

  if( arch == "k1" )
    
    # Validation in the valid project
    create_goto_dir! build_path 
    
    # Build native, just to create the build directories
    b.run("../configure")
    b.run("make #{make_j}")

    cd "gdb/testsuite"
    
    b.run(:cmd => "LANG=C PATH=#{options["toolroot"]}/bin:$PATH LD_LIBRARY_PATH=#{options["toolroot"]}/lib:$LD_LIBRARY_PATH DEJAGNU=../../../gdb/testsuite/site.exp runtest --tool_exec=k1-gdb --target_board=k1-iss  gdb.base/*.exp gdb.mi/*.exp gdb.kalray/*.exp; true")
    b.valid(:cmd => "../../../gdb/testsuite/regtest.rb ../../../gdb/testsuite/gdb.sum.ref gdb.sum")
  end
end

b.launch
