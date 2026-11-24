"""Entry point for the sequential MagicToonEngine build pipeline."""

import argparse

# ========================================================  environment_stage
from environment_stage import add_arguments as add_environment_arguments
from environment_stage import run_environment_stage
# ========================================================

def build_parser():
    parser = argparse.ArgumentParser(
        prog="main.py",
        description="MagicToonEngine build pipeline",
    )
    add_environment_arguments(parser)
    return parser


def main(argv=None):
    options = build_parser().parse_args(argv)

    # Pipeline stages run in order; a failing stage stops the pipeline.
    return run_environment_stage(options)


if __name__ == "__main__":
    raise SystemExit(main())
