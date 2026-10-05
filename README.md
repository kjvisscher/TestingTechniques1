# Running
If you've installed and configured your container already, you can simply start it after stopping it. First, get all containers (running and stopped):
```bash
docker ps -a
```
You should get something like this:
```bash
CONTAINER ID IMAGE COMMAND CREATED STATUS PORTS NAMES
1fa59f56d69d ...
```
Copy the container id (`1fa59f56d69d`) and run:
```
docker start 1fa59f56d69d
```

# Installing and running
Our SUT is the [Synapse server](https://github.com/element-hq/synapse), which we will install and configure to run locally. The client can be accessed online via [app.element.io](https://app.element.io/) by entering localhost as the host environment.

We will run the server using Docker, but the installation will differ slightly between Windows and Linux.

Note that step 1 through 4 differ between Linux and Windows, after that they are the same.

## Linux
### 1) Install + enable docker
```sh
sudo apt update
sudo apt install docker.io docker-compose-v2
sudo systemctl enable --now docker
docker --version
docker compose version
```

### 2) Data directory
This is where the data for the server will be stored. Below code uses the home `~` directory, change if you'd rather have it stored somewhere else.
```sh
mkdir -p ~/synapse-local
cd ~/synapse-local
mkdir data
```
### 3) Config Synapse
```sh
docker run -it --rm \
  --mount type=bind,src="$PWD/data",dst=/data \
  -e SYNAPSE_SERVER_NAME=localhost \
  -e SYNAPSE_REPORT_STATS=no \
  matrixdotorg/synapse:latest generate
```

### 4) Run Synapse
```sh
docker run -d \
  --name synapse \
  --mount type=bind,src="$PWD/data",dst=/data \
  -p 8008:8008 \
  matrixdotorg/synapse:latest
```
Go to [localhost:8008](http://localhost:8008/). 

## Windows 

### 1) Install Docker
https://www.docker.com/products/docker-desktop/ 

### 2) Clone the synapse repositoy
https://github.com/element-hq/synapse

```sh
git clone https://github.com/element-hq/synapse
cd synapse
```

### 3) Config Synapse
```sh
docker run -it --rm `
  -v "${PWD}/data:/data" `
  -e SYNAPSE_SERVER_NAME=localhost `
  -e SYNAPSE_REPORT_STATS=no `
  synapse-dev generate
```

### 4) Run Synapse
```sh
docker run -d --name synapse `
  -v "${PWD}/data:/data" `
  -p 8008:8008 `
  synapse-dev
```
Go to [localhost:8008](http://localhost:8008/).

## Linux & Windows

### 5) Create admin user
```sh
docker exec -it synapse register_new_matrix_user \
  http://localhost:8008 \
  -c /data/homeserver.yaml
```
Enter your personal credentials
```sh
New user localpart [root]: dirk
Password: 
Confirm password: 
Make admin [no]: yes
```

### 6) Log in
1. Go to [app.element.io](https://app.element.io/)
2. Sign in
3. Edit homeserver
4. Other homeserver, enter `http://localhost:8008`
5. Log in with username/password

# Simple unit testing example with Check and CMake

This project is a complete C unit test with the Check library, using CMake as
a build tool.

## Status

[![BSD licensed](https://img.shields.io/github/license/vndmtrx/check-cmake-example.svg)](https://github.com/vndmtrx/check-cmake-example/blob/master/LICENSE)
[![Build Status](https://travis-ci.org/vndmtrx/check-cmake-example.svg?branch=master)](https://travis-ci.org/vndmtrx/check-cmake-example)
[![codecov](https://img.shields.io/codecov/c/github/vndmtrx/check-cmake-example.svg)](https://codecov.io/gh/vndmtrx/check-cmake-example)

## Installing

To run this project the following programs need to be installed on your system:
- Cmake
- Check (`libcheck-dev` on Debian/Ubuntu)
- Valgrind

run:
```
$ sudo apt update
$ sudo apt install build-essential cmake libcheck-dev valgrind lcov
```
> If you have trouble installing `libcheck-dev`, use `sudo apt install check` instead

Then, do as follows:

```
$ mkdir build && cd build
$ cmake ..
$ make
$ make test
```

Don't do `make install` unless you want to install the `sample` library.

To compile the Synapse room integration test directly, link it with Check and its dependencies:

```sh
$ cc tests/test_room.c -o test_room $(pkg-config --cflags --libs check)
```

`sample.c` and `sample.h` are built as a library. `src/main.c:main()` is a
client of `libsample.a`, just as `tests/test_sample.c:main()`.

After running `make test`, you will find the test files into `Testing` folder.

### Code Coverage Support

This example implements Code Coverage Reports using either using either `gcov` or `lcov`.
If you want to check them, you should run the following command after `make test`:

```
$ make gcov
$ make lcov
```

The coverage reports will be into `Coverage` folder. In the case of `lcov`, you
can see into the browser, opening the `index.html` file on the folder above.

### Valgrind Support

This example also implements a memory leak check with `valgrind`, thus allowing
a full test of the application. If you want to check them, you
should run the following command after `make`:

```
$ make test_sample_valgrind
```

It is important to note that, unlike code coverage generation, for each
`add_executable` in the build, you will need to create a specific target for the
valgrind with `add_custom_target`.

In the future, we plan to make a macro to simplify this task. At this moment, we
only provide the means to add valgrind into build script.

## Thanks

We have done our best to keep the code as simple as possible, both in the
building scripts and in the test scripts, in order to be very straightforward
and easy to understand.

Corrections and suggestions are welcome. Feel free to fork this project and
propose suggestions through issues and pull requests.
