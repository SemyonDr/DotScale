from argparse import ArgumentTypeError
from random import gauss

import scipy as sci
import sympy as sym
from sympy import Expr, exp, symbols, Symbol, Rational, Float, sqrt, Abs, Interval, maximum, integrate, diff, simplify, expand, together, \
    cancel, maximum

from StatItem import StatItem
from GaussFunction import GaussFunction
from HermiteSpline import HermiteSpline
from SplineCoefsInt import SplineCoefsInt


class GaussTool:
    # -----------
    # CONSTANTS
    # -----------

    x_cutoff : Rational = Rational(65536*6 + 50051,65536)

    # -----------
    # EXPRESSIONS
    # -----------

    __err_f : Expr = (
              3 * Symbol('sigma')**4
            - 6 * Symbol('sigma')**2 * Symbol('x')**2
            +     Symbol('x')**4
    ) / Symbol('sigma')**8
    __err_g : Expr = exp(-(((Symbol('x0')/Symbol('sigma'))**2)/2))
    __err_diff : Expr = ((Symbol('x1') - Symbol('x0'))**4)/384
    __err_f_min : Expr = sqrt(3) * Symbol('sigma')




    # -----------
    # ERROR
    # EVALUATIONS
    # -----------

    @classmethod
    def max_err_bound(cls, g : GaussFunction, a : HermiteSpline) -> Float:
        """
        Finds maximum max bound for error of approximation on interval of that approximation.
        :param g:
        :param a:
        :return:
        """

        # Aliases
        sigma = g.get_sigma()
        f_min = GaussTool.__err_f_min.subs('sigma',sigma)
        left = a.get_left()
        right = a.get_right()

        # Searching for expression for max value of
        # f(x) = (3s^4 - 6s^2x^2 + x^4)/s^8
        # on the interval.
        max_f = 0
        if right <= f_min:
            max_f = GaussTool.__err_f.subs({'x' : left, 'sigma' : sigma})
        else:
            if left >= f_min:
                max_f = GaussTool.__err_f.subs({'x': right, 'sigma': sigma})
            else:
                f_left = GaussTool.__err_f.subs({'x': left, 'sigma': sigma})
                f_right = GaussTool.__err_f.subs({'x': right, 'sigma': sigma})
                if f_left > f_right:
                    max_f = f_left
                else:
                    max_f = f_right

        # Evaluating the whole expression
        max_err_expr = max_f * GaussTool.__err_g * GaussTool.__err_diff
        return (max_err_expr
                .subs({'x0' : left,
                       'x1' : right,
                       'sigma' : sigma})
                .evalf(n = g.get_precision())
        )


    @classmethod
    def max_err(cls, g : GaussFunction, a : HermiteSpline) -> float:
        """
        Returns max value of the error of given
        hermite spline approximation
        of a given gauss function.
        :param g:
        :param a:
        :return:
        """
        x = Symbol('x')

        gx = g.get_expression()
        ax = a.get_expression()
        left = a.get_left()
        right = a.get_right()
        # Sympy expression for error
        delta_err_expr = Abs(gx - ax)

        # Converting sympy expression for error to regular python function
        # that can be consumed by numpy optimizer
        delta_err_func = sym.lambdify(x, delta_err_expr, 'numpy')

        # Using scipy (numpy based) algorithm that finds function minimum.
        # We pass -f() because we want maximum, and then we will multiply the result by -1.
        # Note that bounds must be converted to regular python float
        # to comply with scipy interface.
        delta_opt_res = sci.optimize.minimize_scalar(
                                lambda t: -delta_err_func(t),
                                bounds=(float(left), float(right)),
                                method='bounded')

        # Optimizer returns array of info, we take out only the function value
        delta_val = -1*delta_opt_res.fun

        return delta_val


    # -----------
    # SPLINE SPLIT
    # SELECTION
    # -----------

    @classmethod
    def produce_splines(cls, gauss : GaussFunction, splits_in_unit : int) -> list[HermiteSpline]:
        """
        Produces spline approximations for given gauss function until x cutoff is reached.
        Cutoff will be inside last spline interval.
        :param gauss:
        :param splits_in_unit:
        :return:
        """
        # Iterations variables
        splines = []
        spline_num = 1
        left = Rational(0,splits_in_unit)
        right = Rational(spline_num,splits_in_unit)

        while left < GaussTool.x_cutoff:
            # Producing approximation
            spline = gauss.get_approximation(left, right)
            splines.append(spline)
            # Advancing iteration
            spline_num += 1
            left = right
            right = Rational(spline_num,splits_in_unit)

        return splines


    @classmethod
    def spline_statistics_report(cls, gauss :GaussFunction, splines : list[HermiteSpline]):
        """
        Reports various statistic related to given spline split.
        Reports approximation error and coefficients statistics.
        :param gauss:
        :param splines:
        :return:
        """

        # Helper class to hold overall statistics
        class SplitStats:
            def __init__(self):
                self.a = StatItem()
                self.b = StatItem()
                self.c = StatItem()
                self.d = StatItem()
                self.coefs = StatItem()
                self.err = StatItem()

        stats = SplitStats()
        i = 0
        # Reporting values and gathering stats
        print(f'SPLINES LIST')
        for spline in splines:
            # Finding error
            error_val = GaussTool.max_err(gauss, spline)
            stats.err.update(error_val)

            # Coefficients
            coefs = spline.get_coefficients()
            stats.a.update(Abs(coefs[0]))
            stats.b.update(Abs(coefs[1]))
            stats.c.update(Abs(coefs[2]))
            stats.d.update(Abs(coefs[3]))
            stats.coefs.update_list( [ Abs(coefs[0]), Abs(coefs[1]), Abs(coefs[2]), Abs(coefs[3]) ] )

            print(f'#{i}\t[{spline.get_left().evalf(n=5)}, {spline.get_right().evalf(n=5)}]', end='\t\t')
            print(f'Err={Float(error_val).evalf(n=3)}', end='\t\t')
            print(f'A={Abs(coefs[0]).evalf(n=3)}', end='\t\t')
            print(f'B={Abs(coefs[1]).evalf(n=3)}', end='\t\t')
            print(f'C={Abs(coefs[2]).evalf(n=3)}', end='\t\t')
            print(f'D={Abs(coefs[3]).evalf(n=3)}')

            i += 1

        # Reporting stats
        print(f'\nSPLINES STATISTICS')
        print(f'Number of splines {len(splines)}')
        print(f'Required memory for coefficients 16 bit {len(splines) * 4 * 2} bytes')
        print(f'Required memory for coefficients 32 bit {len(splines) * 4 * 4} bytes')
        print(f'ERROR\n\t{stats.err}')
        for bd in (16,32):
            print(f'\t{bd} bit', end='\n\t')
            print(f'Mean=[{int(round(stats.err.get_mean() * (2 ** bd)))}]', end='\t')
            print(f'Max=[{int(round(stats.err.get_max()*(2**bd)))}]', end='\t')
            print(f'StDev=[{int(round(stats.err.get_st_dev() * (2 ** bd)))}]')
        print(f'COEFFICIENTS\n\t{stats.coefs}')
        print(f'COEF A\n\t{stats.a}')
        print(f'COEF B\n\t{stats.b}')
        print(f'COEF C\n\t{stats.c}')
        print(f'COEF D\n\t{stats.d}')


    @staticmethod
    def evaluate_split(splits_in_unit : int):
        gauss_func = GaussFunction(sigma=Rational(1,1), precision=50)
        splines = GaussTool.produce_splines(gauss_func, splits_in_unit)
        GaussTool.spline_statistics_report(gauss_func, splines)

    @staticmethod
    def coefs_to_int(coefs : (int,int,int,int), bit_depth):
        coefs_int = SplineCoefsInt(
            int(round(coefs[0] * 2 ** bit_depth, 0)),
            int(round(coefs[1] * 2 ** bit_depth, 0)),
            int(round(coefs[2] * 2 ** bit_depth, 0)),
            int(round(coefs[3] * 2 ** bit_depth, 0))
        )
        return coefs_int


    # -----------
    # TESTS
    # -----------

    @staticmethod
    def test_max_err():
        left = Rational(0, 1)
        right = Rational(1, 1)
        gauss = GaussFunction(50, Rational(1,1))
        approx = gauss.get_approximation(left, right)
        print(f'Interval [{left} .. {right}]')
        print(f'Delta error\n\t{GaussTool.max_err(gauss,approx)}')
        print('\n')

    @staticmethod
    def test_gauss():
        func = GaussFunction(Rational(1,1), Rational(11,10))
        print(func.value(Rational(0,1)))
        print(func.value(Rational(1, 4)))
        print(func.value(Rational(2, 4)))
        print(func.value(Rational(3, 4)))
        print(func.value(Rational(4, 4)))
        print()

    @staticmethod
    def test_approximation():
        gauss = GaussFunction(precision=50, sigma=Rational(1,1))
        x_bot = 64
        #for x_top in range(0,x_bot - 1):

        left = Rational(1, 1)
        right = Rational(2, 1)
        mid = Rational(3, 2)
        approximation = gauss.get_approximation(left, right)
        print('APPROX COEF')
        print(f'\ta = {approximation.get_a()}')
        print(f'\tb = {approximation.get_b()}')
        print(f'\tc = {approximation.get_c()}')
        print(f'\td = {approximation.get_d()}')
        print(f'L = {left.evalf()}\nMID = {mid.evalf()}\nR = {right.evalf()}')
        print(f'LEFT\tg(L) = {gauss.value(left)}\n\t\ta(L) = {approximation.value(left)}')
        print(f'MID\t\tg(M) = {gauss.value(mid)}\n\t\ta(M) = {approximation.value(mid)}')
        print(f'RIGHT\tg(R) = {gauss.value(right)}\n\t\ta(R) = {approximation.value(right)}')
        print()


