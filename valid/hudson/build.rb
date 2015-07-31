#!/usr/bin/ruby

$LOAD_PATH.push('metabuild/lib')
require 'metabuild'
require 'copyrightCheck'
include Metabuild

options = Options.new({ "target"        => ["k1", "k1nsim"],
                        "clone"         => ".",

                        "board"      => {
                          "type" => "keywords",
                          "keywords" => [:developer, :explorer, :pcie_530, :emb01, :tc2],
                          "default" => "developer",
                          "help" => "Target board (changing things at compilation)."
                        },

                        "processor"     => "processor",
                        "mds"           => "mds",
                        "build_type"    => ["Debug", "Can be Release or Debug." ],
                        "toolroot"      => "",
                        "k1debug_prefix"       => {"type" => "string", "default" => "", "help" => "Path to installed prefix where ISS is installed." },
                        "version"       => ["unknown", "Version of the delivered GDB."],
                        "variant"       => {"type" => "keywords", "keywords" => [:nodeos, :elf, :rtems, :linux, :gdb], "default" => "elf", "help" => "Select build variant."},
                        "prefix"        => ["devimage", "Install prefix"],
                        "pkg_prefix"    => {"type" => "string", "default" => "", "help" => "Where to install software to build packages." },
                        "host"          => ["x86", "Host for the build"],
                        "sysroot"       => ["sysroot", "Sysroot directory"],
                        "march_valid"   => ["k1a:k1dp,k1io::k1b:k1bio,k1bdp", "List of mppa_architectures to validate on execution_platform."],
                        "execution_platform" => {
                          "type" => "keywords",
                          "keywords" => [:hw, :sim],
                          "default" => "sim",
                          "help" => "Execution platform: can be hardware (jtag) or simulation (k1-mppa, k1-cluster)."
                        },
                      })

workspace = options["workspace"]
gdb_clone =  options["clone"]
gdb_path  =  File.join(workspace, gdb_clone)

variant = options["variant"].to_s
build_type= options['build_type']

repo = Git.new(gdb_clone,workspace)

clean = CleanTarget.new("clean", repo, [])
build = ParallelTarget.new("#{variant}_build", repo, [], [])
build_valid = ParallelTarget.new("#{variant}_post_build_valid", repo, [build], [])
install = Target.new("#{variant}_install", repo, [build_valid], [])
install_valid = ParallelTarget.new("#{variant}_post_install_valid", repo, [build], [])
gdb_long_valid = Target.new("gdb_long_valid", repo, [], [])
copyright_check = Target.new("copyright_check", repo, [], [])

package = Target.new("package", repo, [], [])

install.write_prefix()

march_valid_hash = Hash[*options["march_valid"].split(/::/).map{|tmp_arch| tmp_arch.split(/:/)}.flatten]

march_valid_list    = march_valid_hash.keys
board   = options['board'].to_s

b = Builder.new("gdb", options, [clean, build, build_valid, install, install_valid, gdb_long_valid, package, copyright_check])


arch = options["target"]
b.logsession = arch

processor_path = File.join(workspace,options["processor"])
mds_path       = File.join(workspace,options["mds"])

host       = options["host"]

toolroot          = options["toolroot"]
k1debug_prefix    = options.fetch("k1debug_prefix", toolroot)

build_path        = File.join(gdb_path, arch + "_build_#{variant}_#{host}")
prefix            = options.fetch("prefix", "#{build_path}/release")
pkg_prefix        = options.fetch("pkg_prefix",prefix)
pkg_prefix_name   = options.fetch("pi-prefix-name","#{arch}-")

gdb_install_prefix = File.join(pkg_prefix,"gdb","devimage")
k1debug_prefix     = k1debug_prefix

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
    mds_gbu_path = "#{family_prefix}/BE/GBU/#{arch}"
  when "elf" then
    build_target = "k1-#{variant}"
    mds_gbu_path = "#{family_prefix}/BE/GBU/#{arch}"
  when "rtems" then
    build_target = "k1-#{variant}"
    program_prefix += "#{variant}-"
    mds_gbu_path = "#{family_prefix}/BE/GBU/#{arch}"
  when "nodeos" then
    build_target = "k1-#{variant}"
    program_prefix += "#{variant}-"
    mds_gbu_path = "#{family_prefix}/BE/GBU/#{arch}"
  else
    raise "Unknown variant #{variant}"
  end
else 
  raise "Unknown Target #{arch}"
