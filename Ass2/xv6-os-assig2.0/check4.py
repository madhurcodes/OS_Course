import os

os.system("make clean")
os.system("make")

print("Running 4th Testcase...")
os.system("./test_assig2.sh assig2_4 > res_assig2_4")
os.system("grep \"pid: 3 pr: 10\" res_assig2_4 | wc -l > testCount")
f = open('testCount', 'r')
result = f.read()
result = int(result)
f.close()
if result > 0:
	print("Pass")
else:
	print("Fail")
