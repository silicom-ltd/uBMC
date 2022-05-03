# uBMC Docker Build
## Getting the code
Simply clone the repo by 
```
git clone https://github.com/silicom-ltd/uBMC.git
```

## Create Docker Image
```
cd uBMC
docker build -t silicom/ubmc-build docker
```

## Enter uBMC build container
```
docker run -v ${PWD}:/srv/ubmc --rm -it silicom/ubmc-build bash
```

## Enter uBMC build container
```
docker run -v ${PWD}:/srv/ubmc --rm -it silicom/ubmc-build bash
```

## In build container
```
root@e52fb553c314:/srv/ubmc# . ./path_toolchain.sh
root@e52fb553c314:/srv/ubmc# make
```

