# Overall assumptions
 * Single-threaded
 * Every function is fast

## Robot states
Robot is in one primary global state.

## Subsystem states
There can also be subsystem states, which are just another type of state, making
the total number of states the product of these.

#### Questions
 * How are they different from global states?
 * Are they necessary?
 * Does adding a concept of subsystems make things too complex?
 * Do we need to add subsystems to other places to get useful subsystem states?

# Functions
_Specify more formally how different functions work_

### `define_state_function`
Can take multiple states a function is active in

### `define_interrupt`
Should take an optional parameter for which states the interrupt is active in

### `wait_for`

### `wait`
`wait` will act very similarly to `wait_for` but will automatically use the
clock defined by the user as the condition
