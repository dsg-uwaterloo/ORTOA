# ORTOA SDK

## Installation

Our package requires `python>=3.8.*` and can be initialized via `pip`:

```bash
# Create & activate a virtual environment
ORTOA/ $ python3 -m venv .venv
ORTOA/ $ . .venv/bin/activate

# Install the package
ORTOA/ $ pip install -e extras/
```

The dev dependencies can be installed via `pip` as well:

```bash
ORTOA/ $ pip install -e extras/[dev]
```

## Tests

Our unit tests are located in `extras/test/`. They can be executed via `pytest`:

```bash
# Invoking pytest directly
ORTOA/ $ pytest extras/test/

# Using ortoa-lib.sh
ORTOA/ $ source scripts/ortoa-lib.sh
ORTOA/ $ ortoa-test-python
```
