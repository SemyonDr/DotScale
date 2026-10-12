"""
Here we pre-calculate some values for a division algorithm.
See Blurer::InitializeDistanceWeights() for details.
"""

N = 11528573512042642983
ncand = [
    (1 << (31-i))*N >> 32
    for i in range(32)
]



for v in ncand:
    print(f"0x{v:016X},")