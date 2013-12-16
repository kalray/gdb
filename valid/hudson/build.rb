#!/usr/bin/ruby

$LOAD_PATH.push('metabuild/lib')
require 'metabuild'
include Metabuild

options = Options.new({ "target"        => "k1",
                        "clone"         => ".",
                        "cores"          => ["none", "List of family cores."],
                        "processor"     => "processor",
                        "mds"           => "mds",
                        "toolroot"      => "",
                        "version"       => ["unknown", "Version of the delivered GDB."],
                        "variant"        => {"type" => "keywords", "keywords" => [:nodeos, :elf, :rtems, :linux, :gdb], "default" => "elf", "help" => "Select build variant."},
                        "prefix"         => ["devimage", "Install prefix"],
                        "host"           => ["x86", "Host for the build"],
                        "sysroot"        => ["sysroot", "Sysroot directory"],
                      })

workspace = options["workspace"]
gdb_clone =  options["clone"]
gdb_path  =  File.join(workspace, gdb_clone)

variant = options["variant"].to_s

repo = Git.new(gdb_clone,workspace)

clean = CleanTarget.new("clean", repo, [])
build = ParallelTarget.new("#{variant}_build", repo, [], [])
build_valid = ParallelTarget.new("#{variant}_post_build_valid", repo, [build], [])
install = Target.new("#{variant}_install", repo, [build], [])
install_valid = ParallelTarget.new("#{variant}_post_install_valid", repo, [build], [])
gdb_long_valid = Target.new("gdb_long_valid", repo, [], [])

install.write_prefix()

b = Builder.new("gdb", options, [clean, build, build_valid, install, install_valid, gdb_long_valid])

arch = options["target"]
b.logsession = arch

processor_path = File.join(workspace,options["processor"])
mds_path       = File.join(workspace,options["mds"])

host       = options["host"]

build_path = File.join(gdb_path, arch + "_build_#{variant}_#{host}")
prefix = options["prefix"].empty? ? "#{build_path}/release" : options["prefix"]

b.default_targets = [install]

cores      = options["cores"]


program_prefix = "#{arch}-"
family_prefix = "#{processor_path}/#{arch}-family"

skip_build = false
skip_valid = false
skip_install = false

case arch
when "k1"
  case variant
  when "gdb" then
    build_target = "k1-elf"
  when "linux" then
    build_target = "k1-#{variant}"
    program_prefix += "#{variant}-"
    sysroot_option = "--with-sysroot="+options["sysroot"]
    if(cores == "none") then
      cores = "k1dp,k1io"
    end
    mds_gbu_path = "#{family_prefix}/BE/GBU/#{arch}"
  when "elf" then
    build_target = "k1-#{variant}"
    if(cores == "none") then
      cores = "k1dp,k1io"
    end
    mds_gbu_path = "#{family_prefix}/BE/GBU/#{arch}"
  when "rtems" then
    build_target = "k1-#{variant}"
    program_prefix += "#{variant}-"
    if(cores == "none") then
      cores = "k1dp,k1io"
    end
    mds_gbu_path = "#{family_prefix}/BE/GBU/#{arch}"
  when "nodeos" then
    build_target = "k1-#{variant}"
    program_prefix += "#{variant}-"
    if(cores == "none") then
      cores = "k1dp,k1io"
    end
    mds_gbu_path = "#{family_prefix}/BE/GBU/#{arch}"
  else
    raise "Unknown variant #{variant}"
  end
else 
  raise "Unknown Target #{arch}"
end

core_list  = cores.split(/,/)

build.add_result build_path

