# import argparse
from tkinter import Tk
from tkinter.filedialog import askopenfilenames


# def cmd_show(args):
#     filenames = args.filenames
#     if len(filenames) == 0:
        


def main():
    # parser = argparse.ArgumentParser()
    # subparsers = parser.add_subparsers()

    # show_parser = subparsers.add_parser('show', help='Shows graphs from saved files')
    # show_parser.add_argument('filenames', nargs='*', type=str, default=(), help='Filenames to load graphs')
    # show_parser.add_argument('--saved-with', nargs='?', type=str, default=(), help='Path to file with custom attributes')
    # show_parser.set_defaults(callback=cmd_show)

    # args = parser.parse_args()
    # args.callback(args)
    Tk().withdraw()
    filenames = askopenfilenames(filetypes=[
        ('All', '.*'),
        ('Text', '.txt'),
        ('CSV', '.csv'),
    ])

    timestamps  = []
    for filename in filenames:
        with open(filename) as f:
            timestamps.append([int(number) for number in f])

    print(timestamps)


if __name__ == '__main__':
    main()
