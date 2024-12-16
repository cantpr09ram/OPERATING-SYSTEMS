```bash
docker build -t linux-kernel-dev .
```

```bash
docker run -it --name kernel-dev --privileged linux-kernel-dev
```

```bash
git clone --depth 1 https://github.com/torvalds/linux.git
```

```bash
make defconfig
```

```bash
make -j$(nproc)
```œ`