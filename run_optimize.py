import os
import numpy as np
from tqdm import tqdm
from datetime import datetime
from concurrent.futures import ProcessPoolExecutor
from skopt import gp_minimize
from skopt.space import Real, Integer
from run_experiment import run_experiment, parse_log
from logging import getLogger, basicConfig, INFO
logger = getLogger(__name__)


def objective(x):
    logger.info('x: {}'.format(x))
    now = datetime.now()
    logdir = os.path.join('./log', now.strftime('%Y%m%d-%H%M%S'))
    os.makedirs(logdir, exist_ok=True)
    executor = ProcessPoolExecutor()
    inpdir = './testcase/'
    args_list = []
    for fname in sorted(os.listdir(inpdir)):
        if fname.find('.in') == -1:
            continue
        fpath = os.path.join(inpdir, fname)
        args_list.append([fpath, logdir, x])
    total = len(args_list)
    for res in tqdm(executor.map(run_experiment, args_list), total=total, ncols=0):
        pass

    regrets = []
    for res in tqdm(executor.map(parse_log, args_list), total=total, ncols=0):
        regrets.append(res)

    mean = np.mean(regrets)
    std = np.std(regrets)
    logger.info('mean: {}, std: {}'.format(mean, std))
    return -mean


def optimize():
    space = [
        Integer(100, 1000, name='note-coin'),
    ]

    x0 = [500]
    res = gp_minimize(objective, space, x0=x0, n_calls=11, random_state=215)
    logger.info('best mean: {}, best x: {}'.format(-res.fun, res.x))


def main():
    basicConfig(level=INFO)
    optimize()


if __name__ == '__main__':
    main()
