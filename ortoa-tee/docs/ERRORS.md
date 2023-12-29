# ORTOA-TEE Encountered Errors & Resolutions

## Failure linking `hiredis` and `redis-plus-plus`

When linking with shared libraries, and running the application, you might get the following error message:

```bash
error while loading shared libraries: xxx: cannot open shared object file: No such file or directory.
```

That's because the linker cannot find the shared libraries. In order to solve the problem, you can add the path where you installed `hiredis` and `redis-plus-plus` libraries, to `LD_LIBRARY_PATH` environment variable. For example:

```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
```

Check [this StackOverflow question](https://stackoverflow.com/questions/480764/linux-error-while-loading-shared-libraries-cannot-open-shared-object-file-no-s) for details on how to solve the problem.

## Failure linking ORTOA-TEE built and installed library

The ORTOA-TEE project installs the software in the `${REPO_ROOT}/install/` directory rather than copying it to a directory on the `PATH`. As a result, developers may have to export the following variable when running the project. This can be added to a `bashrc` or similar file, or can be run in the shell before running the project.

```sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:<REPO_ROOT>/install/lib
```


Linking failure example:

```txt
error while loading shared libraries: liblibstorage.so: cannot open shared object file: No such file or directory
```

Cause: linked cannot find the shared libraries
