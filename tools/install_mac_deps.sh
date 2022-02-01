#! /bin/bash

# Install some custom requirements on OS X using brew
BREW=/usr/local/bin/brew
if [[ ! -x $BREW ]]; then
    /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
fi

$BREW update
$BREW install \
    autoconf \
    automake \
    bison \
    boost \
    ccache \
    cmake \
    git \
    libatomic_ops \
    libtool \
    openssl \
    pkg-config \
    protobuf \
    llvm \
    python \
    doxygen \
    gmp

# Prefer Homebrew's bison over the macOS-provided version
$BREW link --force bison
echo 'export PATH="/usr/local/opt/bison/bin:$PATH"' >> ~/.bash_profile
export PATH="/usr/local/opt/bison/bin:$PATH"

git clone --depth=1 -b v8.2.0 https://github.com/ivmai/bdwgc
cd bdwgc
./autogen.sh
./configure \
    --prefix=/usr/local \
    --enable-cplusplus
make
sudo make install

# install pip and required pip packages
# curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py
# python get-pip.py --user
pip3 install --user scapy==2.4.0 ply==3.8
