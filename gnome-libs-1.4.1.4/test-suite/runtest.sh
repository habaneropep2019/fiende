# this is a -*- sh -*- script fragment
# source this file with $srcdir and $the_test set.
# $the_test is the name of the desired test

export srcdir

mkdir results > /dev/null 2>&1 || true

./tests/$the_test > results/$the_test.out 2>&1
exit_code=$?
echo "Exit code $exit_code" >> results/$the_test.out

# magic exit code 77 (EX_NOPERM: permission denied)
if test $exit_code = 77; then exit 77; fi

# check the results
# -----------------
# don't bother to test for $exit_code = 0.  
# If the test failed, at least the "Exit code" line will be different
#
# If test `foo' can have only one expected output, let expected result be
#   expected/foo.out
# If `foo' can have one of N outputs, let the expected results files be
#   expected/foo_1.out, expected/foo_2.out, ..., expected/foo_N.out
# If you don't care about the output, don't put any such file.  

if test -f $srcdir/expected/$the_test.out; then
  cmp -s $srcdir/expected/$the_test.out results/$the_test.out && exit 0
  exit 1
fi

if test ! -f $srcdir/expected/${the_test}_1.out; then
  # neither foo.out, nor foo_1.out => nothing matters
  exit 0
fi

i=1
while test -f $srcdir/expected/${the_test}_${i}.out; do
  cmp -s $srcdir/expected/${the_test}_${i}.out results/$the_test.out && exit 0
  i=`expr $i + 1`
done

# none of the files matched
exit 1
