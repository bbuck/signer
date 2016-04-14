require "benchmark"
require "./signer"

def rb_join(strs)
  "".tap do |result|
    strs.each do |str|
      result << str
    end
  end
end

n = 100_000
words = ["one", "two", "three"]

a = words.join
b = rb_join(words)
c = Signer.join(words)

unless a == b && b == c
  [a, b, c].each { |z| p z }
  raise RuntimeError, "Values do not match"
end

Benchmark.bm(10) do |x|
  x.report("#join:") { n.times { words.join } }
  x.report("ruby_join:") { n.times { rb_join(words) } }
  x.report("signer:") { n.times { Signer.join(words) } }
end

def rb_dump(hash:, namespace: nil)
  "".tap do |output|
    hash.keys.sort.each do |key|
      full_key = namespace ? "#{namespace}.#{key}" : key
      value = hash[key]

      if value.is_a?(Hash)
        output << rb_dump(hash: hash[key], namespace: full_key)
      else
        output << "#{full_key}: #{hash[key]}"
        output << "\n"
      end
    end
  end
end

test_hash = {
  response_uri: "https://example.com/callback",
  remote_id: "abc",
  transaction: {
    amount: "10.00",
    order_id: "xyz"
  }
}

a = rb_dump(hash: test_hash)
b = Signer.dump(test_hash)

unless a == b
  [a, b].each { |z| p z }
  raise RuntimeError, "Value do not match"
end

Benchmark.bm(8) do |x|
  x.report("rb_dump:") { n.times { rb_dump(hash: test_hash) } }
  x.report("signer:") { n.times { Signer.dump(test_hash) } }
end

require "openssl"

key = "abcdef"
digest = OpenSSL::Digest.new("sha256")
a = OpenSSL::HMAC.hexdigest(digest, key, rb_dump(hash: test_hash))
b = Signer.sign(test_hash, key)

unless a == b
  [a, b].each { |z| p z }
  raise RuntimeError, "Values do not match"
end

Benchmark.bm(5) do |x|
  x.report("ruby:") { n.times { OpenSSL::HMAC.hexdigest(digest, key, rb_dump(hash: test_hash)) } }
  x.report("c:") { n.times { Signer.sign(test_hash, key) } }
end
