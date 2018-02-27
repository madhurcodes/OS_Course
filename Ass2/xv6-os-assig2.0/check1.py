import os

os.system("make clean")
os.system("make")

print("Running 1st Testcase...")
os.system("./test_assig2.sh assig2_1 | grep -i pid > res_assig2_1")
with open('out_assig2_1', 'r') as file1:
    with open('res_assig2_1', 'r') as file2:
        same1 = set(file1).difference(file2)
        same2 = set(file2).difference(file1)

same1.discard('\n')
same1.discard('\n')

if len(same1) == 0 and len(same2) == 0:
	print("Test1: Pass")
else:
	print("Test1: Fail")