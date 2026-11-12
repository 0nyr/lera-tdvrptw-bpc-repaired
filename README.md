# Branch-price-and-cut for the TDVRPTW of Lera-Romero, Miranda-Bront and Soulignac, with repairs

This repository holds the Branch-Price-and-Cut (BPC) solver for the Time-Dependent Vehicle Routing Problem with Time Windows (TDVRPTW) of Gonzalo Lera-Romero, Juan José Miranda-Bront and Francisco Soulignac, described in the article [Linear edge costs and labeling algorithms: The case of the time-dependent vehicle routing problem with time windows](https://doi.org/10.1002/net.21937) (Networks 76(1):24-53, 2020), with 4 changes of 1 line each to the solver. 2 of them repair the labeling algorithm, 1 guards the lower bound reported after a limit, and 1 sets CPLEX to 1 thread. The original code is the public repository https://github.com/gleraromero/networks2020 (MIT license), whose head commit `cabc56a` is the base of this repository and stays in its history unchanged.

We found the repairs while working on KAYROS (https://github.com/0nyr/kayros), a solver whose exact component derives from this code. This repository is not affiliated with, and not endorsed by, the authors of the original code. If you use it, please cite this repository, the original repository and their article (see [Citation](#citation)).

The original README states that its code "is not the exact version executed for the article" and was "improved for readibility". The same holds here, and we do not claim that this repository reproduces the times of the article.

## Changes to the solver

The 4 changes are on the branch `main`. `git diff cabc56a..main -- code/src code/goc code/include` shows exactly these 4 lines.

1. **Partial dominance on a decreasing piece** (`code/src/labeling/pwl_domination_function.cpp`, `PWLDominationFunction::DominatePieces`, case D). When the interval on which a stored label dominates a candidate lies strictly inside one linear piece of the candidate's duration function, the piece is split and its right part rebuilt. The original line rebuilt the right part with `p1.image.right`, the maximum of the piece, as its value at the right end. This is correct on a non-decreasing piece only. On a decreasing piece, the common case where a later arrival means less waiting, the rebuilt part rose where it should have kept falling, so the label was overestimated on that part and a later comparison could erase it although no label dominates it. The exact labeling could then end without returning a route of negative reduced cost that exists, so a node bound could exceed the optimum and the search could close on a suboptimal value. The defect only removes labels, which means that every route the solver returns is still feasible and exactly priced. Lower bounds and optimality claims are what it can make wrong. The repaired line evaluates the piece at its right end, `p1.Value(p1.domain.right)`.
2. **Uninitialized label-correction flag** (`code/src/labeling/bidirectional_labeling.cpp`). The constructor of `BidirectionalLabeling` did not initialize the member `correcting`, and `code/src/main_bp.cpp` does not assign it before the labeling copies it, so the `bp` executable read an indeterminate value, which is undefined behavior. The constructor now sets it to false. Label correction is therefore off in `bp`, which is also the default of the `correcting` option of the `pricing` executable. This default does not establish which setting the authors intended for every experiment of the article. Since the original executable could run with either setting, the work done by `bp` on `main` can differ from that of an unmodified build.
3. **Lower bound after a limit** (`code/src/bcp/bcp.cpp`, `BCP::Run`). When a time or memory limit interrupts the processing of a node, that node is discarded. The original line `if (q.empty()) z_lb = z_ub;` then set the reported lower bound to the upper bound whenever the node queue was empty, although the status was a limit and the search was not finished. The line now applies only when no limit was hit. After a limit, the lower bound keeps its previous value, the bound of the last node selected for branching. A run that ends with the status Optimum is not affected. We changed this line only. The time limit is still not enforced in every loop (see [Known limitations](#known-limitations-not-repaired-on-main)), and we tested bounds at a limit on the runs of the [validation](#validation-of-v100) only.
4. **CPLEX on 1 thread** (`code/goc/src/linear_programming/cplex/cplex_solver.cpp`, `apply_configuration`). The original code leaves the number of CPLEX threads at its default. On our machine (20 hardware threads, CPLEX 12.10.0.0), CPLEX then solved the LPs of column generation with its concurrent optimizers, and 11 of the 112 runs of the original code on the instances with 25 customers aborted with the CPLEX error 1217 ("No solution exists") raised in `CPXgetpi` (12 with the 2 repairs of the labeling). In the abort that we traced, `CPXlpopt` returned without an error and without a solution, and an immediate second call solved the same LP. The code reads the dual values after every LP solve without checking that a solution exists. With 1 thread, none of these runs aborted. This is an observation on one machine with one CPLEX version, and we did not test others. All other CPLEX parameters keep the values of the original code, which leaves the tolerances at the CPLEX defaults.

**Where the partial-dominance defect was observed.** It was observed in KAYROS before its version 1.7.0, not in this code. On the Dabia2013 instance RC107 with 50 customers, one pricing configuration of KAYROS closed the search at 10632.144948 while the optimum is 10614.011180. Disabling partial dominance in a final verification pass recovered 2 routes of negative reduced cost, and the trace of one of them showed a piece falling from 606.665 to 546.2 rebuilt as rising from 606.665 to 642.2. The same line is in this code, and we repaired it identically. As far as our records show, no wrong optimal value has been observed from this code itself.

## Known limitations (not repaired on `main`)

These behaviors come from the original code and are unchanged on `main`.

- **Time limit not enforced in every loop.** After column generation reaches its optimum, the final LP is solved with `lp_solver->time_limit = Duration::Max()` (`code/goc/src/linear_programming/colgen/colgen.cpp`). The strong-branching loop of `BCP::BranchNode` does not check the time limit between candidates, and `BCP::FreezeHeuristic` receives the whole time limit of the run instead of the remaining time. A run can therefore exceed its time limit.
- **Missing LP solutions are not handled.** The code still reads the dual values after every LP solve without checking that a solution exists. Running CPLEX on 1 thread removed the aborts that we observed. It does not handle the case in general.
- **Label correction.** Only the flag is repaired. A source audit made for KAYROS, with synthetic tests of the actual methods, found that the correction step can leave the sorted dominance and merge structures out of order and loses ownership of the labels it removes. These tests did not show a wrong closure on a complete instance. The step is off in `bp` on `main`, while the `pricing` executable can still enable it with its `correcting` option.
- **Other defects corrected in KAYROS.** KAYROS also corrects 2 out-of-bound reads in the composition routine, a time-window tightening rule of the preprocessing that an unintended conversion had reduced to a near no-op, floating-point comparison primitives, and sums of arc flows and cut violations taken in the iteration order of a hash table, which can change the strong-branching candidates between runs. They are not repaired here, and we did not measure their effect on this code.
- **CPLEX tolerances.** `main` keeps the CPLEX default tolerances of the original code.

The branch `component-study` changes the time-limit handling and the CPLEX tolerances (see [below](#the-component-study-branch)).

## Getting started

The instructions below are those of the original README, with corrections. We built the code and ran `experiments/bp_smoke.json` and `experiments/bp_n25.json` with them (see [Validation](#validation-of-v100)). We did not run the other experiment files or the visualization tool.

### Prerequisites

- Python >= 3.6 [(more info)](https://www.python.org/)
- CPLEX >= 12.8 [(more info)](https://www.ibm.com/products/ilog-cplex-optimization-studio)
- Boost Graph Library >= 1.66 [(more info)](https://www.boost.org/doc/libs/1_66_0/libs/graph/doc/index.html)
    - On Linux: `sudo apt-get install libboost-all-dev`
- CMake >= 2.8.4 [(more info)](https://cmake.org/)
    - On Linux: `sudo apt-get install cmake`
- C++14 or higher

With CMake 4, the configuration fails with "Compatibility with CMake < 3.5 has been removed from CMake", because `code/CMakeLists.txt` declares `cmake_minimum_required(VERSION 2.8.4)`. Set `export CMAKE_POLICY_VERSION_MINIMUM=3.5` before running the runner.

### Built with

- Kaleidoscope: A tool to visualize the outputs of Optimization Problems [(more info)](https://github.com/gleraromero/kaleidoscope)
- Runner: A script to ease the process of running experiments [(more info)](https://github.com/gleraromero/runner)
- GOC lib: A library that includes interfaces for using (Mixed Integer) Linear Programming solvers, and some useful resources [(more info)](https://github.com/gleraromero/goc)

### Running the experiments

1. Add environment variables with the paths to the libraries.
    1. Add two environment variables with the CPLEX include and library paths.
        1. `export CPLEX_INCLUDE=<path_to_cplex_include_dir>`
            - Usually on Linux: _/opt/ibm/ILOG/CPLEX_Studio\<VERSION\>/cplex/include_
        1. `export CPLEX_BIN=<path_to_cplex_lib_binary_file>`
            - Usually on Linux: _/opt/ibm/ILOG/CPLEX_Studio\<VERSION\>/cplex/lib/x86-64_linux/static_pic/libcplex.a_
    1. Add two environment variables with the Boost Graph Library include and library paths.
        1. `export BOOST_INCLUDE=<path_to_boost_include_dir>`
            - Usually on Linux: _/usr/include_
        1. `export BOOST_BIN=<path_to_boost_lib_binary_file>`
            - Usually on Linux: _/usr/lib/x86_64-linux-gnu/libboost_graph.a_
1. Go to the root directory of the repository.
1. Execute `python3 runner/runner.py <experiment_file>`. The runner compiles the code with CMake in `runner/obj/`, then runs every configuration of the experiment file on every selected instance.
1. The output is saved periodically to the `output/` folder, as `output/<date>-<experiment_file_name>.json`.

### Experiments

There are 3 experiment files from the article, and 2 short experiments added for the validation of this repository.

- _Section 7.1_ of the article: `experiments/pricing.json`
- _Section 7.2_: `experiments/bp.json`, 5 configurations (`BP-BASE`, `BP-IMPL`, `BP-PART`, `BP-CUTS`, `BP-BASE-CUTS`) on the 168 Dabia instances with 25, 50 and 100 customers, with a time limit of 7,200 s per run
- _Section 7.4_: `experiments/bp_heur.json`
- `experiments/bp_smoke.json` runs the configurations `BP-PART` and `BP-CUTS` of `bp.json`, with a time limit of 60 s, on the instances C101, R101, RC101 and RC107 with 25 customers. It takes less than 1 minute after compilation on our machine.
- `experiments/bp_n25.json` runs the same 2 configurations, with a time limit of 20 s, on the 56 instances with 25 customers. It takes about 4 minutes after compilation on our machine.

The runner accepts `--instances` (`-I`) and `--exps` (`-E`) to select instances and configurations by name, and `--memlimit` (`-M`) for a memory limit in GB (15 by default).

### Visualizing the experiment results

1. Go to https://gleraromero.github.io/kaleidoscope/networks2020
1. Add the output file.
1. Select the experiments.
1. Add some attributes to visualize.
1. Click on Refresh.
1. If more details on an experiment are desired click on the + icon in a specific row.

### Checker

The checker validates the routes reported in an output file.

```
python3 checker/checker.py output/<output_file.json>
```

For each run, the checker validates the following points.

- The route of the exact solution is feasible with respect to all resources.
- The reported duration of the route is correct.
- If the status is Optimum, the value is better than or equal to every solution of the `solutions.json` file of its dataset.

The checker does not verify lower bounds or optimality proofs.

### Regression test of partial dominance

`code/tests/test_partial_dominance.cpp` splits a decreasing piece in the middle with `DominatePieces` and checks that the remaining parts keep the values of the original function. Its target is not part of the default build. After the runner has compiled the code once, run it from the build directory as follows.

```
cd runner/obj/release
make test_partial_dominance
./test_partial_dominance
```

It prints `PASS` and exits with code 0 on `main`. With the original line of `DominatePieces`, it prints the wrong values of the right part, then `FAIL`, and exits with code 1.

## Validation of v1.0.0

Versions after 1.0.0 that change the documentation only, such as 1.0.1, have the same solver code, so this validation applies to them unchanged.

All runs below were made on one Linux machine with 20 hardware threads, GCC 15.2.0, CMake 4.1.2 (with `CMAKE_POLICY_VERSION_MINIMUM=3.5`), CPLEX 12.10.0.0, Boost 1.89 and Python 3.13, each build from a separate copy of the sources. The 2 experiments were run through `python3 runner/runner.py`, and the sweep of time limits called the `bp` executable built by the runner directly. We compare `main` with 3 other builds.

- The *original* build is the commit `cabc56a` with its solver code unchanged. The original runner fails at its first run with Python 3.13 and does not create its output directory, so this copy used the 1-line memory-limit fix of the runner of `main` and an output directory created by hand.
- The *flag-only* build adds the initialization of the label-correction flag.
- The *labeling-repairs* build adds both repairs of the labeling (changes 1 and 2), without the 2 other changes.

The original, flag-only and labeling-repairs builds leave the number of CPLEX threads at its default.

**Smoke experiment.** We ran `experiments/bp_smoke.json` with the original build and with `main`, then `python3 checker/checker.py` on each output.

| Instance | Configuration | Status, original `cabc56a` | Value, original | Status, `main` | Value, `main` |
| --- | --- | --- | ---: | --- | ---: |
| C101_25 | BP-PART | Optimum | 24709.177158 | Optimum | 24709.177158 |
| C101_25 | BP-CUTS | Optimum | 24709.177158 | Optimum | 24709.177158 |
| R101_25 | BP-PART | Optimum | 9079.419193 | Optimum | 9079.419193 |
| R101_25 | BP-CUTS | Optimum | 9079.419193 | Optimum | 9079.419193 |
| RC101_25 | BP-PART | Optimum | 7038.613082 | Optimum | 7038.613082 |
| RC101_25 | BP-CUTS | Optimum | 7038.613082 | Optimum | 7038.613082 |
| RC107_25 | BP-PART | Optimum | 5030.412032 | Optimum | 5030.412032 |
| RC107_25 | BP-CUTS | Optimum | 5030.412032 | Optimum | 5030.412032 |

The checker reported `ok: 8 wrong: 0 suboptimal: 0 skipped: 0 memlim: 0 error: 0` for each build. Each of these runs took less than 0.5 s. The regression test of partial dominance passes on `main`.

**The 56 instances with 25 customers.** We ran `experiments/bp_n25.json` (112 runs) with each build.

| Build | Optimum | Time limit | Abort with CPLEX error 1217 |
| --- | ---: | ---: | ---: |
| Original | 98 | 3 | 11 |
| Flag-only | 97 | 3 | 12 |
| Labeling-repairs | 97 | 3 | 12 |
| `main` | 108 | 4 | 0 |

On the 98 runs that the original build proved, `main` proved the same optimal value. The checker reported `ok: 112 wrong: 0 suboptimal: 0 skipped: 0 memlim: 0 error: 0` for `main`. The 4 runs of `main` that reach the time limit are C109 and RC203 with BP-PART and RC204 with both configurations. The 2 runs on RC204 found no solution within 20 s, and the checker counts a run without a solution as `ok`, so it checked the solutions of 110 runs.

The aborts of the 3 builds with the default number of threads are those described under change 4. The set of aborted runs differs between these builds. The flag-only build aborted on R204 with both configurations and not on R211 with BP-CUTS, unlike the original build, although no run of either build reported a corrected label, and the labeling-repairs build aborted on C104 with BP-CUTS, which the original build proved. We therefore do not attribute these aborts to either repair of the labeling.

**Effect of the partial-dominance repair on the work.** Among the 96 runs that the original build and the flag-only build both proved, the numbers of columns generated at the root are identical. The flag-only build and the labeling-repairs build differ in 24 of the 95 runs that both proved. Since the partial-dominance line is the only difference between these 2 builds, the repaired line changed the work of the solver in these runs, and no optimal value changed. This is consistent with case D on decreasing pieces, although on a non-decreasing piece the old and new expressions can also differ by rounding, and we did not tell the 2 cases apart.

**Lower bound at a time limit.** In the runs above, 3 runs reported a wrong lower bound at the time limit on RC203, for which `instances/dabia_et_al_2013/solutions.json` holds a solution of value 6190.765422. The original build and the flag-only build with BP-CUTS reported 6197.302706 as both bounds, and the labeling-repairs build with BP-PART reported 6215.215162 as both bounds, each with the status TimeLimitReached. To test change 3, we ran BP-PART on 6 instances that need branching (C104, R203, R204, R208, R211 and RC203) with 30 time limits intended to fall during branching, with `main` and with a build of `main` without change 3. 2 of the 30 runs of the build without the change and 1 run of `main` finished before their limit with the status Optimum. Over these 30 runs, the build without change 3 reported a lower bound equal to its upper bound together with the status TimeLimitReached in 18 runs, and in 11 of them this value was above the value of `solutions.json`. `main` did so in no run. Every lower bound that it reported at a limit was below its upper bound and below the value of `solutions.json`. A first repetition of the same sweep had given 17 and 10 for the build without the change, since runs that end close to their limit vary with timing.

**What this shows and what it does not.** `main` builds, runs through the runner of the repository and passes the checker on the 2 experiments above, and on the instances with 25 customers the 4 changes altered no optimal value that the original build had proved. It does not show that `main` is correct. The checker verifies the routes and compares the values with `solutions.json`, which does not certify optimality, and it does not check lower bounds. The runs in which the repaired partial-dominance line changed the work did not change an optimal value, so they show no case where the defect produced a wrong result in this code. The defect itself is covered by the regression test. We did not run the instances with 50 and 100 customers with `main`.

## The `component-study` branch

The branch `component-study` and the tag `component-study-as-run` hold the sources from which the baseline binary of a component study of KAYROS 1.7.0 was built. The tag is on these sources, unchanged, and the branch adds 1 commit that only puts a note at the top of the README. The sources are the original commit `cabc56a` plus 3 commits.

1. An instrumentation for the harness of the study. It prints JSON event lines instead of the output of the original code, sets CPLEX to 1 thread with feasibility and optimality tolerances of 1e-7, an integrality tolerance of 1e-6 and a relative gap of 0, makes every loop of the search take its remaining time from one common deadline, and reports lower bounds from the node queue only while no limit has been hit, without closing the gap when a limit empties the queue.
2. The initialization of the label-correction flag to false, as on `main`.
3. The partial-dominance repair, as on `main` (with a different trailing comment).

Changes 3 and 4 of `main` take the condition on the lower bound and the number of threads from these sources. `main` does not take the tolerances, the common deadline or the event output.

The executable of this branch takes its clock from the harness. It reads its start time and its deadline from the environment variables `PROBE_START_MONOTONIC_NS` and `PROBE_DEADLINE_MONOTONIC_NS` (monotonic clock, in nanoseconds), and the index of the end depot, used to check and print routes, from `UPSTREAM_EXPECTED_END_DEPOT`. From our reading of the code, it stops at once when the deadline variable is absent. It therefore cannot be driven by `runner/runner.py`. The run records of the study and its harness are being archived separately. Its dataset names the 3 commits and ships them as patches. The run records store the sha256 of the baseline binary, `04073bef66d50273d018df9381e2a86b7a6cd744a68951b83fc60d04952e04f2`. It identifies the binary of the study, and a build with another compiler or other library versions will have another hash. The commit messages and some identifiers of this branch carry internal working labels, which we kept so that the sources stay identical to those of the study.

In the study, this baseline ran the `BP-CUTS` configuration on the 56 Dabia instances with 100 customers, with 5 trials per instance, a budget of 7,200 s from process launch, an address-space limit of 16 GiB and 1 pinned core per run, with CPLEX 12.10.0.0. It had an accepted proof in 165 of its 280 runs, and 46 runs ended at the memory limit. These results belong to the `component-study-as-run` sources with these settings. `main` was not run in the study.

## Citation

If you use this repository, please cite these 3 items.

1. This repository. Lera-Romero, G., Miranda-Bront, J. J., Soulignac, F. and Rascoussier, F. (2026). lera-tdvrptw-bpc-repaired: the branch-price-and-cut of Lera-Romero, Miranda-Bront and Soulignac for the TDVRPTW, with repairs. https://github.com/0nyr/lera-tdvrptw-bpc-repaired. Give the version you used. `CITATION.cff` holds this metadata, which GitHub exposes through the "Cite this repository" button.
2. The original repository. Lera-Romero, G., Miranda-Bront, J. J. and Soulignac, F. (2020). networks2020. https://github.com/gleraromero/networks2020, commit `cabc56a`.
3. The article. Lera-Romero, G., Miranda Bront, J. J. and Soulignac, F. J. (2020). Linear edge costs and labeling algorithms: The case of the time-dependent vehicle routing problem with time windows. Networks 76(1):24-53. https://doi.org/10.1002/net.21937

## Authors

The authors of this program are Gonzalo Lera-Romero, Juan José Miranda-Bront, Francisco Soulignac and Florian Rascoussier (see `AUTHORS`). The first 3 wrote the original code. Florian Rascoussier (onyr.maintainer@gmail.com) made the changes of this repository, wrote its tests and maintains it.

The original code uses [JSON for Modern C++](https://github.com/nlohmann/json) and the [Boost Graph Library](https://www.boost.org/doc/libs/1_66_0/libs/graph/doc/index.html).

## License

This project is licensed under the MIT License, see the `LICENSE` file. The copyright of the original code (Copyright (c) 2019 Gonzalo Lera Romero) is kept, and a second copyright line covers the modifications.
