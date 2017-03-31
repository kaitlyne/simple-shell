#!/bin/bash

#Make simpsh executable
if [ -e simpsh ]
then
	rm -f simpsh;
fi
make || exit;

#Make temporary directory
if ! [ -d tmpdir-for-makecheck ]
then
	mkdir tmpdir-for-makecheck
fi
cd tmpdir-for-makecheck
cp ../simpsh simpsh
echo "";

echo "TESTS FOR LAB 1B";
echo "";

#Test case 1
echo "Test case 1: trunc oflag";
echo "hi" > in.txt;
echo "Truncate me" > out.txt;
echo "Truncate me" >> out.txt;
touch err.txt;
./simpsh --rdonly in.txt --trunc --rdwr out.txt --wronly err.txt --command 0 1 2 cat;
grep -q "Truncate me" out.txt;
if [ $? -eq 0 ]
then
	echo "---> Failed";
else
	echo "---> Passed";
fi
echo "";
rm -f in.txt out.txt err.txt;

#Test case 2
echo "Test case 2: creat oflag";
./simpsh --creat --rdwr test2.txt;
if [ -e test2.txt ]
then
	echo "---> Passed";
else
	echo "---> Failed";
fi
echo "";
rm -f test2.txt;

#Test case 3
echo "Test case 3: pipe";
echo "abc" > begin.txt
touch end.txt;
touch err.txt;
./simpsh --rdonly begin.txt --wronly end.txt --wronly err.txt --pipe \
	--command 0 4 2 cat --command 3 1 2 cat;
grep -q "abc" end.txt;
if [ $? -eq 0 ]
then
	echo "---> Passed";
else
	echo "---> Failed";
fi
echo "";
rm -f begin.txt end.txt err.txt;

#Test case 4
echo "Test case 4: ignore";
touch in.txt;
touch out.txt;
touch err.txt;
./simpsh --ignore 2 --rdonly in.txt --wronly out.txt --wronly err.txt --command 0 1 2 sleep 6 --wait > /dev/null 2>&1 &
sleep 1;
kill -2 $!;
ps | grep -q $!;
if [ $? -eq 0 ]
then
	echo "---> Passed";
else
	echo "---> Failed";
fi
echo "";
rm -f in.txt out.txt err.txt;

#Test case 5
echo "Test case 5: wait";
echo "---> IF WE WAIT FOR 5 SECONDS THEN SEE '0 sleep 5' OUTPUTTED, THEN TEST IS PASSED"
echo "---> OTHERWISE, TEST IS FAILED"
touch in.txt;
touch out.txt;
touch err.txt;
./simpsh --rdonly in.txt --wronly out.txt --wronly err.txt --command 0 1 2 sleep 5 --wait;
echo "";
rm -f in.txt out.txt err.txt;

#Test case 6
echo "Test case 6: pipe, command, close, wait";
echo "---> IF WE WAIT FOREVER, THEN TEST CASE 5 IS FAILED";
echo "c" > begin.txt;
echo "a" >> begin.txt;
echo "b" >> begin.txt;
touch end.txt;
touch err.txt;
./simpsh --pipe --rdonly begin.txt --wronly end.txt --wronly err.txt \
	--command 2 1 4 sort --command 0 3 4 tr a-z A-Z --close 0 --close 1 --wait > /dev/null 2>&1;
echo "A" > correct.txt;
echo "B" >> correct.txt;
echo "C" >> correct.txt;
cmp correct.txt end.txt > /dev/null 2>&1;
if [ $? -eq 0 ]
then
	echo "---> Passed";
else
	echo "---> Failed";
fi
echo "";
rm -f begin.txt end.txt err.txt correct.txt;

#Test case 7
echo "Test case 7: close";
echo "---> IF THERE'S AN ERROR MESSAGE REGARDING FILE DESCRIPTORS BELOW, TEST IS PASSED"
echo "---> OTHERWISE, TEST IS FAILED";
touch in.txt;
touch out.txt;
touch err.txt;
./simpsh --rdonly in.txt --wronly out.txt --wronly err.txt --close 1 --command 0 1 2 sleep 0.05
cat err.txt;
echo "";
rm -f in.txt out.txt err.txt;

