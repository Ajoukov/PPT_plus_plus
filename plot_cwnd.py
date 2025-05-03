#!/usr/bin/env python3
import sys
import argparse
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as mticker

SMOOTHING_WINDOW = 3

LINE_STYLES = {
    'left':  {'linestyle': '-',  'color': 'black', 'label': 'HCP window'},
    'right': {'linestyle': '--', 'color': 'black', 'label': 'LCP window'},
    'sum':   {'linestyle': ':',  'color': 'black', 'label': 'HCP + LCP'},
}

def main(file_path, output_path, rank):
    df = pd.read_csv(file_path, sep=r'\s+', header=None,names=['timestamp','id','left','right','max'])

    df = df.sort_values(['id','timestamp'])
    df['right_diff'] = df.groupby('id')['right'].diff()
    df['dir'] = df['right_diff'].apply(lambda d: 1 if d > 0 else (-1 if d < 0 else 0))
    df['dir_nz'] = (df.groupby('id')['dir'].transform(lambda s: s.where(s != 0).ffill()).fillna(0))
    df['prev_dir_nz'] = df.groupby('id')['dir_nz'].shift(1)
    df['inc_to_dec'] = (df['prev_dir_nz'] == 1) & (df['dir_nz'] == -1)

    flips = df.groupby('id')['inc_to_dec'].sum()
    sorted_flips = flips.sort_values(ascending=False)

    if rank < 1 or rank > len(sorted_flips):
        print(f'Error: rank must be between 1 and {len(sorted_flips)}')
        sys.exit(1)

    target_id = sorted_flips.index[rank - 1]
    count = sorted_flips.iloc[rank - 1]
    print(f'Rank {rank}: ID {target_id} with {count} inc-dec flips')

    df_t = df[df['id'] == target_id].copy()
    df_t['left_smooth'] = df_t['left'].rolling(window=SMOOTHING_WINDOW, center=True, min_periods=1).mean()

    df_t['right_smooth'] = df_t['right'].rolling(window=SMOOTHING_WINDOW, center=True, min_periods=1).mean()

    df_t['sum_smooth'] = df_t['left_smooth'] + df_t['right_smooth']

    plt.plot(df_t['timestamp'], df_t['left_smooth'], linestyle=LINE_STYLES['left']['linestyle'],
        color=LINE_STYLES['left']['color'], label=LINE_STYLES['left']['label'])

    plt.plot(df_t['timestamp'], df_t['right_smooth'], linestyle=LINE_STYLES['right']['linestyle'],
        color=LINE_STYLES['right']['color'], label=LINE_STYLES['right']['label'])

    plt.plot(df_t['timestamp'], df_t['sum_smooth'], linestyle=LINE_STYLES['sum']['linestyle'],
        color=LINE_STYLES['sum']['color'], label=LINE_STYLES['sum']['label'])

    plt.xlabel('Timestamp')
    plt.ylabel('Window size')

    ax = plt.gca()
    ax.yaxis.set_major_formatter(mticker.FuncFormatter(lambda x, pos: f'{int(x/1000)}k'))

    plt.legend()
    plt.tight_layout()
    plt.savefig(output_path)
    print(f'Plot saved to {output_path}')

if __name__ == '__main__':
    p = argparse.ArgumentParser(description='Plot windows for the ID with the Nth-highest inc-dec flips.')
    p.add_argument('data_file',    help='input data file')
    p.add_argument('output_image', help='output PDF path (e.g. .pdf)')
    p.add_argument('-r', '--rank', type=int, default=1, help='which rank (1=most flips, 2=2nd, …)')
    args = p.parse_args()
    main(args.data_file, args.output_image, args.rank)

