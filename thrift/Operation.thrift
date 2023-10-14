struct Operation {
  1: string op,
  2: string key,
  3: string value
}

service RPC {
  binary access(1:Operation operation),
}