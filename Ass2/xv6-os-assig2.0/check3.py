import os

os.system("make clean")
os.system("make")

print("Running 3rd Testcase...")
os.system("./test_assig2.sh assig2_3 > res_assig2_3")
os.system("sed -i '1,23d' res_assig2_3")
os.system("head -n -2 res_assig2_3 | tee res_assig2_3")
f = open('res_assig2_3', 'r')
result = f.read()
print(result)
print(result.find('c'))
print(result.rfind('a'))
if result.find('c') > result.rfind('a'):
    print("Test3: Pass")
else:
    print("Test3: Fail")
