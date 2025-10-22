from __future__ import  annotations
from sympy import Expr, exp, symbols, Symbol, Rational, Float, integrate, diff, simplify, expand, together, cancel, solve, sqrt


# ----------------
# STAT ITEM
# ----------------

# Helper class to hold statistics for one item
class StatItem:

    def update(self, index, value):
        self.__values.append(value)
        if value > self.__max:
            self.__max = value
            self.__max_index = index
        if abs(value) > abs(self.__abs_max):
            self.__abs_max = value
            self.__abs_max_index = index

    # Expects (index, value) tuple
    def update_list(self, values: [(int, Float)]):
        for v in values:
            self.update(v[0], v[1])

    def get_max(self) -> Float:
        return Float(self.__max)

    def get_max_index(self) -> int:
        return self.__max_index

    def get_abs_max(self) -> Float:
        return Float(self.__abs_max)

    def get_abs_max_index(self) -> int:
        return self.__abs_max_index

    def get_mean(self) -> Float:
        val_sum = Float(0)
        for v in self.__values:
            val_sum += v
        return Float(val_sum / len(self.__values))

    def get_st_dev(self) -> Float:
        mean = self.get_mean()
        dev_sum = Float(0)
        for v in self.__values:
            dev_sum += Float((mean - v) ** 2)
        return sqrt(dev_sum / len(self.__values), evaluate=True)

    def __str__(self):
        return f'Mean=[{self.get_mean().evalf(n=3)}]\t Max=[{self.get_max().evalf(n=3)}] at [{self.__max_index}]\t StDev=[{self.get_st_dev().evalf(n=3)}]'

    def __init__(self):
        self.__max = 0
        self.__max_index = 0
        self.__abs_max = 0
        self.__abs_max_index = 0
        self.__values = []


# ----------------
# FUNCTION
# INTERVAL INFO
# ----------------

class FuncIntervalInfo:
    """
    Helper class that holds info
    about a function interval.
    """

    @staticmethod
    def new_polynomial_calc_info(poly : Expr, left : Rational, right : Rational) -> FuncIntervalInfo:
        """
        Finds max and min of polynomial on the interval [left..right].
        """
        info = FuncIntervalInfo()
        x = Symbol('x')
        # These are points to check for min-max.
        # Initially we check interval edges
        # and if we find critical points on the interval we check them too.
        x_crit = [left, right]
        drv = diff(poly, x)  # Derivative
        drv_solutions = solve(drv, x)  # Finding x where derivative is 0.
        for x_sol in drv_solutions:
            if x_sol.is_real and (left < x_sol) and (x_sol < right):
                x_crit.append(x_sol)
        # Finding min and max
        for xi in x_crit:
            val = poly.subs(x, xi)
            info.update(xi, val)
        return info

    def __init__(self):
        self.max = None
        self.min = None
        self.abs_max = None
        self.x_max = None
        self.x_min = None
        self.x_abs_max = None

    def update(self, x, val):
        # Initial value
        if self.max is None:
            self.max = val
            self.min = val
            self.abs_max = val
            self.x_max = x
            self.x_min = x
            self.x_abs_max = x
            return
        # Update
        if val > self.max:
            self.max = val
            self.x_max = x
        if val < self.min:
            self.min = val
            self.x_min = x
        if abs(val) > abs(self.abs_max):
            self.abs_max = val
            self.x_abs_max = x


# ----------------
# CALCULATION
# STEP STATISTICS
# ----------------

class CalcStepStats:
    """
    This class holds stats for certain
    calculation step over some set of splines.
    """
    def __init__(self):
        self.abs_max = None
        self.x_abs_max = None
        self.spline_index_abs_max = None
        self.max = None
        self.x_max = None
        self.spline_index_max = None
        self.min = None
        self.x_min = None
        self.spline_index_min = None

    def update(self, spline_index : int, spline_step_info : FuncIntervalInfo):
        # Initial value
        if self.abs_max is None:
            self.abs_max = spline_step_info.abs_max
            self.x_abs_max = spline_step_info.x_abs_max
            self.spline_index_abs_max = spline_index
            self.max = spline_step_info.max
            self.x_max = spline_step_info.x_max
            self.spline_index_max = spline_index
            self.min = spline_step_info.min
            self.x_min = spline_step_info.x_min
            self.spline_index_min = spline_index
            return
        # Update Max
        if spline_step_info.max > self.max:
            self.max = spline_step_info.max
            self.x_max = spline_step_info.x_max
            self.spline_index_max = spline_index
        # Update Min
        if spline_step_info.min < self.min:
            self.min = spline_step_info.min
            self.x_min = spline_step_info.x_min
            self.spline_index_min = spline_index
        # Update Abs Max
        if abs(spline_step_info.abs_max) > abs(self.abs_max):
            self.abs_max = spline_step_info.abs_max
            self.x_abs_max = spline_step_info.x_abs_max
            self.spline_index_abs_max = spline_index



class CalculationStepValues:
    """
    Values for intermediate steps of calculation
    of approximated gauss function value
    and area polynomial value.
    """

    def __init__(self):
        self.step_1 = None
        self.step_2 = None
        self.step_3 = None
        self.step_4 = None
        self.step_5 = None
        self.step_6 = None
        self.step_7 = None
        self.step_8 = None
        self.step_9 = None
        self.step_10 = None
        self.step_11 = None

    def result_value(self):
        return self.step_6

    def result_area(self):
        return self.step_11
