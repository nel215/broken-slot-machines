import os
import subprocess
import numpy as np
from tqdm import tqdm
from concurrent.futures import ProcessPoolExecutor
from datetime import datetime
from logging import getLogger, basicConfig, INFO
logger = getLogger(__name__)


def run_experiment(args):
    fpath, logdir, x = args
    fname = os.path.basename(fpath).split('.')[0]
    lpath = os.path.join(logdir, '{}.ltsv'.format(fname))
    args = ['./a.out']
    if x is not None:
        args += ['note-coin', str(x[0])]
    with open(fpath) as inp_fp, open(lpath, 'w') as log_fp:
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


def run_experiments():
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
        args_list.append([fpath, logdir, None])
    # args_list = args_list[:100]
    total = len(args_list)
    for res in tqdm(executor.map(run_experiment, args_list), total=total, ncols=0):
        pass

    regrets = []
    for res in tqdm(executor.map(parse_log, args_list), total=total, ncols=0):
        regrets.append(res)

    mean = np.mean(regrets)
    std = np.std(regrets)
    logger.info('mean: {}, std: {}'.format(mean, std))


def main():
    basicConfig(level=INFO)
    run_experiments()


if __name__ == '__main__':
    main()