end

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
      b.run(:cmd => "../configure --target=#{build_target} --program-prefix=#{arch}- --disable-werror --without-gnu-as --without-gnu-ld --without-python --with-expat=yes --with-babeltrace=no --with-bugurl=no --prefix=#{gdb_install_prefix}")
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
  
    b.run(:cmd => "PATH=\$PATH:#{prefix}/bin ../configure --enable-64-bit-bfd --target=#{build_target} #{build_host} --program-prefix=#{program_prefix} --disable-gdb --without-gdb --disable-werror  --prefix=#{install_prefix} --with-expat=yes --with-babeltrace=no --with-bugurl=no #{sysroot_option}",
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
      if ("#{build_type}" == "Release") then
        b.run(:cmd => "make install-strip-gdb FAMDIR=#{family_prefix} ARCH=#{arch}")
      else
        b.run(:cmd => "make install-gdb FAMDIR=#{family_prefix} ARCH=#{arch}")
      end
      # Copy to k1debug.
      b.run("mkdir -p #{k1debug_prefix}") unless File.exist?(k1debug_prefix)
      b.run("rsync -av #{gdb_install_prefix}/* #{k1debug_prefix}")
    end
  else
    cd build_path
    if ("#{build_type}" == "Release") then
      b.run(:cmd => "PATH=\$PATH:#{prefix}/bin make FAMDIR='#{family_prefix}' ARCH=#{arch} install-strip", :skip=>skip_install)
    else
      b.run(:cmd => "PATH=\$PATH:#{prefix}/bin make FAMDIR='#{family_prefix}' ARCH=#{arch} install", :skip=>skip_install)
    end
    b.run(:cmd=>"ls #{prefix}/bin/#{arch}-*", :skip=>skip_install)
  end
end


execution_platform = options['execution_platform'].to_s

b.target("#{variant}_post_build_valid") do
  b.logtitle = "Report for GDB  #{variant}_post_build_valid, arch = #{arch}"

  march_valid_list.each do |march|
    extra_flags = "CFLAGS_FOR_TARGET='-march=#{march} -mboard=#{board}'"
    if ( execution_platform == "hw" )
      execution_board = "k1-jtag-runner"
      execution_ref = "gdb.sum.hw.ref"
    else
      execution_board = "k1-iss"
      execution_ref = "gdb.sum.iss.ref"
    end

    if (variant == "gdb")
      if( arch == "k1" )
        Dir.chdir build_path + "/gdb/testsuite"

        cmd = "LANG=C " +
              "PATH=#{k1debug_prefix}/bin:#{toolroot}/bin:$PATH " +
              "make "+
              "check "+
              "DEJAGNU=../../../gdb/testsuite/site.exp " +
              "RUNTEST=runtest " +
              "RUNTESTFLAGS=\"#{extra_flags} --target_board=#{execution_board}  gdb.base/*.exp gdb.mi/*.exp gdb.kalray/*.exp\" ; " +
              "true"

        b.valid(:cmd => cmd)

        b.valid(:cmd => "../../../gdb/testsuite/regtest.rb #{File.join(gdb_path, 'valid', 'hudson', 'testsuite-refs', march, execution_ref)} gdb.sum")
      end
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
      march_valid_hash.each do |k,v|
        v.split(/,/).each do |core|

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
end


b.target("gdb_long_valid") do
  b.logtitle = "Report for GDB gdb_long_valid, arch = #{arch}"
  if( arch == "k1" )
    # Validation in the valid project
    b.create_goto_dir! build_path 
    # Build native, just to create the build directories
    b.run("../configure --without-gnu-as --without-gnu-ld --without-python")
    b.run("make")

    march_valid_list.each do |march|
      extra_flags = "CFLAGS_FOR_TARGET='-march=#{march} -mboard=#{board}'"

      if ( execution_platform == "hw" )
        execution_board = "k1-jtag-runner"
        execution_ref = "gdb.sum.hw.ref"
      else
        execution_board = "k1-iss"
        execution_ref = "gdb.sum.iss.ref"
      end

      extra_flags = "CFLAGS_FOR_TARGET='-march=#{march} -mboard=#{board}'"

      Dir.chdir build_path + "/gdb/testsuite"
      b.valid(:cmd => "LANG=C " +
                    "PATH=#{k1debug_prefix}/bin:#{toolroot}/bin:$PATH LD_LIBRARY_PATH=#{toolroot}/lib:$LD_LIBRARY_PATH " +
                    "make check " +
                    "DEJAGNU=../../../gdb/testsuite/site.exp "+
                    "RUNTEST=runtest " +
                    "RUNTESTFLAGS=\"#{extra_flags} --tool_exec=k1-gdb --target_board=#{execution_board}  gdb.base/*.exp gdb.mi/*.exp gdb.kalray/*.exp\" ; " +
                    "true")
      b.valid(:cmd => "../../../gdb/testsuite/regtest.rb #{File.join(gdb_path, 'valid', 'hudson', 'testsuite-refs', march, execution_ref)} gdb.sum")
    end
  end
end


b.target("copyright_check") do
    #do nothing here
end

b.target("package") do
  b.logtitle = "Report for GDB packaging, arch = #{arch}"

  # GDB package
  cd gdb_install_prefix

  gdb_name = "#{pkg_prefix_name}gdb"
  gdb_tar  = "#{gdb_name}.tar"
  b.run("tar cf #{gdb_tar} ./*")
  tar_package = File.expand_path(gdb_tar)

  depends = []

  package_description = "#{arch.upcase} GDB package.\n"
  package_description += "This package provides GNU Debugger for MPPA."

  tools_version = options["version"]

  (version,buildID) = tools_version.split("-")
  release_info = b.release_info(version,buildID)
  pinfo = b.package_info(gdb_name, release_info,
                         package_description, depends)

  b.create_package(tar_package, pinfo)
  b.run("rm #{tar_package}")
end

b.launch
