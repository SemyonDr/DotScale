from argparse import ArgumentTypeError

from HermiteSpline import HermiteSpline
from sympy import Expr, exp, symbols, Symbol, Rational, Float, integrate, diff, simplify, expand, together, cancel


class GaussFunction:
    """
    Returns values for gauss function with given sigma.
    """

    # -----------
    # SETTINGS
    # -----------

    __precision : int
    __sigma : Rational

    # -----------
    # EXPRESSIONS
    # -----------

    # Gauss Function
    # g(x) = e^(-(x/s)^2 * 1/2)
    __gauss = exp(-(((Symbol('x')/Symbol('sigma'))**2)/2))
    # Derivative
    # g'(x) = -x * e^(-(x/s)^2 * 1/2)
    __gauss_drv = diff(exp(-(((Symbol('x')/Symbol('sigma'))**2)/2)), 'x')

    # Expressions for 3rd degree polynomial coefficients
    # for spline approximation
    # ax^3 + bx^2 + cx + d = P(x)
    # Set when first object of this class is created
    __a : Expr
    __b : Expr
    __c : Expr
    __d : Expr
    __is_coef_initialized = False

    # -----------
    # ACCESSORS
    # -----------

    def get_precision(self) -> int:
        return self.__precision

    def get_sigma(self) -> Rational:
        return self.__sigma

    def get_expression(self) -> Expr:
        return self.__gauss

    # -----------
    # METHODS
    # -----------

    def value(self, x : Rational) -> Float:
        """
        Returns function value at given point.
        """
        if not isinstance(x, Rational):
            x = Rational(x)

        return (
            self.__gauss
                .subs('x', x)
                .evalf(n = self.__precision)
        )

    def area(self, left : Rational, right : Rational) -> Float:
        """
        Returns function integral between left and right points.
        """
        if not isinstance(left, Rational):
            left = Rational(left)
        if not isinstance(right, Rational):
            right = Rational(right)

        integral = integrate(self.__gauss, ('x', left, right))
        return integral.evalf(n=self.__precision)


    def get_approximation(self, left : Rational, right : Rational) -> HermiteSpline:
        lr_subs_map = {'sigma' : self.__sigma, 'x0' : left, 'x1' : right }

        a_val = GaussFunction.__a.subs(lr_subs_map).evalf(n = self.__precision)
        b_val = GaussFunction.__b.subs(lr_subs_map).evalf(n=self.__precision)
        c_val = GaussFunction.__c.subs(lr_subs_map).evalf(n=self.__precision)
        d_val = GaussFunction.__d.subs(lr_subs_map).evalf(n=self.__precision)

        return HermiteSpline(a_val, b_val, c_val, d_val, left, right, self.__precision)


    #----------------
    # CONSTRUCTOR
    #----------------

    def __init__(self, precision : int, sigma : Rational = Rational(1,1)):
        if not isinstance(sigma, Rational):
            sigma = Rational(sigma)

        # Saving precision value and sigma
        self.__sigma = sigma
        self.__precision = precision

        self.__gauss = GaussFunction.__gauss.subs('sigma', sigma)
        self.__gauss_drv = GaussFunction.__gauss_drv.subs('sigma', sigma)

        # Initializing approximation
        # coefficients if not yet set
        if GaussFunction.__is_coef_initialized is False:
            GaussFunction.__is_coef_initialized = True
            GaussFunction.__init_approx_coef()
            # Alternative init
            #GaussFunction.__init_approx_coef_reference()


    # ----------------
    # COEF EXPRESSIONS
    # ----------------

    @classmethod
    def __init_approx_coef(cls):
        """
        Sets up expressions for producing approximation
        polynomial ax^3 + bx^2 + cx + d.
        This method sets coefficients with expanded formulas.
        """

        x0 = Symbol('x0')
        x1 = Symbol('x1')
        delta = x1 - x0

        g_x0 = cls.__gauss.subs('x', x0)
        g_x1 = cls.__gauss.subs('x', x1)
        gd_x0 = cls.__gauss_drv.subs('x', x0)
        gd_x1 = cls.__gauss_drv.subs('x', x1)

        # a =     [g`(x1) + g`(x0)] / d^2
        #     - 2 [g(x1)  - g(x0)]  / d^3
        cls.__a = (
                (gd_x1 + gd_x0) / (delta ** 2)
                - 2 * (g_x1 - g_x0) / (delta ** 3)
        )

        # b =   3    [g(x1)  -  g(x0)]  / d^2
        #     -      [g`(x1) + 2g`(x0)] / d
        #     - 3 x0 [g`(x1) +  g`(x0)] / d^2
        #     + 6 x0 [g(x1)  -  g(x0)]  / d^3
        cls.__b = (
                3 * (g_x1 - g_x0) / (delta ** 2)
                - (gd_x1 + 2 * gd_x0) / delta
                - 3 * x0 * (gd_x1 + gd_x0) / (delta ** 2)
                + 6 * x0 * (g_x1 - g_x0) / (delta ** 3)
        )

        # c = g`(x0)
        #     + 3 x0^2 [g`(x1) +  g`(x0)] / d^2
        #     - 6 x0^2 [g(x1)  -  g(x0)]  / d^3
        #     - 6 x0   [g(x1)  -  g(x0)]  / d^2
        #     + 2 x0   [g`(x1) + 2g`(x0)] / d
        cls.__c = (
                gd_x0
                + 3 * x0 ** 2 * (gd_x1 + gd_x0) / (delta ** 2)
                - 6 * x0 ** 2 * (g_x1 - g_x0) / (delta ** 3)
                - 6 * x0 * (g_x1 - g_x0) / (delta ** 2)
                + 2 * x0 * (gd_x1 + 2 * gd_x0) / delta
        )

        # d =          g(x0)
        #     -   x0   g`(x0)
        #     -   x0^3 [g`(x1) +  g`(x0)] / d^2
        #     + 2 x0^3 [g(x1)  -  g(x0)]  / d^3
        #     + 3 x0^2 [g(x1)  -  g(x0)]  / d^2
        #     -   x0^2 [g`(x1) + 2g`(x0)] / d
        cls.__d = (
                g_x0
                - x0 * gd_x0
                - x0 ** 3 * (gd_x1 + gd_x0) / (delta ** 2)
                + 2 * x0 ** 3 * (g_x1 - g_x0) / (delta ** 3)
                + 3 * x0 ** 2 * (g_x1 - g_x0) / (delta ** 2)
                - x0 ** 2 * (gd_x1 + 2 * gd_x0) / delta
        )




    @classmethod
    def __init_approx_coef_reference(cls):
        """
        Sets up expressions for producing approximation
        polynomial ax^3 + bx^2 + cx + d.
        This method sets coefficients with self-referential formulas.
        """

        x0 = Symbol('x0')
        x1 = Symbol('x1')
        delta = x1 - x0

        g_x0 = cls.__gauss.subs('x', x0)
        g_x1 = cls.__gauss.subs('x', x1)
        gd_x0 = cls.__gauss_drv.subs('x', x0)
        gd_x1 = cls.__gauss_drv.subs('x', x1)

        # a =     [g`(x1) + g`(x0)] / d^2
        #     - 2 [g(x1)  - g(x0)]  / d^3
        cls.__a = (
                  (gd_x1 + gd_x0) / (delta ** 2)
            - 2 * ( g_x1 -  g_x0) / (delta ** 3)
        )

        # b =   [g(x1) - g(x0)] / d^2
        #     - g`(x0) / d
        #     - (x1 + 2x0) * a
        cls.__b = (
              (g_x1 - g_x0) / (delta ** 2)
            -  gd_x0 / delta
            - (x1 + 2 * x0) * cls.__a
        )

        # c =   g`(x0) - 3a x0^2 - 2b x0
        cls.__c = (
                  gd_x0
            - 3 * g_x0 ** 2 * cls.__a
            - 2 * g_x0      * cls.__b
        )

        # d = g(x0) - a x0^3 - b x0^2 - c x0
        cls.__d = (
                  g_x0
                - g_x0 ** 3 * cls.__a
                - g_x0 ** 2 * cls.__b
                - g_x0 * cls.__c
        )







