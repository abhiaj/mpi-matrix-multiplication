# Matrix Multiplication in MPI

C implementation of Matrix Multiplication in MPI. This work was made for the "Sistemas Distribuídos" class on Computer Science course in Federal University of Tocantins.

## Getting Started

To use this code you first need to install MPI. I used [Open MPI](https://www.open-mpi.org/).

### Compiling

```
$ mpicc mm.c -o mm.out
```

### Executing

I recommend runing with at least 4 slots to see the work in action.

```
$ mpirun -np 4 mm.out
```

## Authors

* **Everton Barbosa Jr** - *Initial Work* - [ejkun](https://github.com/ejkun)

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details
