import os
import random
dist = "AABBBBCCCCCDDDDDDEEEEEEFFFFFFFGGGGGGGG"


def generate(seed):
    random.seed(seed)
    coins = random.randint(100, 10000)
    max_time = random.randint(100, 10000)
    note_time = random.randint(2, 10)
    num_machines = random.randint(3, 10)
    path = os.path.join('./testcase', '%05d.in' % (seed))
    with open(path, 'w') as fp:
        params = [coins, max_time, note_time, num_machines]
        params = list(map(str, [coins, max_time, note_time, num_machines]))
        fp.write(' '.join(params))
        fp.write('\n')
        for i in range(num_machines):
            size = random.randint(10, 30)
            for j in range(3):
                w = ""
                for k in range(size):
                    w += dist[random.randint(0, len(dist)-1)]
                fp.write(w)
                fp.write('\n')


def main():
    for seed in range(1000):
        generate(seed)


if __name__ == '__main__':
    main()
