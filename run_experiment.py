import os
import subprocess
from datetime import datetime
from logging import getLogger, basicConfig, INFO
from concurrent.futures import ProcessPoolExecutor
logger = getLogger(__name__)


def run_experiment(args):
    fpath, logdir = args
    fname = os.path.basename(fpath).split('.')[0]
    lpath = os.path.join(logdir, '{}.ltsv'.format(fname))
    with open(fpath) as inp_fp, open(lpath, 'w') as log_fp:
        args = ['./a.out']
        args += ['win-prior', '10']
        args += ['win-count', '0.1']
        args += ['min-exp', '0.8']
        args += ['min-var', '0.2']
        res = subprocess.run(
            args, stdin=inp_fp, stderr=log_fp)
        logger.info('{}:{}'.format(fpath, res.returncode))


def main():
    basicConfig(level=INFO)
    now = datetime.now()
    logdir = os.path.join('./log', now.strftime('%Y%m%d-%H%M%S'))
    os.makedirs(logdir, exist_ok=True)
    executor = ProcessPoolExecutor()
    inpdir = './testcase/'
    args_list = []
    for fname in os.listdir(inpdir):
        if fname.find('.in') == -1:
            continue
        fpath = os.path.join(inpdir, fname)
        args_list.append([fpath, logdir])
    for res in executor.map(run_experiment, args_list):
        pass


if __name__ == '__main__':
    main()
