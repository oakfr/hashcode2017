def reduce_list (b):
    a = b
    done = False
    while not done:
        done = True
        start = -1
        end = -1
        sum = 0
        for k in range(len(a)-1):
            if a[k] == a[k+1]:
                start = k
                end = k+1
                sum = a[k]+a[k+1]
                while end+1 < len(a) and a[end+1]==a[start]:
                    sum += a[end+1]
                    end += 1
                done = False
                break
        if not done:
            c = a[:start+1]
            c[-1] = sum
            if end+1<len(a):
                c += a[end+1:]
            a = c
    return a


if __name__ == "__main__":

    a = [1,1,0,0,0,2,0,1,3,3,3,0]
    b = reduce_list(a)
    print(b)