# cscript
#### by [Jason A. Donenfeld](mailto:Jason@zx2c4.com)

`cscript` executes C code from stdin using any variety of compiler arguments. It can be used from the command line:

```
$ echo 'main(){puts("hello world");}' | cscript
```

Or it can be used at the top of scripts:

```
 #!/usr/bin/cscript
 main(){puts("hello world");}
```

One might even register .c as an executable file type:

```
# echo ':cfile:E::c::/usr/bin/cscript:' > /proc/sys/fs/binfmt_misc/register
$ echo 'main(){puts("hello world");}' > a.c
$ chmod +x a.c
$ ./a.c
```

It respects the `CC` environment variable, and does not create any temporary dentries that need to be cleaned up ever.

### Building

```
$ make
$ sudo make install PREFIX=/usr
```

### License

This project is released under the [GPLv2](COPYING).
