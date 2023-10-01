# weboptsrv
## Backend for [optim8](https://optim8.org/)

### Running the server locally
Clone the Repository
```bash
git clone https://github.com/rainerzufalldererste/weboptsrv.git
cd weboptsrv
```

On Windows: Launch Wsl2 `bash` / `ubuntu` / etc.

Ensure you have the necessary packages installed (In order to use different compiler versions, those obviously need to exist, llvm-17 may need to be installed via their special install script)
For Ubuntu / Debian based distos
```bash
sudo apt-get install zstd zlib1g zlib1g-dev llvm-17-dev gcc-13 gcc-12 gcc-11 clang-17 clang-16 clang-15 clang-14 build-essential
```

```bash
premake/premake5 gmake2; config=releaseclang_x64 make
```

Now launch the server.
```bash
builds/bin/weboptsrv
```
