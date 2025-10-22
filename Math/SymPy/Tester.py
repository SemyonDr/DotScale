import random

from sympy import Float, Rational, sqrt, pi

from Definitions import GT_Definitions
from GaussFunction import GaussFunction
from GaussFunctionTool import GaussTool
from GaussFunctionApprox import GaussApprox
from SplineCoefs import VSplineCoefsBitDepth
from Statistics import StatItem
from printer import binary_to_str_64bit, binary_64_legend, fxd_to_str


class GaussTester:



    # -----------
    # GAUSS FUNCTION
    # -----------

    @staticmethod
    def test_gauss_calculations():
        """ Calculates precise gauss function values. """
        func = GaussFunction(Rational(1,1), Rational(11,10))
        print(func.value(Rational(0,1)))
        print(func.value(Rational(1, 4)))
        print(func.value(Rational(2, 4)))
        print(func.value(Rational(3, 4)))
        print(func.value(Rational(4, 4)))
        print()

    @staticmethod
    def test_value_approximation_production():
        """ Produces value approximation. """
        gauss = GaussFunction(precision=50, sigma=Rational(1,1))
        x_bot = 64
        #for x_top in range(0,x_bot - 1):

        left = Rational(1, 1)
        right = Rational(2, 1)
        mid = Rational(3, 2)
        approximation = gauss.get_value_approximation(left, right)
        aprx_coefs = approximation.get_coefficients()
        print('APPROX COEF')
        print(f'\ta = {aprx_coefs.a}')
        print(f'\tb = {aprx_coefs.b}')
        print(f'\tc = {aprx_coefs.c}')
        print(f'\td = {aprx_coefs.d}')
        print(f'L = {left.evalf()}\nMID = {mid.evalf()}\nR = {right.evalf()}')
        print(f'LEFT\tg(L) = {gauss.value(left)}\n\t\ta(L) = {approximation.value(left)}')
        print(f'MID\t\tg(M) = {gauss.value(mid)}\n\t\ta(M) = {approximation.value(mid)}')
        print(f'RIGHT\tg(R) = {gauss.value(right)}\n\t\ta(R) = {approximation.value(right)}')
        print()

    # -----------
    # APPROXIMATED VALUE
    #
    # -----------

    @staticmethod
    def test_max_err():
        """ Finds error bound for value approximation for one spline. """
        left = Rational(0, 1)
        right = Rational(1, 1)
        gauss = GaussFunction(50, Rational(1,1))
        approx = gauss.get_value_approximation(left, right)
        print(f'Interval [{left} .. {right}]')
        print(f'Delta error\n\t{GaussTool.max_err(gauss,approx)}')
        print('\n')

    @staticmethod
    def test_value_approx_x60_rng(num_points : int):
        """ Tests value approximation calculation. X points are randomly selected. """
        rng = random
        x_points = []
        for _ in range(num_points):
            x = rng.randint(0, rng.randint(0, GaussApprox.get_x_cutoff_60()))
        GaussTester.test_value_approx_x60(x_points)

    @staticmethod
    def test_value_approx_x60(x_points : list[int]):
        """ Tests value approximation calculation. Expects list of x int 60 bits precision """
        g_func = GaussFunction(50)
        g_aprx = GaussApprox()
        g_aprx.print_vsplines()
        delta_32_stat = StatItem()
        for i in range(len(x_points)):
            x = x_points[i]
            print(f"Calculating value for x60={fxd_to_str(x, 60)}")
            approx_val = g_aprx.value_x60(x, report=False).result_value()
            precise_val = g_func.value(Rational(x, 2 ** 60))
            precise_32 = int(round((precise_val * (2 ** 32)).evalf(n=50)))
            delta_32 = abs(precise_32 - (approx_val >> 28))
            delta_32_stat.update(i, delta_32)
            print(f"Approx value 60 bit is {fxd_to_str(approx_val, 60)}")
            print(f"Precise expected value is {precise_val}")
            print(f"32 bit precise -- {precise_32}")
            print(f"32 bit approx  -- {approx_val >> 28}")
            print(f"32 bit delta   -- {delta_32}")
            print()
        print(f"\nAverage delta for 32 bit for {len(x_points)} points is {delta_32_stat.get_mean()}.")

    # -----------
    # PRECALCULATED
    # AREAS TEST
    # -----------

    @staticmethod
    def test_areas_for_split(splits_in_unit):
        """ Calculates areas for the split and checks the sum. """
        areas = GaussTool.produce_areas_for_split(splits_in_unit)
        total_area = Float(0)
        for area in areas:
            total_area += area
        print(f"Total areas for {splits_in_unit}-split is {total_area}")
        total_area_normalized = total_area/(sqrt(2*pi, False)).evalf(n=50)
        print(f"Total area normalized is {total_area_normalized}")


    # -----------
    # APPROXIMATED AREA
    # CALCULATION TESTS
    # -----------

    @staticmethod
    def test_area_approx_rng_single():
        GaussTester.test_area_approx_rng(1, True)

    @staticmethod
    def test_area_approx_rat_single(left : Rational, right : Rational):
        GaussTester.test_area_approx_rat([(left, right)], True)

    @staticmethod
    def test_area_approx_x60_single(left : int, right : int):
        GaussTester.test_area_approx_x60([(left, right)], True)

    @staticmethod
    def test_area_approx_rat(intervals : list[(Rational, Rational)], report=False):
        intervals_60 = []
        for interval in intervals:
            left_60 = int(round((interval[0] * (2 ** 60)).evalf(n=50)))
            right_60 = int(round((interval[1] * (2 ** 60)).evalf(n=50)))
            intervals_60.append((left_60, right_60))
        GaussTester.test_area_approx_x60(intervals_60, report)

    @staticmethod
    def test_area_approx_rng(num_areas : int = 1, report=False):
        rng = random
        intervals = []
        for _ in range(num_areas):
            left = rng.randint(0, GaussApprox.get_x_area_cutoff_60())
            right = rng.randint(left, GaussApprox.get_x_area_cutoff_60())
            intervals.append((left, right))
        GaussTester.test_area_approx_x60(intervals, report)

    @staticmethod
    def test_area_approx_x60(intervals : list[(int,int)], report=False):
        g_func = GaussFunction(50)
        g_aprx = GaussApprox()
        delta_stat = StatItem()
        single = False
        if len(intervals) == 1:
            single = True
        for i in range(len(intervals)):
            interval = intervals[i]
            left_60 = interval[0]
            right_60 = interval[1]
            area_precise = g_func.area(Rational(left_60, 2**60), Rational(right_60,2**60))
            area_aprx_63 = g_aprx.area_x60(left_60, right_60, report)
            area_precise_63 = int(round((area_precise * (2 ** 63)).evalf(n=50)))
            delta_63 = abs(area_precise_63 - area_aprx_63)
            delta_stat.update(i, delta_63)
            if not single:
                print(f"Calculating area for interval {i}:")
            else:
                print(f"Calculating area for interval:")
            print(f"\tLeft {fxd_to_str(left_60, 60)}")
            print(f"\tRight {fxd_to_str(right_60, 60)}")
            print(f"Precise area\t{area_precise}")
            print(f"63 bit precise\t{fxd_to_str(area_precise_63, 63)}")
            print(f"63 bit approx\t{fxd_to_str(area_aprx_63, 63)}")
            print(f"63 bit delta\t{fxd_to_str(delta_63, 63)}")
            print()

        if not single:
            print(f"Error statistics:")
            print(f"\t{delta_stat}")
        print()

    # -----------
    # APPROXIMATED AREA
    # APPROXIMATING INTEGRAL
    # CALCULATION TESTS
    # -----------

    @staticmethod
    def test_area_subscale_integral_x60_single(left : int, right : int, report=False):
        GaussTester.test_area_subscale_integral_x60([(left, right)], report)

    @staticmethod
    def test_area_subscale_integral_rat_single(left: Rational, right: Rational, report=False):
        GaussTester.test_area_subscale_integral_rat([(left, right)], report)

    @staticmethod
    def test_area_subscale_integral_rng_single(report=False):
        GaussTester.test_area_subscale_integral_rng(report)


    @staticmethod
    def test_area_subscale_integral_rng(num_intervals: int = 1, report=False):
        rng = random
        segment_width = 9007199254740992
        intervals = []
        for _ in range(num_intervals):
            segment_index = rng.randint(0,830)
            segment_left = segment_index * segment_width
            segment_right = segment_left + segment_width
            left_60 = rng.randint(segment_left, segment_right)
            right_60 = rng.randint(left_60, segment_right)
            intervals.append((left_60, right_60))
        GaussTester.test_area_subscale_integral_x60(intervals, report)

    @staticmethod
    def test_area_subscale_integral_rat(intervals : list[(Rational, Rational)], report=False):
        intervals_60 = []
        for interval in intervals:
            left = interval[0]
            right = interval[1]
            left_60 = int(round((left * (2 ** 60)).evalf(50)))
            right_60 = int(round((right * (2 ** 60)).evalf(50)))
            intervals_60.append((left_60, right_60))
        GaussTester.test_area_subscale_integral_x60(intervals_60, report)

    @staticmethod
    def test_area_subscale_integral_x60(intervals : list[(int, int)], report=False):
        g_func = GaussFunction(50)
        g_aprx = GaussApprox()
        delta_stat = StatItem()
        single = False
        if len(intervals) == 1:
            single = True
        for i in range(len(intervals)):
            interval = intervals[i]
            left_60 = interval[0]
            right_60 = interval[1]
            width_60 =  right_60 - left_60
            segment_index = left_60 >> 54
            segment_width_60 = 9007199254740992
            segment_left_60 = segment_index * segment_width_60
            segment_right_60 = segment_left_60 + segment_width_60
            area_precise = g_func.area(Rational(left_60,2**60), Rational(right_60, 2**60))
            area_precise_63 = int(round((area_precise * (2 ** 63)).evalf(n=50)))
            area_aprx_63 = g_aprx.subscale_integral_x60(left_60, right_60, report)
            delta_63 = abs(area_precise_63 - area_aprx_63)
            delta_stat.update(i, delta_63)
            if single:
                print(f"Calculating subscale area integral:")
            else:
                print(f"Calculating subscale area integral {i}:")
            print(f"\tSegment index [{segment_index}]")
            print(f"\tSegment Left {fxd_to_str(segment_left_60, 60)}")
            print(f"\tSegment Right {fxd_to_str(segment_right_60, 60)}")
            print(f"\tLeft {fxd_to_str(left_60, 60)}")
            print(f"\tRight {fxd_to_str(right_60, 60)}")
            print(f"\tWidth {fxd_to_str(width_60, 60)}")
            print(f"Precise area\t{area_precise}")
            print(f"63 bit precise\t{fxd_to_str(area_precise_63, 63)}")
            print(f"63 bit approx\t{fxd_to_str(area_aprx_63, 63)}")
            print(f"63 bit delta\t{fxd_to_str(abs(delta_63), 63)}")
            print()
        if not single:
            print(f"Error statistics:")
            print(f"\tMean:\t{fxd_to_str(int(delta_stat.get_mean()),63)}")
            print(f"\tMax:\t{fxd_to_str(int(delta_stat.get_abs_max()),64)}")
            print(f"\tMax at:\t{delta_stat.get_abs_max_index()}")
            print(f"\tStDev:\t{fxd_to_str(int(delta_stat.get_st_dev()),63)}")


    # -----------
    # AVERAGE VALUE
    # -----------

    @staticmethod
    def test_average_value_single_x60(left : int, right : int):
        GaussTester.test_average_value_x60([(left, right)])

    @staticmethod
    def test_average_value_x60(intervals : list[(int, int)]):
        g_func = GaussFunction(50)
        g_aprx = GaussApprox()
        single = False
        if len(intervals) == 1:
            single = True
        for i in range(len(intervals)):
            interval = intervals[i]
            left_60 = interval[0]
            right_60 = interval[1]
            width_60 =  right_60 - left_60
            area_aprx_63 = g_aprx.area_x60(left_60, right_60)
            # Note, that here we are promoting width to 63 bit precision...
            average_32 = int(round(((Float(area_aprx_63) / Float(8*width_60))*(2**32)).evalf(50)))
            if single:
                print(f"Average value:")
            else:
                print(f"Average value {i}:")
            print(f"\tLeft\t{fxd_to_str(left_60, 60)}")
            print(f"\tRight\t{fxd_to_str(right_60, 60)}")
            print(f"\tWidth\t{fxd_to_str(width_60, 60)}")
            print(f"63 bit approx area\t{fxd_to_str(area_aprx_63, 63)}")
            print(f"Avg value:\t{fxd_to_str(average_32,32)}")
            print()

