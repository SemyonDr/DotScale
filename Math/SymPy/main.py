from GaussFunctionTool import GaussTool
from GaussFunctionApprox import GaussApproxC32

# GaussTool.evaluate_split(32)


def check_for_overflows_x16():
    apprx = GaussApproxC32()
    for x in range(0, 7 * 65536):
        val = apprx.value_x16(x)
        #print(f'ga({x}) = {apprx.value(x)}')

def check_for_overflows_x32():
    apprx = GaussApproxC32()
    for x in range(0, 7 * (2**32)):
        val = apprx.value_x32(x)
        #print(f'ga({x}) = {apprx.value(x)}')

#check_for_overflows_x16()

check_for_overflows_x32()