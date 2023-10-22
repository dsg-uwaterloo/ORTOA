# Data Generation Specification

## `seed_data.csv`

A file consisting of `SET <KEY> <VAL>` entries.

```csv
SET 1 1
SET 2 2
SET 3 3
...
```

This format was chosen so that the client doesn't have to distinguish between operations and seed data. The seed data comes as operations. This may change if we decide to batch the PUTs for the seed data.

## `sample_operations.csv`

A file consisting of entries of two possible types:

- `GET <KEY>`
- `PUT <KEY> <UPDATED_VAL>`

```csv
GET 1
PUT 2 00
GET 2
...
```

Each line gets parsed by the client and the corresponding operation is performed. For example, the operation `"GET 1"` attempts to retrieve the value for the key `"1"`. Similarly, `"PUT 2 200"` attempt to update the value of key `"2"` to `"200"`.
