
import scipy as sci
import sympy as sym
from sympy import Expr, exp, symbols, Symbol, Rational, Float, integrate, diff, simplify, expand, together, cancel, solve
from SplineCoefs import VSplineCoefsFloat
from Statistics import FuncIntervalInfo


class ValueSpline:
    # -----------
    # EXPRESSIONS
    # -----------

    __polynomial : Expr
    __integral : Expr
    __indefinite_integral : Expr

    # -----------
    # EXPRESSIONS
    # FOR CALC STEPS
    # -----------

    __v_step : list[Expr]
    __a_step : list[Expr]

    # -----------
    # DATA
    # -----------

    __coefs : VSplineCoefsFloat
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

    def get_coefficients(self) -> VSplineCoefsFloat:
        return self.__coefs

    def get_left(self) -> Rational:
        return self.__left

    def get_right(self) -> Rational:
        return self.__right

    def get_interval(self) -> (Rational, Rational):
        return self.__left, self.__right

    def get_expression(self) -> Expr:
        return self.__polynomial


    # ----------------
    # METHODS
    # ----------------

    def value(self, x : Rational) -> Float:
        """
        Calculates value approximation for gauss function.
        """
        return (self.__polynomial
                .subs('x', x)
                .evalf(n = self.__precision))

    # ----------------
    # EVALUATION
    # METHODS
    # ----------------

    def value_calc_info(self) -> list[FuncIntervalInfo]:
        """
        Produces information about each step of calculations
        for value approximation.
        """
        calc_info = []
        for step_expr in self.__v_step:
            step_info = FuncIntervalInfo.new_polynomial_calc_info(step_expr, self.__left, self.__right)
            calc_info.append(step_info)
        return calc_info

    def area_calc_info(self) -> list[FuncIntervalInfo]:
        """
        Produces information about each step of calculations
        for indefinite integral value approximation.
        """
        calc_info = []
        for step_expr in self.__a_step:
            step_info = FuncIntervalInfo.new_polynomial_calc_info(step_expr, self.__left, self.__right)
            calc_info.append(step_info)
        return calc_info

    #----------------
    # CONSTRUCTOR
    #----------------

    def __init__(self, coefs : VSplineCoefsFloat,
                 left : Rational, right : Rational,
                 precision : int):
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
        a = coefs.a
        b = coefs.b
        c = coefs.c
        d = coefs.d

        # Setting expression for the value
        self.__polynomial = (
                  a * x ** 3
                + b * x ** 2
                + c * x
                + d
        )

        # Setting expression for the area
        self.__indefinite_integral = (
                  (a * x**4) / 4
                + (b * x**3) / 3
                + (c * x**2) / 2
                + (d * x) # Const is omitted
        )
        self.__integral = self.__indefinite_integral.subs(x, x1) - self.__indefinite_integral.subs(x, x0)

        # Setting expressions for steps of calculating the value
        self.__v_step = []
        self.__v_step.append(a * x)
        self.__v_step.append(a * x + b)
        self.__v_step.append(a * (x**2) + b*x)
        self.__v_step.append(a * (x ** 2) + b*x + c)
        self.__v_step.append(a * (x ** 3) + b * (x**2) + c*x)
        self.__v_step.append(a * (x ** 3) + b * (x ** 2) + c*x + d)

        # Setting expression for steps of calculating a value of
        # indefinite integral of this spline
        self.__a_step = []
        self.__a_step.append(a * x / 4)
        self.__a_step.append(a * x / 4 + b / 3)
        self.__a_step.append((a * x / 4 + b / 3) * x)
        self.__a_step.append((a * x / 4 + b / 3) * x + c / 2)
        self.__a_step.append(((a * x / 4 + b / 3) * x + c / 2)*x)
        self.__a_step.append(((a * x / 4 + b / 3) * x + c / 2) * x + d)
        self.__a_step.append((((a * x / 4 + b / 3) * x + c / 2) * x + d)*x)

