# Changelog

All notable changes of this repository with respect to the original code of Lera-Romero, Miranda-Bront and Soulignac (https://github.com/gleraromero/networks2020, commit `cabc56a`) are recorded here. The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [1.0.0]

First release of this repository. The solver is the original code with 2 repairs of 1 line each.

### Fixed

- **Partial dominance could remove a part of a label that no label dominates.** In `PWLDominationFunction::DominatePieces` (`code/src/labeling/pwl_domination_function.cpp`), when the dominated interval lies strictly inside one linear piece, the piece is split and its right part rebuilt. The right part was rebuilt with the maximum of the piece (`p1.image.right`) as its value at the right end, which is correct only on a non-decreasing piece. On a decreasing piece the rebuilt part was too high, a later comparison could erase it, and the exact labeling could then end without returning a route of negative reduced cost that exists. The line now evaluates the piece at its right end. The defect only removes labels, so the routes the solver returns stay feasible and exactly priced, while lower bounds and optimality claims can be wrong. The defect was found in KAYROS (https://github.com/0nyr/kayros), whose exact component derives from this code and which repaired it in its version 1.7.0. There, Dabia2013 RC107 with 50 customers closed at 10632.144948 on one pricing trajectory while the optimum is 10614.011180. As far as our records show, no wrong optimal value has been observed from this code itself.
- **The label-correction flag of `bp` was read without being initialized.** The constructor of `BidirectionalLabeling` did not initialize the member `correcting`, and `main_bp.cpp` does not assign it, so `bp` read an indeterminate value. The constructor now sets it to false, which is also the default of the `correcting` option of the `pricing` executable. Label correction is therefore off in `bp`.

### Added

- `code/tests/test_partial_dominance.cpp` and the CMake target `test_partial_dominance`, which the default build does not compile. The test fails with the previous line of `DominatePieces` and passes with the repaired one.
- `experiments/bp_smoke.json`, a short experiment on 4 instances with 25 customers and 2 configurations of `experiments/bp.json`.
- `AUTHORS`, `CHANGELOG.md` and `CITATION.cff`.

### Changed

- `runner/runner.py` converts the memory limit to an integer before it calls `resource.setrlimit`, which Python 3.13 and 3.14 require, and it creates the `output/` directory before the first save.
- `LICENSE` keeps the original MIT text and copyright line and adds a copyright line for the modifications.
- `README.md` is rewritten. It keeps the build and run instructions of the original README.

### Not repaired

The known limitations listed in `README.md` are unchanged from the original code. Among them, after a time or memory limit the reported lower bound can equal the upper bound, the time limit is not enforced in every loop, and in our environment some runs with 25 customers abort with the CPLEX error 1217 raised in `CPXgetpi`.

[1.0.0]: https://github.com/0nyr/lera-tdvrptw-bpc-repaired/releases/tag/v1.0.0
