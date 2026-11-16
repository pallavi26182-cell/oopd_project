# AcadERP-Lite: Academic Records & Graduation Audit System

**Course:** Object-Oriented Programming and Design (OOPD)  
**Group Project ID:** 111  
**Team Members:**
- **Pallavi Gupta** (`MT26182`)
- **Toshi Sinha** (`MT26198`)

---

## 1. The Problem Statement & Core Challenges
Modern university ERP systems must enforce complex academic rules across multiple course categories while remaining resilient against malformed input and edge-case arithmetic. Building **AcadERP-Lite** in strict C++11 introduces five primary engineering challenges:

1. **Zero Standard Template Library (STL) & No Exceptions:**
   - Standard containers (`std::vector`, `std::map`, `std::list`), `<algorithm>`, `<sstream>`, smart pointers, and `try/catch` blocks (`-fno-exceptions`) are strictly prohibited[cite: 1].
   - All collections must be backed by manually managed arrays (`new` and `delete[]`) with explicit bounds checking (`MAX_STUDENTS = 100`, `MAX_COURSES = 50`, `MAX_REGS = 40`)[cite: 1, 4].
2. **Floating-Point Precision Loss in GPA:**
   - Using `float` or `double` for GPA computation causes binary rounding errors (e.g., 65 / 8 = 8.125 formats as `8.12` under standard `printf("%.2f")`, whereas academic rounding requires `8.13`)[cite: 3].
3. **Multi-Stage Thesis State Transitions & Repeat Rules:**
   - Unlike `REGULAR` and `SU` courses, a `THESIS` follows a two-step lifecycle (`REGISTERED` -> `IN_PROGRESS` via `GRADE S` -> `FINAL` via `CONVERT <letter>`, or `FAILED` via `GRADE X`)[cite: 2].
   - Students may re-register for a course **only** when their latest attempt is a fail (`F`, `X`, or a thesis converted to `F`), and only the latest graded attempt ("counting attempt") counts toward credits, prerequisites, and GPA[cite: 2].
4. **Strict Validation Precedence & Atomicity:**
   - Commands must validate syntax (`E_PARSE`), capacity (`E_FULL`), entity existence (`E_UNKNOWN_STUDENT` before `E_UNKNOWN_COURSE`), and eligibility rules (`E1` through `E6`) in an exact deterministic order[cite: 4].
   - Any failing command must print `ERROR <line> <CODE>` without mutating any stored records and continue processing subsequent lines[cite: 4].
5. **Multi-Rule Graduation Audit (`GRAD`):**
   - Graduation requires passing all five rules (`R1`–`R5`) simultaneously, and when a student does not qualify, every failing rule must be reported in ascending order (`R1, R2, ...`)[cite: 3, 5].

---

## 2. How We Solved It (Architecture & Implementation)
To solve these challenges cleanly without STL or exceptions, we decoupled input tokenization, domain state management, and audit queries across a modular object-oriented design:

### A. Manual Heap Memory & Encapsulated Classes (`src/erp.h`)
- **`ProgramConfig`**: Encapsulates institutional thresholds (`totalCredits = 160`, `electiveCredits = 24`, `minGpaHundredths = 500`, `maxLoad = 24`, `thesisCredits = 100`), initialized via constructor and configurable once via `PROGRAM`[cite: 5, 6].
- **`Course`**: Stores course metadata (`code`, `CourseType`, `credits`, `isCore`) and resolves prerequisites as integer indices (`prereq`) into `courses[]` at insertion time for O(1) lookup during eligibility checks[cite: 6].
- **`Registration`**: Captures a single enrollment attempt (`course` index, `term`, `grade` string, and `ThesisState`)[cite: 6, 7].
- **`Student`**: Dynamically allocates `regs = new Registration[MAX_REGS]` in its constructor and frees it via `delete[] regs` in `~Student()`, ensuring zero memory leaks[cite: 7].
- **`ERPSystem`**: Owns `courses = new Course[MAX_COURSES]` and `students = new Student[MAX_STUDENTS]`, exposing clean methods that return `ErrorCode` values[cite: 6, 7].

### B. Custom Tokenizer & Atomic Error Handling (`src/erp.cpp`, `src/main.cpp`)
- **Whitespace Scanner (`splitTokens`)**: Replaces `<sstream>` by scanning character-by-character with `std::isspace`, extracting up to `maxTokens` words while detecting overflow token counts as `E_PARSE`.
- **Two-Phase Command Execution**: Every mutating command (`addCourse`, `addStudent`, `registerCourse`, `submitGrade`, `convertThesis`) performs **all** parsing, lookup, state, and capacity checks before writing a single byte to `courses[]` or `students[]`. If any check fails, it immediately returns the corresponding `ErrorCode`[cite: 4].

### C. Pure Integer Half-Up GPA Rounding
- `gpaHundredths()` accumulates integer numerator `n = sum(c_i * g_i)` and denominator `d = sum(c_i)` using `long long` across counting attempts of `REGULAR` courses and `FINAL` theses[cite: 3].
- Exact half-up rounding to two decimal places is computed purely with integer division: `h = (200 * n + d) / (2 * d)`[cite: 3].
- Output is formatted via `std::printf("%d.%02d", h / 100, h % 100)`[cite: 3], and `queryAvg()` compares `h` directly against integer thresholds (`950`, `850`, `750`, `650`, `550`, `450`, `200`)[cite: 3].

