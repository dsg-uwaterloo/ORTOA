# Data Generation

## Generating Seed Data with CLI

`docs/DATA_GENERATION.md`: [See the docs here](https://github.com/ySteinhart1/ORTOA/blob/ortoa-tee/docs/DATA_GENERATION.md)

## Generating Custom Seed Data

Ideally, all you should need to do is replace the `key_generator` and `value_generator` properties in the script. Then, you can run the data generation command as specified in the CLI docs.

### Custom Key Generation

Look in `generators/key_generator.py` for examples of implemented key generators. If you decide to make your own, ensure that every key will be unique.

A key generator should inherit from the abstract base class `KeyFactory` and define a `generate_key()` method that creates a key. The class' state can be used to keep track of existing keys if necessary.

For example:

```python
class SequentialIntKeyGenerator(KeyFactory[int]):
    def __init__(self, start_key: int = 1):
        self.key = start_key

    def generate_key(self) -> int:
        generated_key = self.key
        self.key += 1
        return generated_key
```

## Custom Value Generation

Look in `generators/value_generator.py` for examples of implemented value generators.

A value generator should inherit from the `ValueFactory` abstract base class and define a `generate_value()` method that generates values.

For example:

```py
class RandomIntegerGenerator(ValueFactory[int]):
    def __init__(self, min_val: int = 0, max_val: int = 1000000):
        self.min_val = min_val
        self.max_val = max_val
    
    def generate_value(self):
        return random.randint(self.min_val, self.max_val)
    
```
