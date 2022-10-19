for i in {1..17}
do
	echo "test$i:"
	./main test_for_lab2/test$i.cmm
done
echo "--------------------------------------"
for i in {1..6}
do
	echo "test$i:"
	./main test_for_lab2/otest$i.cmm
done
