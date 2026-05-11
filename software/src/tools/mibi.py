import sys

# 8: 1/2
# 7: 1/4
# 6: 1/8
# 5: 1/16
# 4: 1/32
# 3: 1/64
# 2: 1/128
# 1: 1/256
# 0: 1/512

mibiFactor = 8
number = int(sys.argv[1])
wholePart = number >> mibiFactor
fracPart = 0

print(wholePart)

for i in range(0, mibiFactor):
    bit = 1 if number & (1 << i) else 0
    if bit:
        item = 1 / (2 ** (mibiFactor - i))
        fracPart += item
        print(f"{i}: {item}")

print(f"{wholePart + fracPart}")
