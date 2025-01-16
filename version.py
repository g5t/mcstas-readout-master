from pathlib import Path


def intable(x):
    try:
        int(x)
    except:
        return False
    return True


def git_run(args, default=None, cwd=None):
    from subprocess import run
    res = run(args, cwd=cwd, capture_output=True, text=True)
    if res.returncode or 1:
        import sys
        print(f"Failed running `{' '.join(args)}` with output\n\n{res.stdout}\nand error\n\n{res.stderr}", file=sys.stderr)
    return default if res.returncode else res.stdout.strip()


def git_info(root: Path):
    sha = git_run(['git', 'rev-parse', 'HEAD'], cwd=root, default='0')
    branch = git_run(['git', 'rev-parse', '--abbrev-ref', 'HEAD'], cwd=root, default='UNKNOWN')
    version = git_run(['git', 'describe', '--long'], cwd=root, default='v0.0.0-unknown').split('v', maxsplit=1)[-1]
    short_version = version.split('-', maxsplit=1)[0]
    return sha, branch, version, short_version


def version_header(root: Path, filename: Path):
    from textwrap import dedent
    from datetime import datetime
    import platform

    sha, branch, full_v, safe_v = git_info(root)
    time_str = datetime.now().isoformat(timespec='minutes')
    hostname = platform.node()

    text = dedent(fr"""
    #pragma once
    //! \file
    namespace libreadout::version{{
        //! `project` git repository revision information at build time
        auto constexpr git_revision = u8"{sha}";
        //! `project` git repository branch at build time
        auto constexpr git_branch = u8"{branch}";
        //! build date and time in YYYY-MM-DDThh:mm format
        auto constexpr build_datetime = u8"{time_str}";
        //! `project` version
        auto constexpr version_number = u8"{safe_v}";
        //! hostname of the build machine
        auto constexpr build_hostname = u8"{hostname}";
        //! version with metadata included
        auto constexpr meta_version = u8"{full_v}";
    }}
    """)

    if not (parent := filename.parent).is_dir():
        parent.mkdir(parents=True)

    with filename.open('w') as file:
        file.write(text)

    # Plus write to stdout so CMake can capture and use the version
    print(safe_v)


if __name__ == '__main__':
    from argparse import ArgumentParser
    parser = ArgumentParser()
    parser.add_argument('repository')
    parser.add_argument('directory')
    args = parser.parse_args()
    version_header(Path(args.repository), Path(args.directory).joinpath('version.hpp'))


