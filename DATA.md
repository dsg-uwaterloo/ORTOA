# `seed_data.csv`



A file consisting of `PUT, <KEY>, <VAL>` entries

```csv
PUT, 1, 1
PUT, 2, 2
PUT, 3, 3
...
```

# `sample_operations.csv`

A file consisting of entries of two possible types:

- `GET, <KEY>`
- `PUT, <KEY>, <UPDATED_VAL>`

```csv
GET, 1
PUT, 2, 200
GET, 2
...
```
