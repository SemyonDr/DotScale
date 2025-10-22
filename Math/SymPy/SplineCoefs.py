from __future__ import annotations

import re
from sympy import Float

from Definitions import GT_Definitions


# ----------------
# VALUE SPLINE
# FLOAT
# ----------------

class VSplineCoefsFloat:
    def to_int(self, fraction_bit_depth : int = None) -> VSplineCoefsInt:
        if fraction_bit_depth is None:
            fraction_bit_depth = GT_Definitions.val_aprx_coef_bit_depth
        a_int = int(round((self.a * (2 ** fraction_bit_depth)).evalf(n=50)))
        b_int = int(round((self.b * (2 ** fraction_bit_depth)).evalf(n=50)))
        c_int = int(round((self.c * (2 ** fraction_bit_depth)).evalf(n=50)))
        d_int = int(round((self.d * (2 ** fraction_bit_depth)).evalf(n=50)))
        return VSplineCoefsInt(a_int, b_int, c_int, d_int)

    def to_int_bdlist(self, fraction_bit_depth : VSplineCoefsBitDepth = None):
        if fraction_bit_depth is None:
            fraction_bit_depth = GT_Definitions.val_aprx_coef_bit_depth
        a_int = int(round((self.a * (2 ** fraction_bit_depth)).evalf(n=50)))
        b_int = int(round((self.b * (2 ** fraction_bit_depth)).evalf(n=50)))
        c_int = int(round((self.c * (2 ** fraction_bit_depth)).evalf(n=50)))
        d_int = int(round((self.d * (2 ** fraction_bit_depth)).evalf(n=50)))
        return VSplineCoefsInt(a_int, b_int, c_int, d_int)

    @staticmethod
    def from_list(coefs : list[Float]):
        return VSplineCoefsFloat(coefs[0], coefs[1], coefs[2], coefs[3])

    def __init__(self, a : Float, b : Float, c : Float, d : Float):
        self.a = a
        self.b = b
        self.c = c
        self.d = d


# ----------------
# VALUE SPLINE
# INTEGER
# ----------------

class VSplineCoefsInt:
    __regex = r"-?\d+"

    def __init__(self, a : int, b : int, c : int, d : int):
        self.a = a
        self.b = b
        self.c = c
        self.d = d

    def to_struct_initializer_str(self) -> str:
        return "{ " + f"{self.a}, {self.b}, {self.c}, {self.d}" + " }"

    @staticmethod
    def from_struct_initializer_str(struct_init_str : str) -> VSplineCoefsInt:
        matches = re.findall(VSplineCoefsInt.__regex, struct_init_str)
        return VSplineCoefsInt(int(matches[0]), int(matches[1]), int(matches[2]), int(matches[3]))


# ----------------
# VALUE SPLINE
# BIT DEPTH LIST
# ----------------

class VSplineCoefsBitDepth:
    def __init__(self, a : int, b : int, c : int, d : int):
        self.a = a
        self.b = b
        self.c = c
        self.d = d


# ----------------
# AREA SPLINE
# FLOAT
# ----------------

class ASplineCoefsFloat:

    def to_int(self, fraction_bit_depth : int = None) -> ASplineCoefsInt:
        if fraction_bit_depth is None:
            fraction_bit_depth = GT_Definitions.area_aprx_coef_bit_depth
        c1_int = int(round((self.c1 * (2 ** fraction_bit_depth)).evalf(n=50)))
        c2_int = int(round((self.c2 * (2 ** fraction_bit_depth)).evalf(n=50)))
        c3_int = int(round((self.c3 * (2 ** fraction_bit_depth)).evalf(n=50)))
        c4_int = int(round((self.c4 * (2 ** fraction_bit_depth)).evalf(n=50)))
        c5_int = int(round((self.c5 * (2 ** fraction_bit_depth)).evalf(n=50)))
        c6_int = int(round((self.c6 * (2 ** fraction_bit_depth)).evalf(n=50)))
        return ASplineCoefsInt(c1_int, c2_int, c3_int, c4_int, c5_int, c6_int)

    def to_int_scaled(self, fraction_bit_depth : int = None) -> ASplineCoefsInt:
        if fraction_bit_depth is None:
            fraction_bit_depth = GT_Definitions.area_aprx_coef_bit_depth
        c1_int = int(round(((self.c1 / 6) * (2 ** fraction_bit_depth)).evalf(n=50)))
        c2_int = int(round(((self.c2 / 5) * (2 ** fraction_bit_depth)).evalf(n=50)))
        c3_int = int(round(((self.c3 / 4) * (2 ** fraction_bit_depth)).evalf(n=50)))
        c4_int = int(round(((self.c4 / 3) * (2 ** fraction_bit_depth)).evalf(n=50)))
        c5_int = int(round(((self.c5 / 2) * (2 ** fraction_bit_depth)).evalf(n=50)))
        c6_int = int(round(((self.c6 / 1) * (2 ** fraction_bit_depth)).evalf(n=50)))
        return ASplineCoefsInt(c1_int, c2_int, c3_int, c4_int, c5_int, c6_int)

    @staticmethod
    def from_list(coefs_list : list[Float]) -> ASplineCoefsFloat:
        return ASplineCoefsFloat(coefs_list[0], coefs_list[1], coefs_list[2],
                                 coefs_list[3], coefs_list[4], coefs_list[5] )

    def __init__(self, c1 : Float, c2 : Float, c3 : Float, c4 : Float, c5 : Float, c6 : Float):
        self.c1 = c1
        self.c2 = c2
        self.c3 = c3
        self.c4 = c4
        self.c5 = c5
        self.c6 = c6

# ----------------
# AREA SPLINE
# INTEGER
# ----------------

class ASplineCoefsInt:
    __regex = r"-?\d+"

    def to_struct_initializer_str(self) -> str:
        return "{ " + f"{self.c1}, {self.c2}, {self.c3}, {self.c4}, {self.c5}, {self.c6}" + " }"

    @staticmethod
    def from_struct_initializer_str(struct_init_str : str) -> ASplineCoefsInt:
        matches = re.findall(ASplineCoefsInt.__regex, struct_init_str)
        coefs = [ int(match) for match in matches]
        return ASplineCoefsInt.from_list(coefs)

    @staticmethod
    def from_list(coefs_list : list[int]) -> ASplineCoefsInt:
        return ASplineCoefsInt(coefs_list[0], coefs_list[1], coefs_list[2],
                                 coefs_list[3], coefs_list[4], coefs_list[5] )

    def __init__(self, c1: int, c2: int, c3: int, c4: int, c5: int, c6: int):
        self.c1 = c1
        self.c2 = c2
        self.c3 = c3
        self.c4 = c4
        self.c5 = c5
        self.c6 = c6

