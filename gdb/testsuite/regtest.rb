#!/usr/bin/env ruby

require 'tempfile'

def usage
  STDERR.puts "Usage: #{File.basename __FILE__} REF_SUM_FILE SUM_FILE"
  exit 1
end

begin
  $ref = File.new(ARGV[0], "r");
  $sum = File.new(ARGV[1], "r");
rescue
  puts $!
  usage
end

def colorize(text, color_code)
  "#{color_code}#{text}\033[0m"
end

def red(text); colorize(text, "\033[31m"); end
def green(text); colorize(text, "\033[32m"); end

$tests_ref = {}
$tests = {}

$output_ref = ""
$output = ""

def parse (file, tests, output)
  
  cur_test_results = {}

  while line = file.gets
    case line
    when /Running (.*\.exp) \.\.\./
      cur_test_results = {}
      tests[$1] = cur_test_results
    when /.?PASS: [^:]+: (.*)$/
      cur_test_results[$1] = 'PASS'
    when /.?FAIL: [^:]+: (.*)$/
      cur_test_results[$1] = 'FAIL'
    when /UNTESTED: [^:]+: (.*)$/
      cur_test_results[$1] = 'UNTESTED'
    when /UNSUPPORTED: [^:]+: (.*)$/
      cur_test_results[$1] = 'UNSUPPORTED'
    else
      output << line
    end
  end
end

$last_exp_file = ""
def notify exp_file, msg
  if exp_file != $last_exp_file
    $last_exp_file = exp_file
    puts "Comparing results for #{exp_file}"
  end
  puts msg
end

parse $ref, $tests_ref, $output_ref
parse $sum, $tests, $output

$regressions = 0
$progressions = 0

$tests.each_pair do |exp, tests|

  ref = $tests_ref.delete exp

  if (ref == nil)
    notify exp, "New test file in #{ARGV[0]}: #{exp}"
    tests.each_pair { |t, res| notify exp, "\tNew test #{res}: #{t}" }
    next
  end

  tests.each_pair do |t, res|
    res_ref = ref.delete t

    if (res_ref == nil)
      if (res == 'PASS')
        $progressions += 1
        notify exp, "\t     => #{green(res)}\t#{t}"
      else
        $regressions += 1
        notify exp, "\t     => #{red(res)}\t#{t}"
      end
      next
    end
    
    if (res_ref != res)
      if (res_ref == "PASS")
        $regressions += 1
        notify exp, "\t#{red(res_ref)} => #{red(res)}\t#{t} "
      else
        $progressions += 1
        notify exp, "\t#{green(res_ref)} => #{green(res)}\t#{t} "
      end
      next
    end
  end

  if (not ref.empty?)
    ref.each_pair do |t, res|
      if (res == 'PASS')
        $regressions += 1
        notify exp, "\t#{red(res)} =>     \t#{t}"
      else
        $progressions += 1
        notify exp, "\t#{green(res)} =>     \t#{t}"
      end
    end
  end

end


if (not $tests_ref.empty?)
  $tests_ref.each_pair do |exp, tests|
    notify exp, "Only in ref: #{exp}"
    tests.each_pair do |t, res| 
      if (res == 'PASS')
        notify exp, "\tTest disappeared #{red(res)}: #{t}"
        $regressions += 1
      else
        notify exp, "\tTest disappeared #{res}: #{t}"
      end
    end
  end
end

tmp_output = Tempfile.new("output")
tmp_output_ref = Tempfile.new("output_ref")

tmp_output << $output
tmp_output_ref << $output_ref

tmp_output.close
tmp_output_ref.close

diff = `diff -u #{tmp_output_ref.path} #{tmp_output.path}`

puts <<EOF

############## OUTPUT ##############
#{diff}

############## SUMMARY ##############
    Regressions : #{$regressions}
    Progressions: #{$progressions}

EOF

tmp_output.delete
tmp_output_ref.delete

exit $regressions == 0
