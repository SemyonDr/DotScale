
from sympy import Expr, exp, symbols, Symbol, Rational, Float, integrate, diff, simplify, expand, together, cancel


class HermiteSpline:
    # -----------
    # EXPRESSIONS
    # -----------

    __polynomial : Expr
    __integral : Expr

    # -----------
    # DATA
    # -----------

    __a : Float
    __b : Float
    __c : Float
    __d : Float
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

    def get_a(self) -> Float:
        return self.__a

    def get_b(self) -> Float:
        return self.__b

    def get_c(self) -> Float:
        return self.__c

    def get_d(self) -> Float:
        return self.__d

    def get_coefficients(self) -> (Float, Float, Float, Float):
        return self.__a, self.__b, self.__c, self.__d

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
        return (self.__polynomial
                .subs('x', x)
                .evalf(n = self.__precision))

    def area(self, left : Rational, right : Rational) -> Float:
        return (self.__integral
                .subs({'x0' : left, 'x1' : right})
                .evalf(n = self.__precision))


    #----------------
    # CONSTRUCTOR
    #----------------

    def __init__(self, a : Float, b : Float, c : Float, d : Float,
                 left : Rational, right : Rational,
                 precision : int):
        # Setting coefficient values
        self.__a = a
        self.__b = b
        self.__c = c
        self.__d = d

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

        # Setting expression for the value
        self.__polynomial = (
                  a * x ** 3
                + b * x ** 2
                + c * x
                + d
        )

        # Setting expression for the area
        indefinite_integral = (
                  (a * x**4) / 4
                + (b * x**3) / 3
                + (c * x**2) / 2
                + (d * x) # Const is omitted
        )
        self.__integral = indefinite_integral.subs(x, x1) - indefinite_integral.subs(x, x0)