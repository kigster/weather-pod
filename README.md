
# WeatherPod

Please refer to the README for the template project, available here:
[arli-cmake](https://github.com/kigster/arli-cmake).

## Usage

Let's cd into the project folder:

```bash
cd WeatherPod
```

The directory structure should look as follows:

```
  WeatherPod
     |
     |__ bin/
     |   |___ setup
     |   |___ build
     |
     |__ cmake/
     |   |___ Arli.cmake
     |   |___ ArduinoToolchain.cmake          <———— provided by arduino-cmake project
     |   |___ Platform/                       <———— provided by arduino-cmake project
     |
     |__ src/
     |   |___ Arlifile
     |   |___ CMakeLists.txt
     |   |___ WeatherPod.cpp
     |
     |__ example/
         |___ Arlifile
         |___ CMakeLists.txt
         |___ Adafruit7SDisplay.cpp
```

You might need to run `bin/setup` first to ensure you have all the dependencies. 

Once you do that, you can build any of the source folders (i.e. either `src` or `example`) by
running `bin/build src` or `bin/build example`.

### Building Manually

The process to build and upload manually is super simple too:

```bash
cd src
rm -rf build && mkdir build && cd build
cmake ..
make 
make upload
```

