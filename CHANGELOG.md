# Changelog

All notable changes of this repository with respect to the original code of Lera-Romero, Miranda-Bront and Soulignac (https://github.com/gleraromero/networks2020, commit `cabc56a`) are recorded here. The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [1.0.1] - 2026-11-12

Documentation and citation metadata only. The solver code is unchanged, and results are those of 1.0.0.

### Changed

- **Citation.** `CITATION.cff` no longer declares the article as the preferred citation. It describes this repository, with its 4 authors, and lists the original repository and the article as references. The README asks users to cite the 3 items.
- **Authors.** `AUTHORS` and the README give one list of 4 authors, the 3 authors of the original code and Florian Rascoussier, with what each wrote.

## [1.0.0] - 2026-11-12

First release of this repository. The solver is the original code with 4 changes of 1 line each: 2 repairs of the labeling algorithm, a guard on the lower bound reported after a limit, and CPLEX set to 1 thread.

### Fixed

- **Partial dominance could remove a part of a label that no label dominates.** In `PWLDominationFunction::DominatePieces` (`code/src/labeling/pwl_domination_function.cpp`), when the dominated interval lies strictly inside one linear piece, the piece is split and its right part rebuilt. The right part was rebuilt with the maximum of the piece (`p1.image.right`) as its value at the right end, which is correct only on a non-decreasing piece. On a decreasing piece the rebuilt part was too high, a later comparison could erase it, and the exact labeling could then end without returning a route of negative reduced cost that exists. The line now evaluates the piece at its right end. The defect only removes labels, so the routes the solver returns stay feasible and exactly priced, while lower bounds and optimality claims can be wrong. The defect was found in KAYROS (https://github.com/0nyr/kayros), whose exact component derives from this code and which repaired it in its version 1.7.0. There, Dabia2013 RC107 with 50 customers closed at 10632.144948 with one pricing configuration while the optimum is 10614.011180. As far as our records show, no wrong optimal value has been observed from this code itself.
- **The label-correction flag of `bp` was read without being initialized.** The constructor of `BidirectionalLabeling` did not initialize the member `correcting`, and `main_bp.cpp` does not assign it, so `bp` read an indeterminate value. The constructor now sets it to false, which is also the default of the `correcting` option of the `pricing` executable. Label correction is therefore off in `bp`.
- **The lower bound was set to the upper bound when a limit had emptied the node queue.** In `BCP::Run` (`code/src/bcp/bcp.cpp`), the line `if (q.empty()) z_lb = z_ub;` now applies only when no time or memory limit was hit. A limit discards the node it interrupts, so an empty queue is then not a proof. Over 30 runs of BP-PART on 6 instances with 25 customers, with time limits chosen to fall during branching, a build without this change reported a lower bound equal to its upper bound in 18 runs, above the known optimal value in 11 of them, and this version in none. A run that ends with the status Optimum is not affected.

### Changed

- **CPLEX runs on 1 thread.** `apply_configuration` (`code/goc/src/linear_programming/cplex/cplex_solver.cpp`) sets `CPX_PARAM_THREADS` to 1. With the default number of threads, on a machine with 20 hardware threads and CPLEX 12.10.0.0, 11 of the 112 runs of the original code on the instances with 25 customers aborted with the CPLEX error 1217 raised in `CPXgetpi` (BP-PART and BP-CUTS, 20 s). In the abort that we traced, `CPXlpopt` returned without an error and without a solution, and the code reads the dual values without checking. With 1 thread none of the 112 runs aborted. The other CPLEX parameters are unchanged.
- `runner/runner.py` converts the memory limit to an integer before it calls `resource.setrlimit`, which Python 3.13 and 3.14 require, and it creates the `output/` directory before the first save.
- `LICENSE` keeps the original MIT text and copyright line and adds a copyright line for the modifications.
- `README.md` is rewritten. It keeps the build and run instructions of the original README.

### Added

- `code/tests/test_partial_dominance.cpp` and the CMake target `test_partial_dominance`, which the default build does not compile. The test fails with the previous line of `DominatePieces` and passes with the repaired one.
- `experiments/bp_smoke.json`, a short experiment on 4 instances with 25 customers, and `experiments/bp_n25.json`, the 56 instances with 25 customers with a time limit of 20 s. Both use 2 configurations of `experiments/bp.json`.
- `AUTHORS`, `CHANGELOG.md` and `CITATION.cff`.

### Not repaired

The known limitations listed in `README.md` are unchanged from the original code. Among them, the time limit is not enforced in every loop, the dual values are read without checking that the LP has a solution, and the label-correction step itself is not repaired.

[1.0.1]: https://github.com/0nyr/lera-tdvrptw-bpc-repaired/releases/tag/v1.0.1
[1.0.0]: https://github.com/0nyr/lera-tdvrptw-bpc-repaired/releases/tag/v1.0.0
