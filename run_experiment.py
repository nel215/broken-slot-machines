import os
import subprocess
import numpy as np
from tqdm import tqdm
from datetime import datetime
from logging import getLogger, basicConfig, INFO
from concurrent.futures import ProcessPoolExecutor
from skopt import gp_minimize
from skopt.space import Real
logger = getLogger(__name__)


def run_experiment(args):
    fpath, logdir, x = args
    fname = os.path.basename(fpath).split('.')[0]
    lpath = os.path.join(logdir, '{}.ltsv'.format(fname))
    with open(fpath) as inp_fp, open(lpath, 'w') as log_fp:
        args = ['./a.out']
        args += ['win-prior', str(x[0])]
        args += ['win-count', str(x[1])]
        args += ['min-exp', str(x[2])]
        args += ['min-var', str(x[3])]
        res = subprocess.run(
            args, stdin=inp_fp, stderr=log_fp)


def parse_log(args):
    fpath, logdir, _ = args
    fname = os.path.basename(fpath).split('.')[0]
    lpath = os.path.join(logdir, '{}.ltsv'.format(fname))

    res = {}
    with open(lpath) as fp:
        for row in fp:
            d = {}
            for chunk in row.strip().split('\t'):
                k, v = chunk.split(':')
                d[k] = v
            tag = d.get('tag')
            if tag == 'optimal':
                res['optimal'] = float(d['optimal'])
            if tag == 'result':
                res['result'] = float(d['coins'])

    regret = res['result'] / res['optimal']
    return regret


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
        Real(5.0, 15.0, name='win_prior'),
        Real(0.05, 3.0, name='win_count'),
        Real(0.5, 2.0, name='min_exp'),
        Real(0.01, 0.5, name='min_var'),
    ]

    x0 = [10, 0.1, 0.8, 0.2]
    res = gp_minimize(objective, space, x0=x0, n_calls=100, random_state=215)
    logger.info('best mean: {}, best x: {}'.format(-res.fun, res.x))


def main():
    basicConfig(level=INFO)
    optimize()


if __name__ == '__main__':
    main()
