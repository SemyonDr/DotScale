
from GaussFunction import GaussFunction
from GaussFunctionTool import GaussTool
from SplineCoefsInt import SplineCoefsInt

class SplineCoefsInt:
    def __init__(self, a : int, b : int, c : int, d : int):
        self.a = a
        self.b = b
        self.c = c
        self.d = d



class GaussApproxC32:
    """
    coefs are 32 bit
    splines is 32 per unit
    sigma is 1
    """

    def value_x16(self, x : int) -> int:
        # Coefficients supposed to be 32 bit for one unit.
        # Multiplication is supposed to be performed in 64 bit variable to give room.

        if x >= (6*65536 + 50051):
            return 0

        cf = self.__splines[ x // 2 ** 11]
        max64 = (2**63) - 1

        res = cf.a * x
        if abs(res) > max64:
            print(f'OVERFLOW! x={x / 2**16} x_int={x} step=ax')
        res = res // (2 ** (32 - 16))

        res = res + cf.b
        if abs(res) > max64:
            print(f'OVERFLOW! x={x / 2**16} x_int={x} step=(ax+b)')

        res = res * x
        if abs(res) > max64:
            print(f'OVERFLOW! x={x / 2**16} x_int={x} step=(ax+b)x')
        res = res // (2**(32 - 16))

        res = res + cf.c
        if abs(res) > max64:
            print(f'OVERFLOW! x={x / 2**16} x_int={x} step=(ax+b)x+c')

        res = res * x
        if abs(res) > max64:
            print(f'OVERFLOW! x={x / 2**16} x_int={x} step=((ax+b)x+c)x')
        res = res // (2**(32 - 16))

        res = res + cf.d
        if abs(res) > max64:
            print(f'OVERFLOW! x={x / 2**16} x_int={x} step=((ax+b)x+c)x+d')

        return res



    def value_x32(self, x : int) -> int:
        # Coefficients supposed to be 32 bit for one unit.
        # Multiplication is supposed to be performed in 64 bit variable to give room.

        if x >= (6*4294967296 + 3280090727):
            return 0

        cf = self.__splines[ x // 2 ** 27]
        max64 = (2**63) - 1

        res = cf.a * x
        if abs(res) > max64:
            print(f'OVERFLOW! x={x / 2**32} x_int={x} step=ax')
        res = res // (2 ** 32)

        res = res + cf.b
        if abs(res) > max64:
            print(f'OVERFLOW! x={x / 2**32} x_int={x} step=(ax+b)')

        res = res * x
        if abs(res) > max64:
            print(f'OVERFLOW! x={x / 2**32} x_int={x} step=(ax+b)x')
        res = res // (2**32)

        res = res + cf.c
        if abs(res) > max64:
            print(f'OVERFLOW! x={x / 2**32} x_int={x} step=(ax+b)x+c')

        res = res * x
        if abs(res) > max64:
            print(f'OVERFLOW! x={x / 2**32} x_int={x} step=((ax+b)x+c)x')
        res = res // (2**32)

        res = res + cf.d
        if abs(res) > max64:
            print(f'OVERFLOW! x={x / 2**32} x_int={x} step=((ax+b)x+c)x+d')

        return res


    def __init__(self):
        gauss_func = GaussFunction(precision = 50)
        splines = GaussTool.produce_splines(gauss_func, 32)
        self.__splines = []
        for spl in splines:
            coefs_int = GaussTool.coefs_to_int(spl.get_coefficients(), 32)
            self.__splines.append(coefs_int)

