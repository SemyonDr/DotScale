from sympy import (Expr, exp, symbols, Symbol, Rational, Float, integrate, diff,
                   simplify, expand, together, cancel, solve)
from SplineCoefs import VSplineCoefsFloat, ASplineCoefsFloat
from Statistics import FuncIntervalInfo, CalculationStepValues


class AreaSpline:
    # -----------
    # EXPRESSIONS
    # -----------

    __polynomial : Expr
    __indefinite_integral : Expr
    __integral : Expr

    # -----------
    # EXPRESSIONS
    # FOR CALC STEPS
    # -----------

    __v_step : list[Expr]

    # -----------
    # DATA
    # -----------

    __coefs : ASplineCoefsFloat
    __left : Rational
    __right : Rational
    __sigma : Rational


    # -----------
    # SETTINGS
    # -----------

    __precision : int

    # ----------------
    # ACCESSORS
    # ----------------

    def get_coefficients(self) -> ASplineCoefsFloat:
        return self.__coefs

    def get_left(self) -> Rational:
        return self.__left

    def get_right(self) -> Rational:
        return self.__right

    def get_interval(self) -> (Rational, Rational):
        return self.__left, self.__right

    def get_expression(self) -> Expr:
        return self.__indefinite_integral

    @staticmethod
    def get_num_steps_area_poly_value() -> int:
        return 11

    # ----------------
    # METHODS
    # ----------------

    def area(self, left : Rational, right : Rational) -> Float:
        return (self.__integral
                .subs({'x0' : left, 'x1' : right})
                .evalf(n = self.__precision))

    def area_poly_value(self, x : Rational) -> Float:
        return (self.__indefinite_integral
                .subs('x', x)
                .evalf(n = self.__precision))

    def area_poly_step_values(self, x : Rational) -> CalculationStepValues:
        steps_values = CalculationStepValues()
        steps_values.step_1 = self.__v_step[0].subs('x',x).evalf(self.__precision)
        steps_values.step_2 = self.__v_step[1].subs('x', x).evalf(self.__precision)
        steps_values.step_3 = self.__v_step[2].subs('x', x).evalf(self.__precision)
        steps_values.step_4 = self.__v_step[3].subs('x', x).evalf(self.__precision)
        steps_values.step_5 = self.__v_step[4].subs('x', x).evalf(self.__precision)
        steps_values.step_6 = self.__v_step[5].subs('x', x).evalf(self.__precision)
        steps_values.step_7 = self.__v_step[6].subs('x', x).evalf(self.__precision)
        steps_values.step_8 = self.__v_step[7].subs('x', x).evalf(self.__precision)
        steps_values.step_9 = self.__v_step[8].subs('x', x).evalf(self.__precision)
        steps_values.step_10 = self.__v_step[9].subs('x', x).evalf(self.__precision)
        steps_values.step_11 = self.__v_step[10].subs('x', x).evalf(self.__precision)
        return steps_values


    # ----------------
    # EVALUATION
    # METHODS
    # ----------------

    def area_poly_value_calc_info(self) -> list[FuncIntervalInfo]:
        calc_info = []
        for step_expr in self.__v_step:
            step_info = FuncIntervalInfo.new_polynomial_calc_info(step_expr, self.__left, self.__right)
            calc_info.append(step_info)
        return calc_info

    #----------------
    # ITERATOR
    #----------------

    def __iter__(self):
        return self

    def __next__(self):
        if self.__index < 6:
            return

    #----------------
    # CONSTRUCTOR
    #----------------

    def __init__(self, coefs : ASplineCoefsFloat,
                 left : Rational, right : Rational,
                 precision : int):
        # Setting up iterator
        self.__index = 0

        # Setting coefficient values
        self.__coefs = coefs

        if not isinstance(left, Rational):
            left = Rational(left)
        self.__left = left

        if not isinstance(right, Rational):
            right = Rational(right)
        self.__right = right

        self.__precision = precision

        # Symbols for expressions
        x = Symbol('x')
        x0 = Symbol('x0')
        x1 = Symbol('x1')
        c1 = coefs.c1
        c2 = coefs.c2
        c3 = coefs.c3
        c4 = coefs.c4
        c5 = coefs.c5
        c6 = coefs.c6

        # Setting expression for the approximation of the function value
        self.__polynomial = (
                  c1 * x ** 5
                + c2 * x ** 4
                + c3 * x ** 3
                + c4 * x ** 2
                + c5 * x ** 1
                + c6 * x
        )

        # Setting expression for the indefinite integral of approximating polynomial
        self.__indefinite_integral = (
                  (c1 * x ** 6) / 6
                + (c2 * x ** 5) / 5
                + (c3 * x ** 4) / 4
                + (c4 * x ** 3) / 3
                + (c5 * x ** 2) / 2
                + c6 * x
        )

        # Setting expression for integral over approximation function
        self.__integral = self.__indefinite_integral.subs(x, x1) - self.__indefinite_integral.subs(x, x0)

        # Setting expressions for steps of calculating the value
        # of indefinite integral
        self.__v_step = []
        self.__v_step.append((c1 * x)/6)
        self.__v_step.append((c1 * x)/6 + c2/5)
        self.__v_step.append(((c1 * x)/6 + c2/5)*x)
        self.__v_step.append(((c1 * x)/6 + c2/5)*x + c3/4)
        self.__v_step.append((((c1 * x)/6 + c2/5)*x + c3/4)*x)
        self.__v_step.append((((c1 * x)/6 + c2/5)*x + c3/4)*x + c4/3)
        self.__v_step.append(((((c1 * x)/6 + c2/5)*x + c3/4)*x + c4/3)*x)
        self.__v_step.append(((((c1 * x)/6 + c2/5)*x + c3/4)*x + c4/3)*x + c5/2)
        self.__v_step.append((((((c1 * x)/6 + c2/5)*x + c3/4)*x + c4/3)*x + c5/2) * x)
        self.__v_step.append((((((c1 * x)/6 + c2/5)*x + c3/4)*x + c4/3)*x + c5/2) * x + c6)
        self.__v_step.append(((((((c1 * x)/6 + c2/5)*x + c3/4)*x + c4/3)*x + c5/2) * x + c6) * x)
