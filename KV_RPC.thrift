
struct Entry {
  1: string keyName,
  2: binary encryptedLabelsA,
  3: binary encryptedLabelsB
}

service KV_RPC {
  oneway void create(1:Entry entry),
  binary access(1:Entry entry),
}
