from pathlib import Path


def intable(x):
    try:
        int(x)
    except:
        return False
    return True


def semver_groups(s):
    import re
    r = re.compile(r"^(?P<major>0|[1-9]\d*)\.(?P<minor>0|[1-9]\d*)\.(?P<patch>0|[1-9]\d*)(?:-(?P<prerelease>(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\.(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?(?:\+(?P<buildmetadata>[0-9a-zA-Z-]+(?:\.[0-9a-zA-Z-]+)*))?$")
    if m := r.match(s):
        d = m.groupdict()
        return d
    return None


def git_run(args, default=None, cwd=None):
    from subprocess import run
    res = run(args, cwd=cwd, capture_output=True, text=True)
    return default if res.returncode else res.stdout.strip()


def get_version(root: Path):
    file = Path(__file__).parent.joinpath('VERSION')
    fallback = file.read_text()
    version = git_run(['git', 'describe', '--long'], cwd=root, default=fallback).split('v', maxsplit=1)[-1]
    d = semver_groups(version)
    if not d:
        # warn about a non-semantic-version value?
        d = semver_groups(fallback)
        version = fallback
    elif version != fallback:
        file.write_text(version)
    return version, f"{d['major']}.{d['minor']}.{d['patch']}"


def git_info(root: Path):
    sha = git_run(['git', 'rev-parse', 'HEAD'], cwd=root, default='0')
    branch = git_run(['git', 'rev-parse', '--abbrev-ref', 'HEAD'], cwd=root, default='UNKNOWN')
    return sha, branch


def version_header(root: Path, filename: Path):
    from textwrap import dedent
    from datetime import datetime
    import platform

    sha, branch= git_info(root)
    full_v, safe_v = get_version(root)
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


def main():
    from argparse import ArgumentParser
    parser = ArgumentParser()
    parser.add_argument('repository')
    parser.add_argument('directory')
    args = parser.parse_args()
    version_header(Path(args.repository), Path(args.directory).joinpath('version.hpp'))


if __name__ == '__main__':
    main()