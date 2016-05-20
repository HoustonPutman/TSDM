
def toHex(num):
    return "0x%0.16X" % num

def writeLine(f,addr,pc):
    mult = 64*64*64
    line = toHex(pc*mult) + " " + toHex(addr*mult) + " L\n"
    f.write(line)

def makeTest(file_name,pcs,loops,intervals):
    f = open(file_name,'w')
    pcs = range(0,pcs)
    num_times = loops
    for i in range(0,num_times):
        for pc in pcs:
            for inter in intervals:
                base=pc*12345
                writeLine(f,base+inter,pc)
            f.flush()
    f.close()

file_name = '../data/alias_test.L2.trace'
intervals = [0,1,2,3,0,4,5,3,0,6,7,3,0,8,9,3]
makeTest(file_name,1000,10,intervals)
