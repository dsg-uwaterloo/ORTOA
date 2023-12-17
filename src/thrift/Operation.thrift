enum OpType {
  GET, 
  PUT, 
  EOD
}

struct Operation {
  1: OpType op,
  2: string key,
  3: string value
}

service RPC {
  void access(1:Operation operation),
}