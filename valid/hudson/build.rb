#!/usr/bin/ruby

$LOAD_PATH.push('metabuild/lib')
require 'metabuild'
include Metabuild

options = Options.new({ "target"        => "k1",
                        "clone"         => ".",
                        "processor"     => "processor",
                        "mds"           => "mds",
                        "open64"        => "open64",
                        "toolroot"      => "",
                        "version"       => ["unknown", "Version of the delivered GDB."],
                        "compiler"      => ["gcc", "Enable build of GCC or Open64. Valid values: [gcc,open64]"],
                      })

workspace = options["workspace"]
gdb_clone =  options["clone"]
gdb_path  =  File.join(workspace, gdb_clone)

repo = Git.new(gdb_clone,workspace)

clean = CleanTarget.new("clean", repo, [])
build = ParallelTarget.new("build", repo, [], [])
valid = ParallelTarget.new("valid", repo, [build], [])
install = Target.new("install", repo, [valid], [])
install.write_prefix()

valid_valid = Target.new("gdb", repo, [], [])

b = Builder.new("gdb", options, [clean, build, valid, install, valid_valid])

arch = options["target"]
b.logsession = arch

processor_clone =  workspace + "/" + options["processor"]
mds_clone =  workspace + "/" + options["mds"]
open64_clone =  workspace + "/" + options["open64"]

compiler = options["compiler"]

build_path = gdb_path + "/" + arch + "_build"

prefix = options["prefix"].empty? ? "#{build_path}/release" : options["prefix"]

b.default_targets = [install]

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
    b.create_goto_dir! build_path

    version = options["version"] + " " + `git rev-parse --verify --short HEAD 2> /dev/null`.chomp
    version += "-dirty" if not `git diff-index --name-only HEAD 2> /dev/null`.chomp.empty?

    b.run(:cmd => "echo #{machine_type}" )
    b.run(:cmd => "../configure --target=#{build_target} --program-prefix=#{arch}- --disable-werror --without-python --with-libexpat-prefix=$PWD/../bundled_libraries/expat#{machine_type} --with-bugurl=no --prefix=#{prefix}")
    b.run(:cmd => "make clean")
    b.run(:cmd => "make FAMDIR=#{family_path} ARCH=#{arch} KALRAY_VERSION=\"#{version}\"")
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
    
    b.valid(:cmd => "LANG=C PATH=#{options["toolroot"]}/bin:$PATH make check DEJAGNU=../../../gdb/testsuite/site.exp RUNTEST=runtest RUNTESTFLAGS=\"--target_board=k1-iss  gdb.base/*.exp gdb.mi/*.exp gdb.kalray/*.exp\"; true")
    if(compiler == "open64") then
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
    b.create_goto_dir! build_path 
    
    # Build native, just to create the build directories
    b.run("../configure")
    b.run("make")

    cd "gdb/testsuite"
    
    b.run(:cmd => "LANG=C PATH=#{options["toolroot"]}/bin:$PATH LD_LIBRARY_PATH=#{options["toolroot"]}/lib:$LD_LIBRARY_PATH DEJAGNU=../../../gdb/testsuite/site.exp runtest --tool_exec=k1-gdb --target_board=k1-iss  gdb.base/*.exp gdb.mi/*.exp gdb.kalray/*.exp; true")
    b.valid(:cmd => "../../../gdb/testsuite/regtest.rb ../../../gdb/testsuite/gdb.sum.ref gdb.sum")
  end
end

b.launch
