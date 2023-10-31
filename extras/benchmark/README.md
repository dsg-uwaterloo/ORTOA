# Benchmarking


## Config

Requirements:

- Should be easy to define (YAML)
- Should throw an error if you try to declare the same flag twice

## Musings

How do I actually want to run an experiment?

- 1) We need a way to generate all combinations of flags (put that into the Experiment class?) from the experiment config provided
- 2) We need a way to create a `Job` for each generated combination of flags.
- 3) We should be able to orchestrate those jobs and run them (at the very least, sequentially)
- 4) An experiment should record the exact configuration it was run with
- 5) An experiment should obviously record its results
- 6) We should be able to aggregate the results of all the experiments into a dataframe and then produce some artifacts with that
- 7) An experiment should flush the DB at the beginning and end
- 8) There should be checks in place to make sure that inputs are valid, propagate errors, etc...
- 9) Logging?
