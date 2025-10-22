binary_64_legend = "        8         7         6         5         4         3         2         1"

def binary_to_str_64bit( x : int) -> str:
	str_64_lst = list("0000_0000 0000_0000 0000_0000 0000_0000 0000_0000 0000_0000 0000_0000 0000_0000")
	mask = 1

	for bt in range(0,8):
		# Bottom
		for i in range(2,6):
			if (x & mask) == 0:
				str_64_lst[(8 - bt) * 10 - i] = '0'
			else:
				str_64_lst[(8 - bt) * 10 - i] = '1'
			mask = mask << 1

		# Top
		for i in range(7,11):
			if (x&mask) == 0:
				str_64_lst[(8 - bt) * 10 - i] = '0'
			else:
				str_64_lst[(8 - bt) * 10 - i] = '1'
			mask = mask << 1

	return "".join(str_64_lst)


def fxd_to_str(fxd : int, precision : int):
    return f"{fxd} = {fxd >> precision}.[{fxd & (2 ** precision - 1)}] = {fxd / (2 ** precision)}"
