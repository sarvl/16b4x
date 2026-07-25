from os import system
import mmap

with open("./tests/test_groups.txt", "r") as f_tg:
    
    for group in f_tg:
        groupid = group[1] + group[2]

        count = int(group[7] + group[8])


        for ii in range(count):
            tst = groupid + "t" + str(ii).zfill(2)

            print(tst, end='')

            r = system("./cosi ./tests/bin/g" + tst + ".bin") 

            with open("dump.txt", 'rb') as f1, open("./tests/out/g" + tst + ".out", 'rb') as f2:
                m1 = mmap.mmap(f1.fileno(), 0, access=mmap.ACCESS_READ)
                m2 = mmap.mmap(f2.fileno(), 0, access=mmap.ACCESS_READ)

                if m1.read() == m2.read():
                    print(" ok")
                else:
                    print(" bad")