b.target("#{variant}_build") do 
  b.logtitle = "Report for GDB #{variant}_build, arch = #{arch}"
  if( variant == "gdb")
    machine_type = `uname -m`.chomp() == "x86_64" ? "64" : "32"
    if( arch == "k1" )
      b.create_goto_dir! build_path
      version = options["version"] + " " + `git rev-parse --verify --short HEAD 2> /dev/null`.chomp
      version += "-dirty" if not `git diff-index --name-only HEAD 2> /dev/null`.chomp.empty?

      b.run(:cmd => "echo #{machine_type}" )
      b.run(:cmd => "../configure --target=#{build_target} --program-prefix=#{arch}- --disable-werror --without-gnu-as --without-gnu-ld --without-python --with-bugurl=no --prefix=#{prefix}")
      b.run(:cmd => "make clean")
      b.run(:cmd => "make FAMDIR=#{family_prefix} ARCH=#{arch} KALRAY_VERSION=\"#{version}\"")
    end
  else
    b.create_goto_dir! build_path

    version = options["version"] + " " + `git rev-parse --verify --short HEAD 2> /dev/null`.chomp
    version += "-dirty" if not `git diff-index --name-only HEAD 2> /dev/null`.chomp.empty?
  
    build_host = ""
    if (host == "k1-linux") then
      build_host = "--host=k1-linux"
    end
    install_prefix = prefix
  
    b.run(:cmd => "PATH=\$PATH:#{prefix}/bin ../configure --target=#{build_target} #{build_host} --program-prefix=#{program_prefix} --without-gdb --disable-werror  --prefix=#{install_prefix} --with-bugurl=no #{sysroot_option}",
        :skip=>skip_build)
    b.run(:cmd => "make clean",
        :skip=>skip_build)

    additional_flags = "CFLAGS=-g"

    b.run(:cmd => "PATH=\$PATH:#{prefix}/bin make FAMDIR='#{family_prefix}' ARCH=#{arch} #{additional_flags} KALRAY_VERSION=\"#{version}\" all",
        :skip=>skip_build)

  end
end

b.target("clean") do 
  b.logtitle = "Report for GDB clean, arch = #{arch}"

  b.run("rm -rf #{build_path}")
end

b.target("#{variant}_install") do
  b.logtitle = "Report for GDB #{variant}_install, arch = #{arch}"
  if( variant == "gdb")
    if( arch == "k1" )
      cd build_path
      b.run(:cmd => "make install FAMDIR=#{family_prefix} ARCH=#{arch}")
    end
  else
    cd build_path
    b.run(:cmd => "PATH=\$PATH:#{prefix}/bin make FAMDIR='#{family_prefix}' ARCH=#{arch} install",
        :skip=>skip_install)
    b.run(:cmd=>"ls #{prefix}/bin/#{arch}-*",
        :skip=>skip_install)
  end
end

b.target("#{variant}_post_build_valid") do
  b.logtitle = "Report for GDB  #{variant}_post_build_valid, arch = #{arch}"
  if (variant == "gdb")
    if( arch == "k1" )
      Dir.chdir build_path + "/gdb/testsuite"
      b.valid(:cmd => "LANG=C PATH=#{options["toolroot"]}/bin:$PATH make check DEJAGNU=../../../gdb/testsuite/site.exp RUNTEST=runtest RUNTESTFLAGS=\"--target_board=k1-iss  gdb.base/*.exp gdb.mi/*.exp gdb.kalray/*.exp\"; true")
      b.valid(:cmd => "../../../gdb/testsuite/regtest.rb ../../../gdb/testsuite/gdb.sum.ref gdb.sum")
    end
  end
end


b.target("#{variant}_post_install_valid") do
  b.logtitle = "Report for Gbu #{variant}_post_install_valid, arch = #{arch}"
  if (variant == "elf" || variant == "rtems" || variant == "nodeos" || variant == "linux")
    gas = "#{build_path}/gas/as-new"
    objdump = "#{build_path}/binutils/objdump"

    if(not skip_valid) then
      raise "Unknown MDS Directory #{mds_gbu_path}" unless File.directory? mds_gbu_path
      raise "Gas was not built in #{gas}" unless File.exists? gas
      raise "Objdump was not built in #{objdump}" unless File.exists? objdump
    end

    ["test"].each do |test|
      core_list.each do |core|
        asm_test= "#{mds_gbu_path}/#{core}/#{test}.s"
        bin_test= "#{mds_gbu_path}/#{core}/#{test}.bin"
        out_test= "#{build_path}/#{core}/#{test}.out"
        obj_test= "#{build_path}/#{core}/#{test}.o"

        mkdir_p "#{build_path}/#{core}"
        b.run(:cmd=>"#{gas} -mcore #{core} -o #{obj_test} #{asm_test}",
              :skip=>skip_valid)
        b.run(:cmd=>"#{objdump} -d #{obj_test} > #{out_test}",
              :skip=>skip_valid) 
        puts "Diff between: #{out_test} #{bin_test}"
        b.valid(:cmd => "diff -w -bu -I 'test\\.o: ' -I '^$' #{out_test} #{bin_test}", 
                :fail_msg => "Get some diff between GAS output and ref #{test}.bin for #{core}.", 
                :success_msg => "GBU : No diff, test OK",
                :skip=>(skip_valid))
      end
    end
  end
end


b.target("gdb_long_valid") do
  b.logtitle = "Report for GDB gdb_long_valid, arch = #{arch}"
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
