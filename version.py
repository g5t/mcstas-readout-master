from texttools import dedent
from setuptools_scm import get_version

def git_info():
    import git
    repo = git.Repo(search_parent_directories=True)
    info = {
            'sha': repo.head.objec.hexsha,
            'branch': repo.active_branch.name,
            }
    return info

def version_header():
    v = get_version()
    gi = git_info()

    text = dedent(f"""#pragma once
    //! \file
    namespace libreadout::version{
        //! `project` git repository revision information at build time
        auto constexpr git_revision = u8"{gi['sha']}";
        //! `project` git repository branch at build time
        auto constexpr git_branch = u8"{gi['branch']}";
        //! build date and time in YYYY-MM-DDThh:mm format
        auto constexpr build_datetime = u8"@GIT_CONFIGURE_TIME@";
        //! `project` version
        auto constexpr version_number = u8"@GIT_SAFE_VERSION@";
        //! hostname of the build machine
        auto constexpr build_hostname = u8"@GIT_HOSTNAME@";
        //! version with metadata included
        auto constexpr meta_version = u8"@GIT_VERSION@";
    }
    """
