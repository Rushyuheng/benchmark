f = open('test.txt','r')
inputlist = []
for item in f:
    inputlist.append(int(item))
inputlist.sort()
f.close()

f = open('output.txt','r')
outputlist = []
for item in f :
    outputlist.append(int(item))
f.close()

if inputlist == outputlist:
    print('correct')
else:
    print('something wrong')
    i = 0
    while i < len(inputlist):
        if inputlist[i] != outputlist[i]:
            print('wrong answer at line %d' %i)
            print('The answer is %d' %inputlist[i])
            print('your answer is %d' %outputlist[i])
            i = i + 1
