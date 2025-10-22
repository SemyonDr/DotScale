from concurrent.futures.process import ProcessPoolExecutor

import scipy as sci
import sympy as sym
from sympy import Expr, exp, Symbol, Rational, Float, sqrt, Abs

from AreaSpline import AreaSpline
from Definitions import GT_Definitions
from Statistics import StatItem, CalcStepStats, FuncIntervalInfo
from GaussFunction import GaussFunction
from ValueSpline import ValueSpline
from SplineCoefs import VSplineCoefsInt, VSplineCoefsBitDepth, ASplineCoefsInt

# -----------
# PARALLEL TASKS
# PRODUCE AREA
# APPROXIMATIONS
# -----------

# Gauss function for producing areas
GFUNC_AREAS_SOURCE : GaussFunction = None

class IntervalRat:
    """ X interval argument. """
    def __init__(self, left : Rational, right : Rational, index : int = 0):
        self.left = left
        self.right = right
        self.index = index

def produce_areas_worker_init():
    """ Initializes worker process for producing areas. """
    global GFUNC_AREAS_SOURCE
    GFUNC_AREAS_SOURCE= GaussFunction(50)

def produce_area_aprx_task(interval : IntervalRat) -> (int, AreaSpline):
    """ Parallel task that produces area approximation with given gauss function. """
    return interval.index, GFUNC_AREAS_SOURCE.get_area_approximation(interval.left, interval.right)

def area_poly_value_calc_info_task(index : int, spline : AreaSpline) -> (int,list[FuncIntervalInfo]):
    """ Parallel task that finds info of calculation steps of area polynomial for certain spline. """
    return index, spline.area_poly_value_calc_info()


# -----------
# TOOL CLASS
# -----------

