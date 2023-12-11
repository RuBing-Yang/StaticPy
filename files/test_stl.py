def func1() -> None:
    i: int = 1 + 1 * 1 - 1
    if i == 1:
        print("i is ", i)

def func2(l: List[int]) -> List[int]:
    l[1] = 5
    return l

def main() -> int:
    func1()

    l: List[int] = [1, 2, 3]
    l2: List[int] = func2(l)
    i: int = 0
    while i < 3:
        print("List[int] [", i, "] = ", l2[i])
        i = i + 1

    d: Dict[int, int] = {1:11, 2:22}
    i = 1
    while i < 10:
        print("Dict[int, int] [", i, "] = ", d[i])
        i = i + 1
        if i >= 3:
            break

    ll: List[List[float]] = [[1.1, 2.2], [3.3, 4.4, 5.5]]
    i = 0
    while i < 2:
        j:int = 0
        while j < 5:
            if (i == 0 and j >= 2) or (i == 1 and j >= 3):
                j = j + 1
                continue
            print("List[List[float]] [", i, ", ", j, "] = ", ll[i][j] + i * 100 + j * 10)
            j = j + 1
        i = i + 1
    return 0