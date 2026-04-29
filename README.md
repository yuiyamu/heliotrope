# heliotrope

a highly specialized zlib wrapper for light zip extraction/creation

## info

unlike other mature zip libraries like zlib's own [minizip](https://github.com/madler/zlib/tree/develop/contrib/minizip) or [miniz](https://github.com/richgel999/miniz), heliotrope is quite immature and only really made for one purpose - being a *very* lightweight `.osz` file manager. that means no >4gb files, nothing other than deflate, just simple and well behaved `.zip` files. this makes it quite unstable for production purposes that deal with general `.zip` files - but perfect when know what kinds of `.zip` files you're dealing with and don't need all the additional features that other zip libraries come with. it is not thoroughly tested, so please let me know if you run into any issues :D i'll be happy to fix them~

## compilation/dependencies

as expected, heliotrope depends on zlib for parsing and creating [deflate](https://en.wikipedia.org/wiki/Deflate) compressed streams. your computer already likely has it as it is such an essential library, but make sure zlib already exists before compiling~ should be as simple as running `make` in the root directory, and then `./heliotrope [zip file]` to extract any file into a local folder. you can also see all other options (like `--verbose` and `--create`) by doing `./heliotrope -h`.

if you'd like to install the heliotrope cli globally, copy it to your `/usr/local/bin` (or `/usr/bin` if you feel spicy) and make sure that it's in your PATH (`echo $PATH` to double check~)
