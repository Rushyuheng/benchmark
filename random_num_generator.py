import os
import random

maxint = 2147483647
minint = -2147483648
counter = 0
size = input('enter the numbers of random number to generate:')
with open('./test.txt', 'w+',encoding = 'utf-8') as f:
    while counter < int(size):
        content = random.randint(minint,maxint)
        f.write(str(content) + os.linesep)
        if counter % 1000000 == 0:
            print(counter)
        counter = counter + 1
f.close()
