# Metro Map Planning using SAT Solver

This project formulates and solves a constrained metro routing problem by reducing it to a Boolean Satisfiability (SAT) problem and solving it using a complete SAT solver (MiniSAT). The objective is to design valid metro paths on a 2D city grid while respecting strict routing and capacity constraints.

The problem models a real-world planning scenario where multiple metro lines must be routed simultaneously from predefined start points to end points, ensuring correctness, efficiency, and feasibility under limited resources.

# Problem Overview

The city is represented as an N × M grid, where:

Each metro line has a unique start and end cell.

A path must be constructed for every metro line from its start to its end.

Each grid cell can be used by at most one metro line.

Each metro line can have at most J turns (direction changes).

In an extended scenario, certain grid cells are marked as popular, and the solution must ensure that each popular cell is covered by at least one metro line.

# Approach

The problem is solved in three major stages:

SAT Encoding

The metro routing constraints (path continuity, exclusivity, turn limits, and popular cell coverage) are encoded into a CNF SAT formula.

The encoding is guaranteed to be polynomial in size.

SAT Solving

The generated CNF formula is solved using MiniSAT (called externally, as per constraints).

The solver determines whether a feasible metro layout exists.

SAT Decoding

If the instance is satisfiable, the solver’s assignment is decoded back into explicit metro paths, represented as sequences of directions (L, R, U, D).

If no valid routing exists, the system correctly reports the instance as unsatisfiable.

# Key Constraints Modeled

Single occupancy per grid cell

Valid, continuous paths for each metro line

Bounded number of turns per line

Mandatory coverage of popular cells (extended scenario)

Exactly one SAT solver invocation

# Input and Output

Input: City grid size, number of metro lines, turn limit, start/end points, and optional popular cells

Output:

For satisfiable instances: direction sequences for each metro line

For unsatisfiable instances: 0
