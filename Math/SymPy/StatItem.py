from sympy import Float, sqrt

# Helper class to hold statistics for one item
class StatItem:

    def update(self, value):
        self.__values.append(value)
        if value > self.__max:
            self.__max = value

    def update_list(self, values: []):
        for v in values:
            self.update(v)

    def get_max(self) -> Float:
        return Float(self.__max)

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
        return f'Mean=[{self.get_mean().evalf(n=3)}]\t Max=[{self.get_max().evalf(n=3)}]\t StDev=[{self.get_st_dev().evalf(n=3)}]'

    def __init__(self):
        self.__max = 0
        self.__values = []