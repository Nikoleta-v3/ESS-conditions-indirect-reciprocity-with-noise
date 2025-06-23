# ESS-conditions-indirect-reciprocity-with-noise

This repository contains the source code for the numerical verification of the
results from the project ``Exact conditions for evolutionary stability in
indirect reciprocity under noise'', along with scripts to generate data and plot
the figures shown in the manuscript.

The project is currently under review and is by Nikoleta E. Glynatsi, Christian
Hilbe, and Yohsuke Murase.

The code is written in C++, and the plotting scripts are in Python.

## Build

To build the project, clone the repository and use CMake:

```shell
git clone git@github.com:Nikoleta-v3/ESS-conditions-indirect-reciprocity-with-noise.git
cd ESS-conditions-indirect-reciprocity-with-noise
mkdir build
cd build
cmake ..
cmake --build .
```

## Source Code and Executables

The main source files in the project are:

1. `Norms.hpp`: Defines norms, including the assessment and action rules.
2. `NormsWithPunishment.hpp`: Similar to `Norms.hpp` but for the three-action case (Cooperation, Defection, Punishment).
3. `Game.hpp`: Defines the game. This includes the resident's payoff and
   cooperation rate, $\Delta_v$, mutants payoffs, and includes functions such as
   the ESS conditions.
4. `GameWithPunishment.hpp`: Same as `Game.hpp` but adapted for the three-action game.

Each file has associated unit tests. After building the project, the following
executables will be available in the `build` directory:

* `test_game`: Tests that the ALLD action rule is always an ESS, and verifies results for the leading eight and secondary 16 norms using Theorem 1.
* `test_game_with_punishment`: Tests that the ALLD action rule is always an ESS using Equations 28–30.
* `test_norms`: Unit tests for `Norms.hpp`.
* `test_norms_with_punishment`: Unit tests for `NormsWithPunishment.hpp`, including those used in Table 3.
* `main_nash_search_with_P`: Verifies the results shown in Table 3.

