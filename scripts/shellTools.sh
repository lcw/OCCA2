#!/bin/bash

#---[ Library Information ]-------------

function uniqueAddToPath {
    local path=$1
    local dir=$2

    if [ ! -z $path ]; then
        case ":$path:" in
            *":$dir:"*)         ;; # Already in the path
            *) path="$path:$dir";;
        esac
    else
        path="$dir"
    fi

    command echo $path
}

function removeDuplicatesInPath {
    local path=$1

    for dir_ in ${path//:/ }; do
        if ls $dir_ > /dev/null 2>&1; then
            path=$(uniqueAddToPath $path $dir_)
        fi
    done

    command echo $path
}

function getIncludePath {
    local path=$1

    path=$(command echo "$path:" | command sed 's/\/lib[^:]*:/\/include:/g')

    path=$(removeDuplicatesInPath $path)

    command echo $path
}

function dirWithFileInPath {
    local path=$1
    local filename=$2

    if [ ! -z $path ]; then
        for dir_ in ${path//:/ }; do
            if ls $dir_/$filename > /dev/null 2>&1; then
                command echo $dir_
                return
            fi
        done
    fi

    command echo ""
}

function dirWithFileInIncludePath {
    local path=$(getIncludePath $1)
    local filename=$2

    if [ ! -z $path ]; then
        for dir_ in ${path//:/ }; do
            if ls $dir_/$filename > /dev/null 2>&1; then
                command echo $dir_
                return
            fi
        done
    fi

    command echo ""
}

function dirWithLibrary {
    local libName="lib$1.so"
    local result=""

    local mergedLibPaths=""

    mergedLibPaths=$mergedLibPaths:"/usr/local/cuda*/lib*"
    mergedLibPaths=$mergedLibPaths:$OCCA_LIBRARY_PATH
    mergedLibPaths=$mergedLibPaths:$LD_LIBRARY_PATH
    mergedLibPaths=$mergedLibPaths:$DYLD_LIBRARY_PATH
    mergedLibPaths=$mergedLibPaths:"/lib:/usr/lib:/usr/lib32:/usr/lib64:"
    mergedLibPaths=$mergedLibPaths:"/usr/lib/*-gnu/"

    result=$(dirWithFileInPath "$mergedLibPaths" $libName)

    if [ ! -z $result ]; then command echo $result; return; fi

    if hash ldconfig 2> /dev/null; then
        command echo $(command ldconfig -p | command grep -m 1 $libName | command sed 's/.*=>\(.*\/\).*/\1/g')
        return
    fi

    case "$(uname)" in
        Darwin)
            if ls /System/Library/Frameworks/$1.framework > /dev/null 2>&1; then
                command echo "Is A System/Library Framework"
                return
            fi
            if ls /Library/Frameworks/$1.framework > /dev/null 2>&1; then
                command echo "Is A Library Framework"
                return
            fi;;
    esac

    command echo ""
}

function dirWithHeader {
    local filename="$1"
    local result=""

    local mergedPaths=""
    local mergedLibPaths=""

    mergedPaths=$mergedPaths:"/usr/local/cuda*/include"
    mergedPaths=$mergedPaths:"/Developer/NVIDIA/CUDA*/include"
    mergedPaths=$mergedPaths:$OCCA_INCLUDE_PATH
    mergedPaths=$mergedPaths:$CPLUS_INCLUDE_PATH
    mergedPaths=$mergedPaths:$C_INCLUDE_PATH
    mergedPaths=$mergedPaths:$INCLUDEPATH
    mergedPaths=$mergedPaths:"/usr/include"

    mergedLibPaths=$mergedLibPaths:"/usr/local/cuda*/lib*"
    mergedLibPaths=$mergedLibPaths:$OCCA_LIBRARY_PATH
    mergedLibPaths=$mergedLibPaths:$LD_LIBRARY_PATH
    mergedLibPaths=$mergedLibPaths:$DYLD_LIBRARY_PATH
    mergedLibPaths=$mergedLibPaths:"/lib:/usr/lib:/usr/lib32:/usr/lib64:"
    mergedLibPaths=$mergedLibPaths:"/usr/lib/*-gnu/"

    result=$(dirWithFileInPath "$mergedPaths" $filename)
    if [ ! -z $result ]; then command echo $result; return; fi

    result=$(dirWithFileInIncludePath "$mergedLibPaths" $filename)

    if [ ! -z $result ]; then command echo $result; return; fi

    command echo ""
}

function dirsWithHeaders {
    local headers=$1
    local path=""

    if [ ! -z $headers ]; then
        for header in ${headers//:/ }; do
            local inc=$(dirWithHeader $header)

            if [ ! -z $inc ]; then
                path=$(uniqueAddToPath $path $inc)
            else
                command echo ""
                return
            fi
        done
    fi

    command echo $path
}

function libraryFlags {
    local libName=$1

    local libDir=$(dirWithLibrary $libName)
    local flags=""
    local isAFramework=0

    if [ -z "$libDir" ]; then command echo ""; return; fi

    if [ "$libDir" == "Is A System/Library Framework" ]; then
        flags="-framework $libName"
        isAFramework=1
    elif [ "$libDir" == "Is A Library Framework" ]; then
        flags="-F/Library/Frameworks -framework $libName"
        isAFramework=1
    else
        flags="-L$libDir -l$libName"
    fi

    command echo $flags
}


function headerFlags {
    local headers=$1

    local incDirs
    local flags=""

    if [ ! -z $headers ]; then
        incDirs=$(dirsWithHeaders $headers)

        if [ -z $incDirs ]; then command echo ""; return; fi

        incDirs=${incDirs%?}        # Remove the last :
        flags="-I${incDirs//:/ -I}" # : -> -I
    fi

    command echo $flags
}
#=======================================


#---[ Compiler Information ]------------
function getPath {
    command echo ${1%/*}
}

function stripPath {
    command echo ${1##*/}
}

function resolveRelativePath {
    local from=$1
    local to=$2

    if [[ $to == /* ]]; then
        echo $to
    else
        echo $(getPath $from)/$to
    fi
}

function manualWhich {
    local input=$1

    local typeOutput=$(command type $input)

    if [[ $typeOutput == *" is hashed "* ]]; then
        local mWhich=$(command type $input | sed "s/.*(\(.*\)).*/\1/g")
    else
        local mWhich=$(command type $input | sed "s/.* is \(.*\)/\1/g")
    fi

    if [ ! -z "$mWhich" ]; then
        echo $mWhich
    else
        echo $input
    fi
}

function realCommand {
    local a=$(manualWhich $1)
    local b

    case "$(uname)" in
        Darwin) b="$(command readlink    $a)";;
        *)      b="$(command readlink -f $a)";;
    esac

    if [ -z $b ]; then
        command echo $a
        return
    fi

    while [ "$a" != "$b" ]; do
        b=$(resolveRelativePath $a $b)
        a=$(manualWhich $b)

        case "$(uname)" in
            Darwin) b="$(command readlink    $a)";;
            *)      b="$(command readlink -f $a)";;
        esac

        if [ -z $b ]; then
            command echo $a
            return
        fi
    done

    command echo "$a"
}

function unaliasCommand {
    typeOutput=$(command type $1 2> /dev/null)

    aliasedTo=$(command echo $typeOutput | command grep -m 1 "$1 is aliased to" | command sed "s/[^\`]*\`\([^ \t']*\)[ \t']/\1/g")

    if [ ! -z $aliasedTo ]; then
        command echo $aliasedTo
        return
    fi

    command echo $1
}

function mpiCompilerVendor {
    local mpiCompiler=$1
    local compiler

    # gcc, clang
    compiler=$($mpiCompiler --chichamanga 2>&1 > /dev/null | command grep -m 1 error | command sed 's/\([^:]*\):.*/\1/g')

    if [ ! -z $compiler ]; then command echo $compiler; return; fi

    # intel
    compiler=$($mpiCompiler --chichamanga 2>&1 > /dev/null | command grep -m 1 "command not found" | command sed 's/[^:]*:[^:]*:[ \t]*\([^:]*\):.*/\1/g')

    if [ ! -z $compiler ]; then command echo $compiler; return; fi

    command echo ""
}

function compilerName {
    local chosenCompiler=$1
    local realCompiler=$(realCommand $chosenCompiler)
    local unaliasedCompiler=$(unaliasCommand $realCompiler)
    local strippedCompiler=$(stripPath $unaliasedCompiler)
    local compiler

    case $strippedCompiler in
        mpi*) compiler=$(mpiCompilerVendor $strippedCompiler) ;;
        *)    compiler=$strippedCompiler                      ;;
    esac

    command echo $compiler
}

