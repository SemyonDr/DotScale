from numbers import Rational

from Definitions import GT_Definitions
from GaussFunction import GaussFunction
from GaussFunctionTool import GaussTool
from SplineCoefs import VSplineCoefsInt, ASplineCoefsInt
from sympy import Rational
from Statistics import CalculationStepValues
from printer import fxd_to_str


class GaussApprox:
    """
    coefs are 32 bit
    splines are 64 per unit
    sigma is 1
    """

    # -----------
    # CONSTANTS
    # -----------

    __X_CUTOFF_16 : int = 6*65536 + 50050
    __X_CUTOFF_60 : int = 7798021677424194372
    __X_CUTOFF_AREA_60 : int = 7468738554291142405
    __X_AREA_LAST_60 : int = 7475975381435023360 # x for right side of the last segment of smallest segment scale
    __MASK_32_TOP : int = 18446744069414584320
    __MASK_32_BOT : int = 4294967295
    __SCALE_SEGMENT_WIDTH_60 : list[int] = [
        9007199254740992,
        18014398509481984,
        36028797018963968,
        72057594037927936,
        144115188075855872,
        288230376151711744,
        576460752303423488,
        1152921504606846976,
        2305843009213693952,
        4611686018427387904
    ]


    # -----------
    # CONSTANTS
    # ACCESSORS
    # -----------

    @staticmethod
    def get_x_cutoff_60() -> int:
        return GaussApprox.__X_CUTOFF_60

    @staticmethod
    def get_x_area_cutoff_60() -> int:
        return GaussApprox.__X_CUTOFF_AREA_60

    @staticmethod
    def get_mask_32_top() -> int:
        return GaussApprox.__MASK_32_TOP

    @staticmethod
    def get_mask_32_bot() ->int:
        return GaussApprox.__MASK_32_BOT


    # -----------
    # DATA
    # -----------

    __vsplines : list[VSplineCoefsInt]
    __asplines : list[ASplineCoefsInt]
    __areas : list[list[int]]

    # -----------
    # VALUE
    # -----------

    def value_x60(self, x: int, report=False) -> CalculationStepValues:
        if report:
            print(f"Calculating value for x60={fxd_to_str(x,60)}")

        # Step values
        sv = CalculationStepValues()

        if x == 0 or x > GaussApprox.__X_CUTOFF_60:
            if report:
                if x==0:
                    print(f"\tx is zero. g(x) returns 0.")
                else:
                    print(f"\tx is out of range. g(x) returns 0.")
            sv.step_1 = 0
            sv.step_2 = 0
            sv.step_3 = 0
            sv.step_4 = 0
            sv.step_5 = 0
            sv.step_6 = 0
            return sv

        # Retrieving relevant spline
        spline_index = x >> 54
        cf = self.__vsplines[spline_index]
        if report:
            print(f"\tRelevant spline is [{spline_index}]:")
            print(f"\t\tLeft = [{spline_index / 64}]")
            print(f"\t\tRight = [{(spline_index + 1) / 64}]")
            print(f"\t\tA = {fxd_to_str(cf.a, 60)}")
            print(f"\t\tB = {fxd_to_str(cf.b, 60)}")
            print(f"\t\tC = {fxd_to_str(cf.c, 60)}")
            print(f"\t\tD = {fxd_to_str(cf.d, 60)}")

        x_bot = x & GaussApprox.__MASK_32_BOT
        x_top = (x - x_bot) >> 32

        # STEP 1 -- ax
        a_bot = cf.a & GaussApprox.__MASK_32_BOT
        a_top = (cf.a - a_bot) >> 32
        res = ((a_top*x_top)<<4) + ((a_top*x_bot)>>28) + ((a_bot*x_top)>>28) + ((a_bot*x_bot)>>60)
        sv.step_1 = res
        if report:
            print(f"\tStep 1 -- [ a * x ]:\t\t\t{fxd_to_str(sv.step_1, 60)}.")

        # STEP 2 -- ax+b
        res = res + cf.b
        sv.step_2 = res
        if report:
            print(f"\tStep 2 -- [ ax + b ]:\t\t\t{fxd_to_str(sv.step_2, 60)}.")

        # STEP 3 -- (ax+b)x
        res_bot = res & GaussApprox.__MASK_32_BOT
        res_top = (res - res_bot) >> 32
        res = ((res_top * x_top) << 4) + ((res_top * x_bot) >> 28) + ((res_bot * x_top) >> 28) + ((res_bot*x_bot)>>60)
        sv.step_3 = res
        if report:
            print(f"\tStep 3 -- [ (ax+b) * x ]:\t\t{fxd_to_str(sv.step_3, 60)}.")

        # STEP 4 -- (ax+b)x + c
        res = res + cf.c
        sv.step_4 = res
        if report:
            print(f"\tStep 4 -- [ (ax+b)x + c ]:\t\t{fxd_to_str(sv.step_4, 60)}.")

        # STEP 5 -- ((ax+b)x+c)x
        res_bot = res & GaussApprox.__MASK_32_BOT
        res_top = (res - res_bot) >> 32
        res = ((res_top * x_top) << 4) + ((res_top * x_bot) >> 28) + ((res_bot * x_top) >> 28) + ((res_bot*x_bot)>>60)
        sv.step_5 = res
        if report:
            print(f"\tStep 5 -- [ ((ax+b)x+c) * x ]:\t{fxd_to_str(sv.step_5, 60)}.")

        # STEP 6 -- ((ax+b)x+c)x + d
        res = res + cf.d
        sv.step_6 = res
        if report:
            print(f"\tStep 6 -- [ ((ax+b)x+c)x + d ]:\t{fxd_to_str(sv.step_6, 60)}.")

        return sv


    # -----------
    # AREA
    # -----------

    def area_x60(self, left: int, right : int, report=False):
        """
        Takes left and right of 60-bit precision.
        Returns 63 bit precision result.
        """
        if report:
            print(f"Calculating area approximation between:")
            print(f"\tLeft:\t{fxd_to_str(left,60)}")
            print(f"\tRight:\t{fxd_to_str(right, 60)}")
            print()

        # Edges
        if right == left:
            return 0

        if right < left:
            return 0

        if left > GaussApprox.__X_CUTOFF_AREA_60:
            return 0

        if right > GaussApprox.__X_CUTOFF_AREA_60:
            if right > GaussApprox.__X_AREA_LAST_60:
                if left == 0:
                    return self.__areas[10][0]
                else:
                    right = GaussApprox.__X_AREA_LAST_60

        # Result
        area = 0

        # For tracking precise area
        g_func = GaussFunction(50)

        # 1 -- Finding first scale segment of which fits in the range
        found = False
        scale = 10 # We have checked for this segment already
        left_edge = right
        right_edge = right
        if report:
            print("========================================================================")
            print(f"Stage 1 -- Searching for biggest scale segment of which fits the range:\n")
        while scale > 0:
            scale -= 1
            shamt = 53 + scale
            segment_width = GaussApprox.__SCALE_SEGMENT_WIDTH_60[scale]
            if report:
                print(f"\tScale {scale}. Width {fxd_to_str(segment_width,60)}:")
            next_segment_index = right >> shamt
            segment_index = next_segment_index - 1
            if next_segment_index == 0:
                if report:
                    print(f"\tRange is within the first segment. Continue.\n")
                continue
            segment_right = next_segment_index << shamt
            segment_left = segment_right - segment_width
            if report:
                print(f"\tChecking segment {segment_index}")
                print(f"\t\tLeft {fxd_to_str(segment_left,60)}")
                print(f"\t\tRight {fxd_to_str(segment_right, 60)}")
            if left >= segment_right:
                if report:
                    print(f"\tRange is not in the segment. Continue.\n")
                continue
            if segment_left >= left:
                # Counting segment of current scale towards the area
                segment_area = self.__areas[scale][segment_index]
                area += segment_area
                left_edge = segment_left
                right_edge = segment_right
                if report:
                    segment_area_precise = int(round((g_func.area(Rational(segment_left,2**60), Rational(segment_right, 2**60))*(2**63)).evalf(50)))
                    total_area_precise = int(round((g_func.area(Rational(left_edge,2**60), Rational(right_edge, 2**60))*(2**63)).evalf(50)))
                    print(f"\tSegment {segment_index} of scale {scale} fits in the range.")
                    print(f"\t\tSegment Area = {fxd_to_str(segment_area, 63)}")
                    print(f"\t\tSegment Precise = {fxd_to_str(segment_area_precise, 63)}")
                    print(f"\t\tTotal Area = {fxd_to_str(area,63)}")
                    print(f"\t\tTotal Area Precise = {fxd_to_str(total_area_precise,63)}")
                # Checking if additional segment of this scale fits the range
                # There cannot be more than two
                if segment_index > 0 and left <= segment_left - segment_width:
                    segment_area = self.__areas[scale][segment_index - 1]
                    area += segment_area
                    left_edge = left_edge - segment_width
                    if report:
                        segment_area_precise = int(round((g_func.area(Rational(segment_left - segment_width, 2 ** 60),Rational(segment_left, 2 ** 60)) * (2 ** 63)).evalf(50)))
                        total_area_precise = int(round((g_func.area(Rational(left_edge, 2 ** 60),Rational(right_edge, 2 ** 60)) * (2 ** 63)).evalf(50)))
                        print(f"\tSecond segment {segment_index - 1} of scale {scale} also fits in the range.")
                        print(f"\t\tSegment Area = {fxd_to_str(segment_area, 63)}")
                        print(f"\t\tSegment Precise = {fxd_to_str(segment_area_precise, 63)}")
                        print(f"\t\tTotal Area = {fxd_to_str(area,63)}")
                        print(f"\t\tTotal Area Precise = {fxd_to_str(total_area_precise, 63)}")
                if report:
                    print()
                break # THIS IS THE BREAK OF THE LOOP
            else:
                if report:
                    print(f"\tSegment does not fit. Continue.")
            if report:
                print()


        # 2 -- Now we iterate scales to find additional segments for left and right parts
        # right part is range [right_edge, right]
        # left part is range [left, left_edge]
        if report:
            print("==============================================")
            print("Stage 2 -- Searching for additional segments\n")
        while scale > 0:
            scale -= 1
            shamt = 53 + scale
            segment_width = GaussApprox.__SCALE_SEGMENT_WIDTH_60[scale]
            if report:
                print(f"\tScale {scale}. Width {fxd_to_str(segment_width, 60)}:")

            # RIGHT SIDE
            segment_right = right_edge + segment_width
            segment_index = right_edge >> shamt

            if report:
                print(f"\t\tChecking on the right:")
                print(f"\t\t\tSegment {segment_index}:")
                print(f"\t\t\t\tLeft {fxd_to_str(right_edge,60)}:")
                print(f"\t\t\t\tRight {fxd_to_str(segment_right,60)}:")
            if segment_right <= right:
                segment_area = self.__areas[scale][segment_index]
                area += segment_area
                right_edge = segment_right
                if report:
                    segment_area_precise = int(round((g_func.area(Rational(segment_width*segment_index, 2 ** 60),Rational(segment_width*(segment_index+1), 2 ** 60)) * (2 ** 63)).evalf(50)))
                    total_area_precise = int(round((g_func.area(Rational(left_edge, 2 ** 60), Rational(right_edge, 2 ** 60)) * (2 ** 63)).evalf(50)))
                    print(f"\t\tSegment {segment_index} of scale {scale} fits on the right.")
                    print(f"\t\t\tSegment Area = {fxd_to_str(segment_area, 63)}")
                    print(f"\t\tSegment Precise = {fxd_to_str(segment_area_precise, 63)}")
                    print(f"\t\t\tTotal Area = {fxd_to_str(area,63)}")
                    print(f"\t\tTotal Area Precise = {fxd_to_str(total_area_precise, 63)}")
            else:
                if report:
                    print(f"\t\tSegment does not fit.")
            # Left
            if report:
                print(f"\t\tChecking on the left:")
            next_segment_index = (left_edge >> shamt)
            if next_segment_index == 0:
                print(f"\t\t\tSegment does not fit between left edge and zero")
                print()
                continue
            segment_index = next_segment_index - 1
            segment_left = left_edge - segment_width
            if report:
                print(f"\t\t\tSegment {segment_index}:")
                print(f"\t\t\t\tLeft {fxd_to_str(segment_left,60)}:")
                print(f"\t\t\t\tRight {fxd_to_str(left_edge,60)}:")
            if segment_left >= left:
                segment_area = self.__areas[scale][segment_index]
                area += segment_area
                left_edge = segment_left
                if report:
                    segment_area_precise = int(round((g_func.area(Rational(segment_width * (segment_index), 2 ** 60),Rational(segment_width * (segment_index+1), 2 ** 60)) * (2 ** 63)).evalf(50)))
                    total_area_precise = int(round((g_func.area(Rational(left_edge, 2 ** 60), Rational(right_edge, 2 ** 60)) * (2 ** 63)).evalf(50)))
                    print(f"\t\tSegment {segment_index} of scale {scale} fits on the left.")
                    print(f"\tSegment Area = {fxd_to_str(segment_area, 63)}")
                    print(f"\t\tSegment Precise = {fxd_to_str(segment_area_precise, 63)}")
                    print(f"\tTotal Area = {fxd_to_str(area, 63)}")
                    print(f"\t\tTotal Area Precise = {fxd_to_str(total_area_precise, 63)}")
            else:
                if report:
                    print(f"\t\tSegment does not fit.")
            if report:
                print()



        # Stage 3 -- Now we find integrals for sub-scale left and right parts
        if report:
            print("==============================================")
            print("Stage 3 -- Taking integrals of sub-scale segments.\n")
        # Right
        right_subscale_area = self.subscale_integral_x60(right_edge, right)
        area += right_subscale_area
        if report:
            right_subscale_area_precise = int(round((g_func.area(Rational(right_edge,2**60), Rational(right,2**60))*(2**63)).evalf(50)))
            total_area_precise = int(round((g_func.area(Rational(left_edge, 2 ** 60), Rational(right, 2 ** 60)) * (2 ** 63)).evalf(50)))
            print("Adding integral area of the right sub-scale range.")
            print(f"\tLeft {fxd_to_str(right_edge,60)}")
            print(f"\tRight {fxd_to_str(right, 60)}")
            print(f"\tRight subscale area {fxd_to_str(right_subscale_area,63)}")
            print(f"\t\tRight subscale area precise {fxd_to_str(right_subscale_area_precise,63)}")
            print(f"\tTotal Area = {fxd_to_str(area, 63)}")
            print(f"\t\tTotal Area Precise = {fxd_to_str(total_area_precise, 63)}")
            print()
        # Left
        left_subscale_area = self.subscale_integral_x60(left, left_edge)
        area += left_subscale_area
        if report:
            left_subscale_area_precise = int(round((g_func.area(Rational(left, 2 ** 60), Rational(left_edge, 2 ** 60)) * (2 ** 63)).evalf(50)))
            total_area_precise = int(round((g_func.area(Rational(left, 2 ** 60), Rational(right, 2 ** 60)) * (2 ** 63)).evalf(50)))
            print("Adding integral area of the left sub-scale range.")
            print(f"\tLeft {fxd_to_str(left,60)}")
            print(f"\tRight {fxd_to_str(left_edge, 60)}")
            print(f"\tLeft subscale area {fxd_to_str(left_subscale_area,63)}")
            print(f"\t\tLeft subscale area precise {fxd_to_str(left_subscale_area_precise, 63)}")
            print(f"\tTotal Area = {fxd_to_str(area, 63)}")
            print(f"\t\tTotal Area Precise = {fxd_to_str(total_area_precise, 63)}")
            print()

        if report:
            print(f"Total area is {fxd_to_str(area,63)}.")

        return area




    def subscale_integral_x60(self, left : int, right : int, report=False) -> int:
        """ Returns 63 bit precision value. (3 last bits are padded zeros). """
        if left == right:
            return 0
        spline_index = left >> 53
        return (self.area_poly_value_x60(right, spline_index, report).result_area() - self.area_poly_value_x60(left, spline_index, report).result_area()) << 3




    def area_poly_value_x60(self, x: int, spline_index : int = None, report=False) -> CalculationStepValues:
        """
        Returns value of 60 bit precision.
        """
        if report:
            print(f"Calculating area approximating polynomial integral value for x60={fxd_to_str(x, 60)}")

        # Step values
        sv = CalculationStepValues()

        if x == 0 or x > GaussApprox.__X_AREA_LAST_60:
            if report:
                if x == 0:
                    print(f"\tx is zero. G(x) returns 0.")
                else:
                    print(f"\tx is out of range. G(x) returns 0.")
            sv.step_1 = 0
            sv.step_2 = 0
            sv.step_3 = 0
            sv.step_4 = 0
            sv.step_5 = 0
            sv.step_6 = 0
            sv.step_7 = 0
            sv.step_8 = 0
            sv.step_9 = 0
            sv.step_10 = 0
            sv.step_11 = 0
            return sv

        # Retrieving relevant spline
        if spline_index is None:
            spline_index = x >> 53
        cf = self.__asplines[spline_index]

        if report:
            print(f"\tRelevant spline is [{spline_index}]:")
            print(f"\t\tLeft = [{spline_index / GT_Definitions.area_splines_in_unit }]")
            print(f"\t\tRight = [{(spline_index + 1) / GT_Definitions.area_splines_in_unit }]")
            print(f"\t\tC1 = {fxd_to_str(cf.c1, 60)}")
            print(f"\t\tC2 = {fxd_to_str(cf.c2, 60)}")
            print(f"\t\tC3 = {fxd_to_str(cf.c3, 60)}")
            print(f"\t\tC4 = {fxd_to_str(cf.c4, 60)}")
            print(f"\t\tC5 = {fxd_to_str(cf.c5, 60)}")
            print(f"\t\tC6 = {fxd_to_str(cf.c6, 60)}")

        x_bot = x & GaussApprox.__MASK_32_BOT
        x_top = (x - x_bot) >> 32

        # STEP 1 -- c1*x
        c1_bot = cf.c1 & GaussApprox.__MASK_32_BOT
        c1_top = (cf.c1 - c1_bot) >> 32
        res = ((c1_top * x_top) << 4) + ((c1_top * x_bot) >> 28) + ((c1_bot * x_top) >> 28) + ((c1_bot * x_bot) >> 60)
        sv.step_1 = res
        if report:
            print(f"\tStep 1 -- [ c1*x ]:\t\t\t{fxd_to_str(sv.step_1, 60)}.")

        # STEP 2 -- c1x+c2
        res = res + cf.c2
        sv.step_2 = res
        if report:
            print(f"\tStep 2 -- [ c1*x+c2 ]:\t\t\t{fxd_to_str(sv.step_2, 60)}.")

        # STEP 3 -- (c1x+c2)x
        res_bot = res & GaussApprox.__MASK_32_BOT
        res_top = (res - res_bot) >> 32
        res = ((res_top * x_top) << 4) + ((res_top * x_bot) >> 28) + ((res_bot * x_top) >> 28) + (
                    (res_bot * x_bot) >> 60)
        sv.step_3 = res
        if report:
            print(f"\tStep 3 -- [ (c1*x+c2)*x ]:\t\t{fxd_to_str(sv.step_3, 60)}.")

        # STEP 4 -- (c1x+c2)x+c3
        res = res + cf.c3
        sv.step_4 = res
        if report:
            print(f"\tStep 4 -- [ (c1*x+c2)*x+c3 ]:\t\t{fxd_to_str(sv.step_4, 60)}.")

        # STEP 5 -- ((c1x+c2)x+c3)x
        res_bot = res & GaussApprox.__MASK_32_BOT
        res_top = (res - res_bot) >> 32
        res = ((res_top * x_top) << 4) + ((res_top * x_bot) >> 28) + ((res_bot * x_top) >> 28) + (
                    (res_bot * x_bot) >> 60)
        sv.step_5 = res
        if report:
            print(f"\tStep 5 -- [ ((c1*x+c2)*x+c3)*x ]:\t{fxd_to_str(sv.step_5, 60)}.")

        # STEP 6 -- ((c1x+c2)x+c3)x+c4
        res = res + cf.c4
        sv.step_6 = res
        if report:
            print(f"\tStep 6 -- [ ((c1*x+c2)*x+c3)*x+c4 ]:\t{fxd_to_str(sv.step_6, 60)}.")

        # STEP 7 -- (((c1x+c2)x+c3)x+c4)x
        res_bot = res & GaussApprox.__MASK_32_BOT
        res_top = (res - res_bot) >> 32
        res = (((res_top * x_top) << 4) +
               ((res_top * x_bot) >> 28) +
               ((res_bot * x_top) >> 28) +
               ((res_bot * x_bot) >> 60))
        sv.step_7 = res
        if report:
            print(f"\tStep 7 -- [ (((c1*x+c2)*x+c3)*x+c4)x ]:\t{fxd_to_str(sv.step_7, 60)}.")

        # STEP 8 -- (((c1x+c2)x+c3)x+c4)x+c5
        res = res + cf.c5
        sv.step_8 = res
        if report:
            print(f"\tStep 8 -- [ (((c1*x+c2)*x+c3)*x+c4)x+c5 ]:\t{fxd_to_str(sv.step_8, 60)}.")

        # STEP 9 -- ((((c1x+c2)x+c3)x+c4)x+c5)x
        res_bot = res & GaussApprox.__MASK_32_BOT
        res_top = (res - res_bot) >> 32
        res = (((res_top * x_top) << 4) +
               ((res_top * x_bot) >> 28) +
               ((res_bot * x_top) >> 28) +
               ((res_bot * x_bot) >> 60))
        sv.step_9 = res
        if report:
            print(f"\tStep 9 -- [ ((((c1*x+c2)*x+c3)*x+c4)x+c5)x ]:\t{fxd_to_str(sv.step_9, 60)}.")

        # STEP 10 -- ((((c1x+c2)x+c3)x+c4)x+c5)x+c6
        res = res + cf.c6
        sv.step_10 = res
        if report:
            print(f"\tStep 10 -- [ ((((c1*x+c2)*x+c3)*x+c4)x+c5)x+c6 ]:\t{fxd_to_str(sv.step_10, 60)}.")

        # STEP 11 -- ((((c1x+c2)x+c3)x+c4)x+c5)x
        res_bot = res & GaussApprox.__MASK_32_BOT
        res_top = (res - res_bot) >> 32
        res = (((res_top * x_top) << 4) +
               ((res_top * x_bot) >> 28) +
               ((res_bot * x_top) >> 28) +
               ((res_bot * x_bot) >> 60))
        sv.step_11 = res
        if report:
            print(f"\tStep 11 -- [ (((((c1*x+c2)*x+c3)*x+c4)x+c5)x+c6)x ]:\t{fxd_to_str(sv.step_11, 60)}.")


        if report:
            g_func = GaussFunction(50)
            spline = g_func.get_area_approximation(Rational(spline_index, GT_Definitions.area_splines_in_unit), Rational(spline_index + 1, GT_Definitions.area_splines_in_unit))
            res_precise = (spline.area_poly_value(Rational(x, 2**60))).evalf(50)
            res_precise_60 = int(round((res_precise*(2**60)).evalf(50)))
            precise_steps = spline.area_poly_step_values(Rational(x, 2**60))
            coefs_precise = spline.get_coefficients()
            print(f"\tPrecise coefficients (scaled):")
            print(f"\t\tc1 = {coefs_precise.c1/6}")
            print(f"\t\tc2 = {coefs_precise.c2/5}")
            print(f"\t\tc3 = {coefs_precise.c3/4}")
            print(f"\t\tc4 = {coefs_precise.c4/3}")
            print(f"\t\tc5 = {coefs_precise.c5/2}")
            print(f"\t\tc6 = {coefs_precise.c6}")
            print(f"\tPrecise value: \t{fxd_to_str(res_precise_60, 60)}.")
            print(f"\tPrecise value calculation steps:")
            print(f"\t\tStep 1 -- {precise_steps.step_1}")
            print(f"\t\tStep 2 -- {precise_steps.step_2}")
            print(f"\t\tStep 3 -- {precise_steps.step_3}")
            print(f"\t\tStep 4 -- {precise_steps.step_4}")
            print(f"\t\tStep 5 -- {precise_steps.step_5}")
            print(f"\t\tStep 6 -- {precise_steps.step_6}")
            print(f"\t\tStep 7 -- {precise_steps.step_7}")
            print(f"\t\tStep 8 -- {precise_steps.step_8}")
            print(f"\t\tStep 9 -- {precise_steps.step_9}")
            print(f"\t\tStep 10 -- {precise_steps.step_10}")
            print(f"\t\tStep 11 -- {precise_steps.step_11}")

        return sv







    # -----------
    # REPORTS
    # -----------

    def print_vsplines(self):
        for i in range(len(self.__vsplines)):
            cf = self.__vsplines[i]
            print(f"Spline {i}\t{cf.to_struct_initializer_str()}")

    def print_asplines(self):
        for i in range(len(self.__asplines)):
            cf = self.__asplines[i]
            print(f"Spline {i}\t{cf.to_struct_initializer_str()}")


    # -----------
    # INIT
    # -----------

    def __init__(self):
        # Loading pre-calculated values
        self.__vsplines = GaussTool.load_value_spline_coefficients()
        self.__asplines = GaussTool.load_area_spline_coefficients()
        self.__areas = GaussTool.load_areas()

