import sys
import subprocess
import string

def Usage():
    print "Usage demangle.py [proccount.out] [TestHEAAN]"
    print "[proccount.out] must be generated using the proccount.so Pin Tool"
    print "[TestHEEAN] is target application"
    print " output : proccount.demangle "

    return -1

def Main(args):
    if len(sys.argv) < 3:
        Usage()
        return -1


    # read proccount file
    proccount_data = ''
    with open(args[0], 'U') as rf:
        proccount_data = rf.read()
    
    mangling_data = subprocess.check_output('nm ' + args[1], shell=True)
    demangling_data = subprocess.check_output('nm --demangle ' + args[1], shell=True)

    mangling_functions = string.split(mangling_data, '\n')
    demangling_functions = string.split(demangling_data, '\n')

    for i in range(0, len(mangling_functions)):
        mangling_list = string.split(mangling_functions[i], ' ')
        demangling_list = string.split(demangling_functions[i], ' ')

        before = ''
        after =''

        for j in range(2, len(mangling_list)):
            if mangling_list[j] != 'U' : 
                before += mangling_list[j] 

        before = before.replace('@@GLIBCXX_3.4', '@plt')

        for j in range(2, len(demangling_list)):
            if demangling_list[j] != 'U' : 
                after += demangling_list[j] 

        after = after.replace('@@GLIBCXX_3.4', '@plt')


        proccount_data = proccount_data.replace(before, after)

    with open('proccount.demangle', "w") as wf:
        wf.write(proccount_data)



if __name__ == "__main__":
    sys.exit( Main( sys.argv[1:]) )