class GaussTool:
    # -----------
    # CONSTANTS
    # -----------

    x_cutoff : Rational = Rational(65536*6 + 50051,65536)
    x_cutoff_area : Rational = Rational(7468738554291142405, 1152921504606846976)

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
    def max_err_bound(cls, g : GaussFunction, a : ValueSpline) -> Float:
        """
        Finds maximum max bound for error of approximation on interval of that approximation.
        Error is approximated with standard method for hermite spline.
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
    def max_err(cls, g : GaussFunction, a : ValueSpline) -> float:
        """
        Returns max value of the error of given hermite
        spline value approximation of a given gauss function.
        Error is approximated numerically using scipy optimization algorithm.
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
    def produce_value_splines(cls, gauss : GaussFunction, splits_in_unit : int) -> list[ValueSpline]:
        """
        Produces spline approximations of function value
        for given gauss function until x cutoff is reached.
        Cutoff will be inside last spline interval.
        """
        # Iterations variables
        splines = []
        spline_num = 1
        left = Rational(0,splits_in_unit)
        right = Rational(spline_num,splits_in_unit)

        while left < GaussTool.x_cutoff:
            # Producing approximation
            spline = gauss.get_value_approximation(left, right)
            splines.append(spline)
            # Advancing iteration
            spline_num += 1
            left = right
            right = Rational(spline_num,splits_in_unit)

        return splines


    @classmethod
    def spline_statistics_report(cls, gauss :GaussFunction, splines : list[ValueSpline]):
        """
        Reports various statistic related to given spline split.
        Reports approximation error and coefficients statistics.
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
            stats.err.update(i, error_val)

            # Coefficients
            coefs = spline.get_coefficients()
            stats.a.update(i, Abs(coefs.a))
            stats.b.update(i, Abs(coefs.b))
            stats.c.update(i, Abs(coefs.c))
            stats.d.update(i, Abs(coefs.d))
            stats.coefs.update_list( [ (i,Abs(coefs.a)), (i,Abs(coefs.b)), (i,Abs(coefs.c)), (i,Abs(coefs.d)) ] )

            print(f'#{i}\t[{spline.get_left().evalf(n=5)}, {spline.get_right().evalf(n=5)}]', end='\t\t')
            print(f'Err={Float(error_val).evalf(n=3)}', end='\t\t')
            print(f'A={Abs(coefs.a).evalf(n=3)}', end='\t\t')
            print(f'B={Abs(coefs.b).evalf(n=3)}', end='\t\t')
            print(f'C={Abs(coefs.c).evalf(n=3)}', end='\t\t')
            print(f'D={Abs(coefs.d).evalf(n=3)}')

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
        print(f'COEF A\n\t{stats.a}\tPrecise max value is {stats.a.get_max()}')
        print(f'COEF B\n\t{stats.b}\tPrecise max value is {stats.b.get_max()}')
        print(f'COEF C\n\t{stats.c}\tPrecise max value is {stats.c.get_max()}')
        print(f'COEF D\n\t{stats.d}\tPrecise max value is {stats.d.get_max()}')


    @staticmethod
    def evaluate_split(splits_in_unit : int):
        gauss_func = GaussFunction(sigma=Rational(1,1), precision=50)
        splines = GaussTool.produce_value_splines(gauss_func, splits_in_unit)
        GaussTool.spline_statistics_report(gauss_func, splines)


    @classmethod
    def produce_area_splines(cls, gauss: GaussFunction, splits_in_unit: int) -> list[AreaSpline]:
        """
        Produces spline approximations for area calculation
        for given gauss function until x cutoff is reached.
        Cutoff will be inside last spline interval.
        """
        # Iterations variables
        splines = []
        spline_index = 0
        left = Rational(0, splits_in_unit)
        right = Rational(1, splits_in_unit)

        print(f"Producing area splines:")
        while left < GaussTool.x_cutoff:
            print(f"{spline_index:3d}", end=" ")
            if (spline_index + 1) % 32 == 0:
                print()
            # Producing approximation
            spline = gauss.get_area_approximation(left, right)
            splines.append(spline)
            # Advancing iteration
            spline_index += 1
            left = right
            right = Rational(spline_index + 1, splits_in_unit)
        print()
        return splines


    @classmethod
    def produce_area_splines_parallel(cls, gauss: GaussFunction, splits_in_unit: int) -> list[AreaSpline]:
        """
        Produces spline approximations for area calculation
        for given gauss function until x cutoff is reached.
        Cutoff will be inside last spline interval.
        """
        # Iterations variables
        splines = []

        # Producing interval arguments
        intervals = []
        spline_index = 0
        left = Rational(0, splits_in_unit)
        right = Rational(1, splits_in_unit)
        while left < GaussTool.x_cutoff_area:
            interval = IntervalRat(left, right, index=spline_index)
            intervals.append(interval)
            # Advancing iteration
            spline_index += 1
            left = right
            right = Rational(spline_index + 1, splits_in_unit)

        # Running parallel tasks
        # Setting up process executor pool
        executor = ProcessPoolExecutor(initializer=produce_areas_worker_init)
        # Passing tasks
        results = executor.map(produce_area_aprx_task, intervals, chunksize=2)
        # Processing each result as soon as next result in original order of arguments is completed
        for r in results:
            result_index = r[0]
            result_spline = r[1]
            print(f"{r[0]:3d}", end=" ")
            if (result_index + 1) % 32 == 0:
                print()
            splines.append(result_spline)
        print()
        return splines



    # -----------
    # AREAS
    # -----------

    @staticmethod
    def produce_areas_for_split(splits_in_unit : int) -> list[Float]:
        """
        Calculates precise areas for given number of split segments.
        """
        gauss_func = GaussFunction(sigma=Rational(1, 1), precision=50)
        areas = []

        segment_index = 0
        left = Rational(0,splits_in_unit)
        right = Rational(segment_index,splits_in_unit)
        while left < GaussTool.x_cutoff_area:
            # Area
            areas.append(gauss_func.area(left, right))
            # Iteration
            segment_index += 1
            left = right
            right = Rational(segment_index + 1,splits_in_unit)

        return areas


    # -----------
    # CALCULATION
    # STEPS EVALUATION
    # -----------

    @staticmethod
    def evaluate_value_calc_steps() -> list[CalcStepStats]:
        """
        Runs value approximation calculations for 64-segment split and reports
        statistics for each step.
        """

        print(f"Evaluating absolute max values for steps of value calculation.")
        # Producing splines
        gauss_obj = GaussFunction(precision=50, sigma=Rational(1, 1))
        splines = GaussTool.produce_value_splines(gauss_obj, 64)
        print(f"{len(splines)} splines produced.")
        # This is the result
        evaluation = [CalcStepStats() for _ in range(6)]
        # Taking evaluation for all steps of each spline
        # and producing statistics for each individual step over all splines
        print(f"Collecting statistics for individual splines:")
        for spline_index in range(len(splines)):
            print(f"{spline_index:3d}", end=" ")
            if (spline_index+1) % 32 == 0:
                print()
            spline = splines[spline_index]
            # This is list of statistics for each value calculations step for this spline
            spline_calc_info = spline.value_calc_info()
            # Updating overall split statistics for each step
            for step in range(6):
                evaluation[step].update(spline_index, spline_calc_info[step])
        print()
        # We print result right here, why not?
        max_64bit_signed = (2 ** 63) - 1
        min_64bit_signed = -(2 ** 63)
        print(f"Max 64 bit signed [+{max_64bit_signed}]")
        print(f"Min 64 bit signed [{min_64bit_signed}]")
        for step in range(6):
            print(f"Step {step+1}")
            print(f"\tMax Abs:\t{evaluation[step].abs_max}")
            print(f"\tAt x:\t\t{evaluation[step].x_abs_max}")
            print(f"\tIn spline:\t{evaluation[step].spline_index_abs_max}")
            precision_power = 25
            abs_max_int = int(round((evaluation[step].abs_max * (2**precision_power)).evalf(n=50)))
            while (abs_max_int <= max_64bit_signed) and (abs_max_int >= min_64bit_signed):
                precision_power += 1
                abs_max_int = int(round((evaluation[step].abs_max * (2 ** precision_power)).evalf(n=50)))
            precision_power -= 1
            abs_max_int = int(round((evaluation[step].abs_max * (2 ** precision_power)).evalf(n=50)))
            print(f"\tMax Precision:\t{precision_power} bit")
            print(f"\tMax Int With\n\tPrecision {precision_power}:\t{abs_max_int}")
        return evaluation



    @staticmethod
    def evaluate_area_poly_value_calc_steps() -> list[CalcStepStats]:
        print(f"Evaluating absolute max values for steps of area indefinite integral calculation.")
        print(f"Producing area splines.")
        # Producing splines
        g_func = GaussFunction(precision=50, sigma=Rational(1, 1))
        splines = GaussTool.produce_area_splines_parallel(g_func, 64)
        print(f"{len(splines)} area splines produced.")

        # This is the result
        num_steps = AreaSpline.get_num_steps_area_poly_value()
        evaluation = [CalcStepStats() for _ in range(num_steps)]

        # Taking evaluation for all steps of each spline
        # and producing statistics for each individual step over all splines
        print(f"Collecting statistics for individual splines:")
        # Running parallel tasks
        # Setting up process executor pool
        executor = ProcessPoolExecutor()
        # Passing tasks
        results = executor.map(area_poly_value_calc_info_task, range(len(splines)), splines)
        # Processing each result as soon as next result in original order of arguments is completed
        for r in results:
            spline_index = r[0]
            spline_calc_info = r[1]
            print(f"{spline_index:3d}", end=" ")
            if (spline_index + 1) % 32 == 0:
                print()
            # Updating overall split statistics for each step
            for step in range(num_steps):
                evaluation[step].update(spline_index, spline_calc_info[step])
        print()

        # We print result right here, why not?
        max_64bit_signed = (2 ** 63) - 1
        min_64bit_signed = -(2 ** 63)
        print(f"Max 64 bit signed [+{max_64bit_signed}]")
        print(f"Min 64 bit signed [{min_64bit_signed}]")
        for step in range(num_steps):
            print(f"Step {step + 1}")
            print(f"\tMax Abs:\t{evaluation[step].abs_max}")
            print(f"\tAt x:\t\t{evaluation[step].x_abs_max}")
            print(f"\tIn spline:\t{evaluation[step].spline_index_abs_max}")
            precision_power = 25
            abs_max_int = int(round((evaluation[step].abs_max * (2 ** precision_power)).evalf(n=50)))
            while (abs_max_int <= max_64bit_signed) and (abs_max_int >= min_64bit_signed):
                precision_power += 1
                abs_max_int = int(round((evaluation[step].abs_max * (2 ** precision_power)).evalf(n=50)))
            precision_power -= 1
            abs_max_int = int(round((evaluation[step].abs_max * (2 ** precision_power)).evalf(n=50)))
            print(f"\tMax Precision:\t{precision_power} bit")
            print(f"\tMax Int With\n\tPrecision {precision_power}:\t{abs_max_int}")
        return evaluation


    # -----------
    # PRODUCING
    # DATA FILES
    # -----------

    @staticmethod
    def write_value_approximation_coefs(splits_in_unit : int = None, fractional_bit_depth : VSplineCoefsBitDepth = None):
        # Default number of splits
        if splits_in_unit is None:
            splits_in_unit = GT_Definitions.val_splines_in_unit

        # Getting polynomial coefficients
        gauss = GaussFunction(sigma=Rational(1,1), precision=50)
        splines = GaussTool.produce_value_splines(gauss=gauss, splits_in_unit=splits_in_unit)

        # Converting to integer numbers
        coefs = []
        for i in range(len(splines)):
            float_coefs = splines[i].get_coefficients()
            int_coefs = float_coefs.to_int_bdlist(fractional_bit_depth)
            print(f"Spline {i}:")
            print(f"\tA {float_coefs.a} -> {int_coefs.a}")
            print(f"\tB {float_coefs.b} -> {int_coefs.b}")
            print(f"\tC {float_coefs.c} -> {int_coefs.c}")
            print(f"\tD {float_coefs.d} -> {int_coefs.d}")
            coefs.append(int_coefs)

        # Writing to file
        file_name = GT_Definitions.get_value_splines_file_name(splits_in_unit, fractional_bit_depth.a)
        output_file = open(file=file_name,mode='w')
        print(f"Opening file {file_name} for writing coefficients.")
        output_file.write(f"{coefs[0].to_struct_initializer_str()}")
        written_count = 4
        for i in range(1, len(coefs)):
            output_file.write(f",\n{coefs[i].to_struct_initializer_str()}")
            written_count += 4
        output_file.close()
        print(f"{written_count} coefficients were written. File closed.")


    @staticmethod
    def write_area_poly_coefs(splits_in_unit : int = None, fractional_bit_depth : int = None):
        # Default number of splits
        if splits_in_unit is None:
            splits_in_unit = GT_Definitions.area_splines_in_unit

        # Getting polynomial coefficients
        print(f"Producing area approximation splines.")
        gauss = GaussFunction(sigma=Rational(1, 1), precision=50)
        splines = GaussTool.produce_area_splines_parallel(gauss=gauss, splits_in_unit=splits_in_unit)

        # Converting to integer numbers
        coefs = []
        for i in range(len(splines)):
            float_coefs = splines[i].get_coefficients()
            int_coefs = float_coefs.to_int_scaled(fractional_bit_depth)
            print(f"Spline {i}:")
            print(f"\tC1 {float_coefs.c1} -> {int_coefs.c1}")
            print(f"\tC2 {float_coefs.c2} -> {int_coefs.c2}")
            print(f"\tC3 {float_coefs.c3} -> {int_coefs.c3}")
            print(f"\tC4 {float_coefs.c4} -> {int_coefs.c4}")
            print(f"\tC5 {float_coefs.c5} -> {int_coefs.c5}")
            print(f"\tC6 {float_coefs.c6} -> {int_coefs.c6}")
            coefs.append(int_coefs)

        # Writing to file
        file_name = GT_Definitions.get_area_splines_file_name(splits_in_unit, fractional_bit_depth)
        output_file = open(file=file_name, mode='w')
        print(f"Opening file {file_name} for writing coefficients.")
        output_file.write(f"{coefs[0].to_struct_initializer_str()}")
        written_count = 6
        for i in range(1, len(coefs)):
            output_file.write(f",\n{coefs[i].to_struct_initializer_str()}")
            written_count += 6
        output_file.close()
        print(f"{written_count} coefficients were written. File closed.")


    @staticmethod
    def write_precalculated_areas():
        g_func = GaussFunction(50)
        segment_width = Rational(1,GT_Definitions.area_splines_in_unit)
        areas_lists = []
        for scale in range(GT_Definitions.area_num_scales):
            print(f"Areas of scale {scale}:")
            scale_areas = []
            left = Rational(0, 1)
            right = segment_width
            i = 0
            while left < GaussTool.x_cutoff_area:
                segment_area = g_func.area(left,right)
                segment_area_63 = int(round((segment_area * (2**63)).evalf(50)))
                scale_areas.append(segment_area_63)
                print(f"\t{i}:\t{segment_area_63}\t\t{segment_area}")
                left = right
                right += segment_width
                i += 1
            areas_lists.append(scale_areas)
            segment_width = segment_width * 2
            print()

        for scale in range(GT_Definitions.area_num_scales):
            file_name = GT_Definitions.get_area_filename(scale, 63)
            output_file = open(file=file_name, mode='w')
            print(f"Opening file {file_name} for writing areas for scale {scale}.")
            output_file.write(f"{areas_lists[scale][0]}")
            for i in range(1,len(areas_lists[scale])):
                output_file.write(f",\n{areas_lists[scale][i]}")
            output_file.close()
            print(f"{len(areas_lists[scale])} areas were written. File closed.\n")

    # -----------
    # LOADING
    # DATA FILES
    # -----------

    @staticmethod
    def load_value_spline_coefficients() -> list[VSplineCoefsInt]:
        coefs = []

        file_name = GT_Definitions.get_value_splines_file_name()
        file = open(file=file_name, mode='r')
        for line in file:
            cf = VSplineCoefsInt.from_struct_initializer_str(line)
            coefs.append(cf)
        file.close()
        return coefs

    @staticmethod
    def load_area_spline_coefficients() -> list[ASplineCoefsInt]:
        coefs = []

        file_name = GT_Definitions.get_area_splines_file_name()
        file = open(file=file_name, mode='r')
        for line in file:
            cf = ASplineCoefsInt.from_struct_initializer_str(line)
            coefs.append(cf)
        file.close()
        return coefs

    @staticmethod
    def load_areas_for_scale(scale : int) -> list[int]:
        file = open(file=GT_Definitions.get_area_filename(scale), mode='r')
        areas = []
        for line in file:
            # skip empty lines
            if not line:
                continue
            # remove new line and spaces
            line = line.strip()
            # remove a trailing comma if present
            if line.endswith(","):
                line = line[:-1]
            # saving
            areas.append(int(line))
        return areas

    @staticmethod
    def load_areas() -> list[list[int]]:
        areas = []
        for scale in range(GT_Definitions.area_num_scales):
            scale_areas = GaussTool.load_areas_for_scale(scale)
            areas.append(scale_areas)
        return areas







