# Haystacks Arena

## Setup

Test data generation requires `ossp-uuid` and the test runner needs `gdate` and `gtimeout` from `coreutils`.

```bash
$ brew install ossp-uuid coreutils
```

## Generating the test data

```bash
$ generate-data
Generating haystack containing 100,000,000 uuids
Sampling haystack for 1,000 uuids
Sampling haystack for 100,000 uuids
Sampling haystack for 1,000,000 uuids
Generating warmup haystack
Generating warmup needles
```

This will create a `data/` directory with a haystack and several files of needles used by the test runner.


## Running the tests

The test runner can be run on a single application or all of the applicaions in the `round-1` directory. The runner will first do a "warm up" test  which is a tiny data set with a known output. If the warmup tests passes, the runner will then go on to test and time the application with a large haystack and needle files with 1,000, 100,000, and 1,000,000 rows.

```bash
$ run-all-tests --help
USAGE:
    run-all-tests [--test <TEST_NAME>]

FLAGS:
    -h, --help                 Prints help information

OPTIONS:
        --test <TEST_NAME>     Path to the test to run

```

### Testing a single app

```bash
$ run-all-tests --test round-1/haystacks-js
haystacks-js,1000,00:00:11.559
haystacks-js,100000,00:00:17.402
haystacks-js,1000000,00:00:31.166
```

### Running the tests

```bash
$ run-all-tests
haystacks-java8,1000,00:00:11.559
haystacks-java8,100000,00:00:17.402
haystacks-java8,1000000,00:00:31.166
haystacks-js,1000,00:00:11.559
haystacks-js,100000,00:00:17.402
haystacks-js,1000000,00:00:31.166
haystacks-rs,1000,00:00:11.021
haystacks-rs,100000,00:00:13.235
haystacks-rs,1000000,00:00:23.532
```

## Adding a new test

```bash
$ add-test --help
USAGE:
    add-test <NAME>

OPTIONS:
        <NAME>            Names of the new test

$ add-test haystacks-py
Creating test directory: round-1/haystacks-py
Creating test files:
    round-1/haystacks-py/run-test
    round-1/haystacks-py/haystacks-py
```

The `run-test` file will be called by the test runner with the path to a haystack file and with the path to a needles file for each test run.