function compilerVendor {
    local chosenCompiler=$1
    local compiler=$(compilerName $1)

    case $compiler in
        # C/C++ Compilers
        g++* | gcc*)       command echo GCC          ;;
        clang*)            command echo LLVM         ;;
        icc* | icpc*)      command echo INTEL        ;;
        xlc*)              command echo IBM          ;;
        pgcc* | pgc++*)    command echo PGI          ;;
        pathcc* | pathCC*) command echo PATHSCALE    ;;
        aCC*)              command echo HP           ;; # [-] Will fail with cc (Cray's compiler name ...)
        cc* | CC*)         command echo CRAY         ;;
        cl*.exe*)          command echo VISUALSTUDIO ;;

        # Fortran Compilers
        gfortran*)         command echo GCC          ;;
        ifort*)            command echo INTEL        ;;
        ftn*)              command echo CRAY         ;;
        xlf*)              command echo IBM          ;;
        pgfortran*)        command echo PGI          ;;
        pathf9*)           command echo PATHSCALE    ;;

        *)                 command echo N/A          ;;
    esac
}

function compilerReleaseFlags {
    local vendor=$(compilerVendor $1)

    case $vendor in
        GCC | LLVM) command echo "-O3 -D __extern_always_inline=inline"     ;;
        INTEL)      command echo "-O3 -xHost"                               ;;
        CRAY)       command echo "-O3 -h intrinsics -fast"                  ;;
        IBM)        command echo "-O3 -qhot=simd"                           ;;
        PGI)        command echo "-O3 -fast -Mipa=fast,inline -Msmartalloc" ;;
        PATHSCALE)  command echo "-O3 -march=auto"                          ;;
        HP)         command echo "+O3"                                      ;;
        *)          command echo ""                                         ;;
    esac
}

