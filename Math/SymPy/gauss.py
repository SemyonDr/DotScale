from sympy import exp, symbols, Symbol, Rational, Expr, Float, Eq, solve, diff, integrate

class AreaSpline:
	c1 : Float
	c2 : Float
	c3 : Float 
	c4 : Float 
	c5 : Float
	c6 : Float
	left : Rational
	right : Rational
	area_poly : Expr
	
	def area(self, left: Rational, right : Rational) -> Float:
		return integrate(self.area_poly, ('x', left, right)).evalf(50)
	
	
	
	def __init__(self, left : Rational, right : Rational, c1 : Float, c2 : Float, c3 : Float, c4 : Float, c5 : Float, c6 : Float):
		self.c1 = c1
		self.c2 = c2
		self.c3 = c3
		self.c4 = c4
		self.c5 = c5
		self.c6 = c6
		self.left = left
		self.right = right
		x,c1,c2,c3,c4,c5,c6 = symbols("x c1 c2 c3 c4 c5 c6")
		self.area_poly = self.c1*x**5+self.c2*x**4+self.c3*x**3+self.c4*x**2+self.c5*x+self.c6
		
	def __str__(self):
		return f"""Area spline:
	Left = {self.left}
	Right = {self.right}
	c1 = {self.c1}
	c2 = {self.c2}
	c3 = {self.c3}
	c4 = {self.c4}
	c5 = {self.c5}
	c6 = {self.c6}"""	
			

class GaussFunction:
	gauss : Expr
	aprx_area_poly : Expr
	
	
	
	
	# Function integral
	def area(self, left : Rational, right : Rational) -> Float:
		return integrate(self.gauss,('x',left, right)).evalf(50)
	
	
	
	
	# Get approximation
	def produce_area_aprx(self, left: Rational, right : Rational) -> AreaSpline:
		x = Symbol('x')
		c1,c2,c3,c4,c5,c6 = symbols("c1 c2 c3 c4 c5 c6")
		gx = self.gauss
		gx0 = self.gauss.subs(x, left).evalf(50)
		gx1 = self.gauss.subs(x, right).evalf(50)
		dgx = diff(gx, x)
		dgx0 = dgx.subs(x,left).evalf(50)
		dgx1 = dgx.subs(x, right).evalf(50)
		igx01 = integrate(gx, (x, left, right)).evalf(50)
		imgx01 = integrate(x*gx, (x, left, right)).evalf(50)
		px = self.aprx_area_poly
		px0 = px.subs(x, left)
		px1 = px.subs(x, right)
		dpx = diff(px, x)
		dpx0 = dpx.subs(x, left)
		dpx1 = dpx.subs(x, right)
		ipx01 = integrate(px, (x, left, right))
		impx01 = integrate(x*px, (x, left, right))
		equations = [
			Eq(px0,gx0),
			Eq(px1,gx1),
			Eq(dpx0, dgx0),
			Eq(dpx1, dgx1),
			Eq(ipx01, igx01),
			Eq(impx01, imgx01)
		]
		coefs = solve(equations, (c1,c2,c3,c4,c5,c6))
		
		aprx_spline = AreaSpline(left, right, coefs[c1], coefs[c2], coefs[c3], coefs[c4], coefs[c5], coefs[c6])
		
		print(aprx_spline)
		return aprx_spline
		
		
		
	
	# INIT
	def __init__(self):
		x = Symbol('x')
		c1,c2,c3,c4,c5,c6 = symbols("c1 c2 c3 c4 c5 c6")
		self.gauss = exp(-(x**2)/2)
		self.aprx_area_poly = c1*x**5+c2*x**4+c3*x**3+c4*x**2+c5*x+c6
		
		
		
		
		
# tests		
g = GaussFunction()
left_edge = Rational(120, 64)
right_edge = Rational(121, 64)
aprx = g.produce_area_aprx(left_edge, right_edge)
left = Rational(241,128)
right = Rational(242,128)
print(g.area(left,right))
print(aprx.area(left,right))
