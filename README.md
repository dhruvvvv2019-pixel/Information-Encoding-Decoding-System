## Metro Map Planning using SAT Solvers

This project represents my attempt to model a constrained path-planning problem as a Boolean satisfiability (SAT) instance and solve it using MiniSAT. The assignment itself required transforming a grid-based metro design problem into a CNF formula while preserving structural constraints such as non-overlapping paths, bounded turns, and mandatory coverage of specific cells . Instead of treating this as a straightforward implementation task, I approached it as a systems-level modeling problem, focusing on how to encode spatial and graph constraints efficiently into propositional logic.

At a high level, I designed the system as a two-stage pipeline. The encoder translates the input grid and constraints into a CNF formula, which is then passed to MiniSAT. The decoder interprets the satisfying assignment returned by the solver and reconstructs valid metro paths in the required output format. This separation allowed me to reason independently about correctness at the modeling level and reconstruction at the execution level.

## Encoder Design and Modeling Approach

The encoder is the core of this project, where I translate a geometric path problem into a logical representation. My approach was to define a clean and scalable variable mapping and then layer constraints incrementally to enforce correctness.

## Variable Representation

I modeled each decision variable as a Boolean indicating whether a given metro line occupies a specific cell in the grid. This mapping allowed me to express all constraints locally while still maintaining global consistency across the grid. The variable indexing scheme is carefully designed to be deterministic and compact, ensuring that the total number of variables scales linearly with grid size and number of lines. This was important because SAT solvers are highly sensitive to encoding size.

## Enforcing Cell Exclusivity

One of the fundamental constraints is that no two metro lines can occupy the same cell. I encoded this by generating pairwise mutual exclusion clauses for every pair of lines at each grid cell. Although simple in concept, this constraint appears across the entire grid, so I ensured the implementation avoids redundancy and remains efficient. This guarantees that the final solution represents disjoint paths without overlaps.

## Path Consistency and Connectivity

Ensuring that each metro line forms a valid path from its start to its end required careful encoding. I treated start and end points differently from intermediate cells. For endpoints, I enforced that exactly one neighboring cell continues the path, ensuring proper entry and exit behavior. For intermediate cells, I encoded that if a cell is part of a path, it must connect to exactly two neighbors belonging to the same line. This prevents branching and ensures continuity.

The challenge here was expressing “exactly two neighbors” using only CNF clauses. I handled this by combining “at least two” and “at most two” constraints through structured clause generation. This part of the implementation reflects a deeper understanding of how graph connectivity properties can be encoded using purely local logical constraints.

## Turn Constraint Encoding

The most non-trivial part of the encoder is handling the constraint on the maximum number of turns allowed per metro line. Instead of attempting to track turns procedurally, I introduced auxiliary variables to represent whether a turn occurs at a particular cell. A turn is detected when the path changes direction between adjacent segments, and I encoded this using combinations of neighboring cells.

To enforce the global constraint that each line has at most J turns, I implemented a sequential counter encoding over these turn variables. This is a well-known SAT encoding technique for cardinality constraints, but implementing it correctly required careful indexing and clause construction. The benefit of this approach is that it keeps the encoding polynomial while remaining efficient for the solver. This part of the project significantly improved my understanding of how higher-level constraints can be translated into low-level Boolean logic.

## Handling Mandatory Cells (Extended Scenario)

For the extended version of the problem, certain cells must be visited by at least one metro line. I incorporated this constraint by generating a clause for each such cell that ensures at least one line variable corresponding to that cell is true. I designed this extension in a modular way so that it integrates seamlessly with the base encoding without affecting other components.

## Decoder Design and Reconstruction Strategy

Once the SAT solver produces a satisfying assignment, the responsibility shifts to the decoder to interpret this assignment and reconstruct meaningful metro paths. My goal here was to ensure that the decoded output is both correct and consistent with the constraints enforced during encoding.

I begin by parsing the SAT output and constructing a truth table of variable assignments. Using the same variable mapping as the encoder, I can determine which cells belong to which metro line. This consistency between encoding and decoding was critical to avoid mismatches.

For each metro line, I reconstruct the path starting from its source point. At each step, I look for a neighboring cell that is marked true for the same line and has not been visited in the immediate previous step. This ensures forward traversal and avoids trivial cycles. The reconstruction continues until the endpoint is reached, and the corresponding sequence of moves is written in the required format.

This process mirrors the constraints enforced in the encoder, effectively validating that the SAT solution corresponds to a valid path in the original problem space. I also included safeguards to handle unexpected cases, although the correctness of the encoding ensures such cases rarely occur.

## System Design and Engineering Decisions

I structured the entire solution as a clean pipeline with clear separation of concerns. The encoder focuses purely on constraint generation, while the decoder is responsible for interpretation and output formatting. This modularity made debugging significantly easier, especially when verifying correctness of individual components.

I also paid close attention to adhering strictly to the input-output specifications, since even minor deviations can break automated evaluation. The implementation is designed to be robust, scalable, and efficient enough to handle larger grid sizes within reasonable time limits.