### D. Sequential Eligibility (`E1`–`E6`) & Graduation Audit (`R1`–`R5`)
- `checkEligibility()` evaluates `E2` (not already passed) -> `E3` (no active ungraded/in-progress attempt) -> `E4` (prerequisite passed) -> `E5` (term load + course credits <= `maxLoad`) -> `E6` (thesis earned credits threshold) in exact specification order, short-circuiting on the first failure[cite: 4].
- `queryGrad()` independently evaluates `R1` (total earned credits), `R2` (all catalogue `CORE` courses passed), `R3` (passed `ELECTIVE` credits), `R4` (at least one `FINAL` thesis with grade >= `C`), and `R5` (rounded GPA `h >= minGpaHundredths`), printing `YES` or formatting all failing rules separated by `, `[cite: 3, 5].

---

## 3. How to Read & Navigate the Codebase
If you are reviewing or extending this repository, follow this reading order:

1. **Start with `src/erp.h` (Data Structures & Contracts):**
   - Review `MAX_*` bounds, `ErrorCode` enum values, `CourseType`, and `ThesisState`[cite: 6].
   - Inspect the class layouts (`ProgramConfig`, `Course`, `Registration`, `Student`, and `ERPSystem`) and method signatures[cite: 6, 7].
2. **Read `src/main.cpp` (Input Loop & Dispatch):**
   - See how `std::getline` reads `stdin`, strips Windows `\r` endings, skips blank/`#` comment lines while tracking exact line numbers, enforces `PROGRAM` as the first command, and formats `ERROR <line> <CODE>`[cite: 4, 5].
3. **Read `src/erp.cpp` in Logical Sections:**
   - **Lifecycle & Parsing Helpers (Top):** Constructors/destructors (`new`/`delete[]`), `splitTokens()`, `parseNonNegInt()`, and `parseMinGpa()`.
   - **Lookup & Counting Attempt Helpers:** `findCourse()`, `findStudent()`, `getLatestGradedIndex()`, `isCoursePassed()`, `earnedCredits()`, and `termLoad()`.
   - **State Mutators:** `setProgram()`, `addCourse()`, `addStudent()`, `checkEligibility()`, `registerCourse()`, `submitGrade()`, and `convertThesis()`[cite: 7].
   - **Query & Audit Evaluators (Bottom):** `gpaHundredths()`, `queryGpa()`, `queryAvg()`, `queryGrad()`, and `queryEligible()`[cite: 7].
4. **Explore `tests/` & `run_tests.sh` (Verification Suite):**
   - `tests/sample.in` / `sample.out`: End-to-end walkthrough from the project specification[cite: 5, 6].
   - `tests/T01.in` – `tests/T23.in`: Targeted unit tests isolating each error code, rounding boundary, repeat scenario, and graduation rule combination[cite: 7, 8].

---

## 4. Build, Run & Test Commands
- **Compile:** `make` (builds with `g++ -std=c++11 -Wall -Wextra -fno-exceptions`)[cite: 1, 8]
- **Run Single Test:** `./acaderp < tests/sample.in`[cite: 8]
- **Run Automated Test Suite:** `./run_tests.sh` (executes all 24 test cases and verifies output diffs)[cite: 8]
- **Clean Binary:** `make clean`

---

## 5. Memory Allocation & Deallocation Summary
- **`ERPSystem::ERPSystem()`**: Allocates `courses = new Course[MAX_COURSES]` (`50`) and `students = new Student[MAX_STUDENTS]` (`100`)[cite: 4].
- **`Student::Student()`**: Allocates `regs = new Registration[MAX_REGS]` (`40`) per student instance[cite: 4, 7].
- **`Student::~Student()`**: Frees `delete[] regs` when each `Student` is destroyed[cite: 7].
- **`ERPSystem::~ERPSystem()`**: Frees `delete[] courses` and `delete[] students` at program exit, guaranteeing 1-to-1 cleanup with zero leaks[cite: 7].

---

## 6. Assumptions
1. **`PROGRAM` Placement:** `PROGRAM` is permitted only as the first non-blank, non-comment command (allowing leading `#` comment lines as seen in `tests/sample.in`); appearing later yields `E_PARSE`[cite: 5].
2. **Thesis Graduation Rule (`R4`):** Satisfied if at least one `THESIS` course is in `FINAL` state with a converted letter grade of `C` or better (`>= 6` grade points)[cite: 3]. A `D` grade earns credits (`R1`) and counts in GPA (`R5`)[cite: 2, 3], but fails `R4`[cite: 3].
3. **Prerequisite Ordering:** Prerequisites must exist in the catalogue prior to defining a dependent course; self-referential or unknown prerequisites return `E_BAD_COURSE`[cite: 4, 5].
4. **Cross-Platform Line Endings:** Trailing `\r` characters are stripped so CRLF test files created on Windows execute cleanly on Linux.

5. **Bonus Commands (`TRANSCRIPT`, `SAVE`, `LOAD`):**
   - `TRANSCRIPT <roll>` prints `TRANSCRIPT <roll> <count>` followed by each registration in ascending term order (`1..12`) as `<term> <course> <grade|-> <credits>`.
   - `SAVE <file>` and `LOAD <file>` serialize and atomically restore the full ERP state (`ACADERP_SAVE_V1`), returning `E_PARSE` if the file is missing or malformed without corrupting existing records.
