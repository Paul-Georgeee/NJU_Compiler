for i in {1..10}
do
	echo "gen Mips for lab3test$i"
	./build/main test_for_lab4/lab3test$i.cmm tmp/lab3test$i.s > tmp/lab3test$i.ir 
done

for i in {1..8}
do
	echo "gen Mips for mytest$i"
	./build/main test_for_lab4/mytest$i.cmm tmp/mytest$i.s > tmp/mytest$i.ir 
done
