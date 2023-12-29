
struct Operation {
  1: string op,
  2: string key,
  3: string value
}

service Send_Op {
  binary access(1:Operation operation),
}