function compilerDebugFlags {
    local vendor=$(compilerVendor $1)

    case $vendor in
        N/A)                   ;;
        *)   command echo "-g" ;;
    esac
}

function compilerPicFlag {
    local vendor=$(compilerVendor $1)

    case $vendor in
        GCC | LLVM | INTEL | PATHSCALE | CRAY | IBM | PGI) command echo "-fPIC";;
        HP) command echo "+z";;
        *)  command echo ""  ;;
    esac
}

function compilerSharedFlag {
    local vendor=$(compilerVendor $1)

    case $vendor in
        GCC | LLVM | INTEL | PATHSCALE | CRAY | IBM | PGI) command echo "-shared";;
        HP)                                                command echo "-b"     ;;
        *)                                                 command echo ""       ;;
    esac
}

function compilerOpenMPFlags {
    local vendor=$(compilerVendor $1)

    case $vendor in
        GCC   | LLVM)      command echo "-fopenmp" ;;
        INTEL | PATHSCALE) command echo "-openmp"  ;;
        CRAY)              command echo ""         ;;
        IBM)               command echo "-qsmp"    ;;
        PGI)               command echo "-mp"      ;;
        HP)                command echo "+Oopenmp" ;;
        *)                 command echo ""         ;;
    esac
}

function fCompilerModuleDirFlag {
    local vendor=$(compilerVendor $1)

    case $vendor in
        GCC   | CRAY)            command echo "-J"       ;;
        INTEL | PGI | PATHSCALE) command echo "-module"  ;;
        IBM)                     command echo "-qmoddir" ;;
        *)                       command echo ""         ;;
    esac
}

function compilerSupportsOpenMP {
    local compiler=$1
    local vendor=$(compilerVendor $compiler)
    local ompFlag=$(compilerOpenMPFlags $compiler)

    local filename=$OCCA_DIR/scripts/ompTest.cpp
    local binary=$OCCA_DIR/scripts/ompTest

    # Test compilation
    $compiler $ompFlag $filename -o $binary > /dev/null 2>&1

    if [[ ! -a $binary ]]; then
        command echo 0
        return
    fi

    if [[ $? -eq 0 ]]; then
        # Test binary
        $binary

        if [[ $? -eq 0 ]]; then
            command echo 1
        else
            command echo 0
        fi
    else
        command echo 0
    fi

    if [ ! -z $binary ]; then
        rm -f $binary
    fi
}
#=======================================


#---[ System Information ]--------------
function getFieldFrom {
    local command_="$1"
    local field="$2"

    if hash grep 2> /dev/null; then
        command echo $(LC_ALL=C; $command_ | command grep -m 1 "^$field" | sed "s/.*:[ \t]*\(.*\)/\1/g")
        return
    fi

    echo ""
}

function getLSCPUField {
    local field="$1"

    if hash lscpu 2> /dev/null; then
        getFieldFrom "command lscpu" "$field"
        return
    fi

    echo ""
}

function getCPUINFOField {
    local field="$1"

    if hash cat 2> /dev/null; then
        getFieldFrom "command cat /proc/cpuinfo" "$field"
        return
    fi

    echo ""
}
#=======================================