#Test case 8
echo "Test case 8: abort";
echo "HELLO WORLD" > in.txt;
touch out.txt;
touch err.txt;
./simpsh --rdonly in.txt --wronly out.txt --wronly err.txt --abort --command 0 1 2 tr A-Z a-z;
if [ -s out.txt ]
then
	echo "---> Failed";
else
	echo "---> Passed";
fi
echo "";
rm -f in.txt out.txt err.txt;

#Test case 9
echo "Test case 9: catch";
./simpsh --catch 11 --abort;
if [ $? -eq 11 ]
then
	echo "---> Passed";
else
	echo "---> Failed";
fi
echo "";

#Test case 10
echo "Test case 10: default";
./simpsh --catch 11 --default 11 --abort;
if [ $? -eq 139 ]
then
	echo "---> Passed";
else
	echo "---> Failed";
fi
echo "";

#Test case 11
echo "Test case 11: pause";
./simpsh --pause &
sleep 2;
ps | grep -q $!;
if [ $? -eq 0 ]
then
	kill -9 $!;
	sleep 1;
	ps | grep -q $!;
	if [ $? -eq 0 ]
	then
		echo "---> Failed";
	else
		echo "---> Passed";
	fi
else
	echo "---> Failed";
fi

echo "";

echo "TESTS FOR LAB 1A";
echo "";

#Test 12
echo "Test case 12: basic command";
echo "hello world" > in.txt;
echo "erase me" > out.txt;
touch err.txt;
touch shouldnotbeempty.txt;
./simpsh --rdonly in.txt --wronly out.txt --wronly err.txt --command 0 1 2 echo "Hello world";
cat out.txt | grep "Hello world" > shouldnotbeempty.txt;
if [ -s shouldnotbeempty.txt ]
then
	echo "---> Passed";
else
	echo "---> Failed";
fi;
rm -f in.txt out.txt err.txt shouldnotbeempty.txt;
echo "";

#Test 13
echo "Test case 13: multiple commands";
touch in1.txt;
touch out1.txt;
touch err1.txt;
touch shouldnotbeempty1.txt;
echo "I WANT TO STOP YELLING" >  in2.txt;
touch out2.txt;
touch err2.txt;
touch shouldnotbeempty2.txt;
./simpsh --wronly in1.txt --wronly out1.txt --wronly err1.txt --rdonly in2.txt --wronly out2.txt --wronly err2.txt \
	--command 0 1 2 echo "hello" --command 3 4 5 tr A-Z a-z;
cat out1.txt | grep "hello" > shouldnotbeempty1.txt;
cat out2.txt | grep "i want to stop yelling" > shouldnotbeempty2.txt;
if [ -s shouldnotbeempty1.txt ]
then
	echo "---> Passed";
else
	echo "---> Failed";
fi;
rm -f in1.txt out1.txt err1.txt shouldnotbeempty1.txt in2.txt out2.txt err2.txt shouldnotbeempty2.txt;
echo "";

#Test 14
echo "Test case 14: verbose";
touch in.txt;
touch out.txt;
touch err.txt;
touch test.txt;
touch test2.txt;
echo "---> Verbose output should say '--rdonly test.txt\n --wronly test2.txt' and nothing else.";
echo "---> Check the output below:";
./simpsh --rdonly in.txt --wronly out.txt --wronly err.txt --command 0 1 2 sleep 0.05 \
	--verbose --rdonly test.txt --wronly test2.txt;
echo "---> If output is correct, test was passed.";
rm -f in.txt out.txt err.txt test.txt test2.txt;
echo "";

#Test 15
echo "Test case 15: invalid file descriptor";
touch in.txt;
touch out.txt;
touch err.txt;
echo "---> We are testing an invalid file descriptor, so there should be an error message below.";
./simpsh --rdonly in.txt --wronly out.txt --wronly err.txt --command 1 2 3 sleep 0.05;
echo "---> If an error message is outputted above, test was passed.";
rm -f in.txt out.txt err.txt;
echo "";

echo "Finished testing";
