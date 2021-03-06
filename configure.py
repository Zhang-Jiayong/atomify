#!/usr/bin/python

import subprocess
import os
import sys
import shutil
from os.path import join, abspath
from sys import platform as _platform
import glob
def run_command(cmd):
    print(cmd)
    subprocess.call(cmd, shell=True, env=env)

lammps_build_type = "atomify"
env = os.environ.copy()
currentPath = os.getcwd()
# Set up examples
run_command("git submodule update --init --recursive")
os.chdir(os.path.join("src","examples"))
run_command("python generateExamples.py")
os.chdir(currentPath)
# Package update
os.chdir(os.path.join("libs/lammps/src"))
run_command("make pu")
os.chdir(currentPath)
# Set up LAMMPS
ompSupport = True
specifiedCompiler = None

if _platform == "darwin":
    lammps_build_type = "atomify"
    ompSupport = False
    if len(sys.argv) > 1 and sys.argv[1] == "brew":
        ompSupport = True
        specifiedCompiler = sys.argv[2]
elif _platform == "win32":
    print("Windows is not supported yet")
    exit()
elif len(sys.argv) > 1 and sys.argv[1] == "android":
    lammps_build_type = "android"
    if len(sys.argv) < 4:
        print("You need to specify the Android NDK path and Android ABI version (for example \"4.9\")")
        print("python configure.py android <Android NDK path> <Android ABI version>")
        print("Example:")
        print("python configure.py android /home/username/apps/android-ndk-r10d 4.9")
        exit()
    env["ANDROID_NDK_PATH"] = sys.argv[2]
    env["ANDROID_ABI"] = sys.argv[3]
    
root_path = os.path.abspath(".")
patch_path = join(root_path, "lammps-patch")

if not os.path.exists("lammps-build"):
    os.makedirs("lammps-build")

lammps_source_dir = join("libs", "lammps")
os.chdir(root_path)

if os.path.exists("lammps-patch/water"):
    for filename in glob.glob(os.path.join("lammps-patch/water", '*.*')):
        shutil.copy(filename, "lammps-build/lammps/src/")

if not os.path.isdir(lammps_source_dir):
    print("Error, the path '"+lammps_source_dir+"' is not a directory.")
    exit()
lammps_source_dir_src_relative = join(lammps_source_dir, "src")
lammps_source_dir = abspath(lammps_source_dir)
lammps_source_dir_src = join(lammps_source_dir, "src")

if lammps_build_type == "android":
    lammps_android_pri = open("lammps-android.pri", "w")
    lammps_android_pri.write("INCLUDEPATH += " + lammps_source_dir_src + "\n")
    lammps_android_pri.write("INCLUDEPATH += " + lammps_source_dir_src + "/STUBS" + "\n")
    lammps_android_pri.write("LIBS += -L" + lammps_source_dir_src + " -llammps_android" + "\n")
    lammps_android_pri.write("LIBS += -L" + lammps_source_dir_src + "/STUBS -lmpi_stubs_android" + "\n")
    lammps_android_pri.close()
else:
    lammps_pri = open("lammps.pri", "w")
    lammps_pri.write("INCLUDEPATH += $$PWD/" + lammps_source_dir_src_relative + "\n")
    lammps_pri.write("INCLUDEPATH += $$PWD/" + lammps_source_dir_src_relative + "/STUBS" + "\n")
    lammps_pri.write("LIBS += -L$$PWD/" + lammps_source_dir_src_relative + " -llammps_atomify" + "\n")
    lammps_pri.write("LIBS += -L$$PWD/" + lammps_source_dir_src_relative + "/STUBS -lmpi_stubs" + "\n")
    lammps_pri.close()
    
shutil.copy(join(patch_path, "Makefile.atomify"), join(lammps_source_dir_src, "MAKE", "Makefile.atomify"))

if lammps_build_type == "android":
    print("Compiling MPI stubs for Android")
    shutil.copy(join(patch_path, "Makefile.android"), join(lammps_source_dir_src, "MAKE/MACHINES"))
    shutil.copy(join(patch_path, "STUBS", "Makefile.android"), join(lammps_source_dir_src, "STUBS"))

    os.chdir(join(lammps_source_dir_src, "STUBS"))
    run_command("make clean")
    run_command("make -f Makefile.android")
    os.chdir(root_path)
else:
    os.chdir(join(lammps_source_dir_src, "STUBS"))
    run_command("make")

        
print("\nCompiling LAMMPS")

os.chdir(lammps_source_dir_src)
run_command("make yes-rigid yes-class2 yes-manybody yes-mc yes-molecule yes-granular yes-replica yes-kspace yes-shock yes-misc yes-USER-MISC yes-user-reaxc yes-opt yes-qeq yes-snap yes-user-diffraction yes-user-fep yes-user-mgpt yes-user-manifold yes-user-meamc yes-user-phonon yes-user-smtbq")
if ompSupport:
    run_command("make yes-user-omp")

# Copy moltemplate extra files
for filename in glob.iglob(os.path.join(patch_path, "moltemplate_additional_lammps_code", "*.cpp")):
    print("Copying ", filename)
    shutil.copy2(filename, lammps_source_dir_src)
for filename in glob.iglob(os.path.join(patch_path, "moltemplate_additional_lammps_code", "*.h")):
    print("Copying ", filename)
    shutil.copy2(filename, lammps_source_dir_src)
    
if specifiedCompiler is not None:
    run_command("CC="+specifiedCompiler+" OMP=1 make -j4 " + lammps_build_type + " mode=lib")
else:
    if ompSupport:
        run_command("OMP=1 make -j4 " + lammps_build_type + " mode=lib")
    else:
        run_command("make -j4 " + lammps_build_type + " mode=lib")

os.chdir(root_path)
