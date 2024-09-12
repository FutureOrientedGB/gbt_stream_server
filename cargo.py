import os
import platform
import subprocess
import sys


if __name__ == '__main__':
    sys.path.insert(1, 'deps')
    import build
    triplet = build.default_triplet()

    if platform.system() == 'Windows':
        exe_ext = '.exe'
        path_separator = ';'
        python3 = 'python'
    else:
        exe_ext = ''
        path_separator = ':'
        python3 = 'python3'

    pkg_config = os.path.abspath(f'deps/build/{triplet}/vcpkg_installed/{triplet}/tools/pkgconf/pkgconf{exe_ext}')
    pkg_config_path = os.path.abspath(f'deps/build/{triplet}/vcpkg_installed/{triplet}/lib/pkgconfig')

    if not os.path.exists(pkg_config) or not os.path.isfile(pkg_config) or not os.path.exists(pkg_config_path) or not os.path.isdir(pkg_config_path):
        cwd = os.getcwd()
        os.chdir('deps')
        subprocess.run(args=[python3, 'build.py', '--vcpkg-bootstrap=True', '--cmake-generate=True', '--cmake-build=True'])
        os.chdir(cwd)

    env = os.environ.copy()
    env['PKG_CONFIG'] = pkg_config
    env['PKG_CONFIG_PATH'] = pkg_config_path
    
    subprocess.run(args=['cargo'] + sys.argv[1:], env=env)

